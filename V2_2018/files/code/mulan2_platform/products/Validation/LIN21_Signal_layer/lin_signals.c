/*
 * LIN Core API
 * Signal Interaction and Notification
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#include "lin_api.h"

/*
 * Declaration of the frame buffers: data (signals) buffer and flags buffer
 */
/*
 * Frame M2S_1
 */
M2S_1_data_t	volatile M2S_1_data;
M2S_1_flags_t	volatile M2S_1_flags;

/*
 * Frame S2M_1
 */
S2M_1_data_t	volatile S2M_1_data = { 0xffU, 0x01U, 0x0000U, 0x01ffU, 0x00U, 0x7fU, 0x00U, 0x1fU, 0x00U, 0x07U, 0x00U, 0x01U};
S2M_1_flags_t	volatile S2M_1_flags;

/*
 * Frame M2S_2
 */
M2S_2_data_t	volatile M2S_2_data;
M2S_2_flags_t	volatile M2S_2_flags;

/*
 * Frame S2M_2
 */
S2M_2_data_t	volatile S2M_2_data = { 0xffU, 0x3fU, 0x0000U, 0x1fffU, 0x0000U, 0x07ffU};
S2M_2_flags_t	volatile S2M_2_flags;

/*
 * Frame M2S_3
 */
M2S_3_data_t	volatile M2S_3_data;
M2S_3_flags_t	volatile M2S_3_flags;

/*
 * Frame S2M_3
 */
S2M_3_data_t	volatile S2M_3_data = { 0xffU, 0xffU, 0xffU, 0xffU, 0x01U, 0x0000U, 0x7fffU};
S2M_3_flags_t	volatile S2M_3_flags;

/*
 * Frame byte_array_in
 */
byte_array_in_data_t	volatile byte_array_in_data;
byte_array_in_flags_t	volatile byte_array_in_flags;

/*
 * Frame byte_array_out
 */
byte_array_out_data_t	volatile byte_array_out_data = { 0xffU, { 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U}};
byte_array_out_flags_t	volatile byte_array_out_flags;

/*
 * Frame node_status
 */
node_status_data_t	volatile node_status_data;
node_status_flags_t	volatile node_status_flags;

/*
 * Frame counter
 */
counter_data_t	volatile counter_data = { 0x00U, 0x7fU, 0x00U};
counter_flags_t	volatile counter_flags;


/*
 *  typedef struct {
 *      void volatile *dataBuffer;    
 *      void volatile *flagsBuffer;    
 *      l_u16   dataBufferSize;  
 *      l_u8    flagsBufferSize; 
 *      l_u8    frame_type;
 *  } FrameDescBlock_type;
 *
 * Note that event-triggered frame has the same data (signal) buffer and flags buffer as
 * an associated unconditional frame. "Event triggered frames means the event triggered frame
 * header, it will therefore not contain any signals" [LIN spec]
 */

FrameDescBlock_type const frame_list[ML_NUMBER_OF_DYNAMIC_MESSAGES] = {  
							/* Message index / id: */
    { &M2S_1_data,	&M2S_1_flags,	8U,	2U, ML_UNCOND_FREE },	 /* 0x00 0x0000		*/
    { &S2M_1_data,	&S2M_1_flags,	8U,	2U, ML_UNCOND_FREE },	 /* 0x01 0x0001		*/
    { &M2S_2_data,	&M2S_2_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x02 0x0002		*/
    { &S2M_2_data,	&S2M_2_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x03 0x0003		*/
    { &M2S_3_data,	&M2S_3_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x04 0x0004		*/
    { &S2M_3_data,	&S2M_3_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x05 0x0005		*/
    { &byte_array_in_data,	&byte_array_in_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x06 0x0006		*/
    { &byte_array_out_data,	&byte_array_out_flags,	8U,	1U, ML_UNCOND_FREE },	 /* 0x07 0x0007		*/
    { &node_status_data,	&node_status_flags,	2U,	1U, ML_UNCOND_FREE },	 /* 0x08 0x0008		*/
    { &counter_data,	&counter_flags,	2U,	1U, ML_UNCOND_FREE },	 /* 0x09 0x0009		*/
};


#if LIN_VERSION == 20
/*
 * LIN 2.0 message identifier list
 */
const l_u16 MID_list[ML_NUMBER_OF_DYNAMIC_MESSAGES] = {
  0x0000U,
  0x0001U,
  0x0002U,
  0x0003U,
  0x0004U,
  0x0005U,
  0x0006U,
  0x0007U,
  0x0008U,
  0x0009U
};
#endif /* LIN_VERSION == 20 */

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
/*
 * List of associated frames.
 * For event triggered frame, index of associated unconditional frame (not used)
 * For unconditional frame, index of associated event-triggered frame, if any
 * Contains 0xFF if there is no associated frame for this index  
 */
const l_u8 associatedFrames[ML_NUMBER_OF_DYNAMIC_MESSAGES] = {
	0xffU	/* M2S_1 : none */,
	0xffU	/* S2M_1 : none */,
	0xffU	/* M2S_2 : none */,
	0xffU	/* S2M_2 : none */,
	0xffU	/* M2S_3 : none */,
	0xffU	/* S2M_3 : none */,
	0xffU	/* byte_array_in : none */,
	0xffU	/* byte_array_out : none */,
	0xffU	/* node_status : none */,
	0xffU	/* counter : none */
};

/*
 * Contains 16 flags for 16 dynamic frames
 * When signal value is updated via xxx_wr_yyy() function
 * the flag is set for the frame carrying this signal
 */
volatile l_u16 frame_updated;
#endif /* HAS_EVENT_TRIGGERED_FRAMES */

/* EOF */