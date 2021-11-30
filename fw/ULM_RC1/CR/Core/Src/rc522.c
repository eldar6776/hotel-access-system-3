/**
 ******************************************************************************
 * File Name          : mfrc522.c
 * Date               : 28/02/2016 23:16:19
 * Description        : mifare RC522 software modul
 ******************************************************************************
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rc522.h"

#if (__RC522_H__ != FW_BUILD)
    #error "rc522 header version mismatch"
#endif

/* Imported Type  ------------------------------------------------------------*/
/* Imported Variable  --------------------------------------------------------*/
/* Imported Function  --------------------------------------------------------*/
/* Private Define ------------------------------------------------------------*/
#define DEF_SYS_ID							0xABCD
#define DEF_CTRL_ID							101U
#define RC522_CARD_VALID_EVENT_TIME			3210U   // 3 s reader unlisten time after card read
#define RC522_CARD_INVALID_EVENT_TIME		987U    // ~1 s reader unlisten time after card read
#define RC522_POWER_ON_DELAY_TIME           8901U	// 8 s on power up reader is disbled
#define RC522_PROCESS_TIME					123U	// 123ms read rate
#define RC522_BSIZE							16U     // buffer byte lenght
#define RC522_READ    						0x7FU	// RC522 i2c read address
#define RC522_WRITE   						0x7EU	// RC522 i2c write address
#define RC522_TIMEOUT						20U     // RC522 i2c 5 ms transfer time_mout limit
#define RC522_TRIALS						50U		// RC522 i2c 100 time_ms operation retry
#define RC522_BLOCK_SIZE              		16U

