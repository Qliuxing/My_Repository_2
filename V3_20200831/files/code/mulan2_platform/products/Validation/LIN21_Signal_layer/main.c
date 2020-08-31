/*
 * LIN2.1 Signal layer test application
 * 
 * Test frames are defined in busMaster.lin (LIN Commander script) and
 * shall be sent using LIN Commander/USB LIN Master.
 *
 * Copyright (C) 2007-2014 Melexis N.V.
 */

#include "lin_api.h"
#include <plib.h>       /* product libs */

#include "tmr.h"

/*
 * Application version
 */
#define __APP_VERSION_MAJOR__      (1UL)
#define __APP_VERSION_MINOR__      (0UL)
#define __APP_VERSION_REVISION__   (3UL)

const uint32 application_version __attribute__((used, section(".app_version"))) =
    (__APP_VERSION_MAJOR__) |
    (__APP_VERSION_MINOR__ << 8) |
    (__APP_VERSION_REVISION__ << 16);


/*
 * Serial Number definition
 */
#define ML_SERIAL_NUMBER  (0xFEEDBEEFul)      /* serial number */

/*
 *****************************************************************************
 *  Main
 *****************************************************************************
 */
int main (void)
{
    l_bool tmp_bool;
    l_u8   tmp_u8;
    l_u16  tmp_u16;
    l_u8 test_arr[8];

    tmr_init();

    l_sys_init();
    l_ifc_init_i1();

    for (;;) {

        WDG_Manager();     /* Restart watchdog */

        /*
         * Test case for l_ifc_read_status_i1 API function
         */
        tmp_u16 = l_ifc_read_status_i1();
        if (tmp_u16 != 0) {
            l_u16_wr_Status(tmp_u16);
        }
        
        /* Copy signal i1 to signal o1 (1 bit)
         * 
         * RD (lsb first):
         * 1 00 000 00.00 00000 0.00000 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0x01        0x00       0x00      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 00000.0 00000 00.00 000 00 1
         * 0xFF     0x01      0x00      0x00      0x00      0x00      0x00       0x80
         *
         */
        if (l_flg_tst_i1()) {
            l_flg_clr_i1();

            tmp_bool = l_bool_rd_i1();
            l_bool_wr_o1(tmp_bool);
        }

        /* Copy signal i2 to signal o2 (2 bits)
         * 
         * RD (lsb first):
         * 0 11 000 00.00 00000 0.00000 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0x06        0x00       0x00      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 00000.0 00000 00.00 000 11 0
         * 0xFF     0x01      0x00      0x00      0x00      0x00      0x00       0x60
         *
         */
        if (l_flg_tst_i2()) {
            l_flg_clr_i2();

            tmp_u8 = l_u8_rd_i2();
            l_u8_wr_o2(tmp_u8);
        }

        /* Copy signal i3 to signal o3 (3 bits)
         * 
         * RD (lsb first):
         * 0 00 111 00.00 00000 0.00000 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0x38        0x00       0x00      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 00000.0 00000 00.00 111 00 0
         * 0xFF     0x01      0x00      0x00      0x00      0x00      0x00       0x1C
         *
         */
        if (l_flg_tst_i3()) {
            l_flg_clr_i3();

            tmp_u8 = l_u8_rd_i3();
            l_u8_wr_o3(tmp_u8);
        }

        /* Copy signal i4 to signal o4 (4 bits)
         * 
         * RD (lsb first):
         * 0 00 000 11.11 00000 0.00000 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0xC0        0x03       0x00      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 00000.0 00000 11.11 000 00 0
         * 0xFF     0x01      0x00      0x00      0x00      0x00      0xC0       0x03
         *
         */
        if (l_flg_tst_i4()) {
            l_flg_clr_i4();

            tmp_u8 = l_u8_rd_i4();
            l_u8_wr_o4(tmp_u8);
        }

        /* Copy signal i5 to signal o5 (5 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 11111 0.00000 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0x00        0x7C       0x00      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 00000.0 11111 00.00 000 00 0
         * 0xFF     0x01      0x00      0x00      0x00      0x00      0x3E       0x00
         *
         */
        if (l_flg_tst_i5()) {
            l_flg_clr_i5();

            tmp_u8 = l_u8_rd_i5();
            l_u8_wr_o5(tmp_u8);
        }

        /* Copy signal i6 to signal o6 (6 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 00000 1.11111 000.0000 0000.0000 0000.00000 000.0000000 -.--------
         * 0x00        0x80       0x1F      0x00      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 0000.000 11111.1 00000 00.00 000 00 0
         * 0xFF     0x01      0x00      0x00      0x00      0xF8      0x01       0x00
         *
         */
        if (l_flg_tst_i6()) {
            l_flg_clr_i6();

            tmp_u8 = l_u8_rd_i6();
            l_u8_wr_o6(tmp_u8);
        }

        /* Copy signal i7 to signal o7 (7 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 00000 0.00000 111.1111 0000.0000 0000.00000 000.0000000 -.--------
         * 0x00        0x00       0xE0      0x0F      0x00      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 0000.0000 1111.111 00000.0 00000 00.00 000 00 0
         * 0xFF     0x01      0x00      0x00      0xF0      0x07      0x00       0x00
         *
         */
        if (l_flg_tst_i7()) {
            l_flg_clr_i7();

            tmp_u8 = l_u8_rd_i7();
            l_u8_wr_o7(tmp_u8);
        }

        /* Copy signal i8 to signal o8 (8 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 00000 0.00000 000.0000 1111.1111 0000.00000 000.0000000 -.--------
         * 0x00        0x00       0x00      0xF0      0x0F      0x00      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 00000.0000 1111.1111 0000.000 00000.0 00000 00.00 000 00 0
         * 0xFF     0x01      0x00      0xF0      0x0F      0x00      0x00       0x00
         *
         */
        if (l_flg_tst_i8()) {
            l_flg_clr_i8();

            tmp_u8 = l_u8_rd_i8();
            l_u8_wr_o8(tmp_u8);
        }

        /* Copy signal i9 to signal o9 (9 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 00000 0.00000 000.0000 0000.0000 1111.11111 000.0000000 -.--------
         * 0x00        0x00       0x00      0x00      0xF0      0x1F      0x80      0xFF
         *
         * WR (lsb first):
         * --------.- 0000000.000 11111.1111 0000.0000 0000.000 00000.0 00000 00.00 000 00 0
         * 0xFF     0x01      0xF8      0x0F      0x00      0x00      0x00       0x00
         *
         */
        if (l_flg_tst_i9()) {
            l_flg_clr_i9();

            tmp_u16 = l_u16_rd_i9();
            l_u16_wr_o9(tmp_u16);
        }

        /* Copy signal i10 to signal o10 (10 bits)
         * 
         * RD (lsb first):
         * 0 00 000 00.00 00000 0.00000 000.0000 0000.0000 0000.00000 111.1111111 -.--------
         * 0x00        0x00       0x00      0x00      0x00      0xE0      0xFF      0xFF
         *
         * WR (lsb first):
         * --------.- 1111111.111 00000.0000 0000.0000 0000.000 00000.0 00000 00.00 000 00 0
         * 0xFF     0xFF      0x07      0x00      0x00      0x00      0x00       0x00
         *
         */
        if (l_flg_tst_i10()) {
            l_flg_clr_i10();

            tmp_u16 = l_u16_rd_i10();
            l_u16_wr_o10(tmp_u16);
        }

        /* Copy signal i11 to signal o11 (11 bits)
         * 
         * RD (lsb first):
         * 11111111.111 00000.0000000 0.00000000.0000 0000.00000000.00 ------.--------
         * 0xFF     0x07      0x00      0x00     0x00      0x00     0xFC      0xFF
         *
         * WR (lsb first):
         * --------.------ 00.00000000.0000 0000.00000000.0 0000000.00000 111.11111111
         * 0xFF     0x3F      0x00     0x00      0x00       0x00    0xE0      0xFF
         *
         */
        if (l_flg_tst_i11()) {
            l_flg_clr_i11();

            tmp_u16 = l_u16_rd_i11();
            l_u16_wr_o11(tmp_u16);
        }

        /* Copy signal i12 to signal o12 (12 bits)
         * 
         * RD (lsb first):
         * 00000000.000 11111.1111111 0.00000000.0000 0000.00000000.00 ------.--------
         * 0x00     0xF8      0x7F      0x00     0x00      0x00     0xFC      0xFF
         *
         * WR (lsb first):
         * --------.------ 00.00000000.0000 0000.00000000.0 1111111.11111 000.00000000
         * 0xFF     0x3F      0x00     0x00      0x00     0xFE      0x1F      0x00
         *
         */
        if (l_flg_tst_i12()) {
            l_flg_clr_i12();

            tmp_u16 = l_u16_rd_i12();
            l_u16_wr_o12(tmp_u16);
        }

        /* Copy signal i13 to signal o13 (13 bits)
         * 
         * RD (lsb first):
         * 00000000.000 00000.0000000 1.11111111.1111 0000.00000000.00 ------.--------
         * 0x00     0x00      0x80      0xFF     0x0F      0x00     0xFC      0xFF
         *
         * WR (lsb first):
         * --------.------ 00.00000000.0000  1111.11111111.1 0000000.00000 000.00000000
         * 0xFF     0x3F      0x00     0xF0       0xFF     0x01      0x00      0x00
         *
         */
        if (l_flg_tst_i13()) {
            l_flg_clr_i13();

            tmp_u16 = l_u16_rd_i13();
            l_u16_wr_o13(tmp_u16);
        }

        /* Copy signal i14 to signal o14 (14 bits)
         * 
         * RD (lsb first):
         * 00000000.000 00000.0000000 0.00000000.0000 1111.11111111.11 ------.--------
         * 0x00     0x00      0x00      0x00     0xF0      0xFF     0xFF      0xFF
         *
         * WR (lsb first):
         * --------.------ 11.11111111.1111 0000.00000000.0 0000000.00000 000.00000000
         * 0xFF     0xFF      0xFF     0x0F      0x00     0x00      0x00      0x00
         *
         */
        if (l_flg_tst_i14()) {
            l_flg_clr_i14();

            tmp_u16 = l_u16_rd_i14();
            l_u16_wr_o14(tmp_u16);
        }

        /* Copy signal i15 to signal o15 (15 bits)
         * 
         * RD (lsb first):
         * 11111111.1111111 0.00000000.0000000 -.--------.--------.--------.--------
         * 0xFF     0x7F      0x00     0x80      0xFF     0xFF     0xFF     0xFF
         *
         * WR (lsb first):
         * --------.--------.--------.--------.- 0000000.00000000.0 1111111.11111111
         * 0xFF     0xFF     0xFF     0xFF     0x01      0x00     0xFE      0xFF
         *
         */
        if (l_flg_tst_i15()) {
            l_flg_clr_i15();

            tmp_u16 = l_u16_rd_i15();
            l_u16_wr_o15(tmp_u16);
        }

        /* Copy signal i16 to signal o16 (16 bits)
         * 
         * RD (lsb first):
         * 00000000.0000000 1.11111111.1111111 -.--------.--------.--------.--------
         * 0x00     0x80      0xFF     0xFF      0xFF     0xFF     0xFF     0xFF
         *
         * WR (lsb first):
         * --------.--------.--------.--------.- 1111111.11111111.1 0000000.00000000
         * 0xFF     0xFF     0xFF     0xFF     0xFF      0xFF     0x01      0x00
         *
         */
        if (l_flg_tst_i16()) {
            l_flg_clr_i16();

            tmp_u16 = l_u16_rd_i16();
            l_u16_wr_o16(tmp_u16);
        }

        /*
         * Test case for Byte Arrays: l_bytes_rd_sss / l_bytes_wr_sss
         */
        if (l_flg_tst_frm_byte_array_in()) {
            l_flg_clr_frm_byte_array_in();

            switch (l_u8_rd_Cmd()) {
                /*
                 *  Copy byte array "as is"
                 */
                case 0x00:
                     l_bytes_rd_MstArray(0, 7, test_arr);
                     l_bytes_wr_SlvArray(0, 7, test_arr);
                     break;

                /*
                 *  Reverse byte array
                 */
                case 0x01:
                     l_bytes_rd_MstArray(0, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(6, 1, &tmp_u8);

                     l_bytes_rd_MstArray(1, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(5, 1, &tmp_u8);

                     l_bytes_rd_MstArray(2, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(4, 1, &tmp_u8);

                     l_bytes_rd_MstArray(3, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(3, 1, &tmp_u8);

                     l_bytes_rd_MstArray(4, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(2, 1, &tmp_u8);

                     l_bytes_rd_MstArray(5, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(1, 1, &tmp_u8);

                     l_bytes_rd_MstArray(6, 1, &tmp_u8);
                     l_bytes_wr_SlvArray(0, 1, &tmp_u8);

                     break;

                /*
                 *  Swap two parts
                 */
                case 0x02:
                     l_bytes_rd_MstArray(0, 3, test_arr);
                     l_bytes_wr_SlvArray(4, 3, test_arr);

                     l_bytes_rd_MstArray(3, 4, test_arr);
                     l_bytes_wr_SlvArray(0, 4, test_arr);
                     break;

                default:
                    break;
            }  /* !switch */
        }

        /*
         * Test case for flags of write signals
         * Flag is set by LIN core when signal is copied for transmission
         */
        if (l_flg_tst_Cnt()) {
            static l_u8 tx_cntr;
            
            l_flg_clr_Cnt();

            l_u8_wr_Cnt(++tx_cntr);

        }
    }
    return 0;
}


/*
 *****************************************************************************
 *  ld_serial_number_callout
 *
 * Call out from LIN driver to get Serial Number of the device
 *****************************************************************************
 */
void ld_serial_number_callout (l_u8 data[4])
{
    data[0] = (l_u8)ML_SERIAL_NUMBER;           /* LSB */
    data[1] = (l_u8)(ML_SERIAL_NUMBER >> 8);
    data[2] = (l_u8)(ML_SERIAL_NUMBER >> 16);
    data[3] = (l_u8)(ML_SERIAL_NUMBER >> 24);   /* MSB */
}

#if defined (HAS_READ_BY_ID_CALLOUT)
/*
 *****************************************************************************
 *  ld_read_by_id_callout
 *
 *  Call out from LIN driver to serve user defined read_by_ID requests
 *  Return values:
 *      LD_NEGATIVE_RESPONSE
 *      LD_POSITIVE_RESPONSE
 *      LD_NO_RESPONSE
 *
 *****************************************************************************
 */
l_u8 ld_read_by_id_callout (l_u8 id, l_u8* data)
{
    /* 
     * id values are in range from 0x20 to 0x3F (32 to 63),
     * their meanings are application dependent
     */
    if (id == 0x20)
    {
        data[0] = 0xAA;
        data[1] = 0xBB;
        data[2] = 0xCC;
        data[3] = 0xDD;
        data[4] = 0xEE;
        return LD_POSITIVE_RESPONSE;
    }
    else
    {
        return LD_NEGATIVE_RESPONSE;
    }
}
#endif /* HAS_READ_BY_ID_CALLOUT */

/*
 *****************************************************************************
 *  LIN API event: mlu_ApplicationStop
 *****************************************************************************
 */
ml_Status mlu_ApplicationStop(void)
{
    return ML_SUCCESS;  /* return that the application has stopped */
}

/* EOF */
