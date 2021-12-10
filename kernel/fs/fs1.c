// This source file contains what is, retroactively, called fs1.
// This is the original filesystem from xv6.

// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "buf.h"
#include "vfs.h"
#include "fs/fs1.h"
#include "file.h"
#include "kernel/string.h"

#define min(a, b) ((a) < (b) ? (a) : (b))
static void fs1_itrunc(struct inode*);

// Read the super block.
void fs1_readsb(int dev, struct fs1_superblock* sb){
	struct buf* bp;

	bp = bread(dev, 1);
	memmove(sb, bp->data, sizeof(*sb));
	brelse(bp);
}

// Zero a block.
static void fs1_bzero(int dev, int bno){
	struct buf* bp;

	bp = bread(dev, bno);
	memset(bp->data, 0, BSIZE);
	log_write(bp);
	brelse(bp);
}

// Blocks.

// Allocate a zeroed disk block.
static uint fs1_balloc(uint dev){
	int b, bi, m;
	struct buf* bp;
	struct fs1_superblock sb;

	bp = 0;
	fs1_readsb(dev, &sb);
	for (b = 0; b < sb.size; b += BPB) {
		bp = bread(dev, BBLOCK(b, sb.ninodes));
		for (bi = 0; bi < BPB && b + bi < sb.size; bi++) {
			m = 1 << (bi % 8);
			if ((bp->data[bi / 8] & m) == 0) { // Is block free?
				bp->data[bi / 8] |= m; // Mark block in use.
				log_write(bp);
				brelse(bp);
				fs1_bzero(dev, b + bi);
				return b + bi;
			}
		}
		brelse(bp);
	}
	panic("fs1_balloc: out of blocks");
}

// Free a disk block.
static void fs1_bfree(int dev, uint b){
	struct buf* bp;
	struct fs1_superblock sb;
	int bi, m;

	fs1_readsb(dev, &sb);
	bp = bread(dev, BBLOCK(b, sb.ninodes));
	bi = b % BPB;
	m = 1 << (bi % 8);
	if ((bp->data[bi / 8] & m) == 0)
		panic("freeing free block");
	bp->data[bi / 8] &= ~m;
	log_write(bp);
	brelse(bp);
}

// Inodes.
//
// An inode describes a single unnamed file.
// The inode disk structure holds metadata: the file's type,
// its size, the number of links referring to it, and the
// list of blocks holding the file's content.
//
// The inodes are laid out sequentially on disk immediately after
// the superblock. Each inode has a number, indicating its
// position on the disk.
//
// The kernel keeps a cache of in-use inodes in memory
// to provide a place for synchronizing access
// to inodes used by multiple processes. The cached
// inodes include book-keeping information that is
// not stored on disk: ip->ref and ip->flags.
//
// An inode and its in-memory represtative go through a
// sequence of states before they can be used by the
// rest of the file system code.
//
// * Allocation: an inode is allocated if its type (on disk)
//   is non-zero. ialloc() allocates, iput() frees if
//   the link count has fallen to zero.
//
// * Referencing in cache: an entry in the inode cache
//   is free if ip->ref is zero. Otherwise ip->ref tracks
//   the number of in-memory pointers to the entry (open
//   files and current directories). iget() to find or
//   create a cache entry and increment its ref, iput()
//   to decrement ref.
//
// * Valid: the information (type, size, &c) in an inode
//   cache entry is only correct when the I_VALID bit
//   is set in ip->flags. ilock() reads the inode from
//   the disk and sets I_VALID, while iput() clears
//   I_VALID if ip->ref has fallen to zero.
//
// * Locked: file system code may only examine and modify
//   the information in an inode and its content if it
//   has first locked the inode. The I_BUSY flag indicates
//   that the inode is locked. ilock() sets I_BUSY,
//   while iunlock clears it.
//
// Thus a typical sequence is:
//   ip = iget(dev, inum)
//   ilock(ip)
//   ... examine and modify ip->xxx ...
//   iunlock(ip)
//   iput(ip)
//
// ilock() is separate from iget() so that system calls can
// get a long-term reference to an inode (as for an open file)
// and only lock it for short periods (e.g., in read()).
// The separation also helps avoid deadlock and races during
// pathname lookup. iget() increments ip->ref so that the inode
// stays cached and pointers to it remain valid.
//
// Many internal file system functions expect the caller to
// have locked the inodes involved; this lets callers create
// multi-step atomic operations.

struct {
	struct spinlock lock;
	struct inode inode[NINODE];
} fs1_icache;

void fs1_iinit(void){
	initlock(&fs1_icache.lock, "fs1_icache");
}

extern uint64 ROOT_DEV;
static struct inode* fs1_iget(uint dev, uint inum);


