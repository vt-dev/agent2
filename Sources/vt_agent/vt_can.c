/*
 * vt_can.c
 *
 */

/*!
 *  @addtogroup can_module can module documentation
 *  @{
 */
/*------------------------------------------------------------------*
 *                           Includes                               *
 *------------------------------------------------------------------*/
#include "vt_can.h"
#include "vt_timer.h"
#include "vt_fw_oem.h"

/*------------------------------------------------------------------*
 *                          Define Macro                            *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                Define Enumeration and Structure                  *
 *------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

/*------------------------------------------------------------------*
 *                     Define callback functions                    *
 *------------------------------------------------------------------*/

/*------------------------------------------------------------------*
 *                        Private Data Types                        *
 *------------------------------------------------------------------*/
flexcan_msgbuff_t msg_buff[2];
static volatile uint8_t active_buff = 0;

static const flexcan_time_segment_t bitRateTable[] = {
    { 7, 4, 1, 19, 1},  /* 125 kHz */
    { 7, 4, 1,  9, 1},  /* 250 kHz */
    { 7, 4, 1,  4, 1 }, /* 500 kHz */
    { 4, 1, 1,  4, 1},  /* 800 kHz */
    { 7, 6, 3,  1, 1},  /* 1   MHz */
};
/* PE clock 40MHz bitRate for can fd */
static const flexcan_time_segment_t bitRateCbtTable[] = {
	{ 7, 4, 1, 19, 1},  /* 125 kHz */
	{ 7, 4, 1,  9, 1},  /* 250 kHz */
	{ 7, 4, 1,  4, 1 }, /* 500 kHz */
	{ 4, 1, 1,  4, 1},  /* 800 kHz */
	{ 7, 6, 3,  1, 1},  /* 1   MHz */
};

static flexcan_user_config_t vt_can_InitConfig = {
    .fd_enable = false,
    .pe_clock = FLEXCAN_CLK_SOURCE_FXOSC,
    .max_num_mb = 48,
    .num_id_filters = FLEXCAN_RX_FIFO_ID_FILTERS_48,
    .is_rx_fifo_needed = true,
    .flexcanMode = FLEXCAN_NORMAL_MODE,
    .payload = FLEXCAN_PAYLOAD_SIZE_8,
    .bitrate = {
        .propSeg = 7,
        .phaseSeg1 = 4,
        .phaseSeg2 = 1,
        .preDivider = 4,
        .rJumpwidth = 1
    },
    .bitrate_cbt = {
        .propSeg = 7,
        .phaseSeg1 = 4,
        .phaseSeg2 = 1,
        .preDivider = 4,
        .rJumpwidth = 1
    },
    .transfer_type = FLEXCAN_RXFIFO_USING_INTERRUPTS,
    .rxFifoDMAChannel = 0U
};


static flexcan_id_table_t id_filter_table = {.idFilter = rxFifoFilter, .isExtendedFrame = 0, .isRemoteFrame = 0};
static volatile uint8_t tx_led = 0, rx_led = 0;
/*------------------------------------------------------------------*
 *                        Global Data Types                         *
 *------------------------------------------------------------------*/
flexcan_state_t vt_can_State;
int can_error = 0;
uint32_t rxFifoFilter[VT_MAX_FILTER_BUFFER];
uint16_t rxfifo_count = 0;
/*------------------------------------------------------------------*
 *                 Private Function Prototypes                      *
 *------------------------------------------------------------------*/
static inline flexcan_msgbuff_t * _vt_get_msg(uint8_t inst_can);
static inline int _vt_can_bsearch(uint32_t *id_table, int size, uint32_t can_id);

/*------------------------------------------------------------------*
 *                    Callback Functions                            *
 *------------------------------------------------------------------*/
void vt_rcv_callback(uint8_t instance, flexcan_event_type_t eventType, flexcan_state_t *flexcanState)
{
	flexcan_msgbuff_t * msg = NULL;
	(void)flexcanState;

	switch(eventType)
	{
	case FLEXCAN_EVENT_RXFIFO_COMPLETE:
		msg = _vt_get_msg(instance);
#ifdef USING_GATEWAY
		if(vt_fw_can_msg_is_malicious(msg->msgId, msg->dataLen, msg->data) == 0)
			vt_fw_oem_add_message_to_forward_queue(instance, msg);
#endif

		vt_fw_rcv_msg(msg->msgId, msg->dataLen, msg->data);
		rx_led = 1;
		break;
	case FLEXCAN_EVENT_TX_COMPLETE:
#ifdef USING_GATEWAY
		vt_fw_oem_get_and_send_message(instance);
#endif
		tx_led = 1;
		break;
	default:
		break;
	}
}


