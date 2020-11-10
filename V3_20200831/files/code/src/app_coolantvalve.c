#include "Build.h"
#include "lib_mlx315_misc.h"
#include "private_mathlib.h"
#include <mathlib.h>															/* Use Melexis math-library functions to avoid compiler warnings */
#include "MotorDriver.h"														/* use of Macros */
#include "NVRAM_UserPage.h"
#include "LIN_Communication.h"
#include "Timer.h"
#include "app_coolantvalve.h"
#include "ErrorCodes.h"															/* Error logging support */

#include "ADC.h"

/* NVRAM saved data */
typedef struct
{
	uint16 InitState;
	uint16 CPOS;
	uint16 CalibTravel;
}CV_NVMDataType;

/* Valve Event */
#define	C_VALVE_EVENT_NONE 				0u
#define	C_VALVE_EVENT_EMRUN				1u
#define	C_VALVE_EVENT_SLEEP				2u

typedef struct
{
    uint16 m_request;
	uint16 m_opening;
	uint16 m_torque;
	uint16 m_speed;
}CV_RequestStructType;


/* LIN opening:linear */
#define C_VALVE_FULL_CLOSE_LIN			0x00u
//#define C_VALVE_FULL_OPEN_LIN			0xFFu
#define C_VALVE_FULL_OPEN_LIN			0x64u	// NEXT position limit

/* valve opening:linear as LIN */
#define C_VALVE_OPENING_0_PERCENT		0x00u
//#define C_VALVE_OPENING_100_PERCENT		0xFFu
#define C_VALVE_OPENING_50_PERCENT		0x32u	// NEXT calibration position boundary
#define C_VALVE_OPENING_100_PERCENT		0x64u

#define C_VALVE_RESPONSE_UNKONWNPOS		0xFFu

/* valve OBD mechanical state */
#define OBD_VALVE_MECHANICAL_OK         0x00u
#define OBD_VALVE_MECHANICAL_MASK		0x07u
#define OBD_VALVE_RANGE_BLOCK        	0x01u        	/* block */ 
#define OBD_VALVE_RANGE_BROKEN       	0x02u        	/* broken */
#define OBD_VALVE_GEAR_DRIFT			0x04u			/* drift */
#define OBD_VALVE_MECHANICAL_INDET      0x80u        	/* indeterminate */

/* valve OBD electric state */
#define OBD_VALVE_ELECTRIC_OK        	0x00u
#define OBD_VALVE_ELECTRIC_PERM      	0x01u        	/* permanent error */
#define OBD_VALVE_ELECTRIC_TEMP      	0x02u        	/* temperory error */
#define OBD_VALVE_ELECTRIC_INDET     	0x80u	       	/* indeterminate */

#define C_STATUS_NO_FAULT				0x00			/* motor coil ok */
#define C_STATUS_FAULT_COIL_SHORT		0x01			/* motor coil short */
#define C_STATUS_FAULT_COIL_OPEN		0x02			/* motor coil open */

#define C_VALVE_SPEED_FPS               (320)
/* Valve initialize mode:low->high,high->low */
#define C_VALVE_INIT_NO_SELECT		    0u
#define C_VALVE_INIT_CLOSE_OPEN		    1u				/* initialize method:close first then open */
#define C_VALVE_INIT_OPEN_CLOSE		    2u				/* initialize method:open first then close */
/* */
#define C_VALVE_INIT_END_NONE			0x00u
#define C_VALVE_INIT_END_HI				0x01u
#define C_VALVE_INIT_END_LO				0x02u

/* continous endstop check lock */
#define C_ENDSTOP_CHECK_LOCK_LO			0x01u
#define C_ENDSTOP_CHECK_LOCK_HI         0x02u

/* Motor control type */
#define C_MOTOR_NONE					0x00u
#define C_MOTOR_STOP					0x01u			/* stop and configure parameters */
#define C_MOTOR_STOP_ONLY				0x02u			/* stop only */
#define C_MOTOR_START					0x03u			/* start with configured parameters */
#define C_MOTOR_START_ONLY				0x04u			/* start without configuring actual position */

#pragma space nodp
/* valve state */
uint8 l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
/* OBD */
uint8 l_u8StallOcc = FALSE;

uint8 l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_INDET;
uint8 l_u8OBDValveElectricError = (uint8)OBD_VALVE_ELECTRIC_INDET;

uint8 l_e8CalibrationStep = (uint8)C_CALIB_NONE;
uint8 l_u8ValveInitEnds = (uint8)C_VALVE_INIT_END_NONE;

uint8 l_e8GmcvInitDirection = C_GMCV_INIT_DIR_NO_SELECT;

uint16 l_u16PhysicalActualPos;
uint16 l_u16PhysicalTargetPos;
uint16 l_u16PhysicalCalibTravel;		/* calibrated travel,acheived by doing calibration(initialization) */

uint16 l_u16PhysicalEndstopPos = 0u;
uint8  l_u8EndstopCheckLock = 0u;		

uint8 l_u8EmrunStatus = 0u;			/* emergency run status:0-idle;1-in process */

uint8 l_u8MotorControl = C_MOTOR_STOP_ONLY;

/* purpose is to buffer LIN command info */
CV_RequestStructType s_CVRequestStruct = {
    (uint16)C_MOTOR_REQUEST_NONE,
	C_VALVE_OPENING_0_PERCENT,
	0,
	NVRAM_SPEED1,
};

uint8 l_u8AliveRollingCounter = 0u;
uint8 l_u8ValueFaultFlag = 0;

uint8 l_u8OBDValveStatusFault = 0;
uint8 l_u8OBDValveStatusVolt = C_VOLT_OK;
uint8 l_u8OBDValveStatusOverTempWarn = 0;
uint16 l_u16OBDValveStatusPosition = C_VALVE_RESPONSE_UNKONWNPOS;
uint8 l_u8OBDValveStatusMove = C_STATUS_MOVE_IDLE;
uint8 l_u8OBDValveStatusSpeedLevel = 0;

uint16 targetPos = 0;
uint8 CmdArr[8] = {0};
uint8 ReqArr[8] = {0};


#pragma space none

/******************** Private functions ************************/
void handleStartInitialize(void);
void handleInitiliazeProcess(void);
void handleEndstopCheck(void);
void handleStateTransition(void);
void handleOpeningPosition(void);
void handleSynchronizePosition(void);
void handleEmergencyRunEvent(void);
void handleSleepEvent(void);
void Valve_GotoSleep(void);

