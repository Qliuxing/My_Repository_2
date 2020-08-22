/*
 * Copyright (C) 2009-2012 Melexis N.V.
 *
 * Software Platform
 *
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#if !defined EEPROM_USE_ECC
#define EEPROM_USE_ECC  0
#endif

extern int16  EEPROMWrite (uint16 *address, uint16 data);
extern uint16 EEPROMRead (uint16 *address);

#endif /* EEPROM_H_ */