/*------------------------------------------------------------------*
 *                        Private Functions                         *
 *------------------------------------------------------------------*/
static int vt_can_id_compare(const void * a, const void * b)
{
	return (int)(*(uint32_t *)a - *(uint32_t *)b);
}

/*!
 * @brief  This API will search can_id in can id array.
 * @param [in]      *id_table - pointer to can id table.
 * @param [in]      size - size of can id table
 * @param [in]      can_id - is a CAN ID to search in can id table (e.g: 0x123, 0x750 ).
 * @return          position in can id table
 *                  -1.
 */
static inline int _vt_can_bsearch(uint32_t *id_table, int size, uint32_t can_id)
{
   int front = 0, rear = 0, mid = 0;

   if((size <= 0) || (id_table == NULL))
	   return -1;
   rear = size - 1;

   while(front <= rear) {
	  mid = (front + rear)/2;
	  if(id_table[mid] == can_id)
         break;
      else if(id_table[mid] < can_id)
         front = mid + 1;
      else
         rear = mid - 1;
   }

   if (front > rear)
     return -1;
   return mid;
}


/*!
 * @brief  This API will get a CAN message that it received successful.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          a pointer to a local message buffer if success.
 *                  NULL if don't have data coming.
 */
static inline flexcan_msgbuff_t * _vt_get_msg(uint8_t inst_can)
{
	flexcan_msgbuff_t * msg = NULL;

	msg = &msg_buff[active_buff];
	active_buff = !active_buff;
	FLEXCAN_DRV_RxFifo(inst_can, &msg_buff[active_buff]);
	return msg;
}

/*------------------------------------------------------------------*
 *                         Public Functions                         *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will get a CAN message that it received successful.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          a pointer to a local message buffer if success.
 *                  NULL if don't have data coming.
 */
flexcan_msgbuff_t * vt_get_msg(uint8_t inst_can)
{
	flexcan_msgbuff_t * msg = NULL;

	msg = &msg_buff[active_buff];
	active_buff = !active_buff;
	FLEXCAN_DRV_RxFifo(inst_can, &msg_buff[active_buff]);
	return msg;
}

/*!
 * @brief  This API will initialize a CAN port.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @param [in]      callback - callback function.
 * @param [in]      *callbackParam - pointer to parameter.
 * @return          STATUS_SUCCESS, STATUS_FLEXCAN_MB_OUT_OF_RANGE,
 *                  or STATUS_ERROR.
 */
status_t vt_init_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate, flexcan_callback_t callback, void *callbackParam )
{
	status_t result = STATUS_ERROR;
	vt_can_bitrate_type_t btr = VT_BITRATE_500;

	if(bitrate < VT_BITRATE_UNKNOWN)
		btr = bitrate;
	vt_clear_filter_buffer();
	vt_can_State.callback = callback;
	vt_can_State.callbackParam = callbackParam;

	vt_can_InitConfig.bitrate = bitRateTable[(int)btr];

	result = FLEXCAN_DRV_Init(inst_can, &vt_can_State, (const flexcan_user_config_t *)&vt_can_InitConfig);

	FLEXCAN_DRV_RxFifo(inst_can, &msg_buff[active_buff]);

	return result;
}

/*!
 * @brief  This API will re-initialize a CAN port.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @return          STATUS_SUCCESS, STATUS_FLEXCAN_MB_OUT_OF_RANGE,
 *                  or STATUS_ERROR.
 */
status_t vt_re_init_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate)
{
	vt_can_bitrate_type_t btr = VT_BITRATE_500;
	status_t result = STATUS_ERROR;

	if(bitrate < VT_BITRATE_UNKNOWN)
		btr = bitrate;
	vt_can_InitConfig.bitrate = bitRateTable[(int)btr];
	result = FLEXCAN_DRV_Init(inst_can, &vt_can_State, (const flexcan_user_config_t *)&vt_can_InitConfig);

	FLEXCAN_DRV_RxFifo(inst_can, &msg_buff[active_buff]);
	return result;
}

/*!
 * @brief  This API will set a bit-rate to CAN.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      bitrate - is a CAN bit rate in vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500).
 * @return          STATUS_SUCCESS
 *                  or STATUS_ERROR.
 */
status_t vt_set_bitrate_can(uint8_t inst_can, vt_can_bitrate_type_t bitrate)
{
	status_t result = STATUS_ERROR;

	if(bitrate < VT_BITRATE_UNKNOWN)
	{
		FLEXCAN_DRV_SetBitrate(inst_can, (const flexcan_time_segment_t *) &bitRateTable[(int)bitrate]);
		result = STATUS_SUCCESS;
	}

	return result;
}