#define CARD_VALID							0x06U	// card data status type
#define CARD_ERROR							0x15U	// card data status type
#define CARD_FORMATED						0x7FU	// card data status type
/** ==========================================================================*/
/**			M I F A R E		C A R D		S E C T O R		A D D R E S S E		  */
/** ==========================================================================*/
#define SECTOR_0							0x00U
#define SECTOR_1							0x04U
#define SECTOR_2							0x08U
#define SECTOR_3							0x0CU
#define SECTOR_4							0x10U
#define SECTOR_5							0x14U
#define SECTOR_6							0x18U
#define SECTOR_7							0x1CU
#define SECTOR_8							0x20U
/** ==========================================================================*/
/**			R C 5 2 2			C O M M A N D			L I S T				  */
/** ==========================================================================*/
#define PCD_IDLE							0x00U   //NO action; Cancel the current command
#define PCD_AUTHENT							0x0EU   //Authentication Key
#define PCD_RECEIVE							0x08U   //Receive Data
#define PCD_TRANSMIT						0x04U   //Transmit data
#define PCD_TRANSCEIVE						0x0CU   //Transmit and receive data,
#define PCD_RESETPHASE						0x0FU   //Reset
#define PCD_CALCCRC							0x03U   //CRC Calculate
/* Mifare_One card command word */
#define PICC_REQIDL							0x26U   // find the antenna area does not enter hibernation
#define PICC_REQALL							0x52U   // find all the cards antenna area
#define PICC_ANTICOLL						0x93U   // anti-collision
#define PICC_SELECTTAG						0x93U   // election card
#define PICC_AUTHENT1A						0x60U   // authentication key A
#define PICC_AUTHENT1B						0x61U   // authentication key B
#define PICC_READ							0x30U   // Read Block
#define PICC_WRITE							0xA0U   // write block
#define PICC_DECREMENT						0xC0U   // debit
#define PICC_INCREMENT						0xC1U   // recharge
#define PICC_RESTORE						0xC2U   // transfer block data to the buffer
#define PICC_TRANSFER						0xB0U   // save the data in the buffer
#define PICC_HALT							0x50U   // Sleep
/** ==========================================================================*/
/**			R C 5 2 2			R E G I S T E R			L I S T				  */
/** ==========================================================================*/
//Page 0: Command and Status
#define RC522_REG_RESERVED00				0x00U
#define RC522_REG_COMMAND					0x01U
#define RC522_REG_COMM_IE_N					0x02U
#define RC522_REG_DIV1_EN					0x03U
#define RC522_REG_COMM_IRQ					0x04U
#define RC522_REG_DIV_IRQ					0x05U
#define RC522_REG_ERROR						0x06U
#define RC522_REG_STATUS1					0x07U
#define RC522_REG_STATUS2					0x08U
#define RC522_REG_FIFO_DATA					0x09U
#define RC522_REG_FIFO_LEVEL				0x0AU
#define RC522_REG_WATER_LEVEL				0x0BU
#define RC522_REG_CONTROL					0x0CU
#define RC522_REG_BIT_FRAMING				0x0DU
#define RC522_REG_COLL						0x0EU
#define RC522_REG_RESERVED01				0x0FU
//Page 1: Command
#define RC522_REG_RESERVED10				0x10U
#define RC522_REG_MODE						0x11U
#define RC522_REG_TX_MODE					0x12U
#define RC522_REG_RX_MODE					0x13U
#define RC522_REG_TX_CONTROL				0x14U
#define RC522_REG_TX_AUTO					0x15U
#define RC522_REG_TX_SELL					0x16U
#define RC522_REG_RX_SELL					0x17U
#define RC522_REG_RX_THRESHOLD				0x18U
#define RC522_REG_DEMOD						0x19U
#define RC522_REG_RESERVED11				0x1AU
#define RC522_REG_RESERVED12				0x1BU
#define RC522_REG_MIFARE					0x1CU
#define RC522_REG_RESERVED13				0x1DU
#define RC522_REG_RESERVED14				0x1EU
#define RC522_REG_SERIALSPEED				0x1FU
//Page 2: CFG
#define RC522_REG_RESERVED20				0x20U
#define RC522_REG_CRC_RESULT_M				0x21U
#define RC522_REG_CRC_RESULT_L				0x22U
#define RC522_REG_RESERVED21				0x23U
#define RC522_REG_MOD_WIDTH					0x24U
#define RC522_REG_RESERVED22				0x25U
#define RC522_REG_RF_CFG					0x26U
#define RC522_REG_GS_N						0x27U
#define RC522_REG_CWGS_PREG					0x28U
#define RC522_REG_MODGS_PREG				0x29U
#define RC522_REG_T_MODE					0x2AU
#define RC522_REG_T_PRESCALER				0x2BU
#define RC522_REG_T_RELOAD_H				0x2CU
#define RC522_REG_T_RELOAD_L				0x2DU
#define RC522_REG_T_COUNTER_VALUE_H			0x2EU
#define RC522_REG_T_COUNTER_VALUE_L			0x2FU
//Page 3:TestRegister
#define RC522_REG_RESERVED30				0x30U
#define RC522_REG_TEST_SEL1					0x31U
#define RC522_REG_TEST_SEL2					0x32U
#define RC522_REG_TEST_PIN_EN				0x33U
#define RC522_REG_TEST_PIN_VALUE			0x34U
#define RC522_REG_TEST_BUS					0x35U
#define RC522_REG_AUTO_TEST					0x36U
#define RC522_REG_VERSION					0x37U
#define RC522_REG_ANALOG_TEST				0x38U
#define RC522_REG_TEST_ADC1					0x39U
#define RC522_REG_TEST_ADC2					0x3AU
#define RC522_REG_TEST_ADC0					0x3BU
#define RC522_REG_RESERVED31				0x3CU
#define RC522_REG_RESERVED32				0x3DU
#define RC522_REG_RESERVED33				0x3EU
#define RC522_REG_RESERVED34				0x3FU
/* Private Type --------------------------------------------------------------*/
typedef enum
{
	MI_OK = 0, MI_ERR, MI_NOTAGERR, MI_SKIP_OVER

} RC522_StatusTypeDef;

