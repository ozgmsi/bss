/** @file cstr.cc
 * The implementation of the cstr output operator.
 * @version 1.1	2015/02/08
 */
#include "cstr.h"

std::ostream& operator<<(std::ostream& os, const cstr& c)
{
	const char*	cp  = c.cp;
	unsigned	len = c.len;
	// while not maxlen reached and non null character
	for( ; (len>0) && (*cp) ; --len, ++cp) {
		os << *cp;
	}
	return os;
}