/*!
 * @brief  This API will disable filter CAN message in Rxfifo mode.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_disable_filter_rxfifo(uint8_t inst_can)
{
	FLEXCAN_DRV_SetRxFifoGlobalMask(inst_can, FLEXCAN_MSG_ID_EXT, 0x00000000);
}

/*!
 * @brief  This API will disable filter CAN message in mailbox mode.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_disable_filter_mb(uint8_t inst_can)
{
	FLEXCAN_DRV_SetRxMbGlobalMask(inst_can, FLEXCAN_MSG_ID_EXT, 0x00000000);
}

/*!
 * @brief  This API will apply id filter table to RxFifo CAN hardware. You should add all the filter data to the local filter
 *          array before calling this function .
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_set_filter_rxfifo(uint8_t inst_can)
{
	FLEXCAN_DRV_SetRxFifoGlobalMask(inst_can, FLEXCAN_MSG_ID_EXT, 0x1FFFFFFF);
	FLEXCAN_DRV_ConfigRxFifo(inst_can, FLEXCAN_RX_FIFO_ID_FORMAT_A, (const flexcan_id_table_t *)&id_filter_table);
}

/*!
 * @brief  This API will detect current bit rate of CAN bus.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      listen_only - enable or disable listen only foe auto detect mode.
 * @return          vt_can_bitrate_type_t (e.g: VT_BITRATE_125, VT_BITRATE_500 or VT_BITRATE_UNKNOWN).
 */
vt_can_bitrate_type_t vt_autodetect_bitrate(uint8_t inst_can, uint8_t listen_only)
{
	status_t result = STATUS_ERROR;
	int i = 0, mb_idx = 0, h = 0;
	flexcan_msgbuff_t recvBuff;
	uint8_t data = 0xDC;
	flexcan_data_info_t dataInfo =
	  {
	    .data_length = 1U,
	    .msg_id_type = FLEXCAN_MSG_ID_STD,
	    .enable_brs  = true,
	    .fd_enable   = false,
	    .fd_padding  = 0U
	  };
	vt_can_bitrate_type_t bitrate[VT_BITRATE_UNKNOWN] = {VT_BITRATE_500, VT_BITRATE_125, VT_BITRATE_250, VT_BITRATE_800, VT_BITRATE_1M };

	for(i = 0; i < (int)VT_BITRATE_UNKNOWN; i++)
	{

		result = vt_re_init_can(inst_can, bitrate[i]);
		//result = vt_set_bitrate_can(inst_can, bitrate[i]);
		if(result == STATUS_SUCCESS)
		{
	
			vt_disable_filter_rxfifo(inst_can);  /*! Disable filter to receive all coming CAN message */

			for(h = 0; h < 5; h++)
			{
				if( FLEXCAN_DRV_RxFifoBlocking(inst_can, &recvBuff,20) == STATUS_SUCCESS)
					return bitrate[i];
			}
			if(!listen_only)
			{
				mb_idx = vt_can_InitConfig.max_num_mb - 1;
				if(FLEXCAN_DRV_ConfigTxMb(inst_can, mb_idx, &dataInfo, 1) == STATUS_SUCCESS)
				{
					result = FLEXCAN_DRV_Send(inst_can, mb_idx, &dataInfo, 1, &data);
					if((result == STATUS_SUCCESS) || (result == STATUS_BUSY))
					{
						h = 0;
						do{
							result = FLEXCAN_DRV_GetTransferStatus(inst_can, mb_idx);
							if(result == STATUS_SUCCESS)
							{
								return bitrate[i];
							}
						} while(++h < 10000);
					}

					FLEXCAN_DRV_AbortTransfer(inst_can, mb_idx);
				}

			}
		}
	}
	return VT_BITRATE_UNKNOWN;
}

/*!
 * @brief  This API will send a CAN message.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @param [in]      *msgbuff - is a pointer to flexcan message buffer structure.
 * @param [in]      id-type - is ID type of CAN (e.g: FLEXCAN_MSG_ID_STD, FLEXCAN_MSG_ID_EXT).
 * @return          STATUS_SUCCESS
 *                  or STATUS_ERROR.
 */
