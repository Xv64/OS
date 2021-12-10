// Virtual File System implement
// This exists to abstract which underlying filesystem is being used.
// All kernel code should use the vfs as opposed to interfacing
// directly with a given fs driver.

#include "types.h"
#include "vfs.h"
#include "file.h"
#include "stat.h"
#include "fs/fs1.h"
#include "fs/ext2.h"
#include "spinlock.h"
#include "buf.h"
#include "defs.h"
#include "param.h"
#include "kernel/string.h"
#include "mmu.h"
#include "proc.h"

struct fsmap_t {
	fstype type;
	uint64 dev;
	uint8 used;
};

struct icache_node {
    uint8 used;
    struct inode inode;
    struct icache_node *next;
};

struct {
	struct spinlock lock;
    uint16 unused_nodes;
    struct icache_node *head;
} icache;

void growicache();
static inline struct inode* iget(uint64 dev, uint32 inum);
static char* skipelem(char* path, char* name);

extern uint64 ROOT_DEV;
struct fsmap_t fsmap[FS_MAX_INIT_DEV];
struct spinlock lock;

static inline fstype getfstype(uint64 dev) {
	for(uint16 i = 0; i != FS_MAX_INIT_DEV; i++) {
		struct fsmap_t map = fsmap[i];
		if(map.used == 1 && map.dev == dev) {
			return map.type;
		}
	}
	return -1;
}

static inline int8 setfstype(uint64 dev, fstype type) {
	acquire(&lock);
	for(uint16 i = 0; i != FS_MAX_INIT_DEV; i++) {
		struct fsmap_t *map = &fsmap[i];
		if(map->used == 0) {
			map->dev = dev;
			map->type = type;
			map->used = 1;

			release(&lock);
			return 1;
		}
	}
	release(&lock);
	return -1;
}

void readsb(int dev, struct superblock *sb){
	fstype t = getfstype(dev);
	uint8 devtype = GETDEVTYPE(ROOT_DEV);
	uint32 devnum = GETDEVNUM(ROOT_DEV);

	void *sbptr = &(sb->data[0]);
	if(t == FS_TYPE_EXT2) {
		ext2_readsb(devtype, devnum, (struct ext2_superblock *)sbptr);
	} else if(t == FS_TYPE_FS1) {
		fs1_readsb(dev, (struct fs1_superblock *)sbptr);
	} else {
		panic("Unknown fs type");
	}
}

int dirlink(struct inode *dp, char *name, uint inum) {
	fstype t = getfstype(dp->dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_dirlink(dp, name, inum);
	} else if(t == FS_TYPE_FS1) {
		return fs1_dirlink(dp, name, inum);
	} else {
		panic("Unknown fs type");
	}
}

struct inode *dirlookup(struct inode *dp, char *name, uint *poff) {
	fstype t = getfstype(dp->dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_dirlookup(dp, name, poff);
	} else if(t == FS_TYPE_FS1) {
		return fs1_dirlookup(dp, name, poff);
	} else {
		panic("Unknown fs type");
	}
}

struct inode *ialloc(uint dev, short type) {
	fstype t = getfstype(dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_ialloc(dev, type);
	} else if(t == FS_TYPE_FS1) {
		return fs1_ialloc(dev, type);
	} else {
		panic("Unknown fs type");
	}
}

struct inode *idup(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_idup(ip);
	} else if(t == FS_TYPE_FS1) {
		return fs1_idup(ip);
	} else {
		panic("Unknown fs type");
	}
}

void vfsinit() {
	fs1_iinit(); // fs1 needs an explicit init before we can invoke any methods

	initlock(&lock, "vfs");
	growicache();

	// for now, let's just run with the ROOT_DEVd
	// later we will want to enumerate all devices
	// and build fsmap
	uint8 devtype = GETDEVTYPE(ROOT_DEV);
	uint32 devnum = GETDEVNUM(ROOT_DEV);
	if(devtype != DEV_IDE && ext2_init_dev(devtype, devnum) == 1) {
		// The current IDE driver does not work in kernel mode, so let's
		// skip it. Otherwise, test to see if this is an ext2 FS.
		cprintf("ext2 filesystem detected on disk(%d,%d)\n", devtype, devnum);
		setfstype(ROOT_DEV, FS_TYPE_EXT2);
	} else {
		// legacy fallback...
		cprintf("xv6 filesystem assumed on disk(%d,%d)\n", devtype, devnum);
		setfstype(ROOT_DEV, FS_TYPE_FS1);
	}
}

// Lock the given inode.
// Reads the inode from disk if necessary.
void ilock(struct inode* ip){
	if (ip == 0 || ip->ref < 1)
		panic("ilock");

	acquire(&lock);
	while (ip->flags & I_BUSY)
		sleep(ip, &lock);
	ip->flags |= I_BUSY;
	release(&lock);

	if (!(ip->flags & I_VALID)) {
		fstype t = getfstype(ip->dev);
		if(t == FS_TYPE_EXT2) {
			ext2_readinode(ip);
		} else if(t == FS_TYPE_FS1) {
			fs1_readinode(ip);
		} else {
			panic("Unknown fs type");
		}
		ip->flags |= I_VALID;
		if (ip->type == 0) {
			cprintf("Error reading inode %d from disk\n", ip->inum);
			panic("ilock: no type");
		}
	}
}

