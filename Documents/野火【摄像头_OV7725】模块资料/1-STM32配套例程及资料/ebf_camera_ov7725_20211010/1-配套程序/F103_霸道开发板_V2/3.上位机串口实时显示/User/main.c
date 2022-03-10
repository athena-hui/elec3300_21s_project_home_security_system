/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ����ͷ����ov7725��������
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-�Ե� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "./ov7725/bsp_ov7725.h"
#include "./led/bsp_led.h"   
#include "./usart/bsp_usart.h"
#include "./key/bsp_key.h"  
#include "./systick/bsp_SysTick.h"
#include "./WIA/wildfire_image_assistant.h"
#include "./protocol/protocol.h"

extern uint8_t Ov7725_vsync;

unsigned int Task_Delay[NumOfTask]; 


extern OV7725_MODE_PARAM cam_mode;


/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
int main(void) 	
{		
	
	float frame_count = 0;
	uint8_t retry = 0;
  /* ��ʼ�� */
  protocol_init();    // ͨѶЭ���ʼ��
  
  /* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� */
  /*ע����λ��������������Ϊ��1500000��û�����������ѡ�����ֶ��޸ģ�*/
	USART_Config();
	LED_GPIO_Config();
	Key_GPIO_Config();
	SysTick_Init();
	
	/* ov7725 gpio ��ʼ�� */
	OV7725_GPIO_Config();

	/* ov7725 �Ĵ���Ĭ�����ó�ʼ�� */
	while(OV7725_Init() != SUCCESS)
	{
		retry++;
		if(retry>5)
		{
			LED1_ON;
			while(1);
		}
	}


	/*��������ͷ����������ģʽ*/
	OV7725_Special_Effect(cam_mode.effect);
	/*����ģʽ*/
	OV7725_Light_Mode(cam_mode.light_mode);
	/*���Ͷ�*/
	OV7725_Color_Saturation(cam_mode.saturation);
	/*���ն�*/
	OV7725_Brightness(cam_mode.brightness);
	/*�Աȶ�*/
	OV7725_Contrast(cam_mode.contrast);
	/*����Ч��*/
	OV7725_Special_Effect(cam_mode.effect);
	
	/*����ͼ�������ģʽ��С*/
	OV7725_Window_Set(cam_mode.cam_sx,
                    cam_mode.cam_sy,
                    cam_mode.cam_width,
                    cam_mode.cam_height,
                    cam_mode.QVGA_VGA);

	Ov7725_vsync = 0;
	
  /* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� *//* ע�� */
  /*ע����λ��������������Ϊ��1500000��û�����������ѡ����ֶ��޸ģ�*/
	while(1)
	{
    write_rgb_wincc(0, cam_mode.cam_width, cam_mode.cam_height);
    
    receiving_process();    // �������ݴ���
	}
}




/*********************************************END OF FILE**********************/