void App_CoolantValveSMInit(void)
{
	uint16 cv_nvm[3];
	Motor_ControlParams motor_params;
	
	/* Check wake-up from SLEEP (MMP160613-2) */
//	if ( (ANA_INB & WAKEUP_LIN) != 0u )
	if(0)
	{
		(void)NVRAM_Read(C_NVRAM_AREA2_ADDR, cv_nvm, 3);
		/* load real time info from nvram to ram */
		if(cv_nvm[0] == (uint16)C_STATE_INITIALIZED)
		{
			l_u16PhysicalActualPos = cv_nvm[1];
			l_u16PhysicalTargetPos = cv_nvm[1];
			l_u16PhysicalCalibTravel = cv_nvm[2];
			l_e8ValveState = (uint8)C_STATE_INITIALIZED;
			l_e8CalibrationStep = (uint8)C_CALIB_DONE;	
		}
		else
		{
			/* uninitialized,use default value */
			l_u16PhysicalActualPos = C_VALVE_ZERO_POS;
			l_u16PhysicalTargetPos = C_VALVE_ZERO_POS;	
			l_u16PhysicalCalibTravel = C_VALVE_DEF_TRAVEL;
			l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
			l_e8CalibrationStep = (uint8)C_CALIB_NONE;	
		}
	}
	else
	{
		/* unnitialized,use default value */
		l_u16PhysicalActualPos = C_VALVE_ZERO_POS;
		l_u16PhysicalTargetPos = C_VALVE_ZERO_POS;	
		l_u16PhysicalCalibTravel = C_VALVE_DEF_TRAVEL;
		l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
		l_e8CalibrationStep = (uint8)C_CALIB_NONE;
	}

	/* configure motor control params */
	motor_params.MotorCtrl = C_MOTOR_CTRL_STOP;
	motor_params.ActPos = l_u16PhysicalActualPos;
	motor_params.TgtPos = l_u16PhysicalTargetPos;
	motor_params.SpdRPM = NVRAM_SPEED1;
	MotorDriverSetParams(motor_params);
	
#if _SUPPORT_ENDSTOP_DETECTION
	l_u8EndstopCheckLock = 0u; 		/* unlock endstop check when wake-up or power up  */
#endif

	l_u8OBDValveElectricError = (uint8)OBD_VALVE_ELECTRIC_INDET;

#if _SUPPORT_STALLDET
	l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_INDET;
#endif
	
}

void App_CoolantValveSM(void)
{
	switch(l_e8ValveState)
    {
    case C_STATE_UNINITIALIZED:
		/* 1)request calibration;2)Motor not in degrade mode? */
		handleStartInitialize();
		break;
	case C_STATE_INITIALIZING:
		handleInitiliazeProcess();
		break;
	case C_STATE_INITIALIZED:
		/* handle master normal start motor request */
		handleOpeningPosition();
		
#if _SUPPORT_STALLDET
		/* stall endstop check */        
		/* 1)endstop broken or valve over range; 
	  	  * 2)endstop unreachable or stall(valve)  
	  	  */
		handleEndstopCheck();
#endif
		break;
	default:
		l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
		break;
    }
	/* synchronize motor position to valve physical position */
	handleSynchronizePosition();
	/* state transition */
	handleStateTransition();
	/* handle event */
	/* bus time out:emergency run */
	handleEmergencyRunEvent();
	/* goto sleep command */
	handleSleepEvent();

}

void handleStartInitialize(void)
{
//	uint8 ret = Timer_IsExpired(CALIB_PAUSE_TIMER);
	
	if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_CALIBRATION)
	{
		if(s_CVRequestStruct.m_opening <= C_VALVE_RANGE_MAX)
		{
			Timer_Start(FAULT_HOLD_TIMER,C_PI_TICKS_500MS);
			l_e8CalibrationStep = (uint8)C_CALIB_START;
			l_u8ValveInitEnds = (uint8)C_VALVE_INIT_END_NONE;
			/* reset OBD status */
			l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_INDET;
			l_u8OBDValveElectricError = (uint8)OBD_VALVE_ELECTRIC_INDET;
			/* reset motor status */
			MotorDriverClearFaultStatus();
			l_u8ValueFaultFlag = 0;
		}
		else
		{
			/* ignore calibration request */
		}
	}
}

void handleInitiliazeProcess(void)
{
	Motor_ControlParams motor_params;
	uint16 range_temp;	/* range travelled from end to end,with hall sensor detection steps */

	if(l_e8CalibrationStep == (uint8)C_CALIB_START)
	{
		if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_CALIBRATION)
        {
			l_e8CalibrationStep = (uint8)C_CALIB_SETUP_LO_ENDPOS;
#if 0
			if(s_CVRequestStruct.m_opening <= C_VALVE_OPENING_50_PERCENT)
			{
				l_e8CalibrationStep = (uint8)C_CALIB_SETUP_HI_ENDPOS;		/* check for high end-stop */
//				l_e8GmcvInitDirection = C_GMCV_INIT_DIR_OPEN_FIRST;
			}
			else if((s_CVRequestStruct.m_opening > C_VALVE_OPENING_50_PERCENT) && (s_CVRequestStruct.m_opening <= C_VALVE_OPENING_100_PERCENT))
			{
				l_e8CalibrationStep = (uint8)C_CALIB_SETUP_LO_ENDPOS;		/* check for high end-stop */
//				l_e8GmcvInitDirection = C_GMCV_INIT_DIR_CLOSE_FIRST;
			}
			else
			{
				/* unsupported initialize position request */
			}
#endif
		}
	}

#if 0
	if(l_e8CalibrationStep == (uint8)C_CALIB_SETUP_HI_ENDPOS)
	{
		/* setup high endstop */
		if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_CALIBRATION)
		{
            s_CVRequestStruct.m_request = (uint16)C_MOTOR_REQUEST_NONE;
            /* eg.setup motor parameters and start motor */
			l_u16PhysicalActualPos = C_VALVE_ZERO_POS;
			l_u16PhysicalTargetPos = C_VALVE_RANGE_MAX + C_VALVE_ZERO_POS;
			/* client-server:post message */
			l_u8MotorControl = C_MOTOR_START;

		    l_e8CalibrationStep = (uint8)C_CALIB_CHECK_HI_ENDPOS;		/* check for high end-stop */
        }
	}
	else if( l_e8CalibrationStep == (uint8)C_CALIB_SETUP_LO_ENDPOS )
#endif

	if( l_e8CalibrationStep == (uint8)C_CALIB_SETUP_LO_ENDPOS )
	{
		/* setup low endstop */
		if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_CALIBRATION)
		{
            s_CVRequestStruct.m_request = (uint16)C_MOTOR_REQUEST_NONE;
            /* setup motor parameters and start motor */
			l_u16PhysicalActualPos = C_VALVE_RANGE_MAX + C_VALVE_ZERO_POS;
			l_u16PhysicalTargetPos = C_VALVE_ZERO_POS;
			/* client-server:post message */
			l_u8MotorControl = C_MOTOR_START;

		    l_e8CalibrationStep = (uint8) C_CALIB_CHECK_LO_ENDPOS;
        }
	}

#if 0
	else if(l_e8CalibrationStep == (uint8)C_CALIB_CHECK_HI_ENDPOS)
	{
		/* deal with motor error?  */
		/* deal with motor stop? */
#if _SUPPORT_STALLDET
		/* hall stall shall always be detected */
		if ( l_u8StallOcc == TRUE)   /* stall found, and motor enter degrade mode? */
		{
			l_u8StallOcc = FALSE;
			MotorDriverClearFaultStatus();
			l_u8ValveInitEnds |= C_VALVE_INIT_END_HI;
			range_temp = l_u16PhysicalActualPos - C_VALVE_ZERO_POS;
			if((l_u8ValveInitEnds & C_VALVE_INIT_END_LO) == 0u)    /* step:close not complete */
            {
				/* (valve range ok->finish,)or continue next step  */
				if(range_temp > C_VALVE_RANGE_MIN)
				{
					l_u16PhysicalCalibTravel = range_temp;
					l_u16PhysicalActualPos = l_u16PhysicalCalibTravel + C_VALVE_ZERO_POS;
					l_u16PhysicalTargetPos = l_u16PhysicalCalibTravel + C_VALVE_ZERO_POS;
					l_e8CalibrationStep = (uint8)C_CALIB_END;
				}
				else
				{
					l_u8MotorControl = C_MOTOR_STOP_ONLY;
					l_e8CalibrationStep = (uint8)C_CALIB_SETUP_LO_ENDPOS;
					s_CVRequestStruct.m_request = (uint16)C_MOTOR_REQUEST_CALIBRATION;
				}
            }
			else
			{
				/* low endstop check complete */
				/* initialize close->open, second step finished with stall cause of range insufficient */
				if(range_temp > C_VALVE_RANGE_MIN)
				{
					l_u16PhysicalCalibTravel = range_temp;
					l_u16PhysicalActualPos = l_u16PhysicalCalibTravel + C_VALVE_ZERO_POS;
					l_u16PhysicalTargetPos = l_u16PhysicalCalibTravel + C_VALVE_ZERO_POS;
				}
				else
				{
					l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_RANGE_BLOCK;
					l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BLOCK;
				}
				l_e8CalibrationStep = (uint8)C_CALIB_END;
			}
		}
		else
		{
			/* stall not found,or the valve over range detected,valve broken */
			if(l_u16PhysicalActualPos == (C_VALVE_RANGE_MAX + C_VALVE_ZERO_POS))
			{
				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_RANGE_BROKEN;
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
				l_e8CalibrationStep = (uint8)C_CALIB_END;
			}
			else
			{
				/* GM spec. cancel current open process,to close process? */
//				if((s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_CALIBRATION) && (s_CVRequestStruct.m_opening == C_VALVE_OPENING_0_PERCENT))
//				{
					/* stop stepper motor:immediate */
//					l_u8MotorControl = C_MOTOR_STOP_ONLY;
//					l_e8CalibrationStep = (uint8)C_CALIB_SETUP_LO_ENDPOS;

//				}
//				else if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_STOP)
				if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_STOP)
				{
					/* stop stepper motor:immediate */
					l_u8MotorControl = C_MOTOR_STOP_ONLY;
					/* re-start calibration */
				}
				else if(s_CVRequestStruct.m_request == C_MOTOR_REQUEST_CALIBRATION)
				{
					l_u8MotorControl = C_MOTOR_START_ONLY;
				}
				else
				{
					
				}
			}
		}
#else	/* _SUPPORT_STALLDET */
		if(l_u16PhysicalActualPos == C_VALVE_RANGE_MAX + C_VALVE_ZERO_POS)
		{
			if((l_u8ValveInitEnds & (uint8)C_VALVE_INIT_END_LO) == 0u)
            {
			   l_u8ValveInitEnds |= (uint8)C_VALVE_INIT_END_HI;
			   l_e8CalibrationStep = (uint8)C_CALIB_SETUP_LO_ENDPOS;
            }
			else
			{
            	/* initialize finish */
				l_e8CalibrationStep = (uint8)C_CALIB_END;
			}
		}
#endif /* _SUPPORT_STALLDET */
	}
	else if(l_e8CalibrationStep == (uint8)C_CALIB_CHECK_LO_ENDPOS)
