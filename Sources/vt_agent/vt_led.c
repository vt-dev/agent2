/*
 * vt_led.c
 */
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_led.h"

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
#ifdef MPC5748G_DEVKIT
uint32_t leds[] = {LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7};
#else /* MPC5748G-LCEVB */
uint32_t leds[] = {LED0, LED1, LED2, LED3};
#endif
/*------------------------------------------------------------------*
 *                 Private Function Prototypes                      *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Functions                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Interrupt Handler                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                         Public Functions                         *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will delay with input value.
 * @param [in]   val - is value to delay.
 * @return       none.
 */
void vt_delay(volatile int cycles)
{
  while(cycles--);
}

/*!
 * @brief  This API will initialize Led.
 * @param [in]   none.
 * @return       none.
 */
void vt_init_leds(void)
{
	int i = 0;

	for(i = 0; i < VT_MAX_LEDS; i++)
	{
		SIUL2->GPDO[leds[i]/4] = 0U;
		SIUL2->MSCR[leds[i]] = SIUL2_MSCR_OBE(1);
	}
	vt_all_leds_off();
}

/*!
 * @brief  This API will turn Led on.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_led_on(uint32_t led)
{
  SIUL2->GPDO[led/4] &= ~(SIUL2_GPDO_PDO_4n_WIDTH << (SIUL2_GPDO_PDO_4n_SHIFT - (8 * (led & 0x03))));
}

/*!
 * @brief  This API will turn Led off.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_led_off(uint32_t led)
{
  SIUL2->GPDO[led/4] |= (SIUL2_GPDO_PDO_4n_WIDTH << (SIUL2_GPDO_PDO_4n_SHIFT - (8 * (led & 0x03))));
}

/*!
 * @brief  This API will toggle a Led.
 * @param [in]   led - is number of GPIO Led.
 * @return       none.
 */
void vt_toggle_led(uint32_t led)
{
	uint32_t tmp;

	/* Toggle Led to check period */
	tmp = (SIUL2_GPDO_PDO_4n_WIDTH << (SIUL2_GPDO_PDO_4n_SHIFT - (8 * (led & 0x03))));
	if((tmp & SIUL2->GPDO[led/4]) > 0 )
	{
		SIUL2->GPDO[led/4] &= ~tmp;
	}
	else
	{
		SIUL2->GPDO[led/4] |= tmp;
	}
}

/*!
 * @brief  This API will turn all Led off.
 * @param [in]   none.
 * @return       none.
 */
void vt_all_leds_off(void)
{
	int i = 0;

	for(i = 0; i < VT_MAX_LEDS; i ++)
	{
		vt_led_off(leds[i]);
	}
}

/*!
 * @brief  This API will turn all Led on.
 * @param [in]   none.
 * @return       none.
 */
void vt_all_leds_on(void)
{
	int i = 0;

	for(i = 0; i < VT_MAX_LEDS; i ++)
	{
		vt_led_on(leds[i]);
	}
}

/*!
 * @brief  This API will turn Led bits on. We have 8 Leds so the Led bits has value form 0x01 to 0xFF.
 * @param [in]   ledBits - is Led bits.
 * @return       none.
 */
void vt_turn_on_bits_led(uint8_t bitrate)
{
	int i = 0;

	vt_all_leds_off();
	for(i = 0; i < VT_MAX_LEDS; i ++)
	{
		if(((bitrate >> i) & 0x01) == 0x01)
			vt_led_on(leds[i]);
	}
}


/*------------------------------------------------------------------*
 *                           Test Function                          *
 *------------------------------------------------------------------*/
void vt_test_leds(void)
{
	int i = 0;
	uint8_t led_bit = 0x01;

	vt_all_leds_off();
	vt_delay(5000000);
	vt_all_leds_on();
	vt_delay(5000000);
	vt_all_leds_off();
	vt_delay(5000000);
	vt_all_leds_on();
	vt_delay(5000000);
	vt_all_leds_off();
	for(i = 0; i < VT_MAX_LEDS; i++)
	{
		vt_turn_on_bits_led((led_bit << i));
		vt_delay(2000000);

	}
	for(i = 0; i < VT_MAX_LEDS; i++)
	{
		vt_toggle_led(leds[i]);
		vt_delay(2000000);
		vt_toggle_led(leds[i]);
		vt_delay(2000000);
	}
}

#ifdef __cplusplus
}
#endif
