/*
 * vt_timer.c
 *
 */
 
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_timer.h"
#include "vt_fw_if.h"
#include "vt_can.h"
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
/*! Global configuration of pit1 */
pit_config_t vt_pit_InitConfig =
{
    .enableStandardTimers = true,
    .enableRTITimer = false,
    .stopRunInDebug = false
};

/*! User channel configuration 0 */
pit_channel_config_t vt_pit_ChnConfig0 =
{
    .hwChannel = 0U,
    .periodUnit = PIT_PERIOD_UNITS_MICROSECONDS,
    .period = VT_PIT_PERIOD,
    .enableChain = false,
    .enableInterrupt = true
};

/*------------------------------------------------------------------*
 *                 Private Function Prototypes                      *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Functions                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Interrupt Handler                         *
 *------------------------------------------------------------------*/
void PIT_Ch0_IRQHandler(void)
{
	vt_fw_increase_slot_tick_count();
	vt_update_can_led();
	PIT_DRV_ClearStatusFlags(VT_INST_PIT, vt_pit_ChnConfig0.hwChannel);     
}

/*------------------------------------------------------------------*
 *                         Public Functions                         *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will initialize periodic interrupt timer.
 * @param [in]   instance - is number of PIT used.
 * @param [in]   *channel_config - is a pointer to pit_channel_config_t(struct) to set channel configure.
 * @return       none.
 */
void vt_timer_init(uint32_t instance, pit_channel_config_t *channel_config)
{
	/* Initialize PIT */
	PIT_DRV_Init(instance, &vt_pit_InitConfig);
	/* Initialize channel 0 */
	PIT_DRV_InitChannel(instance, channel_config);
	PIT_DRV_StartChannel(instance, channel_config->hwChannel);
}

/*------------------------------------------------------------------*
 *                           Test Function                          *
 *------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
