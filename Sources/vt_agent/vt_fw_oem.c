/*
 * vt_fw_oem.c
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_fw_oem.h"
/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/


/*------------------------------------------------------------------*
 *                     Define callback functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Data Types                        *
 *------------------------------------------------------------------*/
#ifdef USING_GATEWAY
static uint8_t forward_id[VT_MAX_CAN_NUMBER] = {VT_INST_CAN1, VT_INST_CAN0};
static volatile uint8_t tx_flags[VT_MAX_CAN_NUMBER];
#endif

static vt_can_frame_t malicious_frame = {
		.msgId = 0xCD,
		.dataLen = 8,
		.data = {0xCD,0xA0,0xFF,0xFA,0x04,0x26,0x19,0x79}
};

static vt_can_frame_t frames_pattern[2] = {
		{.msgId = 0xAB,
		.dataLen = 8,
		.data = {0xCD,0xA0,0xFF,0xFA,0x04,0x26,0x19,0x79}},
		{.msgId = 0xBA,
		.dataLen = 8,
		.data = {0xCD,0xA0,0xFF,0xFA,0x04,0x26,0x19,0x79}},
};

/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                 Private Function Prototypes                      *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Functions                         *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                         Public Functions                         *
 *------------------------------------------------------------------*/

/*!
 * @brief  This API will send traffic status to report to server or print out.
 * @param [in]   car_status - is traffic status.
 * @param [in]   slot_rate - is slot rate of CAN traffic bus.
 * @param [in]   pattern_rate - is pattern rate of CAN traffic bus.
 * @param [in]   count_frames - is CAN frames of a time window.
 * @return       none.
 */
void vt_fw_traffic_status_event(vt_car_status_t car_status, float slot_rate, float pattern_rate, uint32_t count_frames)
{
	char st[256];
	int size = 0;

	memset(st,'\0', 256);

	if(car_status == VT_CAR_IDLE_STAT)
	{
		snprintf(st, 256, "The CAN bus traffic is idle\r\n");
		vt_led_off(leds[VT_BLOCK_LED]);
	}
	else
	{
		if((car_status & VT_CAR_NORMAL_STAT) == VT_CAR_NORMAL_STAT)
		{
			snprintf(st, 256, "- Slot rate: %.3f%%\r\n- Pattern rate: %.3f%% all frame: %lu\r\nThe CAN bus traffic is normal\r\n", slot_rate * 100.0f, pattern_rate * 100.0f, count_frames);
			vt_led_off(leds[VT_BLOCK_LED]);
		}

		if((car_status & VT_CAR_ABNORMAL_OVER_STAT) == VT_CAR_ABNORMAL_OVER_STAT)
		{
			snprintf(st, 256, "- Slot rate: %.3f%%\r\n- Pattern rate: %.3f%% all frame: %lu\r\nThe CAN bus traffic is abnormal - overload frames\r\n", slot_rate * 100.0f, pattern_rate * 100.0f, count_frames);
			vt_led_on(leds[VT_BLOCK_LED]);
		}

		if((car_status & VT_CAR_ABNORMAL_STAT) == VT_CAR_ABNORMAL_STAT)
		{
			snprintf(st, 256, "- Slot rate: %.3f%%\r\n- Pattern rate: %.3f%% all frame: %lu\r\nThe CAN bus traffic is abnormal\r\n", slot_rate * 100.0f, pattern_rate * 100.0f, count_frames);
			vt_led_on(leds[VT_BLOCK_LED]);
		}

		if((car_status & VT_CAR_ABNORMAL_DS_TP_STAT) == VT_CAR_ABNORMAL_DS_TP_STAT)
		{
			size = strlen(st);
			if(size == 0)
				snprintf(st, 256, "- Slot rate: %.3f%%\r\n- Pattern rate: %.3f%% all frame: %lu\r\nThe CAN bus traffic is abnormal - diagnostic\r\n", slot_rate * 100.0f, pattern_rate * 100.0f, count_frames);
			else
				snprintf(&st[size - 2], (256 - size), " - diagnostic\r\n");
			vt_led_off(leds[VT_BLOCK_LED]);
		}

		if((car_status & VT_CAR_ABNORMAL_MALICIOUS) == VT_CAR_ABNORMAL_MALICIOUS)
		{
			size = strlen(st);
			if(size == 0)
				snprintf(st, 256, "- Slot rate: %.3f%%\r\n- Pattern rate: %.3f%% all frame: %lu\r\nThe CAN bus traffic is abnormal - malicious\r\n", slot_rate * 100.0f, pattern_rate * 100.0f, count_frames);
			else
				snprintf(&st[size - 2], (256 - size), " - malicious\r\n");
			vt_led_off(leds[VT_BLOCK_LED]);
		}

		if((car_status & VT_CAR_IDLE_STAT) == VT_CAR_IDLE_STAT)
		{
			size = strlen(st);
			snprintf(&st[size -2 ], (256 - size), " - idle\r\n");
		}
	}
	UART_SendDataBlocking(INST_UART_PAL1, (const uint8_t*)st, strlen(st),30);
}

