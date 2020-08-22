/*
 * Copyright (C) 2012 Melexis N.V.
 *
 * Generic SENT API Defines
 *
 */

#ifndef SENT_DEFS_H_
    #define SENT_DEFS_H_

    /* -------------------------------- */
    /* SENT Hardware Configurations     */
    /* -------------------------------- */
    /*
     * Tick_Time = (Tick_Divider+1)*Tosc
     *
     * Tick_Divider = (Tick_Time / Tosc) - 1
     *
     * Tosc = 1/FPLL
     */
    #define SENT_TICK(tick)                     ((((tick*FPLL)/1000) - 1) & 0x3FFF)
    /* Frame_Len = reg + 1 */
    #define SENT_FRAME_LEN(fl)                  (fl>0?((fl-1) & 0x0FFF):0)


    /* -------------------------------- */
    /* SENT Slow Channel Message Config */
    /* -------------------------------- */
    #define DEF_DTA_CFG                         0U
    #define DEF_ID_CFG                          1U
    #define DEF_EN_CFG                          2U

    #define DEF_SCID_LOC_EE                     (0U << 6U)
    #define DEF_SCID_LOC_ROM                    (2U << 6U)
    #define DEF_SCID_LOC_IN_DTA                 (1U << 6U)
    #define DEF_SCID_LOC_EE_CMPR                (3U << 6U)

    #define DEF_SCDTA_LOC_EE                    (0U << 6U)
    #define DEF_SCDTA_LOC_EE_4B                 (1U << 6U)
    #define DEF_SCDTA_LOC_ROM                   (2U << 6U)
    #define DEF_SCDTA_LOC_EE_CMPR               (3U << 6U)

    #define DEF_SCEN_STANDARD                   (0U << 6U)
    #define DEF_SCEN_DEFAULT_ON                 (1U << 6U)
    #define DEF_SCEN_FAST_REP                   (2U << 6U)


    /* -------------------------------- */
    /* Slow Channel Message Data Full   */
    /* -------------------------------- */
    #define DEF_8BIT_SC                         0U
    #define DEF_4BIT_SC                         1U
    #define DEF_DTA_DATA                        0U
    #define DEF_DTA_PTR                         1U
    #define DEF_DTA_NO_SHIFT                    0U
    #define DEF_DTA_SHIFT                       1U

    #define DEF_SCDTA_8BIT_ID                   (DEF_8BIT_SC << 15U)
    #define DEF_SCDTA_4BIT_ID                   (DEF_4BIT_SC << 15U)
    #define DEF_SCDTA_DATA                      (DEF_DTA_DATA << 14U)
    #define DEF_SCDTA_PTR                       (DEF_DTA_PTR << 14U)
    #define DEF_SCDTA_NO_SHIFT                  (DEF_DTA_NO_SHIFT << 13U)
    #define DEF_SCDTA_SHIFT                     (DEF_DTA_SHIFT << 13U)

#endif /* SENT_API_DEFS_H_ */