#endif

	if(l_e8CalibrationStep == (uint8)C_CALIB_CHECK_LO_ENDPOS)
	{
#if _SUPPORT_STALLDET
		/* hall stall shall always be detected */
		if ( l_u8StallOcc == TRUE)
		{		
			l_u8StallOcc = FALSE;
			MotorDriverClearFaultStatus();
			l_u8ValveInitEnds |= (uint8)C_VALVE_INIT_END_LO;
//			range_temp = C_VALVE_RANGE_MAX + C_VALVE_ZERO_POS - l_u16PhysicalActualPos;
//			if((l_u8ValveInitEnds & C_VALVE_INIT_END_HI) == 0u)
            {
				/* GM spec.report real position */ 
				/* (Valve already in the range),for the next calibration step */
				/* (valve range ok->finish,)or continue next step  */
//				if(range_temp > C_VALVE_RANGE_MIN)
				{
//					l_u16PhysicalCalibTravel = range_temp;
					l_u16PhysicalActualPos = C_VALVE_ZERO_POS;
					l_u16PhysicalTargetPos = C_VALVE_ZERO_POS;
					l_e8CalibrationStep = (uint8)C_CALIB_END;
				}
            }
//			else
//			{
//
//			}
		}
        /* or the valve os over range */
		else
		{
			if(l_u16PhysicalActualPos <= C_VALVE_ZERO_POS)
			{
				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_RANGE_BROKEN;
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
				l_e8CalibrationStep = (uint8)C_CALIB_END;
			}
			else
			{
				/* GM spec. cancel current close process,to open process? */
//				if((s_CVRequestStruct.m_request == C_MOTOR_REQUEST_CALIBRATION) && (s_CVRequestStruct.m_opening == C_VALVE_OPENING_100_PERCENT))
//				{
					/* stop stepper motor:immediate */
//					l_u8MotorControl = C_MOTOR_STOP_ONLY;
//					l_e8CalibrationStep = (uint8)C_CALIB_SETUP_HI_ENDPOS;
//				}
//				else if(s_CVRequestStruct.m_request == C_MOTOR_REQUEST_STOP)
				if(s_CVRequestStruct.m_request == C_MOTOR_REQUEST_STOP)
				{
					/* stop stepper motor:immediate */
					l_u8MotorControl = C_MOTOR_STOP_ONLY;
					/* re-start calibration */
				}
				else if(s_CVRequestStruct.m_request == C_MOTOR_REQUEST_CALIBRATION)
				{
					l_u8MotorControl = C_MOTOR_START_ONLY;
				}
				else
				{

				}
			}
		}
#else
		if(l_u16PhysicalActualPos == C_VALVE_ZERO_POS)
		{
//			l_u8ValveInitEnds |= C_VALVE_INIT_END_LO;
//			if((l_u8ValveInitEnds & C_VALVE_INIT_END_HI) == 0)
//            {
//				l_e8CalibrationStep = (uint8)C_CALIB_SETUP_HI_ENDPOS;
//            }
//			else
			{
				/* initialize finish */
				l_e8CalibrationStep = (uint8)C_CALIB_END;
			}
		}
		else if(l_u16PhysicalActualPos < (C_VALVE_ZERO_POS - C_VALVE_TOLERANCE_UP))
		{
			l_u8OBDValveMechanicalError = OBD_VALVE_RANGE_BROKEN;
			l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
			l_e8CalibrationStep = (uint8)C_CALIB_NONE;
		}

#endif /* _SUPPORT_STALLDET */
	}
	else
	{
		/* unsupported state,should never enter */
	}

	/* calibration end */
	if(l_e8CalibrationStep == (uint8)C_CALIB_END)
	{
#if _SUPPORT_STALLDET
        if((l_u8OBDValveMechanicalError != OBD_VALVE_RANGE_BLOCK) && (l_u8OBDValveMechanicalError != OBD_VALVE_RANGE_BROKEN))
        {		
			l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_OK;
			/* synchronize step motor position? */
			/* client-server:post message */
			l_u8MotorControl = C_MOTOR_STOP;
		    l_e8CalibrationStep = (uint8)C_CALIB_DONE;
//#if _SUPPORT_ENDSTOP_DETECTION
//			/* endstop check lock if calibration has been done  */
//			/* lock low endstop if calibrated by zero */
//			if(l_u16PhysicalActualPos == C_VALVE_ZERO_POS)
//			{
//				l_u16PhysicalEndstopPos = C_VALVE_ZERO_POS;
//				l_u8EndstopCheckLock = (uint8)C_ENDSTOP_CHECK_LOCK_LO;
//			}
//			else
//			{
//				l_u16PhysicalEndstopPos = C_VALVE_ZERO_POS + l_u16PhysicalCalibTravel;
//				l_u8EndstopCheckLock = (uint8)C_ENDSTOP_CHECK_LOCK_HI;
//			}
//#endif
//			s_CVRequestStruct.m_request = C_MOTOR_REQUEST_START;
        }
		else
		{
			/* stop stepper motor:immediate */
			l_u8MotorControl = C_MOTOR_STOP_ONLY;
			l_e8CalibrationStep = (uint8)C_CALIB_NONE;
		    s_CVRequestStruct.m_request = (uint16)C_MOTOR_REQUEST_NONE;
		}
#else
        l_u8MotorControl = C_MOTOR_STOP;
		l_e8CalibrationStep = (uint8)C_CALIB_DONE;
#endif /* _SUPPORT_STALLDET */
	}
	return;
}

void handleStateTransition(void)
{
	/* unnitialized -> initializing */
	if(l_e8ValveState == (uint8)C_STATE_UNINITIALIZED)
	{
		if(l_e8CalibrationStep == (uint8)C_CALIB_START)
		{
			l_e8ValveState = (uint8)C_STATE_INITIALIZING;
		}
	}
	/* initializing -> unnitialize,initializing -> initialized */
	/* permanent electric error,temperory electric error,mechanical error */
	if(l_e8ValveState == (uint8)C_STATE_INITIALIZING)
	{
		/* error block/hall sensor,or error meichanical valve broken */
		if(((l_u8OBDValveElectricError & OBD_VALVE_ELECTRIC_PERM) != 0u) || 
			((l_u8OBDValveMechanicalError & OBD_VALVE_MECHANICAL_MASK) != 0u))
		{
			l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
			/* start calibration pause timer,prevent from contiuous calibration */
//			Timer_Start(CALIB_PAUSE_TIMER,C_PI_TICKS_500MS);
		}
		/* range error */
		if(l_e8CalibrationStep == (uint8)C_CALIB_NONE)
		{
			l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
			/* start calibration pause timer,prevent from contiuous calibration */
//			Timer_Start(CALIB_PAUSE_TIMER,C_PI_TICKS_500MS);
		}
		else if(l_e8CalibrationStep == (uint8)C_CALIB_DONE)
		{
			l_e8ValveState = (uint8)C_STATE_INITIALIZED;
		}
		else
		{
		}
	}
	/* initialized -> uninitialized */
	/* permanent electric error,valve block or broken(endstop check) */
	if(l_e8ValveState == (uint8)C_STATE_INITIALIZED)
	{
		/* degrade mode caused by non auto-recovery error,motor should be reset to stop state:
		  * 1) mechanical stall(expected/unexpected), 
		  * 2) permanent electric error(coil open/short) 
		  */
		if(((l_u8OBDValveElectricError & (uint8)OBD_VALVE_ELECTRIC_PERM) != 0u) || 
			((l_u8OBDValveMechanicalError & (uint8)OBD_VALVE_MECHANICAL_MASK) != 0u))
		{
			l_e8ValveState = (uint8)C_STATE_UNINITIALIZED;
			/* start calibration pause timer,prevent from contiuous calibration */
//			Timer_Start(CALIB_PAUSE_TIMER,C_PI_TICKS_500MS);
		}
	}
	
}