status_t vt_send_can_msg(uint8_t inst_can, flexcan_msgbuff_t *msgbuff, flexcan_msgbuff_id_type_t id_type)
{
	int i = 0, z = 0;
	status_t result = STATUS_ERROR;

	static flexcan_data_info_t dataInfo =
	{
		.data_length = 1U,
		.msg_id_type = FLEXCAN_MSG_ID_STD,
		.enable_brs  = false,
		.fd_enable   = false,
		.is_remote = false,
		.fd_padding  = 0U
	};

	if(msgbuff == NULL)
		return result;
	dataInfo.data_length = (uint32_t)msgbuff->dataLen;
	dataInfo.msg_id_type = id_type;

	for(i = VT_START_MB_IDX; i < vt_can_InitConfig.max_num_mb; i++)
	{
		result = FLEXCAN_DRV_ConfigTxMb(inst_can, i, (const flexcan_data_info_t *)&dataInfo, msgbuff->msgId);
		if(result == STATUS_SUCCESS)
		{
			result = FLEXCAN_DRV_Send(inst_can, i, (const flexcan_data_info_t *)&dataInfo, msgbuff->msgId,(const uint8_t *) &msgbuff->data[0]);
			if((result == STATUS_SUCCESS) || (result == STATUS_BUSY))
			{
				z = 0;
				do{
					result = FLEXCAN_DRV_GetTransferStatus(inst_can, i);
					if(result == STATUS_SUCCESS)
					{
						vt_toggle_led(leds[VT_CAN_TX_LED]);
						return result;
					}
				} while(++z < 10000);
			}
			FLEXCAN_DRV_AbortTransfer(inst_can, i);
			can_error = -1;
		}
	}
	return result;
}

/*!
 * @brief  This API will add a can id to id filter table.
 * @param [in]      value - is a CAN ID to filter (e.g: 0x123, 0x750 ).
 * @return          position of can_id in RxFifo table
 *                  or -1 if error.
 */
int vt_add_can_id_to_rxfifo_filter(uint32_t value)
{
	int i = -1,z;

	i = _vt_can_bsearch(rxFifoFilter, rxfifo_count, value);
	if(i < 0)
	{
		if(rxfifo_count < VT_MAX_FILTER_BUFFER)
		{
			rxFifoFilter[rxfifo_count] = value;
			rxfifo_count++;
			qsort(rxFifoFilter, rxfifo_count, sizeof(uint32_t), vt_can_id_compare);
			i = _vt_can_bsearch(rxFifoFilter, rxfifo_count, value);
		}
	}

	for(z = rxfifo_count; z < VT_MAX_FILTER_BUFFER; z++)
	{
		rxFifoFilter[z] = value;
	}
	return i;
}

/*!
 * @brief  This API will clear all can id of Rxfifo table.
 * @param [in]      none .
 * @return          none
 */
void vt_clear_filter_buffer(void)
{
	int i = 0;

	for(i = 0; i < VT_MAX_FILTER_BUFFER; i++)
	{
		rxFifoFilter[i] = 0x00000000;
	}
	rxfifo_count = 0;
}

/*!
 * @brief  This API will set buffer to Rxfifo to start receive CAN message.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return          none.
 */
void vt_start_rcv(uint8_t inst_can)
{
	  FLEXCAN_DRV_RxFifo(inst_can, &msg_buff[active_buff]);
}

/*!
 * @brief  This API will update CAN LED for tx and rx.
 * @param [in]      none.
 * @return          none.
 */
void vt_update_can_led(void)
{
	static int count_time = 0;

	if(++count_time > 1000)
	{
		if(rx_led)
		{
			vt_toggle_led(leds[VT_CAN_RX_LED]);
			rx_led = 0;
		}
		else
		{
			vt_led_off(leds[VT_CAN_RX_LED]);
		}
		if(tx_led)
		{
			vt_toggle_led(leds[VT_CAN_TX_LED]);
			tx_led = 0;
		}
		else
		{
			vt_led_off(leds[VT_CAN_TX_LED]);
		}
		count_time = 0;
	}
}

/*------------------------------------------------------------------*
 *                       Test Function                              *
 *------------------------------------------------------------------*/
/*!
 * @brief  This API will be used to test filter buffer of Rxfifo mode.
 * 			It will call vt_add_filter_to_rxfifo function to add CAN ID from 1 to VT_MAX_FILTER_BUFFER into RX fifo filter table,
 * 			and then it will call vt_set_filter_rxfifo function to apply new filter table.
 * @param [in]      inst_can - CAN number (e.g: 0, 1, 2).
 * @return         none.
 */
void vt_test_rxfifo_filter(uint8_t inst_can)
{
	uint32_t can_id = 1;
	int i = 0;

	for(i = 0; i < VT_MAX_FILTER_BUFFER; i++)
	{
		vt_add_can_id_to_rxfifo_filter(can_id++);
	}
	vt_set_filter_rxfifo(inst_can);
}

#ifdef __cplusplus
}
#endif
/*!
 * @}
 */
/* END vt_can. */
