/*
 * Copyright (C) 2009-2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef MARK_H_
#define MARK_H_

/* For markv and markcd, address is :
 * Mulan/Mulan2 : io:0x0C
 * MelexCM : io:0x20
 */


/* mark with register as an argument (1 byte), cst is a word in which
 * only the upper byte is used. The MSB of cst has to be set
 */
#define markv(id, cst, value)                               \
    do {                                                    \
        __asm__ __volatile__ ("movx #" #id ", #" #cst ::);  \
        MC_MARK = value;                                    \
	} while(0)

/* variant to send a constant instead of a value in memory */
#define markcd(id, cst_low, cst_high)                           \
    do {                                                        \
        __asm__ __volatile__ ("movx #" #id ", #" #cst_low ::);  \
        MC_MARK = cst_high;                                     \
	} while(0)

/* single mark with a constant, cst is a word in which only
 * the upper byte is used. The MSB of cst has to be zero
 */
#define markc(id, cst)   \
    __asm__ __volatile__ ("movx #" #id ", #" #cst :: )

#endif /* MARK_H_ */
