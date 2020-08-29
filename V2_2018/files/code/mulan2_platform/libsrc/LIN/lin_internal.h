/*
 * Copyright (C) 2014 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef LIN_INTERNAL_H_
#define LIN_INTERNAL_H_

#include <typelib.h>

#pragma space dp  /* shared RAM is in dp area on MULAN2 */

/*
 * MLX4/MLX16 shared RAM interface
 */
extern volatile uint16_t LinResp;
extern volatile uint16_t LinCmnd;
extern volatile uint16_t LinMess;
extern volatile uint8_t  LinMess2;
extern volatile uint8_t  LinProtectedID;
extern volatile uint8_t  LinFrame[8];

#pragma space none

#endif /* LIN_INTERNAL_H_ */
