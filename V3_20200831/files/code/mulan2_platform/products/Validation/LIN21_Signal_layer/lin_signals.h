/*
 * LIN Core API
 * Signal Interaction and Notification
 *
 * This file is application specific and depends on LDF/NCF files
 * Generated by configuration tool C:\EVWS\_LIBRARY_PLATFORMS\library_platform_mulan2\bin\ldf_nodegen.exe (version 1.4.0)
 *
 * Copyright (C) 2007-2015 Melexis N.V.
 */

#ifndef LIN_SIGNALS_H_
#define LIN_SIGNALS_H_

#define ATTR_PACKED   __attribute__ ((packed))

/*
 * Bit-field stuctures define signals layout for every frame
 * Type of the bit-field shall be selected according to
 * the size of the signal:
 *
 *   Type     Signal size
 *              (bits)
 * -------------------------------------------
 *   l_u8        1.. 8
 *   l_u16       9..16
 *   l_u32      17..32  (used only for gaps)
 *   l_u64      33..64  (used only for gaps)
 */

/*
 * Frame M2S_1
 */
typedef struct ATTR_PACKED {
    l_u8	i1	: 1;
    l_u8	i2	: 2;
    l_u8	i3	: 3;
    l_u8	i4	: 4;
    l_u8	i5	: 5;
    l_u8	i6	: 6;
    l_u8	i7	: 7;
    l_u8	i8	: 8;
    l_u16	i9	: 9;
    l_u16	i10	: 10;
    l_u8	unused55_0	: 1;
    l_u8	unused56_0	: 8;
} M2S_1_data_t;

typedef struct ATTR_PACKED {
    l_u8	i1	: 1;
    l_u8	i2	: 1;
    l_u8	i3	: 1;
    l_u8	i4	: 1;
    l_u8	i5	: 1;
    l_u8	i6	: 1;
    l_u8	i7	: 1;
    l_u8	i8	: 1;
    l_u8	i9	: 1;
    l_u8	i10	: 1;
    l_u8	frm_M2S_1	: 1;
    l_u8	unused	: 5;
} M2S_1_flags_t;

extern M2S_1_data_t	volatile M2S_1_data;
extern M2S_1_flags_t	volatile M2S_1_flags;

/*
 * Frame S2M_1
 */
typedef struct ATTR_PACKED {
    l_u8	unused0_0	: 8;
    l_u8	unused8_0	: 1;
    l_u16	o10	: 10;
    l_u16	o9	: 9;
    l_u8	o8	: 8;
    l_u8	o7	: 7;
    l_u8	o6	: 6;
    l_u8	o5	: 5;
    l_u8	o4	: 4;
    l_u8	o3	: 3;
    l_u8	o2	: 2;
    l_u8	o1	: 1;
} S2M_1_data_t;

typedef struct ATTR_PACKED {
    l_u8	o10	: 1;
    l_u8	o9	: 1;
    l_u8	o8	: 1;
    l_u8	o7	: 1;
    l_u8	o6	: 1;
    l_u8	o5	: 1;
    l_u8	o4	: 1;
    l_u8	o3	: 1;
    l_u8	o2	: 1;
    l_u8	o1	: 1;
    l_u8	frm_S2M_1	: 1;
    l_u8	unused	: 5;
} S2M_1_flags_t;

extern S2M_1_data_t	volatile S2M_1_data;
extern S2M_1_flags_t	volatile S2M_1_flags;

/*
 * Frame M2S_2
 */
typedef struct ATTR_PACKED {
    l_u16	i11	: 11;
    l_u16	i12	: 12;
    l_u16	i13	: 13;
    l_u16	i14	: 14;
    l_u8	unused50_0	: 6;
    l_u8	unused56_0	: 8;
} M2S_2_data_t;

typedef struct ATTR_PACKED {
    l_u8	i11	: 1;
    l_u8	i12	: 1;
    l_u8	i13	: 1;
    l_u8	i14	: 1;
    l_u8	frm_M2S_2	: 1;
    l_u8	unused	: 3;
} M2S_2_flags_t;

