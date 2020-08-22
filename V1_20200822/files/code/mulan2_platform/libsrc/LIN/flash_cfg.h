/*
 * Copyright (C) 2011-2013 Melexis N.V.
 *
 * MelexCM Software Platform
 */
#ifndef FLASH_CFG_H_
#define FLASH_CFG_H_


/* ---- Flash constants ------------------------------------------ */
#define ML_FLASH_START_ADDRESS          0x4000
#define ML_FLASH_SIZE_IN_BYTES          (32 * 1024UL)

#define ML_FLASH_PAGE_SIZE_IN_BYTES     128

#define ML_FLASH_SECTOR_SIZE_IN_PAGES   16
#define ML_FLASH_SECTOR_SIZE_IN_BYTES   (ML_FLASH_SECTOR_SIZE_IN_PAGES * ML_FLASH_PAGE_SIZE_IN_BYTES)
#define ML_FLASH_NUMBER_OF_SECTORS      (ML_FLASH_SIZE_IN_BYTES / ML_FLASH_SECTOR_SIZE_IN_BYTES)

#define ML_FLASH_BUFFER_SIZE_IN_WORDS   (ML_FLASH_PAGE_SIZE_IN_BYTES / 2)
#define ML_FLASH_BUFFER_MASK            (ML_FLASH_PAGE_SIZE_IN_BYTES - 1)

#define ML_MCU_FAR_PAGE_0_ADDRESS       0xBF00  /* Far page 0 is the page containing the Reset Vector */
#define ML_APP_CONTROL_PAGE_ADDRESS     0xBE80

/*
 * Validate ML_FLASH_PAGE_SIZE_IN_BYTES
 */
#if (ML_FLASH_PAGE_SIZE_IN_BYTES < 4)
#error "ML_FLASH_PAGE_SIZE_IN_BYTES size must be at least 4 bytes (normally it's defined by HW and equal to 128 bytes)"
#endif

#if (ML_FLASH_PAGE_SIZE_IN_BYTES & (ML_FLASH_PAGE_SIZE_IN_BYTES - 1))
#error "ML_FLASH_PAGE_SIZE_IN_BYTES size must be a power of 2 (normally it's defined by HW and equal to 128 bytes)"
#endif

#endif /* FLASH_CFG_H_ */
