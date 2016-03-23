// -----------------------------------------------------
/** @file main.cc
 * This is the program that you have to finish.
 */
// -----------------------------------------------------

// Our own includes
#include "ansi.h"		// for: ansi color code strings
#include "asserts.h"	// for: notreached() e.a.
#include "unix_error.h"	// for: the unix_error() exception class
#include "cstr.h"		// for: the cstr() wrapper and it's output operator

// C/C++/STL specific
#include <iostream>	    // for: std::cout, std::endl
#include <fstream>
#include <ctime>		// for: ctime()
#include <cstdlib>		// for: EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>		// for: close() etc
#include <fcntl.h>		// for: O_WRONLY etc

// The include files for the unix 7-th Edition Filesystem
#include "e7filsys.h"
#include "e7ino.h"
#include "e7dir.h"

// Our own classes
#include "Device.h"		// the "device driver"
#include "Block.h"		// the "data blocks"

using namespace std;

// ================================================================
void readFree(Device&, ostream&, daddr_x);
void readInodes(Device&, ostream&, ino_x);
void readWrites(dinode*, ostream&);


void readSuperBlock(Device& device, ostream& out)
{
    Block *sb = device.getBlock(SUPERB);
    out << "      Dump of superblock on " << sb->u.fs.s_fname << "." << sb->u.fs.s_fpack << endl;
    out << "|--------------------------------------------|" << endl;
    out << "userdata area starts in block: " << sb->u.fs.s_isize << endl;
    out << "number of blocks on volume is: " << sb->u.fs.s_fsize << endl;
    out << "number of freeblocks in free[] is: " << sb->u.fs.s_nfree << " (max 50)" << endl;
    for (int i = 0; i < sb->u.fs.s_nfree; i++){
        out << " " << sb->u.fs.s_free[i];
    }
    out << endl;
    out << "number of free inodes in inode[] is: " << sb->u.fs.s_ninode << " (max 100)" << endl;
    for (int i = 0; i < sb->u.fs.s_ninode; i++){
        out << " " << sb->u.fs.s_inode[i];
    }
    out << endl;
    out << "freelist lock flag is: " << (sb->u.fs.s_flock ? "set" : "not set") << endl;
    out << "i-list lock flag is: " << (sb->u.fs.s_ilock ? "set" : "not set") << endl;
    out << "superblock modified is: " << (sb->u.fs.s_fmod ? "set" : "not set") << endl;
    out << "read only flag is: " << (sb->u.fs.s_ronly ? "set" : "not set") << endl;
    out << "last update time was: " << ctime(&sb->u.fs.s_time) << endl;
    out << "total number of free block is: " << sb->u.fs.s_tfree << endl;
    out << "total number of inodes is: " << sb->u.fs.s_tinode << endl;
    out << "Interleave factors are: " << "m=" << sb->u.fs.s_m << " n=" << sb->u.fs.s_n << endl;
    out << "File system name: " << sb->u.fs.s_fname << endl;
    out << "File system pack: " << sb->u.fs.s_fpack << endl;
    out << "|--------------------------------------------|" << endl;
    out << "   Rest of free list continues in block " << sb->u.fs.s_free[0] << endl;
    out << "|--------------------------------------------|" << endl;
    readFree(device, out, sb->u.fs.s_free[0]);
    sb->release();
}

void readFree(Device& device, ostream& out, daddr_x addr)
{
    if (addr == 0){
        return;
    }
    Block *fl = device.getBlock(addr);
    out << "Freeblock " << addr << ": " << fl->u.fb.df_nfree << " (max 50)" << endl;
    for (int i = 0; i < fl->u.fb.df_nfree; i++){
        out << " " << fl->u.fb.df_free[i];
    }
    out << endl;
    readFree(device, out, fl->u.fb.df_free[0]);
    fl->release();
}

void readRootNode(Device& device, ostream& out)
{
    Block *inode = device.getBlock(SUPERB);
    int inodes = (inode->u.fs.s_isize - 2) * INOPB;
    out << "     Examining " << inodes << " inodes" << endl;
    out << "|--------------------------------------------|" << endl;
    for (int i = 1; i < inodes; i++){
        readInodes(device, out, i);
    }
    inode->release();
}

void readInodes(Device& device, ostream& out, ino_x addr)
{
    Block* inode = device.getBlock(itod(addr));
    dinode* dn = &inode->u.dino[itoo(addr)];

    if (!dn->di_mode){
        inode->release();
        return;
    }

    out << "Reading inode " << addr << endl;
    readWrites(dn, out);
    out << "nlink = " << dn->di_nlink << " uid = " << dn->di_uid << " gid = " << dn->di_gid << endl;
    time_t time = dn->di_atime;
    out << "atime = " << ctime(&time);
    time = dn->di_mtime;
    out << "mtime = " << ctime(&time);
    time = dn->di_ctime;
    out << "ctime = " << ctime(&time);
    out << "size = " << dn->di_size << " (this file uses at most ";
    if (dn->di_size == 0){
        out << "0 datablocks)" << endl;
    } else {
        out << (dn->di_size / 512 + 1) << " datablocks)" << endl;
    }
    daddr_x da[NADDR];
    inode->l3tol(da, dn->di_addr);
    out << "addr: ";
    for (int i = 0; i < NADDR; ++i){
        out << da[i] << " ";
    }
    out << endl;
    if (da[0] == 0){
        out << "Direct blocks in inode ";
        for (int i = 0; i < 10; i++){
            if (da[i] != 0){
                out << da[i] << " " ;
            }
        }
        out << endl;
    }

    if (da[10] != 0){
        out << "Block numbers in level 1 indirection block " << da[10] << ":";
        Block* l1 = device.getBlock(da[10]);
        for (int i = 0; i < NINDIR; i++){
            out << l1->u.bno[i] << " ";
        }
        out << endl;
        l1->release();

        if (da[11] != 0){
            out << "Block numbers in level 2 indirection block " << da[11] << ":";
            Block* l2 = device.getBlock(da[11]);
            for (int i = 0; i < NINDIR; ++i){
                out << " [" << l2->u.bno[i] << "]";
                if (l2->u.bno[i] != 0){
                    Block* b = device.getBlock(l2->u.bno[i]);
                    for (int j = 0; j < NINDIR; ++j){
                        out << b->u.bno[j] << " ";
                    }
                    b->release();
                }
            }
            out << endl;
            l2->release();

            if (da[12] != 0){
                out << "Block numbers in level 3 indirection block " << da[12] << ":";
                Block* l3 = device.getBlock(da[12]);
                for (int i = 0; i < NINDIR; ++i){
                    out << "[[" << l3->u.bno[i] << "]]";
                    if (l3->u.bno[i] != 0){
                        Block* b = device.getBlock(l3->u.bno[i]);
                        for (int j = 0; j < NINDIR; ++j){
                            out << " [" << b->u.bno[j] << "]";
                            if (b->u.bno[j] != 0) {
                                Block* b1 = device.getBlock(b->u.bno[j]);
                                for (int k = 0; k < NINDIR; ++k){
                                    out << " " << b1->u.bno[k];
                                }
                                b1->release();
                            }
                        }
                        b->release();
                    }
                }
                out << endl;
                l3->release();
            }
        }
    }
    if ((dn->di_mode & X_IFMT) == X_IFDIR) {
        out << "Contents of directory: " << endl;
        for (int i = 0; i < NADDR; i++){
            if (da[i] != 0){
                Block* dirBlock = device.getBlock(da[i]);
                for (int j = 0; j < NDIRENT; j++){
                    if (dirBlock->u.dir[j].d_name[0] != '\0'){ /// check if the filename is not null
                        if (dirBlock->u.dir[j].d_ino != 0) { /// check if the requested file doesn't have a inode with index 0 which means deleted.
                            out << "\t" << dirBlock->u.dir[j].d_ino << " '" << dirBlock->u.dir[j].d_name << "'" << endl;
                        }
                    }
                }
                dirBlock->release();
            }
        }
    }
    out << endl;
    inode->release();
}

void readWrites(dinode* dinode, ostream& out)
{
    out << "mode = " << oct << dinode->di_mode << dec << " (";
    if ((dinode->di_mode & X_IFMT) == X_IFDIR){
        out << "d";
    } else {
        out << "-";
    }
    out << (((dinode->di_mode & X_IFMT) == X_IFMT) ? "d" : "-");
    out << ((dinode->di_mode & X_ISUID) ? "u" : "-");
    out << ((dinode->di_mode & X_ISGID) ? "g" : "-");
    out << ((dinode->di_mode & X_ISVTX) ? "t" : "-");
    out << ((dinode->di_mode & X_IUREAD) ? "r" : "-");
    out << ((dinode->di_mode & X_IUWRITE) ? "w" : "-");
    out << ((dinode->di_mode & X_IUEXEC) ? "x" : "-");
    out << ((dinode->di_mode & X_IGREAD) ? "r" : "-");
    out << ((dinode->di_mode & X_IGWRITE) ? "w" : "-");
    out << ((dinode->di_mode & X_IGEXEC) ? "x" : "-");
    out << ((dinode->di_mode & X_IOREAD) ? "r" : "-");
    out << ((dinode->di_mode & X_IOWRITE) ? "w" : "-");
    out << ((dinode->di_mode & X_IOEXEC) ? "x" : "-");
    out << ")" << dec << endl;
}

void	dump( const char* floppie)
{
    ofstream out ("log.txt");
    if (out.is_open()){
        out << "       Opening device \'" << floppie << "\'" << endl;
        Device device(floppie);
        out << "|--------------------------------------------|" << endl;
        readSuperBlock(device, out);
        out << endl;
        out << "|--------------------------------------------|" << endl;
        readRootNode(device, out);
        out << endl;
        out << "|--------------------------------------------|" << endl;
    }
    out.close();
}

// ================================================================

// Main is just the TUI
int  main(int argc, const char* argv[])
{
	try {
// Note: To get your output into file 'log.txt', change the 0 below into a 1
#if	1
		std::cerr << "Sending output to log.txt" << std::endl;
		close(1);
		if( open("log.txt", O_WRONLY|O_TRUNC|O_CREAT, 0666) < 0)
			throw unix_error("log.txt");
#endif
		// Tell cout to add a 0 or 0x prefix when printing in non-decimal notation
		std::cout << std::showbase;

		// Pass the given parameter or use the default filename ?
        dump((argc > 1) ? argv[1] : "floppie.img");
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

