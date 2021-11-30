/**
 ******************************************************************************
 * File Name          : one_wire.c
 * Date               : 17/11/2016 00:59:00
 * Description        : one wire communication modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rc522.h"
#include "owire.h"

#if (__OW_H__ != FW_BUILD)
    #error "onewire header version mismatch"
#endif

/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Type --------------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
/* Private Variable ----------------------------------------------------------*/
uint8_t ow_bps;
uint16_t ow_bcnt;
uint32_t ow_flags;
uint32_t ow_rxtmr;
uint32_t ow_txtmr;
uint32_t ow_rxtout;
uint32_t ow_txtout;
uint32_t owtmr;
uint32_t owtout;
static uint8_t ow_rxbuf[OW_BSIZE];
static uint8_t ow_txbuf[OW_BSIZE];
/* Constants ----------------------------------------------------------------*/
/* Private Macro -------------------------------------------------------------*/
/* Private Function Prototype ------------------------------------------------*/
static void
OW_SetUsart(uint8_t brate, uint8_t bsize);
void
OW_Autobaud(void);
/* Program Code  -------------------------------------------------------------*/
/**
 * @brief
 * @param
 * @retval
 */
void OW_Init(void)
{
//	OW_Autobaud();
	OW_SetUsart(BR_9600, WL_9BIT);
	do
	{
		SET_BIT(huart1.Instance->RQR, UART_RXDATA_FLUSH_REQUEST);
	}
	while (0U);
	HAL_UART_Receive_IT(&huart1, ow_rxbuf, 1);
	ow_rxtmr = HAL_GetTick();
	ow_rxtout = REC_TOUT;
	COM_State = RX_START;
	COM_Link = NOLINK;
	ow_bcnt = 0U;
}
/**
 * @brief
 * @param
 * @retval
 */
