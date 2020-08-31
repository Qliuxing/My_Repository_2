#include "build.h"
#include <plib.h>									   /* Use Melexis MLX813xx library (WDG_Manager) */
#include "lib_mlx315_misc.h"
#include "ADC.h"
#include "Timer.h"
#include "MotorDriver.h"
#include "PID_Control.h"
#include "Diagnostic.h"
#include "LIN_Communication.h"
#include "ErrorCodes.h"
#include "app_coolantvalve.h"
#include "system_background.h"

#pragma space nodp

#pragma space none

int16 main( void)
{
	MLX315_SystemInit();                               	/* initialize MLX315 hardware */
	
	SET_PRIORITY(0);							       	/* Enter application mode */

	/* driver initialize area */
	NVRAM_Init();                              			/* Load User NVRAM storage parameters */
	ADC_Init();									       	/* Initialize ADC */
	DiagnosticsInit();							       	/* Initialize Diagnostic */
	MotorDriverInit();							       	/*  Initialize Motor-driver */
	PID_Init();										   	/* PID Control initialization */
	LIN_Init();											/* initialize LIN interface */
	
	SET_PRIORITY(7);									/* Enable interrupts:MASK LEVEL lowest */
	
	/* system service */
	ErrorLogInit();								       	/* Initialize Error-logging management */	
	Timer_Init();								       	/* Initialize (Core) Timer */

#if _SUPPORT_MOTOR_SELFTEST
	MotorDiagnosticSelfTest();							/* Self-test Motor-Driver */
#endif
	/* Application initialize area */
	App_CoolantValveSMInit();
	System_BackgroundTaskInit();
	
	for(;;)
	{
		/* user application */
		App_CoolantValveSM();

		/* driver and service */
		MotorDriver_MainFunction();
		LIN_MainFunction();
		
		/* system background application */
		System_BackgroundMemoryTest();
		System_BackgroundIORegTest();

#if WATCHDOG == ENABLED
		/* Watch-dog acknowledgment */
		WDG_Manager();
#endif	
	}

	return 0;
}