extern M2S_2_data_t	volatile M2S_2_data;
extern M2S_2_flags_t	volatile M2S_2_flags;

/*
 * Frame S2M_2
 */
typedef struct ATTR_PACKED {
    l_u8	unused0_0	: 8;
    l_u8	unused8_0	: 6;
    l_u16	o14	: 14;
    l_u16	o13	: 13;
    l_u16	o12	: 12;
    l_u16	o11	: 11;
} S2M_2_data_t;

typedef struct ATTR_PACKED {
    l_u8	o14	: 1;
    l_u8	o13	: 1;
    l_u8	o12	: 1;
    l_u8	o11	: 1;
    l_u8	frm_S2M_2	: 1;
    l_u8	unused	: 3;
} S2M_2_flags_t;

extern S2M_2_data_t	volatile S2M_2_data;
extern S2M_2_flags_t	volatile S2M_2_flags;

/*
 * Frame M2S_3
 */
typedef struct ATTR_PACKED {
    l_u16	i15	: 15;
    l_u16	i16	: 16;
    l_u8	unused31_0	: 1;
    l_u8	unused32_0	: 8;
    l_u8	unused32_1	: 8;
    l_u8	unused32_2	: 8;
    l_u8	unused32_3	: 8;
} M2S_3_data_t;

typedef struct ATTR_PACKED {
    l_u8	i15	: 1;
    l_u8	i16	: 1;
    l_u8	frm_M2S_3	: 1;
    l_u8	unused	: 5;
} M2S_3_flags_t;

extern M2S_3_data_t	volatile M2S_3_data;
extern M2S_3_flags_t	volatile M2S_3_flags;

/*
 * Frame S2M_3
 */
typedef struct ATTR_PACKED {
    l_u8	unused0_0	: 8;
    l_u8	unused8_0	: 8;
    l_u8	unused8_1	: 8;
    l_u8	unused8_2	: 8;
    l_u8	unused8_3	: 1;
    l_u16	o16	: 16;
    l_u16	o15	: 15;
} S2M_3_data_t;

typedef struct ATTR_PACKED {
    l_u8	o16	: 1;
    l_u8	o15	: 1;
    l_u8	frm_S2M_3	: 1;
    l_u8	unused	: 5;
} S2M_3_flags_t;

extern S2M_3_data_t	volatile S2M_3_data;
extern S2M_3_flags_t	volatile S2M_3_flags;

/*
 * Frame byte_array_in
 */
typedef struct ATTR_PACKED {
    l_u8	MstArray[7];
    l_u8	Cmd	: 8;
} byte_array_in_data_t;

typedef struct ATTR_PACKED {
    l_u8	MstArray	: 1;
    l_u8	Cmd	: 1;
    l_u8	frm_byte_array_in	: 1;
    l_u8	unused	: 5;
} byte_array_in_flags_t;

extern byte_array_in_data_t	volatile byte_array_in_data;
extern byte_array_in_flags_t	volatile byte_array_in_flags;

/*
 * Frame byte_array_out
 */
typedef struct ATTR_PACKED {
    l_u8	unused0_0	: 8;
    l_u8	SlvArray[7];
} byte_array_out_data_t;

typedef struct ATTR_PACKED {
    l_u8	SlvArray	: 1;
    l_u8	frm_byte_array_out	: 1;
    l_u8	unused	: 6;
} byte_array_out_flags_t;

extern byte_array_out_data_t	volatile byte_array_out_data;
extern byte_array_out_flags_t	volatile byte_array_out_flags;

/*
 * Frame node_status
 */
typedef struct ATTR_PACKED {
    l_u16	Status	: 16;
} node_status_data_t;

typedef struct ATTR_PACKED {
    l_u8	Status	: 1;
    l_u8	frm_node_status	: 1;
    l_u8	unused	: 6;
} node_status_flags_t;

extern node_status_data_t	volatile node_status_data;
extern node_status_flags_t	volatile node_status_flags;

/*
 * Frame counter
 */
typedef struct ATTR_PACKED {
    l_u8	Response_Error_s	: 1;
    l_u8	unused1_0	: 7;
    l_u8	Cnt	: 8;
} counter_data_t;

