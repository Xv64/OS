#include "types.h"
#include "fs.h" // TODO: rename to vfs.h and make more generic
#include "file.h"
#include "stat.h"
#include "fs/fs1.h"
#include "fs/ext2.h"
#include "spinlock.h"
#include "buf.h"
#include "defs.h"

struct fsmap_t {
	fstype type;
	uint dev;
	uint8 used;
};

extern uint64 ROOT_DEV;
struct fsmap_t fsmap[FS_MAX_INIT_DEV];
struct spinlock lock;

static inline fstype getfstype(uint dev) {
	for(uint16 i = 0; i != FS_MAX_INIT_DEV; i++) {
		struct fsmap_t map = fsmap[i];
		if(map.used == 1 && map.dev == dev) {
			return map.type;
		}
	}
	return -1;
}

static inline int8 setfstype(uint dev, fstype type) {
	acquire(&lock);
	for(uint16 i = 0; i != FS_MAX_INIT_DEV; i++) {
		struct fsmap_t map = fsmap[i];
		if(map.used == 0) {
			map.dev = dev;
			map.type = type;
			map.used = 1;

			release(&lock);
			return 1;
		}
	}
	release(&lock);
	return -1;
}

void readsb(int dev, struct superblock *sb){
	void *ptr = &(sb->data[0]);
	fs1_readsb(dev, (struct fs1_superblock *)ptr);
}

int dirlink(struct inode *dp, char *name, uint inum) {
	return fs1_dirlink(dp, name, inum);
}

struct inode *dirlookup(struct inode *dp, char *name, uint *poff) {
	return fs1_dirlookup(dp, name, poff);
}

struct inode *ialloc(uint dev, short type) {
	return fs1_ialloc(dev, type);
}

struct inode *idup(struct inode *ip) {
	return fs1_idup(ip);
}

void vfsinit() {
	fs1_iinit(); // fs1 needs an explicit init before we can invoke any methods

	initlock(&lock, "vfs");

	// for now, let's just run with the ROOT_DEVd
	// later we will want to enumerate all devices
	// and build fsmap
	uint8 devtype = GETDEVTYPE(ROOT_DEV);
	uint32 devnum = GETDEVNUM(ROOT_DEV);
	if(ext2_init_dev(devtype, devnum) == 1) {
		cprintf("ext2 filesystem detected on disk(%d,%d)\n", devtype, devnum);
		setfstype(ROOT_DEV, FS_TYPE_EST2);
	} else {
		// legacy fallback...
		cprintf("xv6 filesystem assumed on disk(%d,%d)\n", devtype, devnum);
		setfstype(ROOT_DEV, FS_TYPE_FS1);
	}
}

void ilock(struct inode *ip) {
	fs1_ilock(ip);
}

void iput(struct inode *ip) {
	fs1_iput(ip);
}

void iunlock(struct inode *ip) {
	fs1_iunlock(ip);
}

void iunlockput(struct inode *ip) {
	fs1_iunlockput(ip);
}

void iupdate(struct inode *ip) {
	fs1_iupdate(ip);
}

int namecmp(const char *s, const char *t) {
	return fs1_namecmp(s, t);
}

struct inode *namei(char *path) {
	return fs1_namei(path);
}
struct inode *nameiparent(char *path, char *name) {
	return fs1_nameiparent(path, name);
}

int readi(struct inode *ip, char *dst, uint off, uint n) {
	return fs1_readi(ip, dst, off, n);
}

void stati(struct inode *ip, struct stat *st) {
	fs1_stati(ip, st);
}

int writei(struct inode *ip, char *src, uint off, uint n) {
	return fs1_writei(ip, src, off, n);
}