void OW_Service(void)
{

	if (COM_State != RX_LOCK)
	{
		if ((HAL_GetTick() - ow_rxtmr) >= ow_rxtout) OW_Init();
	}

	if (COM_State == RX_LOCK)
	{
		if (huart1.gState != HAL_UART_STATE_READY)
		{
			if ((HAL_GetTick() - ow_txtmr) >= ow_txtout) COM_State = RX_ERROR;
		}
		else OW_Init();
	}
	else if (COM_State == RX_READY)
	{
		HAL_Delay(2);
		ZEROFILL(ow_txbuf, COUNTOF(ow_txbuf));
		ow_txbuf[3] = 2;  // set two byte response data size
		ow_txbuf[4] = NAK;  // set default response to Negativ Acknowledge
		ow_txbuf[5] = ow_rxbuf[4];  // echo command as default
		/** ==========================================================================*/
		/**         O N E W I R E       P A C K E T         R E C E I V E D           */
		/** ==========================================================================*/
		switch (ow_rxbuf[4])
		{
			case GET_SYS_STAT:
			{
				ow_txbuf[3] = 4;
				ow_txbuf[4] = ow_flags & 0xFF; // new card data ready,  card in stacker, rtc time valid
				ow_txbuf[5] = 0; // all output off
				if (HAL_GPIO_ReadPin(LED_1_GPIO_Port, LED_1_Pin) == GPIO_PIN_RESET) ow_txbuf[5] |= (1U << 0);
				if (HAL_GPIO_ReadPin(LED_2_GPIO_Port, LED_2_Pin) == GPIO_PIN_RESET) ow_txbuf[5] |= (1U << 1);
				if (HAL_GPIO_ReadPin(LED_3_GPIO_Port, LED_3_Pin) == GPIO_PIN_RESET) ow_txbuf[5] |= (1U << 2);
				if (HAL_GPIO_ReadPin(LED_4_GPIO_Port, LED_4_Pin) == GPIO_PIN_RESET) ow_txbuf[5] |= (1U << 3);
				//if (HAL_GPIO_ReadPin(BUZZER_GPIO_Port, BUZZER_Pin) ==GPIO_PIN_SET) ow_txbuf[5] |= (1U << 4);
				ow_txbuf[6] = 0; // optional NTC temperature sensor LSB
				ow_txbuf[7] = 0; // optional NTC temperature sensor LSB
				break;
			}

			case GET_APPL_STAT:
			{
				ow_txbuf[3] = 44;
				ow_txbuf[4] = ACK;
				//ow_txbuf[5] = ow_rxbuf[4];
				mem_cpy(&ow_txbuf[6], sys_id, 2);
				mem_cpy(&ow_txbuf[8], mifare_keya, 6);
				mem_cpy(&ow_txbuf[14], mifare_keyb, 6);
				mem_cpy(&ow_txbuf[20], user_group, 16);
				ow_txbuf[36] = 'C';
				ow_txbuf[37] = 'S';
				ow_txbuf[44] = version >> 16;
				ow_txbuf[45] = version >> 8;
				ow_txbuf[46] = version;
				Hex2Str((char*) &ow_txbuf[38], &ow_txbuf[44], 6);
				ow_txbuf[44] = '1';
				ow_txbuf[45] = '2';
				ow_txbuf[46] = '3';
				ow_txbuf[47] = '4';
				ow_txbuf[48] = '\0';
				break;
			}

			case SET_APPL_STAT:
			{
				mem_cpy(sys_id, &ow_rxbuf[5], 2);
				mem_cpy(mifare_keya, &ow_rxbuf[7], 6);
				mem_cpy(mifare_keyb, &ow_rxbuf[13], 6);
				mem_cpy(user_group, &ow_rxbuf[19], 16);
				ow_txbuf[4] = ACK;
				break;
			}

			case SET_RTC_DATE_TIME:
			{
				sDate.WeekDay = ow_rxbuf[5];
				if (sDate.WeekDay == 0) sDate.WeekDay = 7;
				sDate.Date = ow_rxbuf[6];
				sDate.Month = ow_rxbuf[7];
				sDate.Year = ow_rxbuf[8];
				sTime.Hours = ow_rxbuf[9];
				sTime.Minutes = ow_rxbuf[10];
				sTime.Seconds = ow_rxbuf[11];
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
				HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);
				RtcValidSet();
				ow_txbuf[4] = ACK;
				break;
			}

			case GET_CARD_BLOCK:
			{
				if (RC522_ReadBlock(ow_rxbuf[5], &ow_rxbuf[6]) == 0) ow_txbuf[4] = ACK;
				break;
			}

			case SET_CARD_BLOCK:
			{
				if (RC522_WriteBlock(ow_rxbuf[5], &ow_rxbuf[6]) == 0) ow_txbuf[4] = ACK;
				break;
			}

			case GET_CARD_DATA:
			{
				CardReadyReset();
				ow_txbuf[3] = 17;
				ow_txbuf[4] = sCard.card_status;
				ow_txbuf[5] = sCard.user_group;
				ow_txbuf[6] = sCard.system_id >> 8;
				ow_txbuf[7] = sCard.system_id & 0xFF;
				ow_txbuf[8] = sCard.controller_id >> 8;
				ow_txbuf[9] = sCard.controller_id & 0xFF;
				mem_cpy(&ow_txbuf[10], &sCard.expiry_time, 6);
				mem_cpy(&ow_txbuf[16], &sCard.card_id, 5);
				break;
			}

			case SET_DOUT_STATE:
			case RESTART_CTRL:
			default:
			{
				ow_txbuf[4] = NAK;
				break;
			}
		}
		/**
		 ***********************************************************************************
		 *  send response to direct request send to this interface address and to request
		 *  for bus controller to resolve addresse if this device is bus master controller
		 ***********************************************************************************
		 */
		if (COM_Link == P2P)
		{
			CRC_ResetDR();
			ow_txbuf[0] = STX;
			ow_txbuf[1] = ow_rxbuf[2];
			ow_txbuf[2] = ow_rxbuf[1];
			ow_txbuf[ow_txbuf[3] + 4] = (uint8_t) CRC_Calculate8(ow_txbuf, ow_txbuf[3] + 4);
			HAL_UART_Transmit_IT(&huart1, ow_txbuf, ow_txbuf[3] + 5);
		}
		COM_State = RX_LOCK;
		ow_txtout = OW_PKTIME;
		ow_txtmr = HAL_GetTick();
		if (ow_rxbuf[4] == RESTART_CTRL) NVIC_SystemReset();

	}
	else if (COM_State == RX_ERROR)
	{
		OW_Init();
	}
	else if (COM_State == RX_INIT)
	{
		OW_Init();
	}
}

/**
 * @brief
 * @param
 * @retval
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	uint8_t crc8 = 0;

	switch (COM_State)
	{
		case RX_START:
			OW_SetUsart(BR_9600, WL_8BIT);
			if (ow_rxbuf[ow_bcnt] == STX) COM_State = RX_RECADDR;
			else COM_State = RX_INIT;
			break;
		case RX_RECADDR:
			if (ow_rxbuf[ow_bcnt] == DEF_CR_OWIFA) COM_Link = P2P;
			else if (ow_rxbuf[ow_bcnt] == DEF_CR_OWGRA) COM_Link = GROUP;
			else if (ow_rxbuf[ow_bcnt] == DEF_OWBRA) COM_Link = BROADCAST;
			if (COM_Link == NOLINK) COM_State = RX_INIT;
			else COM_State = RX_SNDADDR;
			break;
		case RX_SNDADDR:
			COM_State = RX_SIZE;
			break;
		case RX_SIZE:
			COM_State = RX_PAYLOAD;
			break;
		case RX_PAYLOAD:
			if (ow_rxbuf[3] == (ow_bcnt - 3)) COM_State = RX_CRC8;
			break;
		case RX_CRC8:
			__HAL_CRC_DR_RESET(&hcrc);
			crc8 = CRC_Calculate8(ow_rxbuf, ow_bcnt);
			if (crc8 == ow_rxbuf[ow_bcnt]) COM_State = RX_READY;
			else COM_State = RX_INIT;
		case RX_READY:
		case RX_LOCK:
		case RX_ERROR:
		case RX_INIT:
			break;
	}

	if (COM_State <= RX_READY) return;
	++ow_bcnt;
	ow_rxtout = RX_TOUT;
	ow_rxtmr = HAL_GetTick();
	HAL_UART_Receive_IT(&huart1, &ow_rxbuf[ow_bcnt], 1);
}
/**
 * @brief
 * @param
 * @retval
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	__HAL_UART_CLEAR_PEFLAG(huart);
	__HAL_UART_CLEAR_FEFLAG(huart);
	__HAL_UART_CLEAR_NEFLAG(huart);
	__HAL_UART_CLEAR_IDLEFLAG(huart);
	__HAL_UART_CLEAR_OREFLAG(huart);
	do
	{
		SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);
	}
	while (0U);
	huart->ErrorCode = HAL_UART_ERROR_NONE;
	COM_State = RX_ERROR;
}
/**
 * @brief
 * @param
 * @retval
 */
