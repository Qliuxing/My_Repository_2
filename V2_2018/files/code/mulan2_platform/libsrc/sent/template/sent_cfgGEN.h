/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT Config
 *
 */

#ifndef SENT_CFG_GEN_H_
    #define SENT_CFG_GEN_H_

    /* -------------------------------- */
    /* SENT Configuration Type          */
    /* -------------------------------- */
    #define DEF_SENT_CFG_TYPE       0   /* Compressed Configuration */
    /*#define DEF_SENT_CFG_TYPE       1*/   /* Extended Configuration */


    /* -------------------------------- */
    /* IO Port SPC Config Register      */
    /* -------------------------------- */
    /*#define DEF_CONFIG_SPC_REGISTERS*/


    /* -------------------------------- */
    /* Use Fast Channel Pointers?       */
    /* -------------------------------- */
    /*#define DEF_USE_FC_CFG_FC0_FC1_EE_PTR*/


    /* -------------------------------- */
    /* If defined check function will   */
    /* use argument error pointer       */
    /* -------------------------------- */
    /*#define DEF_SENT_ERROR_POINTER*/


    /* -------------------------------- */
    /* SENT Fast Channel Configurations */
    /* -------------------------------- */
    #define DEF_SENT_FC_CFG_CH0_ONLY            (0U<<0U)    /* FC0 Only with Standard Output */
    #define DEF_SENT_FC_CFG_CH0_ONLY_4_3BIT     (1U<<0U)    /* FC0 Only with 4x3bit Nibbles */
    #define DEF_SENT_FC_CFG_CH0_CH1_STD         (2U<<0U)    /* FC0 and FC1 Pointer */
    #define DEF_SENT_FC_CFG_CH1_SECURE_CTR      (3U<<0U)    /* Secure Counter Setup */
    #define DEF_SENT_FC_CFG_CH1_INV             (4U<<0U)    /* FC1 uses Inverted Output */
    #define DEF_SENT_FC_CFG_CH1_REV_NIBBLES     (5U<<0U)    /* FC1 uses Reverse Nibbles Order */
    /*#define DEF_SENT_FC_CFG_CH1_REV_INV_NIBBL   (6U<<0U)*/    /* FC1 uses Inverted and Reverse Nibbles Order */
    #define DEF_SENT_FC_CFG_FC0_FC1_EE_PTR      (6U<<0U)    /* FC0 and FC1 are using a pointer stored in EEPROM */

#endif /*SENT_CFG_H_ */
