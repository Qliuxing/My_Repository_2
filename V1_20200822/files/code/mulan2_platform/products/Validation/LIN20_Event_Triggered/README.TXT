
    Example is provided with pre-generated standard LIN driver:

    lin_cfg.c
    lin_core.c
    lin_diag.c
    lin_signals.c
    mlu30.c
    lin_api.h
    lin_cfg.h
    lin_core.h
    lin_diag.h
    lin_signals.h
    lin2b_romtbl.S 

    
    Driver generating/cleaning:

    'make drv'       generates driver from ldf file
    'make clean_drv' deletes driver sources



    Example features
    ----------------------------------------------------------------------------------------------------------------

    4.Supported Services
    Default FrameIDs are assigned before tests: NO
    IUT supports Event Triggered Frames: YES
    IUT can answer to Read By ID (0) with positive response: YES

    Note:
    To activate event-triggered frame (ETF), some signal associated with this ETF shall be changed. 
    E.g. first we send doorFrontLeft_setBackLight frame (FrameId 0x03) with data byte 0xAA (in red below).
    Note, that each time we send this frame the value of data byte shall be different (see e.g. 0x55 in blue below)
 
    4.1. Baud rate
    IUT making use of Synchronization: YES
    IUT supports Autobaud (baud detection on the fly): YES, but only on the Sync-Filed of the first frame after Reset

    IUT supports 2400 baud  : YES, but not in auto-baud mode (separate FW image configured for 2400 bps is needed)
    IUT supports 9600 baud  : YES
    IUT supports 10417 baud : YES
    IUT supports 19200 baud : YES
    IUT supports 20000 baud : YES

    4.2. Slave Network Properties
    IUT goes in Sleep Mode after Goto Sleep command : YES
    IUT goes in Sleep Mode after Bus Idle time out  : YES
    IUT reduces current in Sleep Mode: NO
    IUT can generate a bus Wake Up: NO

    4.3. Slave Node Configuration Support
    IUT supports Assign FrameID : YES
    IUT supports Assign NAD : YES
    IUT supports Conditional Change NAD : YES
    IUT can answer to Read By ID (1) with positive response: YES
