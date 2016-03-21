/** @file 2.charter.cc
 * Charts the usage of blocks on a 7th edition filesystem.
 *
 * NOTE: First look at "1.voorbeeld.cc" !
 */
// -----------------------------------------------------

// Our own includes
#include "ansi.h"		// for: ansi color code strings
#include "asserts.h"	// for: notreached() e.a.
#include "unix_error.h"	// for: the unix_error() exception class
#include "cstr.h"		// for: the cstr() wrapper and it's output operator

// C/C++/STL specific
#include <iostream>	    // for: std::cout, std::endl
#include <ctime>		// for: ctime()
#include <cstdlib>		// for: EXIT_SUCCESS, EXIT_FAILURE

// Our own classes
#include "Device.h"		// the "device driver"
#include "Block.h"		// the "data blocks"


// ================================================================
// To register what blocks are used for ...

/** Codes for blocktypes */
enum BlkUse
{
	B_UNKNOWN,								// default status is "unknown usage"
	B_BOOT, B_SUPER, B_INODE,				// this covers the first part of the storage medium
	B_DATA, B_INDIR1, B_INDIR2, B_INDIR3,	// blocks known to be used in the data area
	B_FREE,									// blocks known to be unused in the data area
	B_MULTI									// blocks claimed multiple times
};

// A table to keep track of what a block is used for
std::map<daddr_x,BlkUse>  usage;

// Function to register what a block is being used for
void	useBlock(daddr_x bno, BlkUse use)
{
	if(usage[bno] == B_UNKNOWN) {			// unknow upto now?
		usage[bno] = use;					// oke, register what it is being used for.
	} else
	if(usage[bno] != use) {					// odd, changing usage to something else ?
		std::cout << AC_BROWN "\nUsage: " << bno << " was " << usage[bno]
				  << ", but now " << use << "?" AA_RESET << std::endl;
		usage[bno] = B_MULTI;				// Oops, claimed multiple times
	}
}


// Finally we are going to jeport the usage of all blocks
void	reportUsage()
{
static char  codes[] = "?BSID123.*";

	std::cout << "BlockUsage: " << usage.size() << " blocks charted" << std::endl;

	// Note: Iterating thru a c++ map goes in ascendig key order
	daddr_x	  n = 0;
	std::cout << AC_BLUE;
	for(auto i : usage) {	// let the compiler decide the type of 'i'
		if(i.first != n) {
			n = i.first;
			std::cerr << "Map starts at block " << n << std::endl;
		}
		std::cout << codes[i.second];
		++n;
	}
	std::cout << AA_RESET << std::endl;
}


// ================================================================
// The code to examine the disk


// Read super-block and save some data for later.
// @param		device	The device "driver"
// @param[out]	isize	Data from superblock
// @param[out]	fchain	Data from superblock
void	readSuperblock(Device& device, daddr_x& isize, daddr_x& fchain)
{
	// Fetch the block containing the super-block
	Block*  sp = device.getBlock(SUPERB);
	// and register it's role
	useBlock(SUPERB, B_SUPER);

	// Using the u.fs view on a block

	// Use some info from the superblock for a title  ...
	std::cout << "Examine " << cstr(sp->u.fs.s_fname, 6)
			  << '.' << cstr(sp->u.fs.s_fpack,6) << std::endl;
	// Limit the range of further allowed device I/O
	device.setLimit(sp->u.fs.s_fsize);
	// Scan the free_list table
	for(unsigned  i = 0; i < sp->u.fs.s_nfree; ++i) {
		useBlock(sp->u.fs.s_free[i], B_FREE);	// register it's role
	}

	// We need a few things for later
	isize  = sp->u.fs.s_isize;
	fchain = sp->u.fs.s_free[0];

	sp->release();	// no longer need this block
}

// Scan the remaining free_list.
// @param[in]	device		The device driver
// @param[in]	fchain		The blocknumber where to begin
void	readFreeList(Device& device, daddr_x fchain)
{
	// Follow the freelist chain
	while (fchain != 0) {
		Block*  bp = device.getBlock(fchain);
		// Scan this table using the u.fb view on a block
		for(int  i = 0; i < bp->u.fb.df_nfree; ++i) {
			daddr_x addr = bp->u.fb.df_free[i];
			if(addr != 0)
				useBlock(addr, B_FREE);
		}
		// Get a new chain value
		fchain = bp->u.fb.df_free[0];
		bp->release();	// this block no longer needed
	}
}


// = = = = = = inode stuff = = = = = = =


// forward declarations:
void	registerBlocks(Device& device, daddr_x da[], off_x size);
void	registerIndir(Device& device, daddr_x addr, int level, off_x& size);


// Read inode 'inum' from the device.
// @param		device	The device to read from
// @param		inum	The inode number
void	readInode(Device& device, ino_x inum)
{
	// Fetch the block containing that inode
	Block*  ip = device.getBlock(itod(inum));

	useBlock(itod(inum), B_INODE);			// register it's usage

	// Using the u.dino view on a block

	// Determine where that inode lives in this block
	dinode*	dp = &ip->u.dino[ itoo(inum) ];
	if(dp->di_mode != 0)	// Is this inode being used ?
	{
		// Does this kind of inode have datablocks?
		bool hasData = (   ((dp->di_mode & X_IFMT) == X_IFREG)	// a regular file
						|| ((dp->di_mode & X_IFMT) == X_IFDIR)	// a directory
					   );
		if(hasData) {
			// Convert the 13, 24-bit, blocknumbers in that inode
			// into ordinary 32-bit daddr_x longs.
			daddr_x  da[NADDR];
			Block::l3tol(da, dp->di_addr);
			// Register the blocks used by this inode
			registerBlocks(device, da, dp->di_size);
		}
	}
	ip->release();
}


