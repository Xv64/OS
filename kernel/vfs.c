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

struct fsmap_t {
	fstype type;
	uint64 dev;
	uint8 used;
};

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
			cprintf("SAT\n");

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

void ilock(struct inode *ip) {
	fstype t = getfstype(ip->dev);
	if(t == FS_TYPE_EXT2) {
		ext2_ilock(ip);
	} else if(t == FS_TYPE_FS1) {
		fs1_ilock(ip);
	} else {
		panic("Unknown fs type");
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

struct inode *namei(char *path) {
	fstype t = getfstype(ROOT_DEV); // TODO: look up device
	if(t == FS_TYPE_EXT2) {
		return ext2_namei(path);
	} else if(t == FS_TYPE_FS1) {
		return fs1_namei(path);
	} else {
		panic("Unknown fs type");
	}
}
struct inode *nameiparent(char *path, char *name) {
	fstype t = getfstype(ROOT_DEV); // TODO: look up device
	if(t == FS_TYPE_EXT2) {
		return ext2_nameiparent(path, name);
	} else if(t == FS_TYPE_FS1) {
		return fs1_nameiparent(path, name);
	} else {
		panic("Unknown fs type");
	}
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
