#ifndef LIN_PROTOCOL_H_
#define LIN_PROTOCOL_H_

#include "Build.h"
#if LINPROT == LIN2J_VALVE_GM
/* LIN communication baudrate:bps */
#define LIN_BR					            10415U

/* supplier ID */
#define C_WILDCARD_SUPPLIER_ID				0x7FFFU
#define C_SH_SUPPLIER_ID					0x0124U                             /* Sanhua supplier ID */
#define C_MLX_SUPPLIER_ID					((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@'))

#define C_SUPPLIER_ID						C_SH_SUPPLIER_ID

/* function ID */
#define C_WILDCARD_FUNCTION_ID				0xFFFFU
#define C_GM_FUNCTION_ID					0x00B0U
#define C_MLX_FUNCTION_ID					((('V'-'@')<<10) | (('L'-'@')<<5) | ('V'-'@'))
#define C_FUNCTION_ID						C_GM_FUNCTION_ID

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
#define C_DEFAULT_NAD                       0x6C                                /* GM (Coolant) Valve */  

/* message IDs */
#define MSG_CONTROL							0x00U			                    /* Actuator Control Message-ID */
#define MSG_STATUS							0x01U								/* Actuator Status Message-ID */

/* PIDs */
#define mlxACT_CTRL							0xF0U				                /* Actuator CfrCtrl:0x30(0xF0) */
#define mlxACT_STATUS						0xB1U				                /* Actuator RfrSta: 0x31(B1) */

/* service */
#define C_SERIAL_NO_LSW						0x0000
#define C_SERIAL_NO_MSW						0x0000

#define C_CUSTOMER_ID						0x1000
#endif


#if LINPROT == LIN21_VALVE_NEXT
#define NEXT_VALVE_TPVC  0
#define NEXT_VALVE_TPVR  1

/* B sample,5th release */
#define C_SW_VER							0x0B05

/* LIN communication baudrate:bps */
#define LIN_BR					            19200U

/* supplier ID */
#define C_WILDCARD_SUPPLIER_ID				0x7FFFU
#define C_SH_SUPPLIER_ID					0x0124U                             /* Sanhua supplier ID */
#define C_MLX_SUPPLIER_ID					((('M'-'@')<<10) | (('L'-'@')<<5) | ('X'-'@'))

#define C_SUPPLIER_ID						C_SH_SUPPLIER_ID

/* function ID */
#define C_WILDCARD_FUNCTION_ID				0xFFFFU
#define C_NEXT_FUNCTION_ID					0x090AU
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

#if NEXT_VALVE_TPVC
/* configuration:NAD(default) */
#define C_DEFAULT_NAD                       0x30U                                /* GM (Coolant) Valve */
/* PIDs */
#define mlxACT_CTRL							0xC4U				                /* Actuator CfrCtrl:0x30(0xF0) */
#define mlxACT_STATUS						0x49U				                /* Actuator RfrSta: 0x31(B1) */
/* variant */
#define C_VARIANT			    		    0
/* SVN version */
#define C_SVN            					512
/* version branch ID*/
#define LIN_VERSION_BRANCH_ID       	    0x00U        					    /* product branch:4bit|0*/

#else
/* configuration:NAD(default) */
#define C_DEFAULT_NAD                       0x32                                /* GM (Coolant) Valve */
/* PIDs */
#define mlxACT_CTRL							0x06U				                /* Actuator CfrCtrl:0x30(0xF0) */
#define mlxACT_STATUS						0x8BU				                /* Actuator RfrSta: 0x31(B1) */
/* variant */
#define C_VARIANT			    		    1
/* SVN version */
#define C_SVN            					513
/* version branch ID*/
#define LIN_VERSION_BRANCH_ID       	    0x01U        					    /* product branch:4bit|0*/
#endif/*NEXT_VALVE_TPVC*/

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
	uint8 byPosition;											/* Byte 1 */
#define C_MIN_POS							0x00U
#define C_MAX_POS							0x64U
	uint8 byMovEn				: 1;							/* Byte 2.[0] */
#define C_CTRL_MOVE_DIS						0U							/* Move disabled */
#define C_CTRL_MOVE_ENA						1U							/* Move enabled */
	uint8 byTorqueLevel			: 2;							/* Byte 2.[2:1] */
#define C_CTRL_TORQUE_NO			0U							/* No Torque Value Requested */
#define C_CTRL_TORQUE_NOMINAL		1U							/* Nominal Torque */
#define C_CTRL_TORQUE_BOOST_10PCT	2U							/* 10% Boost */
	uint8  byReserved2			:5;							    /* Byte 2.[7:3] */
	uint16 byReserved3	        :8;
	uint16 byReserved4	        :8;
	uint16 byReserved5	        :8;
	uint16 byReserved6	        :8;
	uint16 byReserved7	        :8;
	uint16 byReserved8	        :8;
} ACT_CFR_CTRL, *PACT_CFR_CTRL;

typedef struct _ACT_RFR_STA
{
	uint16 byLinErr					: 1;							/* Byte 1.[0] */
#define C_STATUS_LIN_OK							0U
#define C_STATUS_LIN_ERR						1U
	uint16 byFaultState				: 3;							/* Byte 1.[3:1] */
#define C_STATUS_NO_FAULT						0U
#define C_STATUS_FAULT_COIL_SHORT				1U
#define C_STATUS_FAULT_COIL_OPEN                2U
#define C_STATUS_FAULT_OVR_TMP_SHUTDOWN			3U
#define C_STATUS_FAULT_STAT_INDET				4U
#define C_STATUS_FAULT_STALL				    5U
	uint16 byOverTempWarn            :2;							/* Byte 1.[5:4] */
#define C_OVER_TMP_OK                           0U
#define C_OVER_TMP_WARNING                      1U
	uint16 byVoltStat                :2;							/* Byte 1.[7:6] */
#define C_VOLT_OK                               0U
#define C_VOLT_OVER                             1U
#define C_VOLT_UNDER                            2U
	uint16 byMoveState				 : 1;							/* Byte 2.[0] */
#define C_STATUS_MOVE_IDLE						0U
#define C_STATUS_MOVE_ACTIVE					1U
	uint16 byTorqueLevel			 :2;							/* Byte 2.[2:1] */
	uint16 byReserved2_5			 :5;							/* Byte 2.[7:3] */
	uint16 byActPosition             :8;											/* Byte 3: Actual position */
	uint16 byReserved4	             :8;
	uint16 byReserved5	             :8;
	uint16 byReserved6	             :8;
	uint16 byReserved7	             :8;
	uint16 byReserved8	             :8;
} ACT_RFR_STA, *PACT_RFR_STA;
#endif /* (LINPROT == LIN2J_VALVE_GM) */


#endif