typedef struct
{
	uint8_t block_0[RC522_BLOCK_SIZE];
	uint8_t block_1[RC522_BLOCK_SIZE];
	uint8_t block_2[RC522_BLOCK_SIZE];
	uint8_t block_3[RC522_BLOCK_SIZE];

} RC522_SectorTypeDef;

RC522_SectorTypeDef sector_0;
RC522_SectorTypeDef sector_1;
RC522_SectorTypeDef sector_2;
RC522_CardDataTypeDef sCard;
/* Private Variable ----------------------------------------------------------*/

uint8_t mifare_keya[6] =
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t mifare_keyb[6] =
{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t rc522_rx_buff[RC522_BSIZE];
uint8_t rc522_tx_buff[RC522_BSIZE];
/* Private Macro -------------------------------------------------------------*/
#define mRC522_ChipSelect()     (HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_RESET))
#define mRC522_ChipRelease()    (HAL_GPIO_WritePin(SPI_NSS_GPIO_Port, SPI_NSS_Pin, GPIO_PIN_SET))
#define mRC522_ResetAssert()    (HAL_GPIO_WritePin(RS522_RST_GPIO_Port, RS522_RST_Pin, GPIO_PIN_RESET))
#define mRC522_ResetRelease()   (HAL_GPIO_WritePin(RS522_RST_GPIO_Port, RS522_RST_Pin, GPIO_PIN_SET))
/* Private Function Prototype ------------------------------------------------*/
static void RC522_Halt(void);
static void RC522_AntennaOn(void);
static uint8_t RC522_ReadRegister(uint8_t addr);
static uint8_t RC522_SelectTag(uint8_t *serNum);
static void RC522_SetBitMask(uint8_t reg, uint8_t mask);
static void RC522_ClearBitMask(uint8_t reg, uint8_t mask);
static void RC522_WriteRegister(uint8_t addr, uint8_t val);
static RC522_StatusTypeDef RC522_Anticoll(uint8_t *serNum);
static void RC522_WriteRegister(uint8_t addr, uint8_t val);
static RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t *recvData);
static RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t *TagType);
static void RC522_CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData);
static RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum);
static RC522_StatusTypeDef RC522_ToCard(uint8_t cmd, uint8_t *sbuf, uint8_t slen, uint8_t *rbuf, uint16_t *rlen);
/* Program Code  -------------------------------------------------------------*/
/**
 * @brief
 * @param
 * @retval
 */
void RC522_Init(void)
{
	mRC522_ChipRelease();
	mRC522_ResetAssert();
	HAL_Delay(10);
	mRC522_ResetRelease();
	HAL_Delay(100);
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_RESETPHASE);
	RC522_WriteRegister(RC522_REG_T_MODE, 0x8D);
	RC522_WriteRegister(RC522_REG_T_PRESCALER, 0x3E);
	RC522_WriteRegister(RC522_REG_T_RELOAD_L, 30);
	RC522_WriteRegister(RC522_REG_T_RELOAD_H, 0);
	RC522_WriteRegister(RC522_REG_RF_CFG, 0x70);
	RC522_WriteRegister(RC522_REG_TX_AUTO, 0x40);
	RC522_WriteRegister(RC522_REG_MODE, 0x3D);
	RC522_AntennaOn(); // Open the antenna
}
/**
 * @brief
 * @param
 * @retval
 */
