/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   �����õ�printf���ڣ��ض���printf������
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 


#include "./usart/bsp_usart.h"
#include <string.h>

#define RX_MAX_LEN     (1*512)
uint8_t data_rx_buff[RX_MAX_LEN];
uint32_t data_rx_len = 0;
uint8_t data_rx_flag = 0;

 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
static void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;
  /* �������ȼ�Ϊ1 */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  /* �����ȼ�Ϊ1 */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void USART_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// �򿪴���GPIO��ʱ��
	DEBUG_USART_GPIO_APBxClkCmd(DEBUG_USART_GPIO_CLK, ENABLE);
	
	// �򿪴��������ʱ��
	DEBUG_USART_APBxClkCmd(DEBUG_USART_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(DEBUG_USARTx, &USART_InitStructure);
  
  /* Ƕ�������жϿ�����NVIC���� */
	NVIC_Configuration();
  
	/* ʹ�ܴ��ڽ����ж� */
	USART_ITConfig(DEBUG_USARTx, USART_IT_RXNE, ENABLE);
  USART_ITConfig(DEBUG_USARTx, USART_IT_IDLE, ENABLE);

	// ʹ�ܴ���
	USART_Cmd(DEBUG_USARTx, ENABLE);	

  
}

void debug_send_data(uint8_t *data, uint32_t len)
{
  while(len--)
  {
    /* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t)*data++);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
  }
}

///�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
int fputc(int ch, FILE *f)
{
		/* ����һ���ֽ����ݵ����� */
		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
		
		/* �ȴ�������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

///�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
int fgetc(FILE *f)
{
		/* �ȴ������������� */
		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USARTx);
}

/**
  * @brief  ��ȡ���յ���������Ϣ
  * @param  data���������ݵ�ָ��
  * @retval ���յ������ݳ���
  */
uint8_t *get_rx_data(void)
{
  return data_rx_buff;
}

/**
  * @brief  ��ȡ���յ���������Ϣ
  * @param  data���������ݵ�ָ��
  * @retval ���յ������ݳ���
  */
uint32_t get_rx_len(void)
{
  return data_rx_len;
}

/**
  * @brief  ��ȡ���յ��ı�־
  * @param  ��
  * @retval ���յ��ı�־
  */
uint8_t get_rx_flag(void)
{
  return data_rx_flag;
}

/**
  * @brief  ��λ�������ݻ�����
  * @param  ��
  * @retval ��
  */
void reset_rx_data(void)
{
  memset(data_rx_buff, 0, sizeof(data_rx_buff));    // �������
  data_rx_len = 0;                                  // ���ü���
}

/**
  * @brief  �����жϴ��������
  * @param  ��
  * @retval ��
  */
void DEBUG_USART_IRQHandler(void)
{
	if(USART_GetITStatus(DEBUG_USARTx,USART_IT_RXNE)!=RESET)
	{		
    if (data_rx_len < RX_MAX_LEN)
    {
      data_rx_buff[data_rx_len++] = USART_ReceiveData(DEBUG_USARTx);
    }
	}
  
  if(USART_GetITStatus(DEBUG_USARTx,USART_IT_IDLE)!=RESET)
	{		
    USART_ReceiveData(DEBUG_USARTx);
    USART_ClearITPendingBit(DEBUG_USARTx, USART_IT_IDLE);    // ����жϹ���λ
    USART_ClearFlag(DEBUG_USARTx, USART_IT_IDLE);            // ��������ж�
    data_rx_flag = 1;         // ���Ϊ����
	}
}