/*!
 * @brief  This API will send matched of vector data to report to server or print out.
 * @param [in]   *vector_t - pointer to vt_vector_result_t structure.
 * @return       status.
 */
vt_status_t vt_fw_vector_report_matched(vt_vector_result_t *vector_t)
{
	char st[256];

	if(vector_t == NULL)
		return VT_STATUS_NULL;

	memset(st,'\0', 256);
	if(vector_t->matched_flag > 0)
	{
		snprintf(st, 256, "- Vector rate: %lu/%lu = %.3f%% - all vectors: %lu\r\n", vector_t->count_vector_in_rl, vector_t->count_vector_in_rt, vector_t->matched_rate * 100.0f, vector_t->count_all_vector);
	}
	else
	{
		if(vector_t->matched_rate >= 0.96f)
			snprintf(st, 256, "- Vector rate: %lu/%lu = %.3f%% - all vectors: %lu is too small\r\n", vector_t->count_vector_in_rl, vector_t->count_vector_in_rt, vector_t->matched_rate, vector_t->count_all_vector);
		else
			snprintf(st, 256, "- Vector rate: %lu/%lu = %.3f%% - all vectors: %lu\r\n", vector_t->count_vector_in_rl, vector_t->count_vector_in_rt, vector_t->matched_rate * 100.0f, vector_t->count_all_vector);
	}
	UART_SendDataBlocking(INST_UART_PAL1, (const uint8_t*)st, strlen(st),30);

	return VT_STATUS_SUCCESS;
}

/*!
 * @brief  This API will send matched of blacklist data to report to server or print out.
 * @param [in]   *detail_result - pointer to vt_fw_detail_result_t structure.
 * @return       status.
 */
vt_status_t vt_fw_blacklist_report_matched(vt_fw_detail_result_t *detail_result)
{
	if(detail_result == NULL)
		return VT_STATUS_NULL;

	if(detail_result->matched_type > 0)
		UART_SendDataBlocking(INST_UART_PAL1, (const uint8_t*)detail_result->detail, strlen(detail_result->detail),30);

	return VT_STATUS_SUCCESS;
}

/*!
 * @brief  This API will send matched of monitor data to report to server or print out.
 * @param [in]   *detail_result - pointer to vt_fw_detail_result_t structure.
 * @return       status.
 */
vt_status_t vt_fw_monitor_report_matched(vt_fw_detail_result_t *detail_result)
{
	if(detail_result == NULL)
			return VT_STATUS_NULL;

	if(detail_result->matched_type > 0)
		UART_SendDataBlocking(INST_UART_PAL1, (const uint8_t*)detail_result->detail, strlen(detail_result->detail),30);

	return VT_STATUS_SUCCESS;
}

/*!
 * @brief  This API will initialize firewall.
 * @param [in]   none.
 * @return       none.
 */
