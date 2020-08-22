/*
 * Copyright (C) 2011-2012 Melexis N.V.
 *
 * MelexCM Software Platform
 */
#ifndef FLASHUPLOAD_CFG_H_
#define FLASHUPLOAD_CFG_H_

/*
 * Pre-compile time configuration of the loader
 */

/* ----------------------------------------------------------------------------
 * Select flash write test (FLASH_TEST_NONE, FLASH_TEST_NORMAL
 * or FLASH_TEST_MARGIN)
 *
 */
#define FLASH_TEST_NONE         0
#define FLASH_TEST_NORMAL       1
#define FLASH_TEST_MARGIN       2

#define LDR_FLASH_WRITE_TEST    FLASH_TEST_NORMAL

#if (LDR_FLASH_WRITE_TEST == FLASH_TEST_MARGIN)
#error "Margin read SHALL NOT be used for the final release!"
#endif

/* ----------------------------------------------------------------------------
 * EEPROM operations if EEPROM is enabled
 */
#ifndef LDR_HAS_EEPROM_COMMANDS         /* if not externally configured .. */
#define LDR_HAS_EEPROM_COMMANDS  1
#endif


#endif /* FLASHUPLOAD_CFG_H_ */
