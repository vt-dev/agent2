/*
 * vt_can.h
 */

#ifndef VT_CAN_H_
#define VT_CAN_H_

#ifdef __cplusplus
extern "C" {
#endif
 
/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "Cpu.h"
#include "flexcan_driver.h"
#include "vt_led.h"
#include "vt_fw_if.h"

/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/
#if USING_CAN_FD
	#define VT_CAN_MTU 64
#else
	#define VT_CAN_MTU 8
#endif

#define VT_CAN_EXTENDED_MASK_CS 0x00680000
#define VT_CAN_STANDARD_MASK_CS 0x00080000

/*! @brief Device instance number */
#define VT_INST_CAN0 (0U)
#define VT_INST_CAN1 (1U)

#define VT_MAX_FILTER_BUFFER 48  
#define VT_START_MB_IDX (VT_MAX_FILTER_BUFFER -1)     


/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/

typedef enum {
	VT_BITRATE_125 = 0,
	VT_BITRATE_250,
	VT_BITRATE_500,
	VT_BITRATE_800,
	VT_BITRATE_1M,
	VT_BITRATE_UNKNOWN
} vt_can_bitrate_type_t;

/*------------------------------------------------------------------*
 *                     Define Callback Functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/
/*! @brief Driver state structure which holds driver runtime data */
extern flexcan_state_t vt_can_State;
extern volatile uint8_t data_rcv;
extern int can_error;
extern uint32_t rxFifoFilter[VT_MAX_FILTER_BUFFER];
extern uint16_t rxfifo_count;
/*------------------------------------------------------------------*
 *                   Callback Function Prototypes                   *
 *------------------------------------------------------------------*/
/*!
 * @brief  This function will be called when the CAN interrupt occurs. You can overwrite this function in your file.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      eventType - is type of the event which occurred when the callback was invoked
 *                  (e.g: FLEXCAN_EVENT_RX_COMPLETE, FLEXCAN_EVENT_RXFIFO_COMPLETE, FLEXCAN_EVENT_TX_COMPLETE).
 * @param [in]      *flexcanState - is a pointer to flexcan driver state structure.
 * @return          none.
 */
void vt_rcv_callback(uint8_t instance, flexcan_event_type_t eventType, flexcan_state_t *flexcanState);

/*------------------------------------------------------------------*
 *                       Function Prototypes                        *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will initialize a CAN port.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @param [in]      callback - callback function.
 * @param [in]      *callbackParam - pointer to parameter.
 * @return          STATUS_SUCCESS, STATUS_FLEXCAN_MB_OUT_OF_RANGE,
 *                  or STATUS_ERROR.
 */
status_t vt_init_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate, flexcan_callback_t callback, void *callbackParam);

/*!
 * @brief  This API will re-initialize a CAN port.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @return          STATUS_SUCCESS, STATUS_FLEXCAN_MB_OUT_OF_RANGE,
 *                  or STATUS_ERROR.
 */
status_t vt_re_init_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate);

/*!
 * @brief  This API will set a bit-rate to CAN.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @return          STATUS_SUCCESS
 *                  or STATUS_ERROR.
 */
status_t vt_set_bitrate_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate);

/*!
 * @brief  This API will detect current bit rate of CAN bus.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      listen_only - enable or disable listen only foe auto detect mode.
 * @return          vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500 or VT_BITRATE_UNKNOWN).
 */
vt_can_bitrate_type_t vt_autodetect_bitrate(uint8_t inst_can, uint8_t listen_only);

/*!
 * @brief  This API will disable filter CAN message in Rxfifo mode.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_disable_filter_rxfifo(uint8_t inst_can);

/*!
 * @brief  This API will disable filter CAN message in mailbox mode.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_disable_filter_mb(uint8_t inst_can);

/*!
 * @brief  This API will apply id filter table to RxFifo CAN hardware. You should add all the filter data to the local filter
 *          array before calling this function .
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_set_filter_rxfifo(uint8_t inst_can);

/*!
 * @brief  This API will get a CAN message that it received successful.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          a pointer to a local message buffer if success.
 *                  NULL if don't have data coming.
 */
flexcan_msgbuff_t *vt_get_msg(uint8_t inst_can);

/*!
 * @brief  This API will send a CAN message.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      *msgbuff - is a pointer to flexcan message buffer structure.
 * @param [in]      id-type - is ID type of CAN (e.g: FLEXCAN_MSG_ID_STD, FLEXCAN_MSG_ID_EXT).
 * @return          STATUS_SUCCESS
 *                  or STATUS_ERROR.
 */
status_t vt_send_can_msg(uint8_t inst_can, flexcan_msgbuff_t *msgbuff, flexcan_msgbuff_id_type_t id_type);

/*!
 * @brief  This API will add a can id to Rxfifo filter table.
 * @param [in]      value - is a CAN ID to filter (e.g: 0x123, 0x750 ).
 * @return          position of can_id in RxFifo table
 *                  or -1 if error.
 */
int vt_add_can_id_to_rxfifo_filter(uint32_t value);


/*!
 * @brief  This API will clear all can id of Rxfifo table.
 * @param [in]      none .
 * @return          none
 */
void vt_clear_filter_buffer(void);

/*!
 * @brief  This API will set buffer to Rxfifo to start receive CAN message.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_start_rcv(uint8_t inst_can);

/*!
 * @brief  This API will update CAN LED for tx and rx.
 * @param [in]      none.
 * @return          none.
 */
void vt_update_can_led(void);

/*------------------------------------------------------------------*
 *                Test Function and Examples                        *
 *------------------------------------------------------------------*/
/*!
 * @brief  This function will be used to test filter buffer of Rxfifo mode.
 * 			It will call vt_add_filter_to_rxfifo function to add CAN ID from 1 to VT_MAX_FILTER_BUFFER into RX fifo filter table,
 * 			and then it will call vt_set_filter_rxfifo function to apply new filter table. Please see example 2.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return         none.
 */
void vt_test_rxfifo_filter(uint8_t inst_can);

#ifdef __cplusplus
}
#endif

#endif /* VT_CAN_H_ */