void vt_fw_oem_init(void)
{
	int i;

	/* Initialize firewall */
	vt_fw_init(car_policy, car_vector);

#ifdef USING_GATEWAY

	vt_fw_create_tx_queue(VT_INST_CAN0, 256);
	vt_fw_create_tx_queue(VT_INST_CAN1, 256);
	/* Clear tx_flags */
	for(i = 0; i < VT_MAX_CAN_NUMBER; i++)
	{
		tx_flags[i] = 0;
	}
#endif
	/* Add a malicious CAN frame */
	vt_fw_add_malicious_can_frame(malicious_frame.msgId, malicious_frame.dataLen, malicious_frame.data);
	/* Add monitor a CAN frame with operator = 0 */
	vt_fw_monitor_add_can_frame(frames_pattern[0].msgId, frames_pattern[0].dataLen, frames_pattern[0].data, 0, 1,  10);
	vt_fw_monitor_add_can_frame(frames_pattern[1].msgId, frames_pattern[1].dataLen, frames_pattern[1].data, 1, 0,  10);
	vt_fw_monitor_add_pattern(frames_pattern, 2, 0, 1, 10);
	vt_fw_monitor_add_ids_to_range_list(0, 0x600, 0x6ff, 1, 0,  100);
	/* Set slot to rule */
	vt_fw_set_slot_time_unit(VT_PIT_PERIOD);

	vt_fw_install_vector_callback(vt_fw_vector_report_matched);
	vt_fw_install_traffic_status_callback(vt_fw_traffic_status_event);
	vt_fw_install_blacklist_callback(vt_fw_blacklist_report_matched);
	vt_fw_install_monitor_callback(vt_fw_monitor_report_matched);
	vt_led_off(leds[VT_BLOCK_LED]);
}

#ifdef USING_GATEWAY
/*!
 * @brief  This API will add CAN message to forward queue.
 * @param [in]      instant - CAN number (e.g: 0, 1, 2).
 * @param [in]      *msg - is pointer to flexcan message.
 * @return       none.
 */
void vt_fw_oem_add_message_to_forward_queue(uint8_t instant, flexcan_msgbuff_t *msg)
{
	status_t result = STATUS_ERROR;
	int mb_idx;
	static flexcan_data_info_t dataInfo =
	{
		.data_length = 1U,
		.msg_id_type = FLEXCAN_MSG_ID_STD,
		.enable_brs  = false,
		.fd_enable   = false,
		.is_remote = false,
		.fd_padding  = 0U
	};

	if(instant >= VT_MAX_CAN_NUMBER)
		return;
	if(tx_flags[forward_id[instant]] == 0)
	{
		/* Send direct */
		dataInfo.data_length = (uint32_t)msg->dataLen;

		mb_idx = VT_START_MB_IDX;
		result = FLEXCAN_DRV_ConfigTxMb(forward_id[instant], mb_idx, (const flexcan_data_info_t *)&dataInfo, msg->msgId);
		if(result == STATUS_SUCCESS)
		{
			result = FLEXCAN_DRV_Send(forward_id[instant], mb_idx, (const flexcan_data_info_t *)&dataInfo, msg->msgId,(const uint8_t *) &msg->data[0]);
			if(result == STATUS_SUCCESS)
				tx_flags[forward_id[instant]] = 1;
		}

	}
	else
	{
		vt_fw_add_msg_to_tx_queue(forward_id[instant], msg->msgId, msg->dataLen, msg->data);
	}
}

/*!
 * @brief  This API will get and send out a CAN message to a CAN bus.
 * @param [in]   instant - CAN number (e.g: 0, 1, 2).
 * @return       none.
 */
void vt_fw_oem_get_and_send_message(uint8_t instant)
{
	vt_status_t status;
	flexcan_msgbuff_t msg;
	status_t result = STATUS_ERROR;
	int mb_idx;
	static flexcan_data_info_t dataInfo =
	{
		.data_length = 1U,
		.msg_id_type = FLEXCAN_MSG_ID_STD,
		.enable_brs  = false,
		.fd_enable   = false,
		.is_remote = false,
		.fd_padding  = 0U
	};

	status = vt_fw_get_msg_from_tx_queue(instant, &msg.msgId, &msg.dataLen, msg.data);
	if(status == VT_STATUS_SUCCESS)
	{
		dataInfo.data_length = (uint32_t)msg.dataLen;

		mb_idx =  VT_START_MB_IDX;
		result = FLEXCAN_DRV_ConfigTxMb(instant, mb_idx, (const flexcan_data_info_t *)&dataInfo, msg.msgId);
		if(result == STATUS_SUCCESS)
		{
			result = FLEXCAN_DRV_Send(instant, mb_idx, (const flexcan_data_info_t *)&dataInfo, msg.msgId,(const uint8_t *) &msg.data[0]);
			if(result == STATUS_SUCCESS)
				tx_flags[instant] = 1;
		}
	}
	else
	{
		tx_flags[instant] = 0;
	}
}
#endif
/*------------------------------------------------------------------*
 *                       Test Function                              *
 *------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif
