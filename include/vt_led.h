/*
 * vt_led.h
 *
 */

#ifndef VT_LED_H_
#define VT_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "Cpu.h"
#include "vt_fw_oem.h"

/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/
#ifdef MPC5748G_DEVKIT
#define VT_MAX_LEDS 8

#define LED0 4          
#define LED1 0          
#define LED2 148        
#define LED3 117        
#define LED4 36         
#define LED5 125        
#define LED6 7          
#define LED7 10  		
#else
#define VT_MAX_LEDS 4

#define LED0 98          
#define LED1 99          
#define LED2 100        
#define LED3 101        
#endif

#define VT_RTC_LED    0
#define VT_BLOCK_LED  1
#define VT_CAN_RX_LED 2
#define VT_CAN_TX_LED 3

/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                     Define Callback Functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/
extern uint32_t leds[];

/*------------------------------------------------------------------*
 *                   Callback Function Prototypes                   *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                       Function Prototypes                        *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will initialize Led.
 * @param [in]   none.
 * @return       none.
 */
void vt_init_leds(void);

/*!
 * @brief  This API will turn Led on.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_led_on(uint32_t led);

/*!
 * @brief  This API will turn Led off.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_led_off(uint32_t led);

/*!
 * @brief  This API will toggle a Led.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_toggle_led(uint32_t led);

/*!
 * @brief  This API will turn all Led off.
 * @param [in]   none.
 * @return       none.
 */
void vt_all_leds_off(void);

/*!
 * @brief  This API will turn all Led on.
 * @param [in]   none.
 * @return       none.
 */
void vt_all_leds_on(void);

/*!
 * @brief  This API will turn Led bits on. We have 8 Leds so the Led bits has value form 0x01 to 0xFF.
 * @param [in]   ledBits - is Led bits.
 * @return       none.
 */
void vt_turn_on_bits_led(uint8_t ledBits);

/*!
 * @brief  This API will delay with input value.
 * @param [in]   val - is value to delay.
 * @return       none.
 */
void vt_delay(volatile int val);

/*------------------------------------------------------------------*
 *                Test Function and Examples                        *
 *------------------------------------------------------------------*/
void vt_test_leds(void);

#ifdef __cplusplus
}
#endif


#endif /* VT_LED_H_ */
