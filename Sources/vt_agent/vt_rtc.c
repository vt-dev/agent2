/*
 * vt_rtc.c
 *
 */
 
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_rtc.h"
#include "vt_fw_if.h"

/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                     Define Callback Functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/

/*! rtcTimer1 State Structure definition */
rtc_state_t vt_rtcTimer_State;

/*! rtcTimer1 configuration structure */
rtc_init_config_t vt_rtcTimer_Config =
{
    /*! Counter Clock Source is XOSC */
    .clockSelect                =   RTC_CLOCK_SOURCE_XOSC,
    /*! Divide by 32 of the input clock is disabled */
    .divideBy32                 =   false,
    /*! Divide by 512 of the input clock is disabled */
    .divideBy512                =   false,
    /*! Counter continues to run when the chip is in debug */
    .freezeEnable               =   false,
    /*! Non-supervisor mode write accesses are not supported and generate
     * a bus error.
     */
    .nonSupervisorAccessEnable  =   false
};


/*! rtcTimer1 Initial Time and Date */
rtc_timedate_t vt_rtcTimer_StartTime =
{
    /*! Year */
    .year       =   2018U,
    /*! Month */
    .month      =   4U,
    /*! Day */
    .day        =   26U,
    /*! Hour */
    .hour       =   10U,
    /*! Minutes */
    .minutes    =   30U,
    /*! Seconds */
    .seconds    =   35U
};

/*! rtcTimer1 Alarm configuration 0 */
rtc_alarm_config_t vt_rtcTimer_AlarmConfig =
{
    /*! Alarm Date */
    .alarmTime           =
        {
            /*! Year    */
            .year       =  2018U,
            /*! Month   */
            .month      =  4U,
            /*! Day     */
            .day        =  26U,
            /*! Hour    */
            .hour       =  10U,
            /*! Minutes */
            .minutes    =  30U,
            /*! Seconds */
            .seconds    =  36U,
        },
	/*! Alarm repeat interval */
	.repetitionInterval  =       1UL,
	/*! Number of alarm repeats */
	.numberOfRepeats     =       0UL,
	/*! Repeat alarm forever */
	.repeatForever       =       true,
    /*! Alarm interrupt enabled */
    .alarmIntEnable      =      true,
    /*! Alarm interrupt User Callback */
    .alarmCallback       =     vt_rtc_timer_callback,
    /*! Alarm interrupt handler parameters */
    .callbackParams      =     NULL
};

/*------------------------------------------------------------------*
 *                 Private Function Prototypes                      *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Functions                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                         Public Functions                         *
 *------------------------------------------------------------------*/
/*! rtcTimer1 Alarm Configuration 0 Callback declaration */
void vt_rtc_timer_callback(void * callbackParam)
{
	/* Put your code here */
	vt_fw_increase_system_time();
	vt_toggle_led(leds[VT_RTC_LED]);
}

/*!
 * @brief  This API will initialize RTC.
 * @param [in]   instance - is number of RTC used.
 * @param [in]   *startTime - is a pointer to rtc_timedate_t(struct) to set the time and date.
 * @param [in]   *alarmConfig - is a pointer to rtc_alarm_config_t(struct) to configure alarm trigger.
 * @return       none.
 */
void vt_rtc_init(uint32_t instance, rtc_timedate_t * startTime, rtc_alarm_config_t *alarmConfig)
{
	RTC_DRV_Init(instance, &vt_rtcTimer_State, (const rtc_init_config_t *)&vt_rtcTimer_Config);

	/* Set the time and date */
	if(startTime != NULL)
		RTC_DRV_SetTimeDate(instance, (const rtc_timedate_t *)startTime);

	/* Start RTC counter */
	RTC_DRV_StartCounter(instance);

	if(alarmConfig != NULL)
		RTC_DRV_ConfigureAlarm(instance, alarmConfig);
}

/*!
 * @brief  This API will set repeated forever for alarm of RTC.
 * @param [in]   instance - is number of RTC used.
 * @param [in]   sec - interval in second to alarm trigger.
 * @return       status.
 */
status_t vt_set_alarm_repeat_forever(uint32_t instance, uint32_t sec)
{
	rtc_timedate_t tempTime;
	status_t result = STATUS_ERROR;

	if(sec == 0)
		return result;
	/* Get current time */
	RTC_DRV_GetTimeDate(instance, &tempTime);
	tempTime.hour += sec/(60*60);
	tempTime.minutes += (sec/60)%60;
	tempTime.seconds += sec%60;
	vt_rtcTimer_AlarmConfig.alarmTime = tempTime;
	vt_rtcTimer_AlarmConfig.repeatForever = true;
	vt_rtcTimer_AlarmConfig.repetitionInterval = sec;
	/* Configure the alarm */
	result = RTC_DRV_ConfigureAlarm(instance,(rtc_alarm_config_t * const) &vt_rtcTimer_AlarmConfig);

	return result;
}

/*!
 * @brief  This API will set number of repeated for alarm of RTC.
 * @param [in]   instance - is number of RTC used.
 * @param [in]   sec - interval in second to alarm trigger.
 * @param [in]   numberOfRepeats - number of repeat for alarm of RTC.
 * @return       status.
 */
status_t vt_set_alarm_repeat_with_number(uint32_t instance, uint32_t sec, uint32_t numberOfRepeats)
{
	rtc_timedate_t tempTime;
	status_t result = STATUS_ERROR;

	if(sec == 0)
		return result;
	if(numberOfRepeats == 0)
		return result;
	/* Get current time */
	RTC_DRV_GetTimeDate(instance, &tempTime);
	tempTime.hour += sec/(60*60);
	tempTime.minutes += (sec/60)%60;
	tempTime.seconds += sec%60;
	vt_rtcTimer_AlarmConfig.alarmTime = tempTime;
	vt_rtcTimer_AlarmConfig.repeatForever = false;
	vt_rtcTimer_AlarmConfig.repetitionInterval = sec;
	vt_rtcTimer_AlarmConfig.numberOfRepeats = numberOfRepeats;
	/* Configure the alarm */
	result = RTC_DRV_ConfigureAlarm(instance,(rtc_alarm_config_t * const) &vt_rtcTimer_AlarmConfig);

	return result;
}

/*------------------------------------------------------------------*
 *                           Test Function                          *
 *------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/* END vt_rtc. */




