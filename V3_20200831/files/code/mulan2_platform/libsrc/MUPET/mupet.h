/*
 * Copyright (C) 2013 Melexis N.V.
 *
 * MUPeT, Application Layer
 *
 */

#ifndef MUPET_H_
#define MUPET_H_

#include "typelib.h" /* uint16, .. */

/* SW component version */
#define MUPET_SW_MAJOR_VERSION      1u
#define MUPET_SW_MINOR_VERSION      0u
#define MUPET_SW_PATCH_VERSION      1u  /* Don't use in current implementation */

typedef uint32  MUPET_SessionId_t;

/*-----------------------------------------------------------------------------
 *  MUPeT initialization function
 *
 *  Notes:
 *     - only reset internal variables,
 *     - DON'T enable/disable any interrupt;
 */
extern void MUPET_Init (void);

/*-----------------------------------------------------------------------------
 *  MUPeT function is opened session
 *
 *  Notes:
 *
 *    Result: 0 - closed, !0 - opened.
 */
extern uint16  MUPET_IsOpenedSession (void);

/*-----------------------------------------------------------------------------
 *  MUPeT function is freeze application
 *
 *  Notes:
 *
 *    Result: 0 - noFreeze, !0 - freeze.
 */
extern uint16  MUPET_IsAppliFreeze (void);


/*-----------------------------------------------------------------------------
 *  MUPeT function gets current session ID
 *
 *  Notes:
 *
 *    Result: SessionID
 */
extern MUPET_SessionId_t  MUPET_GetSessionID (void);



#endif /* #ifndef MUPET_H_ */

/* EOF */

