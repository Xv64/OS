#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define stat xv6_stat  // avoid clash with host struct stat
#include "../include/types.h"
#include "../include/stat.h"
#include "../include/param.h"

// ~~~~~~~~~~~~~~~~~~~~~~~
// HACK: copy/paste from fs/fs1.h
#define ROOTINO 1  // root i-number
#define BSIZE 512  // block size

#define NDIRECT 28
#define NINDIRECT (BSIZE / sizeof(uint))
#define MAXFILE (NDIRECT + NINDIRECT)

// On-disk inode structure
struct dinode {
  int16  type;               // File type
  int16  major;              // Major device number (T_DEV only)
  int16  minor;              // Minor device number (T_DEV only)
  int16  nlink;              // Number of links to inode in file system
  uint32 size;               // Size of file (bytes)
  uint32 addrs[NDIRECT+1];   // Data block addresses
};

// Inodes per block.
#define IPB           (BSIZE / sizeof(struct dinode))

// Block containing inode i
#define IBLOCK(i)     ((i) / IPB + 2)

// Bitmap bits per block
#define BPB           (BSIZE*8)

// Block containing bit for block b
#define BBLOCK(b, ninodes) (b/BPB + (ninodes)/IPB + 3)

// Directory is a file containing a sequence of dirent structures.
#define DIRSIZ 14

struct dirent {
  ushort inum;
  char name[DIRSIZ];
};


struct fs1_superblock {
  uint size;         // Size of file system image (blocks)
  uint nblocks;      // Number of data blocks
  uint ninodes;      // Number of inodes.
  uint nlog;         // Number of log blocks
};

// ~~~~~~~~~~~~~~~~~~~~~~~

#ifndef static_assert
# define static_assert(a, b) do { switch (0) case 0: case (a):; } while (0)
#endif // static_assert

#define MAX_PATH_LEN 2048
#define FREESPACE 3000
int nblocks = (995-LOGSIZE) + FREESPACE;
int nlog = LOGSIZE;
int ninodes = 200;
int size = 1024 + FREESPACE + 25;
//size MUST EQUAL nblocks + usedblocks + nlog

int fsfd;
struct fs1_superblock sb;
char zeroes[512];
uint freeblock;
uint usedblocks;
uint bitblocks;
uint freeinode = 1;

void balloc(int);
void wsect(uint, void*);
void winode(uint, struct dinode*);
void rinode(uint inum, struct dinode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);

// convert to intel byte order
ushort xshort(ushort x) {
	ushort y;
	uchar *a = (uchar*)&y;
	a[0] = x;
	a[1] = x >> 8;
	return y;
}

uint xint(uint x) {
	uint y;
	uchar *a = (uchar*)&y;
	a[0] = x;
	a[1] = x >> 8;
	a[2] = x >> 16;
	a[3] = x >> 24;
	return y;
}

struct dirent de;

uint mkdir(char *dirname, uint parentdir) {
	uint dirino = ialloc(T_DIR);

	bzero(&de, sizeof(de));
	de.inum = xshort(dirino);
	strcpy(de.name, dirname);
	iappend(parentdir, &de, sizeof(de));

	bzero(&de, sizeof(de));
	de.inum = xshort(dirino);
	strcpy(de.name, ".");
	iappend(dirino, &de, sizeof(de));

	bzero(&de, sizeof(de));
	de.inum = xshort(parentdir);
	strcpy(de.name, "..");
	iappend(dirino, &de, sizeof(de));

	return dirino;
}

int main(int argc, char *argv[]) {
	int i, cc, fd;
	uint rootino, inum, off;
	char buf[512];
	struct dinode din;


	static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

	if(argc < 2) {
		fprintf(stderr, "Usage: mkfs fs.img files...\n");
		exit(1);
	}

	assert((512 % sizeof(struct dinode)) == 0);
	assert((512 % sizeof(struct dirent)) == 0);

	fsfd = open(argv[1], O_RDWR|O_CREAT|O_TRUNC, 0666);
	if(fsfd < 0) {
		perror(argv[1]);
		exit(1);
	}

	sb.size = xint(size);
	sb.nblocks = xint(nblocks); // so whole disk is size sectors
	sb.ninodes = xint(ninodes);
	sb.nlog = xint(nlog);

	bitblocks = size/(512*8) + 1;
	usedblocks = ninodes / IPB + 3 + bitblocks;
	freeblock = usedblocks;

	printf("used %d (bit %d ninode %zu) free %u log %u total %d\n", usedblocks,
	       bitblocks, ninodes/IPB + 1, freeblock, nlog, nblocks+usedblocks+nlog);

	printf("size: %d\n", size);
	assert(nblocks + usedblocks + nlog == size);

	for(i = 0; i < nblocks + usedblocks + nlog; i++)
		wsect(i, zeroes);

	memset(buf, 0, sizeof(buf));
	memmove(buf, &sb, sizeof(sb));
	wsect(1, buf);

	rootino = ialloc(T_DIR);
	assert(rootino == ROOTINO);

	bzero(&de, sizeof(de));
	de.inum = xshort(rootino);
	strcpy(de.name, ".");
	iappend(rootino, &de, sizeof(de));

	bzero(&de, sizeof(de));
	de.inum = xshort(rootino);
	strcpy(de.name, "..");
	iappend(rootino, &de, sizeof(de));

	mkdir("dev", rootino);
	uint binino = mkdir("bin", rootino);
	uint kextino = mkdir("kexts", rootino);

	for(i = 2; i < argc; i++) {
		char *name = argv[i];

		if (!strncmp(name, "fs/", 3)){
			name += 2;
		}

		char localpath[MAX_PATH_LEN + 1];
		strcpy(localpath, name);

		char *lastSlash = strrchr(name, '/');
		while(lastSlash != 0 && name++ != lastSlash)
			;

		for(int j = strlen(localpath); j >= 0; j--){
			if(localpath[j] == '/'){
				localpath[j+1] = '\0';
				break;
			}
		}

		assert(index(name, '/') == 0);

		if((fd = open(argv[i], 0)) < 0) {
			perror(argv[i]);
			exit(1);
		}
		printf("Copying %s%s...", localpath, name);

		uint path = binino;

		if(strcmp(localpath, "/") == 0){
			path = rootino;
		} else if(strcmp(localpath, "/kexts/") == 0){
			path = kextino;
		} //TODO: more dynamic path inode creation/lookup

		inum = ialloc(T_FILE);

		bzero(&de, sizeof(de));
		de.inum = xshort(inum);
		strncpy(de.name, name, DIRSIZ);
		iappend(path, &de, sizeof(de));

		int finalsize = 0;
		while((cc = read(fd, buf, sizeof(buf))) > 0){
			finalsize += cc;
			iappend(inum, buf, cc);
		}

		printf(" %d\n", finalsize);
		close(fd);
	}

	// fix size of bin inode dir
	rinode(binino, &din);
	off = xint(din.size);
	off = ((off/BSIZE) + 1) * BSIZE;
	din.size = xint(off);
	winode(binino, &din);

	// fix size of root inode dir
	rinode(rootino, &din);
	off = xint(din.size);
	off = ((off/BSIZE) + 1) * BSIZE;
	din.size = xint(off);
	winode(rootino, &din);

	balloc(usedblocks);

	exit(0);
}

