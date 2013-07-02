/** 
 * @file
 * @brief Headers of useful bitlevel operations.
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
 * contact [Professor Hank Dietz](http://aggregate.org/hankd/). * 
 */

#ifndef BITSTUFF_H
#define BITSTUFF_H

/** @brief Returns the next larger power of 2 of the given value. */
unsigned int nlpo2(register unsigned int x);

/** @brief Returns the number of one bits in the given value. */
unsigned int ones32(register unsigned int x);

/** @brief Returns the trailing Zero Count (i.e. the log2 of a base 2 number). */
unsigned int tzc(register int x);

/** @brief Returns the series (sum of sequence of 0 to) x. */
unsigned int sum(unsigned int x);

#endif
