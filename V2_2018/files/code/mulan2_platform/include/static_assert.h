/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef STATIC_ASSERT_
#define STATIC_ASSERT_

/*
 * Static (compile time) assertion
 *
 * \note
 * If assert condition fails will try to create a dummy array
 * with a negative size which will be caught by compiler
 *
 */
#define ASSERT(condition)   \
    extern char assertion_failed[(condition) ? 1 : -1]

#endif /* STATIC_ASSERT_ */