void RC522_Service(void)
{
	uint8_t card_id[8], buff[68], sz = 0;
	RC522_StatusTypeDef status;
	HAL_StatusTypeDef state = HAL_ERROR;

	status = RC522_Request(PICC_REQIDL, card_id);
	if (status == MI_OK) status = RC522_Anticoll(card_id);
	if (status == MI_OK) sz = RC522_SelectTag(card_id);

	if (sz != 0)
	{
		ZEROFILL(buff, sizeof(buff));
		mem_cpy(buff, "S00\0", 4);
		status = RC522_Auth(PICC_AUTHENT1A, 0, mifare_keya, card_id);
		if (status == MI_OK) status = RC522_Read(0, &buff[4]);
		if (status == MI_OK) status = RC522_Read(1, &buff[20]);
		if (status == MI_OK) status = RC522_Read(2, &buff[36]);
		if (status == MI_OK) status = RC522_Read(3, &buff[52]);
		if (status == MI_OK) state = HAL_UART_Transmit(&huart1, buff, sizeof(buff), 100);
		else RC522_Halt();

	}

	if (state == HAL_OK)
	{
		ZEROFILL(buff, sizeof(buff));
		mem_cpy(buff, "S01\0", 4);
		status = RC522_Auth(PICC_AUTHENT1A, 4, mifare_keya, card_id);
		if (status == MI_OK) status = RC522_Read(4, &buff[4]);
		if (status == MI_OK) status = RC522_Read(5, &buff[20]);
		if (status == MI_OK) status = RC522_Read(6, &buff[36]);
		if (status == MI_OK) status = RC522_Read(7, &buff[52]);
		if (status == MI_OK) state = HAL_UART_Transmit(&huart1, buff, sizeof(buff), 100);
		else RC522_Halt();
	}

	if (state == HAL_OK)
	{
		ZEROFILL(buff, sizeof(buff));
		mem_cpy(buff, "S02\0", 4);
		status = RC522_Auth(PICC_AUTHENT1A, 8, mifare_keya, card_id);
		if (status == MI_OK) status = RC522_Read(8, &buff[4]);
		if (status == MI_OK) status = RC522_Read(9, &buff[20]);
		if (status == MI_OK) status = RC522_Read(10, &buff[36]);
		if (status == MI_OK) status = RC522_Read(11, &buff[52]);
		if (status == MI_OK) state = HAL_UART_Transmit(&huart1, buff, sizeof(buff), 100);
		else RC522_Halt();
	}

	RC522_Halt();

}
/**
 * @brief
 * @param
 * @retval
 */
static uint8_t RC522_ReadRegister(uint8_t addr)
{
	uint8_t tx[2] =
	{ 0x0U, 0x0U };
	uint8_t rx[2] =
	{ 0x0U, 0x0U };
	
	mRC522_ChipSelect();
	tx[0] = ((addr << 1) & 0x7E) | 0x80;
	HAL_SPI_TransmitReceive(&hspi1, tx, rx, 2, 10);
	mRC522_ChipRelease();
	return (rx[1]);
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_WriteRegister(uint8_t addr, uint8_t val)
{
	uint8_t tx[2];
	
	mRC522_ChipSelect();
	tx[0] = (addr << 1) & 0x7E;
	tx[1] = val;
	HAL_SPI_Transmit(&hspi1, tx, 2, 10);
	mRC522_ChipRelease();
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_SetBitMask(uint8_t reg, uint8_t mask)
{
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) | mask);
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
	RC522_WriteRegister(reg, RC522_ReadRegister(reg) & (~mask));
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_AntennaOn(void)
{
	uint8_t temp;

	temp = RC522_ReadRegister(RC522_REG_TX_CONTROL);
	if ((temp & 0x03U) == 0U) RC522_SetBitMask(RC522_REG_TX_CONTROL, 0x03U);
}
/**
 * @brief
 * @param
 * @retval
 */
static RC522_StatusTypeDef RC522_Request(uint8_t reqMode, uint8_t *TagType)
{
	RC522_StatusTypeDef status;
	uint16_t backBits;                                  //The received data bits

	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0x07U);	// TxLastBits = BitFramingReg[2..0]	???
	TagType[0] = reqMode;
	status = RC522_ToCard(PCD_TRANSCEIVE, TagType, 1U, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10U)) status = MI_ERR;
	return status;
}
/**
 * @brief
 * @param
 * @retval
 */
