/** 
 * @file
 * @brief Implementation of useful bitlevel operations.
 * 
 * [The Aggregate Magic Algorithms](http://aggregate.org/MAGIC/)
 * 
 * @author Henry Gordon Dietz
 * @copyright None of the following coding tricks came from proprietary 
 * sources; further, we believe that each of the tricks we did not invent 
 * is essentially "standard engineering practice" in the specialized 
 * niche where it applies. Thus, although we have not conducted patent 
 * searches, etc., to confirm it, we believe that these are tricks that 
 * freely can be used for any purpose. Of course, The Aggregate accepts
 * no responsibility for your use of these tricks; you must confirm that 
 * the trick does what you want and that you can use it as you intend. 
 * That said, we do intend to maintain this page by adding new algorithms 
 * and/or correcting existing entries. If you have any comments, please 
 * contact [Professor Hank Dietz](http://aggregate.org/hankd/).
 */
#include "bitstuff.h"

/** 
 * @brief Returns the next larger power of 2 of the given value.
 * 
 * @param x Value of which to get the next larger power of 2.
 * @return The next larger power of 2 of x.
 */
unsigned int nlpo2(register unsigned int x)
{
	/* If value already is power of 2, return it has is. */
	if ((x & (x - 1)) == 0) return x;
	/* If not, determine next larger power of 2. */
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return(x + 1);
}

/** 
 * @brief Returns the number of one bits in the given value.
 * 
 * @param x Value of which to get the number of one bits.
 * @return The number of one bits in x.
 */
unsigned int ones32(register unsigned int x)
{
	/* 32-bit recursive reduction using SWAR...
	but first step is mapping 2-bit values
	into sum of 2 1-bit values in sneaky way */
	x -= ((x >> 1) & 0x55555555);
	x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
	x = (((x >> 4) + x) & 0x0f0f0f0f);
	x += (x >> 8);
	x += (x >> 16);
	return(x & 0x0000003f);
}

/** 
 * @brief Returns the trailing Zero Count (i.e. the log2 of a base 2 number).
 * 
 * @param x Value of which to get the trailing Zero Count.
 * @return The trailing Zero Count in x.
 */
unsigned int tzc(register int x)
{
	return(ones32((x & -x) - 1));
}

/** 
 * @brief Returns the series (sum of sequence of 0 to) x.
 * 
 * @param x Series limit.
 * @return The series (sum of sequence of 0 to) x.
 */
unsigned int sum(unsigned int x)
{
	return(x == 0 ? x : sum(x - 1) + x);
}