void wsect(uint sec, void *buf) {
	if(lseek(fsfd, sec * 512L, 0) != sec * 512L) {
		perror("lseek");
		exit(1);
	}
	if(write(fsfd, buf, 512) != 512) {
		perror("write");
		exit(1);
	}
}

uint i2b(uint inum) {
	return (inum / IPB) + 2;
}

void winode(uint inum, struct dinode *ip) {
	char buf[512];
	uint bn;
	struct dinode *dip;

	bn = i2b(inum);
	rsect(bn, buf);
	dip = ((struct dinode*)buf) + (inum % IPB);
	*dip = *ip;
	wsect(bn, buf);
}

void rinode(uint inum, struct dinode *ip) {
	char buf[512];
	uint bn;
	struct dinode *dip;

	bn = i2b(inum);
	rsect(bn, buf);
	dip = ((struct dinode*)buf) + (inum % IPB);
	*ip = *dip;
}

void rsect(uint sec, void *buf) {
	if(lseek(fsfd, sec * 512L, 0) != sec * 512L) {
		perror("lseek");
		exit(1);
	}
	if(read(fsfd, buf, 512) != 512) {
		perror("read");
		exit(1);
	}
}

uint ialloc(ushort type) {
	uint inum = freeinode++;
	struct dinode din;

	bzero(&din, sizeof(din));
	din.type = xshort(type);
	din.nlink = xshort(1);
	din.size = xint(0);
	winode(inum, &din);
	return inum;
}

void balloc(int used) {
	uchar buf[512];
	int i;

	printf("balloc: first %d blocks have been allocated\n", used);
	assert(used < 512*8);
	bzero(buf, 512);
	for(i = 0; i < used; i++) {
		buf[i/8] = buf[i/8] | (0x1 << (i%8));
	}
	printf("balloc: write bitmap block at sector %zu\n", ninodes/IPB + 3);
	wsect(ninodes / IPB + 3, buf);
}

#define min(a, b) ((a) < (b) ? (a) : (b))

void iappend(uint inum, void *xp, int n) {
	char *p = (char*)xp;
	uint fbn, off, n1;
	struct dinode din;
	char buf[512];
	uint indirect[NINDIRECT];
	uint x;

	rinode(inum, &din);

	off = xint(din.size);
	while(n > 0) {
		fbn = off / 512;
		assert(fbn < MAXFILE);
		if(fbn < NDIRECT) {
			if(xint(din.addrs[fbn]) == 0) {
				din.addrs[fbn] = xint(freeblock++);
				usedblocks++;
			}
			x = xint(din.addrs[fbn]);
		} else {
			if(xint(din.addrs[NDIRECT]) == 0) {
				// printf("allocate indirect block\n");
				din.addrs[NDIRECT] = xint(freeblock++);
				usedblocks++;
			}
			// printf("read indirect block\n");
			rsect(xint(din.addrs[NDIRECT]), (char*)indirect);
			if(indirect[fbn - NDIRECT] == 0) {
				indirect[fbn - NDIRECT] = xint(freeblock++);
				usedblocks++;
				wsect(xint(din.addrs[NDIRECT]), (char*)indirect);
			}
			x = xint(indirect[fbn-NDIRECT]);
		}
		n1 = min(n, (fbn + 1) * 512 - off);
		rsect(x, buf);
		bcopy(p, buf + off - (fbn * 512), n1);
		wsect(x, buf);
		n -= n1;
		off += n1;
		p += n1;
	}
	din.size = xint(off);
	winode(inum, &din);
}
