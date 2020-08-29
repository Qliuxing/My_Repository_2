/*
 * Copyright (C) 2009 Melexis N.V.
 *
 * Software Platform
 *
 */

#include <ioports.h>
#include <syslib.h>
#include <eeprom.h>

/* Linker symbols */
extern uint16 _eeprom_start;    /* symbol _eeprom_start should be defined in the linker script  */
extern uint16 _eeprom_end;      /* symbol _eeprom_end should be defined in the linker script    */

#define EEPROM_START    &_eeprom_start
#define EEPROM_END      &_eeprom_end

/* EEPROMWrite
 * Write word data at address in EEPROM
 * Address is a MLX16 memory word address
 *     Address should be a valid EEPROM address
 *     
 *     Returns 0 on success
 *     -1 if invalid address
 *     -2 if data not written correctly (erase or write failure)
 *     -3 if read after write resulted in a hamming code error (if available)
 *
 *  Description:
 *     First checks address validity and any ongoing EEPROM access
 *     EEPROM address is first erased before re-written
 *     Verifies written data
 *     Uses polling to wait for EEPROM access completion
 */
int16 EEPROMWrite (uint16 *address, uint16 data)
{

  /* check address validity */
  if (   (address < EEPROM_START)
      || (address >= EEPROM_END)) { /* ! pointer arithmetic */
    return -1;
  }

  /* check if other EEPROM access ongoing */
  /* alternatively could return with some error */
  /* should be call from SW Platform library */
  while ((EEPROM & EE_BUSY) == EE_BUSY) {
    ; /* polling */
  }

  /* write enable EEPROM */
  CONTROL |= EE_WE; /* should be call from SW Platform library */

  /* erase EEPROM word */
  EEPROM = EE_CTL_ERASE;
  *address = 0; /* actual erase */

  /* should be call from SW Platform library */
  /* wait till erase finished */
  while ((EEPROM & EE_BUSY) == EE_BUSY) {
    ; /* polling */
  }

  /* read verify */
  if (*address != 0 /* whatever erase value is */) {
    /* write disable EEPROM */
    CONTROL &= ~EE_WE; /* should be call from SW Platform library */
    return -2;
  }

  /* write EEPROM word */
  EEPROM = EE_CTL_WRITE;
  *address = data; /* actual write */

  /* should be call from SW Platform library */
  /* wait till write finished */
  while ((EEPROM & EE_BUSY) == EE_BUSY) {
    ; /* polling */
  }

  /* write disable EEPROM */
  CONTROL &= ~EE_WE; /* should be call from SW Platform library */

  /* there is no safe value for EE_CTL, e.g. read */

  /* read verify */
  if (data != *address) {
    return -2;
  }


#if EEPROM_USE_ECC == 1
  /* check ECC */
  if ((VARIOUS & EENV_SEC) != 0) { /* should be call from SW Platform library */
    return -3;
  }

  /* no check for double error detection
     -> read-verify error
  */
#endif /* EEPROM_USE_ECC */

  return 0;
}

/*
 * Read word from specified EEPROM address
 */
uint16 EEPROMRead (uint16 *address)
{
    return *address;
}

/* EOF */