// Allocate a new inode with the given type on device dev.
// A free inode has a type of zero.
struct inode* fs1_ialloc(uint dev, short type){
	int inum;
	struct buf* bp;
	struct dinode* dip;
	struct fs1_superblock sb;

	fs1_readsb(dev, &sb);

	for (inum = 1; inum < sb.ninodes; inum++) {
		bp = bread(dev, IBLOCK(inum));
		dip = (struct dinode*)bp->data + inum % IPB;
		if (dip->type == 0) { // a free inode
			memset(dip, 0, sizeof(*dip));
			dip->type = type;
			log_write(bp); // mark it allocated on the disk
			brelse(bp);
			return fs1_iget(dev, inum);
		}
		brelse(bp);
	}
	panic("ialloc: no inodes");
}

// Copy a modified in-memory inode to disk.
void fs1_iupdate(struct inode* ip){
	struct buf* bp;
	struct dinode* dip;

	bp = bread(ip->dev, IBLOCK(ip->inum));
	dip = (struct dinode*)bp->data + ip->inum % IPB;
	dip->type = ip->type;
	dip->major = ip->major;
	dip->minor = ip->minor;
	dip->nlink = ip->nlink;
	dip->size = ip->size;
	memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
	log_write(bp);
	brelse(bp);
}

// Find the inode with number inum on device dev
// and return the in-memory copy. Does not lock
// the inode and does not read it from disk.
static struct inode* fs1_iget(uint dev, uint inum){
	struct inode* ip, * empty;

	acquire(&fs1_icache.lock);

	// Is the inode already cached?
	empty = 0;
	for (ip = &fs1_icache.inode[0]; ip < &fs1_icache.inode[NINODE]; ip++) {
		if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
			ip->ref++;
			release(&fs1_icache.lock);
			return ip;
		}
		if (empty == 0 && ip->ref == 0) // Remember empty slot.
			empty = ip;
	}

	// Recycle an inode cache entry.
	if (empty == 0)
		panic("fs1_iget: no inodes");

	ip = empty;
	ip->dev = dev;
	ip->inum = inum;
	ip->ref = 1;
	ip->flags = 0;
	release(&fs1_icache.lock);

	return ip;
}

// Increment reference count for ip.
// Returns ip to enable ip = idup(ip1) idiom.
struct inode* fs1_idup(struct inode* ip){
	acquire(&fs1_icache.lock);
	ip->ref++;
	release(&fs1_icache.lock);
	return ip;
}

// Unlock the given inode.
void fs1_iunlock(struct inode* ip){
	if (ip == 0 || !(ip->flags & I_BUSY) || ip->ref < 1)
		panic("iunlock");

	acquire(&fs1_icache.lock);
	ip->flags &= ~I_BUSY;
	wakeup(ip);
	release(&fs1_icache.lock);
}

// Drop a reference to an in-memory inode.
// If that was the last reference, the inode cache entry can
// be recycled.
// If that was the last reference and the inode has no links
// to it, free the inode (and its content) on disk.
// All calls to iput() must be inside a transaction in
// case it has to free the inode.
void fs1_iput(struct inode* ip){
	acquire(&fs1_icache.lock);
	if (ip->ref == 1 && (ip->flags & I_VALID) && ip->nlink == 0) {
		// inode has no links and no other references: truncate and free.
		if (ip->flags & I_BUSY)
			panic("iput busy");
		ip->flags |= I_BUSY;
		release(&fs1_icache.lock);
		fs1_itrunc(ip);
		ip->type = 0;
		iupdate(ip);
		acquire(&fs1_icache.lock);
		ip->flags = 0;
		wakeup(ip);
	}
	ip->ref--;
	release(&fs1_icache.lock);
}


// Inode content
//
// The content (data) associated with each inode is stored
// in blocks on the disk. The first NDIRECT block numbers
// are listed in ip->addrs[].  The next NINDIRECT blocks are
// listed in block ip->addrs[NDIRECT].

// Return the disk block address of the nth block in inode ip.
// If there is no such block, bmap allocates one.
static uint fs1_bmap(struct inode* ip, uint bn){
	uint addr, * a;
	struct buf* bp;

	if (bn < NDIRECT) {
		if ((addr = ip->addrs[bn]) == 0)
			ip->addrs[bn] = addr = fs1_balloc(ip->dev);
		return addr;
	}
	bn -= NDIRECT;

	if (bn < NINDIRECT) {
		// Load indirect block, allocating if necessary.
		if ((addr = ip->addrs[NDIRECT]) == 0)
			ip->addrs[NDIRECT] = addr = fs1_balloc(ip->dev);
		bp = bread(ip->dev, addr);
		a = (uint*)bp->data;
		if ((addr = a[bn]) == 0) {
			a[bn] = addr = fs1_balloc(ip->dev);
			log_write(bp);
		}
		brelse(bp);
		return addr;
	}

	panic("fs1_bmap: out of range");
}