static void OW_SetUsart(uint8_t brate, uint8_t bsize)
{
	huart1.Instance = USART1;
	HAL_UART_DeInit(&huart1);
	huart1.Init.BaudRate = bps[brate];
	if (bsize == WL_9BIT) huart1.Init.WordLength = UART_WORDLENGTH_9B;
	else if (bsize == WL_8BIT) huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_SWAP_INIT;
	huart1.AdvancedInit.Swap = UART_ADVFEATURE_SWAP_ENABLE;
	if (bsize == WL_9BIT)
	{
		if (HAL_MultiProcessor_Init(&huart1, STX, UART_WAKEUPMETHOD_ADDRESSMARK) != HAL_OK) Error_Handler();
		HAL_MultiProcessor_EnableMuteMode(&huart1);
		HAL_MultiProcessor_EnterMuteMode(&huart1);
	}
	else if (bsize == WL_8BIT)
	{
		if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler(); //(MAIN_FUNC, USART_DRV);
	}

}
/**
 * @brief	Configure usart the AutoBaudRate method and
 * 			wait for receend of autobaud phase
 * @param	none
 * @retval	none
 */
void OW_Autobaud(void)
{

	/*##-1- Configure the UART peripheral
	 ######################################*/
	/* Put the USART peripheral in the Asynchronous mode (UART Mode) */
	/* UART configured as follows:
	 - Word Length = 8 Bits
	 - Stop Bit = One Stop bit
	 - Parity = NONE parity
	 - BaudRate = 115200 baud It can be any other value as the USARTx_BRR register will be reprogrammed
	 - Hardware flow control disabled (RTS and CTS signals)
	 - The oversampling mode is 8 or 16 (Both are tested)
	 - The USART1 clock source is a system clock at 48 MHz,
	 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	/*##-2-  */
//	UartHandle.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ONSTARTBIT;
//	UartHandle.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ON0X55FRAME;
// 	huart1.AdvancedInit.AutoBaudRateMode = UART_ADVFEATURE_AUTOBAUDRATE_ON0X7FFRAME;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_AUTOBAUDRATE_INIT;
	huart1.AdvancedInit.AutoBaudRateEnable =
	UART_ADVFEATURE_AUTOBAUDRATE_ENABLE;
	huart1.AdvancedInit.AutoBaudRateMode =
	UART_ADVFEATURE_AUTOBAUDRATE_ONFALLINGEDGE;
	/* Initialization */
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}

	/* Loop until the end of Auto baudrate phase */
	while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_ABRF) == RESET)
	{
	}
	/* If AutoBaudBate error occurred */
	if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ABRE) != RESET)
	{
		Error_Handler();
	}
	/* Wait until RXNE flag is set */
	while (__HAL_UART_GET_FLAG(&huart1,UART_FLAG_RXNE) == RESET)
	{
	}
	__HAL_CRC_DR_RESET(&hcrc);
	ow_txbuf[0] = STX;
	ow_txbuf[1] = DEF_RC_OWIFA;
	ow_txbuf[2] = DEF_CR_OWIFA;
	ow_txbuf[3] = 0x2U;  // set two byte response data size
	ow_txbuf[4] = ACK;  // set default response to Negativ Acknowledge
	ow_txbuf[5] = (uint8_t) (USART1->RDR); /* Receive data, clear flag */
	ow_txbuf[6] = (uint8_t) CRC_Calculate8(ow_txbuf, 0x6U);
	/* Send response ACK message using new baudrate*/
	if (HAL_UART_Transmit(&huart1, (uint8_t*) ow_txbuf, OW_BSIZE, PAK_TOUT(OW_BSIZE, BR_9600)) != HAL_OK)
	{ /* Transfer error in transmission process */
		Error_Handler();
	}
	while (HAL_UART_GetState(&huart1) != HAL_UART_STATE_READY)
	{
	}

}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
