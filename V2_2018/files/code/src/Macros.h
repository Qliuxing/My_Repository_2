#ifndef MACROS_H_
#define MACROS_H_

#include "typelib.h"
#include <ioports.h>

#define M_MOV32(source, dest)	\
	__asm__ __volatile__ (		\
		"mov X, %[src]\n\t"		\
		"mov D, [X]\n\t"		\
		"mov X, %[dst]\n\t"		\
		"mov [X], D"			\
		:						\
		: [src] "ri" (source), [dst] "ri" (dest)		\
		: "X"					\
	)

#define M_DIVI16_I32BYI16(source, divisor)	\
	__asm__ __volatile__ (					\
		"mov YA, %[src]\n\t"				\
		"mov X, %[divis]\n\t"				\
		"divs YA, X\n\t"					\
		"divs YA, X\n\t"					\
		"dadjs YA, X"						\
		:									\
		: [src] = "ri" (source), [divis] "ri" (divisor)	\
		: "X", "Y"							\
	)

#endif /* MACROS_H_ */