void iput(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_iput(ip);
	} else if(t == FS_TYPE_FS1) {
		fs1_iput(ip);
	} else {
		panic("Unknown fs type");
	}
}

void iunlock(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_iunlock(ip);
	} else if(t == FS_TYPE_FS1) {
		fs1_iunlock(ip);
	} else {
		panic("Unknown fs type");
	}
}

void iunlockput(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_iunlockput(ip);
	} else if(t == FS_TYPE_FS1) {
		fs1_iunlockput(ip);
	} else {
		panic("Unknown fs type");
	}
}

void iupdate(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_iupdate(ip);
	} else if(t == FS_TYPE_FS1) {
		fs1_iupdate(ip);
	} else {
		panic("Unknown fs type");
	}
}

int namecmp(const char *s, const char *t) {
	return strncmp(s, t, DIRSIZ);
}

// Copy the next path element from path into name.
// Return a pointer to the element following the copied one.
// The returned path has no leading slashes,
// so the caller can check *path=='\0' to see if the name is the last one.
// If no name to remove, return 0.
//
// Examples:
//   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
//   skipelem("///a//bb", name) = "bb", setting name = "a"
//   skipelem("a", name) = "", setting name = "a"
//   skipelem("", name) = skipelem("////", name) = 0
//
static char* skipelem(char* path, char* name){
	char* s;
	int len;

	while (*path == '/')
		path++;
	if (*path == 0)
		return 0;
	s = path;
	while (*path != '/' && *path != 0)
		path++;
	len = path - s;
	if (len >= DIRSIZ)
		memmove(name, s, DIRSIZ);
	else {
		memmove(name, s, len);
		name[len] = 0;
	}
	while (*path == '/')
		path++;
	return path;
}

// Look up and return the inode for a path name.
// If parent != 0, return the inode for the parent and copy the final
// path element into name, which must have room for DIRSIZ bytes.
// Must be called inside a transaction since it calls iput().
static struct inode* namex(char* path, int nameiparent, char* name){
	struct inode* ip, * next;

	if (*path == '/')
		ip = iget(ROOT_DEV, ROOTINO);
	else
		ip = idup(proc->cwd);

	while ((path = skipelem(path, name)) != 0) {
		ilock(ip);
		if (ip->type != T_DIR) {
			iunlockput(ip);
			return 0;
		}
		if (nameiparent && *path == '\0') {
			// Stop one level early.
			iunlock(ip);
			return ip;
		}
		if ((next = dirlookup(ip, name, 0)) == 0) {
			iunlockput(ip);
			return 0;
		}
		iunlockput(ip);
		ip = next;
	}
	if (nameiparent) {
		iput(ip);
		return 0;
	}
	return ip;
}

struct inode *namei(char *path) {
	char name[DIRSIZ];
	return namex(path, 0, name);
}
struct inode *nameiparent(char *path, char *name) {
	return namex(path, 1, name);
}

int readi(struct inode *ip, char *dst, uint off, uint n) {
	if (ip->type == T_DEV) {
		// if the read request is for a T_DEV, then route it directly there...
		if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
			return -1;
		return devsw[ip->major].read(ip, dst, n);
	}
	// otherwise, we need to route this to the correct fs impl.
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_readi(ip, dst, off, n);
	} else if(t == FS_TYPE_FS1) {
		return fs1_readi(ip, dst, off, n);
	} else {
		panic("Unknown fs type");
	}
}

void stati(struct inode *ip, struct stat *st) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_stati(ip, st);
	} else if(t == FS_TYPE_FS1) {
		fs1_stati(ip, st);
	} else {
		panic("Unknown fs type");
	}
}

int writei(struct inode *ip, char *src, uint off, uint n) {
	if (ip->type == T_DEV) {
		// same idea as readi...
		if (ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
			return -1;
		return devsw[ip->major].write(ip, src, n);
	}
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		return ext2_writei(ip, src, off, n);
	} else if(t == FS_TYPE_FS1) {
		return fs1_writei(ip, src, off, n);
	} else {
		panic("Unknown fs type");
	}
}

static inline struct inode* iget(uint64 dev, uint32 inum){
	struct inode* ip, * empty;

	acquire(&lock);

	// Is the inode already cached?
	empty = 0;
    for (struct icache_node *node = icache.head; node->next != 0; node = node->next) {
        ip = &(node->inode);
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
			ip->ref++;
			release(&lock);
			return ip;
		}
		if (empty == 0 && ip->ref == 0) // Remember empty slot.
			empty = ip;
	}

	// Recycle an inode cache entry.
	if (empty == 0) {
        growicache();
        release(&lock);
		return iget(dev, inum);
    }

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	ip->flags = 0;
	release(&lock);

	return ip;
}

// only call this function if you hold a lock on the cache!
void growicache() {
    void *ptr = (void *)kalloc();
    memset(ptr, 0, 4096);
    uint16 allot = 4096 / sizeof(struct icache_node);
    uint16 offset = 0;
    struct icache_node *last;
    if (icache.head == 0) {
        icache.head = (struct icache_node *)ptr;
        offset++;
        last = icache.head;
    } else {
        last = icache.head;
        while(last->next != 0) {
            last = last->next;
        }
    }
    while(allot > offset) {
        last->next = ((struct icache_node *)ptr) + offset++;
        last = last->next;
    }
    icache.unused_nodes += allot;
}