static RC522_StatusTypeDef RC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData,
        uint16_t *backLen)
{
	uint8_t irqEn = 0U;
	uint8_t waitIRq = 0U;
	uint32_t n, i, lastBits;
	RC522_StatusTypeDef status = MI_ERR;

	switch (command)
	{
		case PCD_AUTHENT:
			irqEn = 0x12U;
			waitIRq = 0x10U;
			break;
		case PCD_TRANSCEIVE:
			irqEn = 0x77U;
			waitIRq = 0x30U;
			break;
		default:
			break;
	}
	RC522_WriteRegister(RC522_REG_COMM_IE_N, irqEn | 0x80U);
	RC522_ClearBitMask(RC522_REG_COMM_IRQ, 0x80U);
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80U);
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_IDLE);

	for (i = 0U; i < sendLen; i++)
	{
		RC522_WriteRegister(RC522_REG_FIFO_DATA, sendData[i]);   //Writing data to the FIFO
	}

	RC522_WriteRegister(RC522_REG_COMMAND, command);            //Execute the command

	if (command == PCD_TRANSCEIVE)
	{
		RC522_SetBitMask(RC522_REG_BIT_FRAMING, 0x80U);     //StartSend=1,transmission of data starts
	}
	/**
	 *   Waiting to receive data to complete
	 */
	i = 200U;	//i according to the clock frequency adjustment, the operator M1 card maximum waiting time_m 25ms???

	do
	{
		/**
		 *   CommIrqReg[7..0]
		 *   Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		 */
//        if (eComState == COM_PACKET_RECEIVED) RS485_Service();
		n = RC522_ReadRegister(RC522_REG_COMM_IRQ);
		--i;
	}
	while ((i != 0U) && ((n & 0x01U) == 0U) && ((n & waitIRq) == 0U));          // End of do...while loop
	/**
	 *   StartSend=0
	 */
	RC522_ClearBitMask(RC522_REG_BIT_FRAMING, 0x80U);
	
	if (i != 0U)
	{
		if ((RC522_ReadRegister(RC522_REG_ERROR) & 0x1BU) == 0U)
		{
			status = MI_OK;

			if (n & irqEn & 0x01U) status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE)
			{
				n = RC522_ReadRegister(RC522_REG_FIFO_LEVEL);
				lastBits = (RC522_ReadRegister(RC522_REG_CONTROL) & 0x07U);
				if (lastBits != 0U) *backLen = ((n - 1U) * 8U + lastBits);
				else *backLen = (n * 8U);
				if (n == 0U) n = 1U;
				if (n > RC522_BSIZE) n = RC522_BSIZE;
				/**
				 *   Reading the received data in FIFO
				 */
				for (i = 0U; i < n; i++)
				{
					backData[i] = RC522_ReadRegister(RC522_REG_FIFO_DATA);
				}
			}
		}
		else status = MI_ERR;
	}
	return (status);
}
/**
 * @brief
 * @param
 * @retval
 */