typedef struct ATTR_PACKED {
    l_u8	Response_Error_s	: 1;
    l_u8	Cnt	: 1;
    l_u8	frm_counter	: 1;
    l_u8	unused	: 5;
} counter_flags_t;

extern counter_data_t	volatile counter_data;
extern counter_flags_t	volatile counter_flags;

/*===========================================================*/
typedef enum ATTR_PACKED {
    ML_EVENT_TRIGGERED,
    ML_UNCOND_ASSOCIATED,
    ML_UNCOND_FREE
} Frame_type;

ASSERT(sizeof(Frame_type) == 1);

/*
 *  Frame Description Block
 */
typedef struct {
    void volatile *dataBuffer;      /* pointer to the data buffer */
    void volatile *flagsBuffer;     /* pointer to the Flag buffer */
    l_u16  dataBufferSize;          /* size of the data buffer */
    l_u8   flagsBufferSize;         /* size of the flag buffer */
    Frame_type   frame_type;        /* type of the frame */
} FrameDescBlock_type;

ASSERT(sizeof(FrameDescBlock_type) == 8);

extern const FrameDescBlock_type frame_list[ML_NUMBER_OF_DYNAMIC_MESSAGES];

#if LIN_VERSION == 20
extern const l_u16 MID_list[ML_NUMBER_OF_DYNAMIC_MESSAGES]; /* LIN2.0 */
#endif /* LIN_VERSION == 20 */

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
extern const l_u8 associatedFrames[ML_NUMBER_OF_DYNAMIC_MESSAGES];
extern volatile l_u16 frame_updated;
#endif /* HAS_EVENT_TRIGGERED_FRAMES */


/*
 *  Template to generate read signal functions 
 */
#define L_SIG_RD(sigType, frameName, sigName) \
    static INLINE sigType sigType##_rd_##sigName(void);\
    static INLINE sigType sigType##_rd_##sigName(void) \
    {                                                  \
        l_irqmask m;                                   \
        sigType s;                                     \
                                                       \
        m = l_sys_irq_disable();                       \
        s = frameName.sigName;                         \
        l_sys_irq_restore (m);                         \
                                                       \
        return s;                                      \
    }

/*
 *  Template to generate write signal functions
 */
#define L_SIG_WR(sigType, frameName, sigName) \
    static INLINE void sigType##_wr_##sigName(sigType v);\
    static INLINE void sigType##_wr_##sigName(sigType v) \
    {                                                    \
        l_irqmask m;                                     \
                                                         \
        m = l_sys_irq_disable();                         \
        frameName.sigName = v;                           \
        l_sys_irq_restore (m);                           \
    }

/*
 *  Template to generate 'l_flg_tst' and
 *  'l_flg_clr' functions
 */
#define L_FLAGS(bufName, flagName) \
    static INLINE l_bool l_flg_tst_##flagName(void);\
    static INLINE l_bool l_flg_tst_##flagName(void) \
    {                                               \
        l_irqmask m;                                \
        l_bool s;                                   \
                                                    \
        m = l_sys_irq_disable();                    \
        s = bufName.flagName;                       \
        l_sys_irq_restore (m);                      \
                                                    \
        return s;                                   \
    }                                               \
                                                    \
    static INLINE void l_flg_clr_##flagName(void);  \
    static INLINE void l_flg_clr_##flagName(void)   \
    {                                               \
        l_irqmask m;                                \
                                                    \
        m = l_sys_irq_disable();                    \
        bufName.flagName = 0U;                      \
        l_sys_irq_restore (m);                      \
    }

/*
 *  Template to generate 'l_bytes_rd_sss' functions
 */
#define L_BYTES_RD(frameName, sigName) \
    static INLINE void l_bytes_rd_##sigName (l_u8 start, l_u8 count, l_u8 *const data);\
    static INLINE void l_bytes_rd_##sigName (l_u8 start, l_u8 count, l_u8 *const data) \
    {                                               \
        l_irqmask m;                                \
        l_u8 volatile *src;                         \
        l_u8 *dest;                                 \
                                                    \
        m = l_sys_irq_disable();                    \
        src  = &frameName.sigName[start];           \
        dest = data;                                \
        while (count-- != 0U) {                     \
            *dest++ = *src++;                       \
        }                                           \
        l_sys_irq_restore (m);                      \
    }