void handleOpeningPosition(void)
{
	Motor_ControlParams motor_params;
	
	/* handle master normal start motor request */
	if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_START)
	{
		s_CVRequestStruct.m_request = (uint16)C_MOTOR_REQUEST_NONE;
#if _SUPPORT_ENDSTOP_DETECTION
		/* when endstop detection is enabled,target position is set to make sure the maximun valve range 
		  * may be reached,for the endstop function to operates normally.
		  */
//		if(s_CVRequestStruct.m_opening == C_VALVE_OPENING_0_PERCENT)
//		{
//			l_u16PhysicalTargetPos = C_VALVE_ZERO_POS - (C_VALVE_RANGE_MAX - C_VALVE_RANGE_MIN);
//		}
//		else if(s_CVRequestStruct.m_opening == C_VALVE_OPENING_100_PERCENT)
//		{
//			l_u16PhysicalTargetPos = C_VALVE_ZERO_POS + C_VALVE_RANGE_MAX;
//		}
//		else
		{
			/* opening to target position */
			l_u16PhysicalTargetPos = s_CVRequestStruct.m_opening + C_VALVE_ZERO_POS;
		}
		/* endstop check unlock process[2]:1)stop request;2)ignore further moving request */
//		if(l_u8EndstopCheckLock == (uint8)C_ENDSTOP_CHECK_LOCK_HI)
//		{
//			if(l_u16PhysicalTargetPos < l_u16PhysicalEndstopPos)
//			{
//				l_u8EndstopCheckLock = 0;
//			}
//		}
//		else if(l_u8EndstopCheckLock == (uint8)C_ENDSTOP_CHECK_LOCK_LO)
//		{
//			if(l_u16PhysicalTargetPos > l_u16PhysicalEndstopPos)
//			{
//				l_u8EndstopCheckLock = 0u;
//			}
//		}
//		else
//		{
//			l_u8EndstopCheckLock = 0u;
//		}

		/* unlock endstop check,need position already gone compensation? */
		/* still lock for endstop check? */
//		if(l_u8EndstopCheckLock == 0u)
		{
			/* only update target position */
			l_u8MotorControl = C_MOTOR_START_ONLY;

		}
#else  /* _SUPPORT_ENDSTOP_DETECTION */
//		l_u16PhysicalTargetPos = divU16_U32byU16( mulU32_U16byU16( s_CVRequestStruct.m_opening, l_u16PhysicalCalibTravel) + C_VALVE_OPENING_100_PERCENT / 2, C_VALVE_OPENING_100_PERCENT) + C_VALVE_ZERO_POS;

		l_u16PhysicalTargetPos = s_CVRequestStruct.m_opening + C_VALVE_ZERO_POS;

		/* only update target position */
		l_u8MotorControl = C_MOTOR_START_ONLY;
#endif /* _SUPPORT_ENDSTOP_DETECTION */

	}
	else if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_STOP)
	{
#if _SUPPORT_ENDSTOP_DETECTION
		/* endstop check unlock process[1]:1)stop request;2)need moving request */
		l_u8EndstopCheckLock = 0u;	
#endif
		/* only stop motor:immediate */
		l_u8MotorControl = C_MOTOR_STOP_ONLY;
	}
	else
	{
		/* other event ,deal with specific event handler */
	}

}

/* stall detection and endstop check */
void handleEndstopCheck(void)
{
	Motor_ControlParams motor_params;
	Motor_RuntimeStatus motor_status;
	
	MotorDriverGetStatus(&motor_status);

  	if(motor_status.Fault.ST != 0u)	/* stall found? */
  	{
		MotorDriverClearFaultStatus();		/* clear motor stall and stop motor */
		l_u8MotorControl = C_MOTOR_STOP_ONLY;

		/* give low endstop check a tolerance:minimum valve range shall be guaranteed */
		if(motor_status.Direction == C_MOTOR_DIR_CCW)
		{
			if(l_u16PhysicalActualPos > C_VALVE_ZERO_POS)
			{
				/* stall detect within normal valve range */
				l_u8OBDValveMechanicalError = OBD_VALVE_RANGE_BLOCK;
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BLOCK;
			}
			else if(l_u16PhysicalActualPos > (C_VALVE_ZERO_POS - C_VALVE_TOLERANCE_POS))
			{
				/* GM spec. to report real position,to prevent from overflow */
				l_u16PhysicalActualPos = C_VALVE_ZERO_POS;
				l_u16PhysicalTargetPos = C_VALVE_ZERO_POS;
				/* client-server:post message */
				l_u8MotorControl = C_MOTOR_STOP;
//				l_u16PhysicalEndstopPos = C_VALVE_ZERO_POS;
//				l_u8EndstopCheckLock = (uint8)C_ENDSTOP_CHECK_LOCK_LO;
				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_OK;
			}
			else
			{
				/* record endstop check position */
//				l_u16PhysicalEndstopPos = l_u16PhysicalActualPos;
//				l_u8EndstopCheckLock = (uint8)C_ENDSTOP_CHECK_LOCK_LO;
//				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_OK;

				l_u8OBDValveMechanicalError = OBD_VALVE_RANGE_BROKEN;
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
			}
		}
		else if(motor_status.Direction == C_MOTOR_DIR_CW)
		{
			if(l_u16PhysicalActualPos < (C_VALVE_ZERO_POS + C_VALVE_DEF_TRAVEL))
			{
				/* stall detect within normal valve range */
				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_RANGE_BLOCK;	
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BLOCK;
			}
			else if(l_u16PhysicalActualPos < (C_VALVE_ZERO_POS + C_VALVE_RANGE_MAX))
			{
				/* GM spec. to report real position,to prevent from overflow */
				l_u16PhysicalActualPos = C_VALVE_ZERO_POS + C_VALVE_DEF_TRAVEL;
				l_u16PhysicalTargetPos = C_VALVE_ZERO_POS + C_VALVE_DEF_TRAVEL;
				/* client-server:post message */
				l_u8MotorControl = C_MOTOR_STOP;
//				l_u16PhysicalEndstopPos = C_VALVE_ZERO_POS + l_u16PhysicalCalibTravel;
//				l_u8EndstopCheckLock = (uint8)C_ENDSTOP_CHECK_LOCK_HI;
				l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_MECHANICAL_OK;
			}
			else
			{
				/* record endstop check position */
//				l_u16PhysicalEndstopPos = l_u16PhysicalActualPos;
//				l_u8EndstopCheckLock = C_ENDSTOP_CHECK_LOCK_HI;
//				l_u8OBDValveMechanicalError = OBD_VALVE_MECHANICAL_OK;

				l_u8OBDValveMechanicalError = OBD_VALVE_RANGE_BROKEN;
				l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
			}
		}
		else
		{
		
		}
  	}
	else
	{
#if _SUPPORT_ENDSTOP_DETECTION
		/* no stall detected while expected,valve range broken */
//		if((l_u16PhysicalActualPos == (C_VALVE_ZERO_POS + C_VALVE_RANGE_MAX)) ||
//			(l_u16PhysicalActualPos == (C_VALVE_ZERO_POS + C_VALVE_RANGE_MIN - C_VALVE_RANGE_MAX)))
//		{
//			l_u8OBDValveMechanicalError = OBD_VALVE_RANGE_BROKEN;
//			l_u8ValueFaultFlag = (uint8)OBD_VALVE_RANGE_BROKEN;
//		}
#endif
	}
}


