#ifndef GM_COOLANT_VALVE_H
#define GM_COOLANT_VALVE_H

#include "Build.h"
#include "MotorDriver.h"

/* Valve Params */
#define C_VALVE_ZERO_POS				((uint16)50 * C_MICROSTEP_PER_FULLSTEP)
#define C_VALVE_FAR_POS					((uint16)10 * C_MICROSTEP_PER_FULLSTEP)
/* valve calibration value? */
#define C_VALVE_TOLERANCE_LO			(NVRAM_DEF_TRAVEL_TOLERANCE_LO * C_MICROSTEP_PER_FULLSTEP)
#define C_VALVE_TOLERANCE_UP			(NVRAM_DEF_TRAVEL_TOLERANCE_HI * C_MICROSTEP_PER_FULLSTEP)
#define C_VALVE_DEF_TRAVEL			    (NVRAM_DEF_TRAVEL * C_MICROSTEP_PER_FULLSTEP)
#define C_VALVE_EMRUN_OPENING           (NVRAM_DEF_EMRUN_POS)
/* valve calibration value? */

//#define C_VALVE_RANGE_MAX				(C_VALVE_DEF_TRAVEL + 2 * C_VALVE_TOLERANCE_UP + C_MOTOR_HALL_STALLDET_STEP)
//#define C_VALVE_RANGE_MIN				(C_VALVE_DEF_TRAVEL - 2 * C_VALVE_TOLERANCE_LO + C_MOTOR_HALL_STALLDET_STEP)

#define C_VALVE_RANGE_MAX				C_VALVE_DEF_TRAVEL
#define C_VALVE_RANGE_MIN				C_VALVE_DEF_TRAVEL

/* Motor Request types */
typedef enum
{
	C_MOTOR_REQUEST_NONE = 0,													/* 0: No request */
	C_MOTOR_REQUEST_STOP,														/* 1: Request to STOP motor */
	C_MOTOR_REQUEST_INIT,														/* 2: Request to initialise motor */
	C_MOTOR_REQUEST_START,														/* 3: Request to START/SET motor */
	C_MOTOR_REQUEST_SERVICE,													/* 4: Request to enter service mode */
	C_MOTOR_REQUEST_CALIBRATION,												/* 5: Request to start calibration */
	C_MOTOR_REQUEST_SLEEP,														/* 6: Request to sleep */
	C_MOTOR_REQUEST_EMRUN,														/* 7: Request to Emergency Run */
	C_MOTOR_REQUEST_SPEED_CHANGE,												/* 8: Request to change speed */
	C_MOTOR_REQUEST_CALIB_FACTORY												/* 9: Request to calibrate (factory mode) */
} MOTOR_REQUEST;

typedef enum
{
	C_CALIB_NONE = 0,															/* 0: No calibration */
	C_CALIB_START,																/* 1: Start calibration */
	C_CALIB_SETUP_HI_ENDPOS,													/* 2: Setup movement towards High Endstop */
	C_CALIB_CHECK_HI_ENDPOS,													/* 3: High Endstop reached */
	C_CALIB_PAUSE_HI_ENDSTOP,													/* 4: High Endstop Pause */
	C_CALIB_SETUP_LO_ENDPOS,													/* 5: Setup movement towards Low Endstop */
	C_CALIB_CHECK_LO_ENDPOS,													/* 6: Low Endstop reached */
	C_CALIB_SETUP_HM_ENDPOS,													/* 7: Low Endstop reached */
	C_CALIB_CHECK_HM_ENDPOS,													/* 8: Low Endstop reached */
	C_CALIB_END,																/* 9: End of calibration */
	C_CALIB_DONE,																/* 10: Calibration successfully done */
} CALIB_MODE;

typedef enum
{
	C_STATE_UNINITIALIZED = 0,		
	C_STATE_INITIALIZING,
	C_STATE_INITIALIZED,
}VALVE_INIT_STATE;

/* err state */
#define OBD_ERR_OK                   	0u
#define OBD_ERR_NOT_OK               	1u
#define OBD_ERR_INDET                	2u

#define C_GMCV_INIT_DIR_NO_SELECT		0
#define C_GMCV_INIT_DIR_OPEN_FIRST		1
#define C_GMCV_INIT_DIR_CLOSE_FIRST		2

/* public functions */
void App_CoolantValveSMInit(void);
void App_CoolantValveSM(void);

/* ****************************************************************************	*
 *	P u b l i c   v a r i a b l e s												*
 * ****************************************************************************	*/
#pragma space nodp
//extern uint8 g_u8ValueFaultFlag;
#pragma space none

#endif
