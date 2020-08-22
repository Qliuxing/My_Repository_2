/*
 * Copyright (C) 2011-2014 Melexis N.V.
 *
 * MelexCM Software Platform
 */
#ifndef FLASHFUNCTIONS_H_
#define FLASHFUNCTIONS_H_

#include "flash_cfg.h"

/*
 * API
 */
#define FLASH_ERR_NONE                      0
#define FLASH_ERR_VERIFICATION_FAILED       1

/* Init Flash driver */
extern void Flash_InitDriver(void);

/* Get Writing time of the page for specified address */
extern uint16_t Flash_GetWriteTime(uint16_t addr);

/* Read Flash Page (128 bytes) into internal RAM buffer */
extern void Flash_PageRead (uint16_t addr);

/* Write byte to Page RAM buffer */
extern void Flash_PageBufferFill (uint16_t offset, uint8_t data);

/* Write internal RAM buffer (128 bytes) to Flash Page at address 'addr' */
extern uint16_t Flash_PageWrite (uint16_t addr);

/* Wrapper for Flash_PageWrite which checks the input addresses to eliminate
 * writing requests to certain Flash pages (depending on the loader state)
 */
extern uint16_t Flash_PageWriteFiltered (uint16_t addr);

/* Change Flash threshold for MarginRead verification procedure */
extern uint16 Flash_IREF_Offset (int16_t offset_iref);

#endif /* FLASHFUNCTIONS_H_ */
