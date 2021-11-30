/**
 ******************************************************************************
 * File Name          : uart.c
 * Date               : 21/08/2016 20:59:16
 * Description        : rs485 function set
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
 
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stdio.h"
#include "uart.h"
#include "hotel_ctrl.h"
#include "main.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t rxbcnt;
uint8_t rx_buff[RS_BSIZE];
uint8_t tx_buff[RS_BSIZE];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void UART_Init(void)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_AF;	
	GPIO_InitStructure.GPIO_Pin 	= GPIO_Pin_6;//RX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource6, GPIO_AF_USART2);

	USART_InitStructure.USART_BaudRate = bps[rsbps];
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx| USART_Mode_Tx ;
	USART_Init(USARTx, &USART_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = UART_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
    
    
	USART_Cmd(USARTx, ENABLE);
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
}



void RS485_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    
    UART_Init();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE , ENABLE); 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    GPIO_WriteBit(GPIOE,GPIO_Pin_4, Bit_RESET);
}


void RS485_Send(uint8_t *buf, uint16_t len)
{
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET){} 
	RS485_MODE(RS485_TX);
  	while(len--)
	{
		USART_SendData(USART2, *buf++);
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TC) == RESET){} 
	}
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET){}
	RS485_MODE(RS485_RX); 
}


void UART_ReInit(uint8_t mode, uint8_t baudrate)
{
    USART_InitTypeDef USART_InitStructure;
    USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE);
    USART_Cmd(USARTx, DISABLE);
    USART_DeInit(USARTx);
	USART_InitStructure.USART_BaudRate = bps[baudrate];
    if      (mode == WL_8BIT) USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	else if (mode == WL_9BIT) USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    else ErrorHandler(RS485_FUNC, USART_DRV);
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx| USART_Mode_Tx ;
	USART_Init(USARTx, &USART_InitStructure);
	USART_Cmd(USARTx, ENABLE);
}
/*************************** End of file ****************************/





