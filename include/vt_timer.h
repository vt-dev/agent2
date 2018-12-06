/*
 * vt_timer.h
 *
 */
#ifndef VT_TIMER_H_
#define VT_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "Cpu.h"
#include "pit_driver.h"

/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/
/*! Device instance number */
#define VT_INST_PIT (0U)

/* period in microsecond */
#define VT_PIT_PERIOD (200U)  

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
extern pit_config_t  vt_pit_InitConfig;
/*! User channel configuration 0 */
extern pit_channel_config_t vt_pit_ChnConfig0;

/*------------------------------------------------------------------*
 *                   Callback Function Prototypes                   *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                       Function Prototypes                        *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will initialize periodic interrupt timer.
 * @param [in]   instance - is number of PIT used.
 * @param [in]   *channel_config - is a pointer to pit_channel_config_t(struct) to set channel configure.
 * @return       none.
 */
void vt_timer_init(uint32_t instance, pit_channel_config_t *channel_config);

/*------------------------------------------------------------------*
 *                Test Function and Examples                        *
 *------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif


#endif /* VT_TIMER_H_ */