// Truncate inode (discard contents).
// Only called when the inode has no links
// to it (no directory entries referring to it)
// and has no in-memory reference to it (is
// not an open file or current directory).
static void fs1_itrunc(struct inode* ip){
	int i, j;
	struct buf* bp;
	uint* a;

	for (i = 0; i < NDIRECT; i++) {
		if (ip->addrs[i]) {
			fs1_bfree(ip->dev, ip->addrs[i]);
			ip->addrs[i] = 0;
		}
	}

	if (ip->addrs[NDIRECT]) {
		bp = bread(ip->dev, ip->addrs[NDIRECT]);
		a = (uint*)bp->data;
		for (j = 0; j < NINDIRECT; j++) {
			if (a[j])
				fs1_bfree(ip->dev, a[j]);
		}
		brelse(bp);
		fs1_bfree(ip->dev, ip->addrs[NDIRECT]);
		ip->addrs[NDIRECT] = 0;
	}

	ip->size = 0;
	iupdate(ip);
}

// Copy stat information from inode.
void fs1_stati(struct inode* ip, struct stat* st){
	st->dev = ip->dev;
	st->ino = ip->inum;
	st->type = ip->type;
	st->nlink = ip->nlink;
	st->size = ip->size;
}


// Read data from inode.
int fs1_readi(struct inode* ip, char* dst, uint off, uint n){
	uint tot, m;
	struct buf* bp;

	if (off > ip->size || off + n < off)
		return -1;
	if (off + n > ip->size)
		n = ip->size - off;

	for (tot = 0; tot < n; tot += m, off += m, dst += m) {
		bp = bread(ip->dev, fs1_bmap(ip, off / BSIZE));
		m = min(n - tot, BSIZE - off % BSIZE);
		memmove(dst, bp->data + off % BSIZE, m);
		brelse(bp);
	}
	return n;
}

// Write data to inode.
int fs1_writei(struct inode* ip, char* src, uint off, uint n){
	uint tot, m;
	struct buf* bp;

	if (off > ip->size || off + n < off)
		return -1;
	if (off + n > MAXFILE * BSIZE)
		return -1;

	for (tot = 0; tot < n; tot += m, off += m, src += m) {
		bp = bread(ip->dev, fs1_bmap(ip, off / BSIZE));
		m = min(n - tot, BSIZE - off % BSIZE);
		memmove(bp->data + off % BSIZE, src, m);
		log_write(bp);
		brelse(bp);
	}

	if (n > 0 && off > ip->size) {
		ip->size = off;
		iupdate(ip);
	}
	return n;
}


// Directories

int fs1_namecmp(const char* s, const char* t){
	return strncmp(s, t, DIRSIZ);
}

// Look for a directory entry in a directory.
// If found, set *poff to byte offset of entry.
struct inode* fs1_dirlookup(struct inode* dp, char* name, uint* poff){
	uint off, inum;
	struct dirent de;

	if (dp->type != T_DIR)
		panic("dirlookup not DIR");

	for (off = 0; off < dp->size; off += sizeof(de)) {
		if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
			panic("dirlink read");
		if (de.inum == 0)
			continue;
		if (namecmp(name, de.name) == 0) {
			// entry matches path element
			if (poff)
				*poff = off;
			inum = de.inum;
			return fs1_iget(dp->dev, inum);
		}
	}

	return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int fs1_dirlink(struct inode* dp, char* name, uint inum){
	int off;
	struct dirent de;
	struct inode* ip;

	// Check that name is not present.
	if ((ip = dirlookup(dp, name, 0)) != 0) {
		iput(ip);
		return -1;
	}

	// Look for an empty dirent.
	for (off = 0; off < dp->size; off += sizeof(de)) {
		if (readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
			panic("dirlink read");
		if (de.inum == 0)
			break;
	}

	strncpy(de.name, name, DIRSIZ);
	de.inum = inum;
	if (writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
		panic("dirlink");

	return 0;
}

void fs1_readinode(struct inode *ip) {
	struct buf* bp;
	struct dinode* dip;

	uint32 sector = IBLOCK(ip->inum);
	bp = bread(ip->dev, sector);
	dip = (struct dinode*)bp->data;
	dip += ip->inum % IPB; // increment dip to the correct inode in the sector
	ip->type = dip->type;
	ip->major = dip->major;
	ip->minor = dip->minor;
	ip->nlink = dip->nlink;
	ip->size = dip->size;
	memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
	brelse(bp);
}