void handleSynchronizePosition(void)
{
	Motor_RuntimeStatus motor_status;
	Motor_ControlParams motor_params;

	/* motor control type,signal to Motor Driver only once */
	if(l_u8MotorControl == C_MOTOR_STOP_ONLY)
	{
		/* stop stepper motor:immediate */
		motor_params.MotorCtrl = C_MOTOR_CTRL_STOP;
		motor_params.TgtPos = 0xFFFFu;
		motor_params.ActPos = 0xFFFFu;
		motor_params.SpdRPM = 0xFFFFu;
		MotorDriverSetParams(motor_params);
	}
	else if(l_u8MotorControl == C_MOTOR_STOP)
	{
		motor_params.MotorCtrl = C_MOTOR_CTRL_STOP;
		motor_params.TgtPos = l_u16PhysicalTargetPos;
		motor_params.ActPos = l_u16PhysicalActualPos;
		motor_params.SpdRPM = s_CVRequestStruct.m_speed;
		MotorDriverSetParams(motor_params);
	}
	else if(l_u8MotorControl == C_MOTOR_START)
	{
		/* stop stepper motor:start with new parameters */
		motor_params.MotorCtrl = C_MOTOR_CTRL_START;
		motor_params.TgtPos = l_u16PhysicalTargetPos;
		motor_params.ActPos = l_u16PhysicalActualPos;
		motor_params.SpdRPM = s_CVRequestStruct.m_speed;
		MotorDriverSetParams(motor_params);
	}
	else if(l_u8MotorControl == C_MOTOR_START_ONLY)
	{
		/* only update target position */
		motor_params.MotorCtrl = C_MOTOR_CTRL_START;
		motor_params.TgtPos = l_u16PhysicalTargetPos;
		motor_params.ActPos = 0xFFFF;
		motor_params.SpdRPM = s_CVRequestStruct.m_speed;
		MotorDriverSetParams(motor_params);
	}
	else
	{

	}
	l_u8MotorControl = C_MOTOR_NONE;
	
		/* Motor Driver Status */
	MotorDriverGetStatus(&motor_status);

	CmdArr[0] = motor_status.Fault.ST;
	CmdArr[1] = g_u16falg;
	CmdArr[2] = g_e8MotorDirectionCCW;

	/* position */
	l_u16PhysicalActualPos = motor_status.ActPos;
	l_u8OBDValveElectricError &= (uint8)(~OBD_VALVE_ELECTRIC_INDET);			/* diagnostic has been done */
	/* temporary electric error is recoverable */
	if((motor_status.Fault.UV != 0u) || (motor_status.Fault.OV != 0u) ||
		(motor_status.Fault.TS != 0u))
	{
		l_u8OBDValveElectricError |= (uint8)OBD_VALVE_ELECTRIC_TEMP;
	}
	else
	{
		l_u8OBDValveElectricError &= (uint8)(~OBD_VALVE_ELECTRIC_TEMP);
	}
	/* permenant electric error is cleared only by a starting a new initialization */
	if((motor_status.Fault.OPEN != 0u) || (motor_status.Fault.SHORT != 0u))
	{
		l_u8OBDValveElectricError |= (uint8)OBD_VALVE_ELECTRIC_PERM;
	}
	/* stall occurence detected,dealing by valve application */
	if(motor_status.Fault.ST != 0u)
	{
		l_u8StallOcc = TRUE;
	}
	else
	{
		l_u8StallOcc = FALSE;
	}
/*
	if(motor_status.Fault.DRIFT != 0u)
	{
		l_u8OBDValveMechanicalError = (uint8)OBD_VALVE_GEAR_DRIFT;
	}
*/
	uint16 temp;
	uint8 wait = Timer_IsExpired(FAULT_HOLD_TIMER);
	/* fault signal */
	/* NEXT:fault state logic,use indeterminate */
	if( (((l_u8OBDValveMechanicalError != OBD_VALVE_MECHANICAL_OK) && (l_u8OBDValveMechanicalError != OBD_VALVE_MECHANICAL_INDET)) ||
			((l_u8OBDValveElectricError != OBD_VALVE_ELECTRIC_OK) && (l_u8OBDValveElectricError != OBD_VALVE_ELECTRIC_INDET))) || (wait ==TRUE) )
	{
		if((l_u8OBDValveElectricError & (uint8)OBD_VALVE_ELECTRIC_PERM) == 0x01)
		{
			if(motor_status.Fault.SHORT != 0u)
			{
				l_u8OBDValveStatusFault = C_STATUS_FAULT_COIL_SHORT;
			}
			else if(motor_status.Fault.OPEN != 0u)
			{
				l_u8OBDValveStatusFault = C_STATUS_FAULT_COIL_OPEN;
			}
			else
			{
				l_u8OBDValveStatusFault = C_STATUS_NO_FAULT;
			}
		}
		else
		{

		}

		if((l_u8OBDValveElectricError & (uint8)OBD_VALVE_ELECTRIC_PERM) != 0x01)
		{
			if(motor_status.Fault.TS != 0u)
			{
				l_u8OBDValveStatusFault = C_FAULT_STATE_OVTEMP_SHUTDOWN;
			}
			else if(l_u8ValueFaultFlag == OBD_VALVE_RANGE_BLOCK)
			{
				l_u8OBDValveStatusFault = C_FAULT_STATE_STALL;
			}
			else if(l_u8ValueFaultFlag == OBD_VALVE_RANGE_BROKEN)
			{
				l_u8OBDValveStatusFault = C_FAULT_STATE_BROKEN;
			}
			else
			{
				l_u8OBDValveStatusFault = C_STATUS_NO_FAULT;
			}
		}
		else
		{

		}
	}

	if((l_u8OBDValveElectricError & (uint8)OBD_VALVE_ELECTRIC_TEMP) == 0x02)
	{
		if(motor_status.Fault.OV != 0u)
		{
			l_u8OBDValveStatusVolt = C_VOLT_OVER;
		}
		else if(motor_status.Fault.UV != 0u)
		{
			l_u8OBDValveStatusVolt = C_VOLT_UNDER;
		}
		else
		{
			l_u8OBDValveStatusVolt = C_VOLT_OK;
		}
	}
	else
	{
		l_u8OBDValveStatusVolt = C_VOLT_OK;
	}

	if(motor_status.Fault.TW != 0u)
	{
		l_u8OBDValveStatusOverTempWarn = C_TEMPERATURE_HIGH_WARNING;
	}
	else
	{
		l_u8OBDValveStatusOverTempWarn = C_TEMPERATURE_OK;
	}

	/* opening signal */
		/* GM spec. report real position valve opening */
		if(l_e8ValveState == C_STATE_INITIALIZED)
		{
			if(l_u16PhysicalActualPos <= C_VALVE_ZERO_POS)
			{
				temp = C_VALVE_FULL_CLOSE_LIN;
			}
			else if(l_u16PhysicalActualPos >= (C_VALVE_DEF_TRAVEL + C_VALVE_ZERO_POS))
			{
				temp = C_VALVE_DEF_TRAVEL;
			}
			else
			{
				temp = l_u16PhysicalActualPos - C_VALVE_ZERO_POS;
			}
		}
		else
		{
			temp = C_VALVE_RESPONSE_UNKONWNPOS;
		}
		l_u16OBDValveStatusPosition = (uint16)temp;

	/* motor status */
	/* application provider-consumer interface */
	MotorDriverGetStatus(&motor_status);
	/* move in process signal */
	if(motor_status.Mode != (uint8)MSM_STOP)
	{
		l_u8OBDValveStatusMove = C_STATUS_MOVE_ACTIVE;
		/* torque signal */
		l_u8OBDValveStatusSpeedLevel = (uint8)((s_CVRequestStruct.m_torque / 10u) + 1u);	/* convert physical value to signal */
	}
	else
	{
		l_u8OBDValveStatusMove = C_STATUS_MOVE_IDLE;
//		l_u8OBDValveStatusSpeedLevel = C_CTRL_TORQUE_NO;
	}
}