/*
 *  Template to generate 'l_bytes_wr_sss' functions
 */
#define L_BYTES_WR(frameName, sigName) \
    static INLINE void l_bytes_wr_##sigName (l_u8 start, l_u8 count, l_u8 const *const data);\
    static INLINE void l_bytes_wr_##sigName (l_u8 start, l_u8 count, l_u8 const *const data) \
    {                                               \
        l_irqmask m;                                \
        l_u8 const *src;                            \
        l_u8 volatile *dest;                        \
                                                    \
        m = l_sys_irq_disable();                    \
        src  = data;                                \
        dest = &frameName.sigName[start];           \
        while (count-- != 0U) {                     \
            *dest++ = *src++;                       \
        }                                           \
        l_sys_irq_restore (m);                      \
    }

#if defined(HAS_EVENT_TRIGGERED_FRAMES)
/*
 * Template to generate write signal functions
 * for event-triggered frames (ETF)
 */
#define L_SIG_WR_ETF(sigType, frameName, sigName) \
    static INLINE void sigType##_wr_##sigName(sigType v);\
    static INLINE void sigType##_wr_##sigName(sigType v) \
    {                                               \
        l_irqmask m;                                \
                                                    \
        m = l_sys_irq_disable();                    \
        frameName.sigName = v;                      \
        frame_updated |= (1U << frameName##_idx);   \
        l_sys_irq_restore (m);                      \
    }

/*
 * Template to generate 'l_bytes_wr_sss' functions
 * for event-triggered frames (ETF)
 */
#define L_BYTES_WR_ETF(frameName, sigName) \
    static INLINE void l_bytes_wr_##sigName (l_u8 start, l_u8 count, l_u8 const *const data);\
    static INLINE void l_bytes_wr_##sigName (l_u8 start, l_u8 count, l_u8 const *const data) \
    {                                               \
        l_irqmask m;                                \
        l_u8 const *src;                            \
        l_u8 volatile *dest;                        \
                                                    \
        m = l_sys_irq_disable();                    \
        src  = data;                                \
        dest = &frameName.sigName[start];           \
        while (count-- != 0U) {                     \
            *dest++ = *src++;                       \
        }                                           \
        frame_updated |= (1U << frameName##_idx);   \
        l_sys_irq_restore (m);                      \
    }
#endif /* HAS_EVENT_TRIGGERED_FRAMES */

/*
 * Define API functions using templates
 */

/* frame M2S_1 */
L_SIG_RD(l_bool, M2S_1_data, i1)
L_SIG_RD(l_u8, M2S_1_data, i2)
L_SIG_RD(l_u8, M2S_1_data, i3)
L_SIG_RD(l_u8, M2S_1_data, i4)
L_SIG_RD(l_u8, M2S_1_data, i5)
L_SIG_RD(l_u8, M2S_1_data, i6)
L_SIG_RD(l_u8, M2S_1_data, i7)
L_SIG_RD(l_u8, M2S_1_data, i8)
L_SIG_RD(l_u16, M2S_1_data, i9)
L_SIG_RD(l_u16, M2S_1_data, i10)

L_FLAGS(M2S_1_flags, i1)
L_FLAGS(M2S_1_flags, i2)
L_FLAGS(M2S_1_flags, i3)
L_FLAGS(M2S_1_flags, i4)
L_FLAGS(M2S_1_flags, i5)
L_FLAGS(M2S_1_flags, i6)
L_FLAGS(M2S_1_flags, i7)
L_FLAGS(M2S_1_flags, i8)
L_FLAGS(M2S_1_flags, i9)
L_FLAGS(M2S_1_flags, i10)
L_FLAGS(M2S_1_flags, frm_M2S_1)

/* frame S2M_1 */
L_SIG_WR(l_u16, S2M_1_data, o10)
L_SIG_WR(l_u16, S2M_1_data, o9)
L_SIG_WR(l_u8, S2M_1_data, o8)
L_SIG_WR(l_u8, S2M_1_data, o7)
L_SIG_WR(l_u8, S2M_1_data, o6)
L_SIG_WR(l_u8, S2M_1_data, o5)
L_SIG_WR(l_u8, S2M_1_data, o4)
L_SIG_WR(l_u8, S2M_1_data, o3)
L_SIG_WR(l_u8, S2M_1_data, o2)
L_SIG_WR(l_bool, S2M_1_data, o1)

L_FLAGS(S2M_1_flags, o10)
L_FLAGS(S2M_1_flags, o9)
L_FLAGS(S2M_1_flags, o8)
L_FLAGS(S2M_1_flags, o7)
L_FLAGS(S2M_1_flags, o6)
L_FLAGS(S2M_1_flags, o5)
L_FLAGS(S2M_1_flags, o4)
L_FLAGS(S2M_1_flags, o3)
L_FLAGS(S2M_1_flags, o2)
L_FLAGS(S2M_1_flags, o1)
L_FLAGS(S2M_1_flags, frm_S2M_1)

/* frame M2S_2 */
L_SIG_RD(l_u16, M2S_2_data, i11)
L_SIG_RD(l_u16, M2S_2_data, i12)
L_SIG_RD(l_u16, M2S_2_data, i13)
L_SIG_RD(l_u16, M2S_2_data, i14)

L_FLAGS(M2S_2_flags, i11)
L_FLAGS(M2S_2_flags, i12)
L_FLAGS(M2S_2_flags, i13)
L_FLAGS(M2S_2_flags, i14)
L_FLAGS(M2S_2_flags, frm_M2S_2)

/* frame S2M_2 */
L_SIG_WR(l_u16, S2M_2_data, o14)
L_SIG_WR(l_u16, S2M_2_data, o13)
L_SIG_WR(l_u16, S2M_2_data, o12)
L_SIG_WR(l_u16, S2M_2_data, o11)

L_FLAGS(S2M_2_flags, o14)
L_FLAGS(S2M_2_flags, o13)
L_FLAGS(S2M_2_flags, o12)
L_FLAGS(S2M_2_flags, o11)
L_FLAGS(S2M_2_flags, frm_S2M_2)

/* frame M2S_3 */
L_SIG_RD(l_u16, M2S_3_data, i15)
L_SIG_RD(l_u16, M2S_3_data, i16)

L_FLAGS(M2S_3_flags, i15)
L_FLAGS(M2S_3_flags, i16)
L_FLAGS(M2S_3_flags, frm_M2S_3)

/* frame S2M_3 */
L_SIG_WR(l_u16, S2M_3_data, o16)
L_SIG_WR(l_u16, S2M_3_data, o15)

L_FLAGS(S2M_3_flags, o16)
L_FLAGS(S2M_3_flags, o15)
L_FLAGS(S2M_3_flags, frm_S2M_3)

/* frame byte_array_in */
L_BYTES_RD(byte_array_in_data, MstArray)
L_SIG_RD(l_u8, byte_array_in_data, Cmd)

L_FLAGS(byte_array_in_flags, MstArray)
L_FLAGS(byte_array_in_flags, Cmd)
L_FLAGS(byte_array_in_flags, frm_byte_array_in)

/* frame byte_array_out */
L_BYTES_WR(byte_array_out_data, SlvArray)

L_FLAGS(byte_array_out_flags, SlvArray)
L_FLAGS(byte_array_out_flags, frm_byte_array_out)

/* frame node_status */
L_SIG_WR(l_u16, node_status_data, Status)

L_FLAGS(node_status_flags, Status)
L_FLAGS(node_status_flags, frm_node_status)

/* frame counter */
L_SIG_WR(l_bool, counter_data, Response_Error_s)
L_SIG_WR(l_u8, counter_data, Cnt)

L_FLAGS(counter_flags, Response_Error_s)
L_FLAGS(counter_flags, Cnt)
L_FLAGS(counter_flags, frm_counter)


#endif /* LIN_SIGNALS_H_ */
