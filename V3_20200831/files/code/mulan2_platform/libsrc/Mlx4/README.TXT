
                        MLX4 LIN firmware

-------------------------------------------------------------------------------
    Build
-------------------------------------------------------------------------------
To build all MLX4 images execute build_mlx4_fw_images.bat (on Windows)
or build_mlx4_fw_images.sh (on Linux)

Based on version:
http://cvs.mucontrol.elex.be/cgi-bin/cvsweb/cvsmucontrol/Projects/lin/platform/libsrc/Mlx4/


-------------------------------------------------------------------------------
    Configuration notes
-------------------------------------------------------------------------------

Sleep timeout calculation
-------------------------

    Sleep_TO = (1/Fosc) * (2 ^ (9+SleepPre_Val)) * (16+SleepCnt_Val) * (256/SleepCntInc)
    Fosc = RCO / 4 = (or XTAL / 16) = 250 KHz

    Sleep TO : Sleep timeout as per LIN specification
    Sleep TO + 5% : Value which should be set to get at least standard "Sleep TO" in case of Fosc+5%


    Baudrate                    Sleep TO,   (Sleep TO + 5%),  SleepPre_Val   SleepCnt_Val  SleepCntInc   MLX4_FW_TO
                                sec         sec               (max 7)        (max 15)      (max 15)      sec
    ---------------------------------------------------------------------------------------------------------------
    LIN 2.x / all baudrates      4.00	     4.20             0               0            2              4.19
    LIN 1.3 / 1 000 bps         25.00       26.25             1               9            1             26.21
    LIN 1.3 / 1 200 bps         20.83       21.88             1               5            1             22.02
    LIN 1.3 / 2 400 bps         10.42       10.94             0               5            1             11.01
    LIN 1.3 / 9 600 bps          2.60        2.73             0               5            4              2.75
    LIN 1.3 / 10 417 bps         2.40        2.52             0               3            4              2.49
    LIN 1.3 / 19 200 bps         1.30        1.37             0               5            8              1.38
    LIN 1.3 / 20 000 bps         1.25        1.31             0               4            8              1.31

