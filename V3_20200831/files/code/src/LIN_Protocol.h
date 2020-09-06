#ifndef LIN_PROTOCOL_H_
#define LIN_PROTOCOL_H_

#include "Build.h"

#if LINPROT == LIN21_VALVE_NEXT

/* B sample,5th release */
#define C_SW_VER							0x0B05

/* LIN communication baudrate:bps */
#define LIN_BR					            19200U

/* supplier ID */
#define C_WILDCARD_SUPPLIER_ID				0x7FFFU
#define C_SH_SUPPLIER_ID					0x0013U                             /* Sanhua supplier ID */
#define C_MLX_SUPPLIER_ID					((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@'))

#define C_SUPPLIER_ID						C_SH_SUPPLIER_ID

/* function ID */
#define C_WILDCARD_FUNCTION_ID				0xFFFFU
#define C_NEXT_FUNCTION_ID					0x0013U
#define C_MLX_FUNCTION_ID					((('V'-'@')<<10) | (('L'-'@')<<5) | ('V'-'@'))
#define C_FUNCTION_ID						C_NEXT_FUNCTION_ID

/* NAD */
#if ((LINPROT & LINXX) == LIN2J)
#define C_MIN_J2602_NAD						0x60U				                /* Lowest J2602 NAD */
#define C_MAX_J2602_NAD						0x6DU				                /* Highest J2602 NAD */
#define C_STEP_J2602_NAD					0x01U				                /* All NAD's */
#define C_DIAG_J2602_NAD					0x6EU				                /* J2602 NAD may be used for 0x3C and 0x3E frame's */
#define C_DEFAULT_J2602_NAD					0x6FU				                /* Default (not programmed) J2602 NAD */
#endif
#define C_BROADCAST_NAD				        0x7FU                               /* broadcast NAD */

/* configuration:NAD(default) */
#define C_DEFAULT_NAD                       0x16                                /* GM (Coolant) Valve */
/* PIDs */
#define mlxACT_CTRL							0xC4U				                /* Actuator CfrCtrl:0x04(0xC4) */
#define mlxACT_STATUS						0x03U				                /* Actuator RfrSta: 0x03(0x03) */
/* variant */
#define C_VARIANT			    		    255
/* SVN version */
#define C_SVN            					513
/* version branch ID*/
#define LIN_VERSION_BRANCH_ID       	    0x01U        					    /* product branch:4bit|0*/

/* message IDs */
#define MSG_CONTROL							0x01U			                    /* Actuator Control Message-ID */
#define MSG_STATUS							0x00U								/* Actuator Status Message-ID */

/* service */
#define C_SERIAL_NO_LSW						0x0000
#define C_SERIAL_NO_MSW						0x0000

#define C_CUSTOMER_ID						0x1000
#endif

/******************* application protocol **********************/
#if ((LINPROT == LIN2J_VALVE_GM) || (LINPROT == LIN21_VALVE_NEXT))
/* GM VALVE V1.0 */
typedef struct  _ACT_CFR_INI
{
	uint8 PositionRequest_L		:8;					/* Byte 0.[0] */
	uint8 PositionRequest_H		:8;					/* Byte 1.[0] */
#define C_MIN_POS							0x0000U
#define C_MAX_POS							0xFFFFU
	uint8 EnableRequest			 :1;							/* Byte 2.[0] */
#define C_CTRL_MOVE_DIS						0U					/* Move disabled */
#define C_CTRL_MOVE_ENA						1U					/* Move enabled */
	uint8 InitRequest			:3;							/* Byte 2.[1:3] */
#define C_CTRL_INIT_DIS			0U							/* Init disabled */
#define C_CTRL_INIT_ENA			1U							/* Init enabled */
	uint8 SleepRequest			:4;							/* Byte 2.[4:7] */
#define	C_CTRL_SLEEP_DIS		0U							/* request sleep enable */
#define	C_CTRL_SLEEP_ENA		1U							/* request sleep disable */

//	uint8 Cmd_MicroStep			:3;							    /* Byte 2.[7:3] */
//#define C_CTRL_FULL_STEP		0U								/* full step running */
//#define C_CTRL_2MICRO_STEP		1U								/* 2microstep running */
//#define C_CTRL_4MICRO_STEP		2U								/* 4microstep running */
//#define C_CTRL_8MICRO_STEP		3U								/* 8microstep running */
//#define C_CTRL_16MICRO_STEP		4U								/* 16microstep running */

/************************************* reserved ************************************************/
	uint16 byReserved3	        :8;
	uint16 byReserved4	        :8;
	uint16 byReserved5	        :8;
	uint16 byReserved6	        :8;
	uint16 byReserved7	        :8;

} ACT_CFR_CTRL, *PACT_CFR_CTRL;

typedef struct _ACT_RFR_STA
{
	uint16 ResponseError			  :1;							/* Byte 0.[0] */
#define C_STATUS_LIN_OK							0U
#define C_STATUS_LIN_ERR						1U
	uint16 CurrentInitState			  :2;							/* Byte 0.[1:2] */
#define C_STATUS_NO_INIT						0U
#define C_STATUS_INIT_IN_PROCESS				1U
#define C_STATUS_INIT_READY		                2U
#define C_STATUS_INIT_ERROR						3U
	uint16 RunState				 	  :1;							/* Byte 0.[3] */
#define C_STATUS_MOVE_IDLE						0U
#define C_STATUS_MOVE_ACTIVE					1U
	uint16 byVoltStat                 :2;							/* Byte 0.[4:5] */
#define C_VOLT_OK                               0U
#define C_VOLT_OVER                             1U
#define C_VOLT_UNDER                            2U
	uint16 byReserved0	             :2;
/******************************************** reserved **************************************/
	uint16 byReserved1	             :8;
	uint16 byReserved2	             :8;
	uint16 byReserved3	             :8;
	uint16 byReserved4	             :8;
	uint16 byReserved5	             :8;
	uint16 byReserved6	             :8;
	uint16 byReserved7	             :8;

} ACT_RFR_STA, *PACT_RFR_STA;
#endif /* (LINPROT == LIN2J_VALVE_GM) */


#endif