// Register the 13 blocks registered with an inode
// and handle the indirection tree as well.
// @param[i]	device	The device to read
// @param[in]	da[]	The 13 block numbers
// @param[in]	size	The (remaining) filesize unaccounted for
void	registerBlocks(Device& device, daddr_x da[], off_x size)
{
	for(unsigned  i = 0 ; (i < NADDR) && (size > 0) ; ++i)
	{
		daddr_x	 addr = da[i];
		switch(i)
		{
		// WARNING: this is a non-standard 'case' (a GCC extension)
		case 0 ... 9:	// the 10 'direct' blocks
			if(addr != 0) {	// not a hole?
				useBlock(addr, B_DATA);
			}
			size -= DBLKSIZ;	// discount size
			break;

		case 10:		// the top level 1 indir block
			if(addr != 0) {	// not a hole?
				useBlock(addr, B_INDIR1);
				registerIndir(device, addr, 1, size);
			} else {
				size -= 128 * DBLKSIZ;	// discount accumulated size
			}
			break;

		case 11:		// the top level 2 indir block
			if(addr != 0) {	// not a hole?
				useBlock(addr, B_INDIR2);
				registerIndir(device, addr, 2, size);
			} else {
				size -= 128 * 128 * DBLKSIZ;	// discount accumulated size
			}
			break;

		case 12:		// the top level 3 indir block
			if(addr != 0) {	// not a hole?
				useBlock(addr, B_INDIR3);
				registerIndir(device, addr, 3, size);
			} else {
				size -= 128 * 128 * 128 * DBLKSIZ;	// discount accumulated size
			}
			break;

		default:
			notreached();
		}
	}
}


// Register data about indirection blocks of an inode
// @param[in]		device	The device to read from
// @param[in]		addr	The address of this indirection block
// @param[in]		level	The indirection level of this block
// @param[in,out]	size	The (remaining) filesize unaccounted for
void	registerIndir(Device& device, daddr_x addr, int level, off_x& size)
{
	Block*  bp = device.getBlock(addr);

	// Using the u.bno view on this block

	// Note: The usage of this block was already registered by the caller!
	for(unsigned  i = 0; (i < NINDIR) && (size > 0) ; ++i)
	{
		daddr_x  refs = bp->u.bno[i];	// Take a blocknumber
		switch(level)	// how deep are we in the tree
		{
		case 1:		// an indir 1 block holds data block numbers
			if(refs != 0) {	// not a hole?
				useBlock(refs, B_DATA);
			}
			size -= DBLKSIZ;	// discount it's size
			break;

		case 2:		// indir 2 => indir 1 blocks
			if(refs != 0) {	// not a hole?
				useBlock(refs, B_INDIR1);
				registerIndir(device, refs, 1, size);
			} else {
				size -= 128 * DBLKSIZ;	// discount accumulated size
			}
			break;

		case 3:		// indir 3 => indir 2 blocks
			if(refs != 0) {	// not a hole?
				useBlock(refs, B_INDIR2);
				registerIndir(device, refs, 2, size);
			} else {
				size -= 128 * 128 * DBLKSIZ;	// discount accumulated size
			}
			break;

		default:
			notreached();
		}
	}
	bp->release();
}


// - - - - - - - - - -

// The main block charter function
void	charter(const char*  floppie)
{
	std::cout << "Opening " << floppie << std::endl;

	Device  device(floppie);				// the device "driver"

	useBlock(0, B_BOOT);					// register that 0 is the bootblock

	// two locals to hold some information from the superblock
	daddr_x	 fchain = 0;					// freelist continuation
	daddr_x	 isize = 0;						// start of userdata area

	readSuperblock(device, isize, fchain);	// gather some data
	readFreeList(device, fchain);			// and follow the free block list

	// how many inodes are there?
	ino_x	ninode = (isize - 2) * INOPB;	// -2 (i.e. bootblock & superblock)
	std::cout << "has " << ninode << " inodes" << std::endl;
	// examine all inodes
	for(ino_x  inum = 1; inum < ninode; ++inum) {
		readInode(device, inum);
	}

	// show the results
	reportUsage();
}

// - - - - - - - - - -

// Main is just the TUI
int  main(int argc, const char* argv[])
{
	try {
		// a given parameter or use the default ?
		charter((argc > 1) ? argv[1] : "floppie.img");
		return EXIT_SUCCESS;
	} catch(const unix_error& e) {
		std::cerr << AC_RED "SYSTEM: " << e.what() << AA_RESET << std::endl;
		return EXIT_FAILURE;
	} catch(const std::exception& e) {
		std::cerr << AC_RED "OOPS: " << e.what() << AA_RESET << std::endl;
		return EXIT_FAILURE;
	} catch(...) {
		std::cerr << AC_RED "OOPS: something went wrong" AA_RESET << std::endl;
		return EXIT_FAILURE;
	}
}


// vim:aw:ai:ts=4:

