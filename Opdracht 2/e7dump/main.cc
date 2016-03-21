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
#include <ctime>		// for: ctime()
#include <cstdlib>		// for: EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>		// for: close() etc
#include <fcntl.h>		// for: O_WRONLY etc

// Our own classes
#include "Device.h"		// the "device driver"
#include "Block.h"		// the "data blocks"


// ================================================================

// TODO: write all the functions etc you need for this assignment
void	dump( const char* floppie )
{
}

// ================================================================

// Main is just the TUI
int  main(int argc, const char* argv[])
{
	try {
// Note: To get your output into file 'log.txt', change the 0 below into a 1
#if	0
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