void handleEmergencyRunEvent(void)
{
	Motor_RuntimeStatus motor_status;
	Motor_ControlParams motor_params;
	
	if(s_CVRequestStruct.m_request == C_MOTOR_REQUEST_EMRUN)
	{
		/* emergency run is triggered only after initialized */
//		if(l_e8ValveState == C_STATE_INITIALIZED)
		if(0)
		{
			/* handling opening,no endstop check */
#if _SUPPORT_ENDSTOP_DETECTION
			/* when endstop detection is enabled,target position is set to make sure the maximun valve range 
			  * may be reached,for the endstop function to operates normally.
			  */
			if(C_VALVE_EMRUN_OPENING == C_VALVE_OPENING_0_PERCENT)
			{
				l_u16PhysicalTargetPos = C_VALVE_ZERO_POS - (C_VALVE_RANGE_MAX - C_VALVE_RANGE_MIN);
			}
			else if(C_VALVE_EMRUN_OPENING == C_VALVE_OPENING_100_PERCENT)
			{
				l_u16PhysicalTargetPos = C_VALVE_ZERO_POS + C_VALVE_RANGE_MAX;
			}
			else
			{
				l_u16PhysicalTargetPos = divU16_U32byU16( mulU32_U16byU16( C_VALVE_EMRUN_OPENING, l_u16PhysicalCalibTravel) + (C_VALVE_OPENING_100_PERCENT / 2u), C_VALVE_OPENING_100_PERCENT) + C_VALVE_ZERO_POS;
			}
#else  /* _SUPPORT_ENDSTOP_DETECTION */
			l_u16PhysicalTargetPos = divU16_U32byU16( mulU32_U16byU16( C_VALVE_EMRUN_OPENING, l_u16PhysicalCalibTravel) + (C_VALVE_OPENING_100_PERCENT / 2u), C_VALVE_OPENING_100_PERCENT) + C_VALVE_ZERO_POS;
#endif /* _SUPPORT_ENDSTOP_DETECTION */
			if(l_u8EmrunStatus == 0u)
			{
				/* emergency run should be triggered only once,emgency run in process */
				l_u8EmrunStatus = 1u;
				/* only update target position-emergency run position */
				l_u8MotorControl = C_MOTOR_START_ONLY;
			}
		}
//		else
//		{
			/* stop motor first if not initialized in case the motor is  running:immediate */
//			l_u8MotorControl = C_MOTOR_STOP_ONLY;
//		}
#if _SUPPORT_BUSTIMEOUT_SLEEP
		/* query motor state */
		MotorDriverGetStatus(&motor_status);
		l_u8MotorControl = C_MOTOR_STOP_ONLY;
		if(motor_status.Mode == (uint8)MSM_STOP)
		{
			Valve_GotoSleep();
		}

#endif
	}
	else
	{
		/* emergency run should be triggered only once */
		l_u8EmrunStatus = 0u;
	}
}

