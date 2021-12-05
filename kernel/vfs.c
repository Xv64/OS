#include "types.h"
#include "fs.h" // TODO: rename to vfs.h and make more generic
#include "file.h"
#include "stat.h"
#include "fs/fs1.h"

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

void iinit() {
	fs1_iinit();
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
