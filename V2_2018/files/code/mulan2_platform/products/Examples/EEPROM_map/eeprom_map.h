/*
 * Copyright (C) 2012 Melexis N.V.
 *
 */

#ifndef EEPROM_MAP_H_
#define EEPROM_MAP_H_



/*
 * \notes
 *  1. Attribute #packed is used for precise control of the struct layout.
 *  2. Attribute #aligned(2) is used to hint the compiler on structure
 *     alignment. Otherwise, byte access is used by compiler for every
 *     access to the structure, since compiler assumes that packed structure
 *     could non-aligned.
 */
typedef struct __attribute__((packed, aligned(2))) {

    uint16_t patch0_a; 
    uint16_t patch0_i; 

    uint8_t crc8;

    uint16_t rom_checksum;

    struct  __attribute__((packed)) {	/* substructure shall be also packed */
		/* lsbit first */
        uint8_t left : 3;
        uint8_t right : 3;
        uint8_t enable : 1;
		uint8_t : 1;
    } direction;

} eeprom_t;


extern eeprom_t ee __attribute__((ep));


#endif /* EEPROM_MAP_H_ */