void handleSleepEvent(void)
{
	Motor_RuntimeStatus motor_status;
	
	if(s_CVRequestStruct.m_request == (uint16)C_MOTOR_REQUEST_SLEEP)
	{
		MotorDriverGetStatus(&motor_status);
		/* conditions:
		  * 1) target position equals current position(motor stop/degrade) 
		  * 2) sleep request
		  */
		if(motor_status.Mode != (uint8)MSM_STOP)
		{
			/* stop motor first before entering sleep:immediate */
			l_u8MotorControl = C_MOTOR_STOP_ONLY;
		}
		else
		{
			Valve_GotoSleep();
		}

	}
}

void Valve_GotoSleep(void)
{
	uint16 cv_nvm[3];
#if 0
	if(l_e8ValveState == C_STATE_INITIALIZED)
	{
		/* save application real time info:actual position,travel,init state */
		cv_nvm[0] = C_STATE_INITIALIZED;
		cv_nvm[1] = l_u16PhysicalActualPos;
		cv_nvm[2] = l_u16PhysicalCalibTravel;
		(void)NVRAM_Write( C_NVRAM_AREA2_ADDR, cv_nvm, 3 );	/* MISRA C:2012 Rule-17.7:The value returned by a function having non-void return type shall be used */
	}
	else
	{
		/* overwritten history record with default values , or the history data may be used when wake-up */
		cv_nvm[0] = C_STATE_UNINITIALIZED;
		cv_nvm[1] = 0u;
		cv_nvm[2] = C_VALVE_DEF_TRAVEL;
		(void)NVRAM_Write( C_NVRAM_AREA2_ADDR, cv_nvm, 3 );
	}
#endif
	/* stop MCU */
	MLX315_GotoSleep();
}

/* Event handler */
void HandleActCfrCtrl(const ACT_CFR_CTRL *pCfrCtrl)
{	
	uint16 u16TempPos;

	/* move enable and torque defined transmit */
//	if((pCfrCtrl->byMovEn == C_CTRL_MOVE_ENA) && (pCfrCtrl->byTorqueLevel >= C_CTRL_TORQUE_NOMINAL)
//		&& (pCfrCtrl->byTorqueLevel <= C_CTRL_TORQUE_BOOST_100PCT))
	u16TempPos = (uint16)((uint8)pCfrCtrl->PositionRequest_H * 256 + (uint8)pCfrCtrl->PositionRequest_L);
	targetPos = u16TempPos;


	if((pCfrCtrl->EnableRequest == C_CTRL_MOVE_ENA))	//torque is valid no matter which value qiang
	{
		if(u16TempPos <= C_VALVE_DEF_TRAVEL)	//position commend is valid;
		{
			if(pCfrCtrl->InitRequest == C_CTRL_INIT_ENA)
			{
				if((l_e8ValveState == C_STATE_UNINITIALIZED) || (l_e8ValveState == C_STATE_INITIALIZING))
				{
					s_CVRequestStruct.m_request = C_MOTOR_REQUEST_CALIBRATION;
					s_CVRequestStruct.m_opening = 0;
					s_CVRequestStruct.m_speed = NVRAM_SPEED0;		//speed is fixed
				}
				else
				{
					s_CVRequestStruct.m_request = C_MOTOR_REQUEST_START;
					s_CVRequestStruct.m_opening = u16TempPos;
					s_CVRequestStruct.m_speed = NVRAM_SPEED1;
				}
			}
			else
			{
				if((l_e8ValveState == C_STATE_UNINITIALIZED) || (l_e8ValveState == C_STATE_INITIALIZING))
				{
					s_CVRequestStruct.m_request = C_MOTOR_REQUEST_STOP;
					s_CVRequestStruct.m_opening = u16TempPos;
					s_CVRequestStruct.m_speed = NVRAM_SPEED1;		//speed is fixed
				}
				else
				{
					s_CVRequestStruct.m_request = C_MOTOR_REQUEST_START;
					s_CVRequestStruct.m_opening = u16TempPos;
					s_CVRequestStruct.m_speed = NVRAM_SPEED1;
				}
			}
		}
		else
		{
			//position commend is invalid;
		}
	}
	else
	{
		/* The actuator shall treat torque values of Not defined and Not used (0xC-0xF) the same way as No Torque Value Requested */
		/* motor request stop */
		s_CVRequestStruct.m_request = C_MOTOR_REQUEST_STOP;
		s_CVRequestStruct.m_opening = u16TempPos;
//		s_CVRequestStruct.m_torque = C_CTRL_TORQUE_NO;
		s_CVRequestStruct.m_speed = NVRAM_SPEED1;
	}
}


void HandleActRfrSta(ACT_RFR_STA *pRfrSta)
{

	/* NEXT  fault signal  */
	pRfrSta->CurrentInitState = l_e8ValveState;
	pRfrSta->RunState = l_u8OBDValveStatusMove;
	pRfrSta->FaultState = l_u8OBDValveStatusFault;
	pRfrSta->byVoltStat = l_u8OBDValveStatusVolt;
	pRfrSta->OverTempWarning = l_u8OBDValveStatusOverTempWarn;
	pRfrSta->PositionFbk_L = (uint8)(l_u16OBDValveStatusPosition & 0x00FF);
	pRfrSta->PositionFbk_H = (uint8)((l_u16OBDValveStatusPosition & 0xFF00) >> 8);
	pRfrSta->byReserved4 = (uint8)g_i16ChipTemperature;

//	pRfrSta->byVoltStat = CmdArr[0];
//	pRfrSta->OverTempWarning = CmdArr[1];
//	pRfrSta->byReserved4 = (uint8)(CmdArr[2] & 0x00FF);

}

/* handle network management event:callback */
void HandleNMReq(uint8 reason)
{
	if(reason == C_ML_REASON_CMD)
    {
		s_CVRequestStruct.m_request = C_MOTOR_REQUEST_SLEEP;
    }
	else if(reason == C_ML_REASON_TIMEOUT)
	{
		s_CVRequestStruct.m_request = C_MOTOR_REQUEST_EMRUN;
	}
	else if(reason == C_ML_REASON_WAKEUP)
	{
		s_CVRequestStruct.m_request = C_MOTOR_REQUEST_STOP;
	}
	else
	{
		/* reason not recognized */
	}
}




