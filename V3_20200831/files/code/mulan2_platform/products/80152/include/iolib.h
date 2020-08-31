/*
 * Copyright (C) 2010-2012 Melexis N.V.
 *
 * Software Platform
 *
 */
 
#ifndef IOLIB_H_
#define IOLIB_H_

#include <ioports.h>

/* macro to toggle an IO */
#define IO_TOGGLE(m) \
do  { \
    EXTIO_OPEN_DRAIN ^= (1 << (m) ); \
} while(0)

/* macro to set an IO */
#define IO_SET(m) \
do  { \
    EXTIO_OPEN_DRAIN |= (1 << (m) ); \
} while(0)

/* macro to reset an IO */
#define IO_RESET(m) \
do  { \
    EXTIO_OPEN_DRAIN &= ~(1 << (m) ); \
} while(0)


#endif /* IOLIB_H_ */