static RC522_StatusTypeDef RC522_Anticoll(uint8_t *serNum)
{
	uint32_t bcnt;
	uint16_t blen;
	uint8_t snum = 0U;
	RC522_StatusTypeDef status;
	
	RC522_WriteRegister(RC522_REG_BIT_FRAMING, 0U);   // TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20U;
	status = RC522_ToCard(PCD_TRANSCEIVE, serNum, 2U, serNum, &blen);
	
	if (status == MI_OK)
	{
		/**
		 *   Check card serial number
		 */
		for (bcnt = 0U; bcnt < 4U; bcnt++)
		{
			snum ^= serNum[bcnt];
		}
		if (snum != serNum[bcnt]) status = MI_ERR;
	}
	return (status);
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
	uint32_t i, n;
	
	RC522_ClearBitMask(RC522_REG_DIV_IRQ, 0x04U);       // CRCIrq = 0
	RC522_SetBitMask(RC522_REG_FIFO_LEVEL, 0x80U);      // Clear the FIFO pointer
//	RC522_WriteRegister(RC522_REG_COMMAND, PCDLE);
	/**
	 *   Write_RC522(CommandReg, PCDLE);
	 *   Writing data to the FIFO
	 */
	for (i = 0U; i < len; i++)
	{
		RC522_WriteRegister(RC522_REG_FIFO_DATA, *(pIndata + i));
	}
	/**
	 *   Start CRC calculation
	 */
	RC522_WriteRegister(RC522_REG_COMMAND, PCD_CALCCRC);
	/**
	 *   Wait for CRC calculation to complete
	 */
	i = 0xFFU;
	
	do
	{
//        if (eComState == COM_PACKET_RECEIVED) RS485_Service();
		n = RC522_ReadRegister(RC522_REG_DIV_IRQ);
		--i;
	}
	while ((i != 0U) && ((n & 0x04U) == 0U));              // wait for CRCIrq = 1
	
//	RC522_WriteRegister(RC522_REG_COMMAND, PCDLE);
	/**
	 *   Read CRC calculation result
	 */
	pOutData[0] = RC522_ReadRegister(RC522_REG_CRC_RESULT_L);
	pOutData[1] = RC522_ReadRegister(RC522_REG_CRC_RESULT_M);
}
/**
 * @brief
 * @param
 * @retval
 */
static uint8_t RC522_SelectTag(uint8_t *serNum)
{
	uint32_t bcnt;
	uint16_t recb;
	uint8_t buffer[9], size;
	RC522_StatusTypeDef status;

	buffer[0] = PICC_SELECTTAG;
	buffer[1] = 0x70U;

	for (bcnt = 0U; bcnt < 5U; bcnt++)
	{
		buffer[bcnt + 2U] = *(serNum + bcnt);
	}
	RC522_CalculateCRC(buffer, 7U, &buffer[7]);		//??
	status = RC522_ToCard(PCD_TRANSCEIVE, buffer, 9U, buffer, &recb);
	if ((status == MI_OK) && (recb == 0x18U)) size = buffer[0];
	else size = 0U;
	return (size);
}
/**
 * @brief
 * @param
 * @retval
 */
static RC522_StatusTypeDef RC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum)
{
	uint32_t bcnt;
	uint16_t recb;
	uint8_t buff[12];
	RC522_StatusTypeDef status;
	
	//Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	
	for (bcnt = 0U; bcnt < 6U; bcnt++)
	{
		buff[bcnt + 2U] = *(Sectorkey + bcnt);
	}
	
	for (bcnt = 0U; bcnt < 4U; bcnt++)
	{
		buff[bcnt + 8U] = *(serNum + bcnt);
	}
	
	status = RC522_ToCard(PCD_AUTHENT, buff, 12U, buff, &recb);
	if ((status != MI_OK) || ((RC522_ReadRegister(RC522_REG_STATUS2) & 0x08U) == 0U)) status = MI_ERR;
	return (status);
}
/**
 * @brief
 * @param
 * @retval
 */
static RC522_StatusTypeDef RC522_Read(uint8_t blockAddr, uint8_t *recvData)
{
	RC522_StatusTypeDef status;
	uint16_t unLen;
	uint8_t sendData[8];
	sendData[0] = PICC_READ;
	sendData[1] = blockAddr;
	RC522_CalculateCRC(sendData, 2U, &sendData[2]);
	status = RC522_ToCard(PCD_TRANSCEIVE, sendData, 4U, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90U)) status = MI_ERR;
	return (status);
}
/**
 * @brief
 * @param
 * @retval
 */
static void RC522_Halt(void)
{
	uint16_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0U;
	RC522_CalculateCRC(buff, 2U, &buff[2]);
	RC522_ToCard(PCD_TRANSCEIVE, buff, 4U, buff, &unLen);
	RC522_ClearBitMask(0x08U, 0x08U);
}
/************************ (C) COPYRIGHT JUBERA D.O.O Sarajevo ************************/
