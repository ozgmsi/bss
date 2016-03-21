#pragma once
#ifndef E7TYPES_H
#define E7TYPES_H 5.1

#include <cstdint>	// for: fixed width integer types

#ifndef	STATIC_ASSERT
# if __cplusplus >= 201103
#  define	STATIC_ASSERT(x) static_assert(x,#x)
# else
#  define	STATIC_ASSERT(x)
# endif
#endif

/* @(#)e7types.h	5.1 2015/11/27 */

/** @file
 * Supplies the platform dependent mappings from logical to physical types.
 * You may have to alter these if you have a different computer or compiler.
 * The comments tell the size expected, but keep in mind that different
 * types may have the same size so they could still be non-compatible.
 */

typedef unsigned char		byte;	/*!< 1 byte : raw data byte type */
STATIC_ASSERT(sizeof(byte)==1);	// verify size

// Normally (by convention) the names of logical types (e.g. size_t)
// end with _t. However in this assignment such names would clash with
// the logical types already defined for your native platform.
// For this reason we use our own _x names instead.
typedef uint16_t	size_x;    	/*!< 2 byte : a plain size */
STATIC_ASSERT(sizeof(size_x)==2);	// verify size
typedef uint16_t	ino_x;     	/*!< 2 byte : an i-node number */
STATIC_ASSERT(sizeof(ino_x)==2);	// verify size
typedef uint16_t	imode_x;	/*!< 2 byte : the type of the i_mode field */
STATIC_ASSERT(sizeof(imode_x)==2);	// verify size
typedef uint16_t	id_x;		/*!< 2 byte : an identification type (e.g. uid,gid) */
STATIC_ASSERT(sizeof(id_x)==2);	// verify size
typedef uint16_t	dev_x;    	/*!< 2 byte : a major:minor tuple */
STATIC_ASSERT(sizeof(dev_x)==2);	// verify size
typedef int32_t		off_x;    	/*!< 4 byte : an offset in a file */
STATIC_ASSERT(sizeof(off_x)==4);	// verify size
typedef int32_t		daddr_x;  	/*!< 4 byte : a disk address aka blocknumber */
STATIC_ASSERT(sizeof(daddr_x)==4);	// verify size
		/* NOTE:
		 *   In an inode_on_disk these daddr_x numbers are encoded as 24 bit quantities.
		 *	 Anywhere else they are ordinary 32 bit daddr_x entities.
		 */

typedef long		time_x;   	/*!< 4 byte : a timestamp in seconds since 1970 */
STATIC_ASSERT(sizeof(time_x)==4);	// verify size
//   Note 1:
// When on your system 'long' is 8 bytes, try using 'typedef int time_x;'
//   Note 2:
// When the definition for time_t on your system is not compatible
// with the definition for time_x here, you can not directly use the
// ctime(...) function to convert a timestamp to human readable text.
// In that case you have to use a two-step approach:
//  1: assign the time_x value to a time_t variable,
//  2: use ctime() to convert that time_t value to text.

#endif /*E7TYPES_H*/
