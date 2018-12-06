/*
 * vt_fw_oem.h
 */

#ifndef VT_FIREWALL_VT_FW_OEM_H_
#define VT_FIREWALL_VT_FW_OEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_fw_if.h"
#include "vt_rtc.h"
#include "vt_timer.h"
#include "vt_can.h"
#include "uart_pal1.h"

/*------------------------------------------------------------------*
 *                          Define macro                            *
 *------------------------------------------------------------------*/
#define USING_GATEWAY     1
#define MPC5748G_DEVKIT 1

#define VT_MAX_CAN_NUMBER 2

/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                     Define Callback Functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                   Callback Function Prototypes                   *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                       Function Prototypes                        *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will initialize firewall.
 * @param [in]   none.
 * @return       none.
 */
void vt_fw_oem_init(void);

#ifdef USING_GATEWAY
/*!
 * @brief  This API will add CAN message to forward queue.
 * @param [in]      instant - CAN number (e.g: 0, 1, 2).
 * @param [in]      *msg - is pointer to flexcan message.
 * @return       none.
 */
void vt_fw_oem_add_message_to_forward_queue(uint8_t instant, flexcan_msgbuff_t *msg);

/*!
 * @brief  This API will get and send out a CAN message to a CAN bus.
 * @param [in]      instant - CAN number (e.g: 0, 1, 2).
 * @return       none.
 */
void vt_fw_oem_get_and_send_message(uint8_t instant);
#endif
/*------------------------------------------------------------------*
 *                Test Function and Examples                        *
 *------------------------------------------------------------------*/


/*------------------------------------------------------------------*
 *   Put example here
 *------------------------------------------------------------------*/
#ifdef __cplusplus
}
#endif



#endif /* VT_FIREWALL_VT_FW_OEM_H_ */
