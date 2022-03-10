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
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "./ov7725/bsp_ov7725.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./led/bsp_led.h"   
#include "./usart/bsp_usart.h"
#include "./key/bsp_key.h"  
#include "./systick/bsp_SysTick.h"
#include "./WIA/wildfire_Image_assistant.h"
#include "ff.h"


extern uint8_t Ov7725_vsync;

unsigned int Task_Delay[NumOfTask]; 

extern OV7725_MODE_PARAM cam_mode;

FATFS fs;													/* FatFs�ļ�ϵͳ���� */
FRESULT res_sd;                /* �ļ�������� */


/**
  * @brief  ������
  * @param  ��  
  * @retval ��
  */
int main(void) 	
{		
	
	float frame_count = 0;
	uint8_t retry = 0;
  int x = 0;

	/* Һ����ʼ�� */
	ILI9341_Init();
	ILI9341_GramScan ( 3 );
	
	LCD_SetFont(&Font8x16);
	LCD_SetColors(RED,BLACK);

  ILI9341_Clear(0,0,LCD_X_LENGTH,LCD_Y_LENGTH);	/* ��������ʾȫ�� */
	
	/********��ʾ�ַ���ʾ��*******/
  ILI9341_DispStringLine_EN(LINE(0),"BH OV7725 Test Demo");

	USART_Config();
	LED_GPIO_Config();
	Key_GPIO_Config();
	SysTick_Init();
	
	/*����sd�ļ�ϵͳ*/
	res_sd = f_mount(&fs,"0:",1);
	if(res_sd != FR_OK)
	{
		printf("\r\n�������������Ѹ�ʽ����fat��ʽ��SD����\r\n");
	}
	
//	printf("\r\n ** OV7725����ͷʵʱҺ����ʾ����** \r\n"); 
//	while(1);
	/* ov7725 gpio ��ʼ�� */
	OV7725_GPIO_Config();
	
	LED_BLUE;
	/* ov7725 �Ĵ���Ĭ�����ó�ʼ�� */
	while(OV7725_Init() != SUCCESS)
	{
		retry++;
		if(retry>5)
		{
			printf("\r\nû�м�⵽OV7725����ͷ\r\n");
			ILI9341_DispStringLine_EN(LINE(2),"No OV7725 module detected!");
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

	/* ����Һ��ɨ��ģʽ */
	ILI9341_GramScan( cam_mode.lcd_scan );
	
	
	
	ILI9341_DispStringLine_EN(LINE(2),"OV7725 initialize success!");
//	printf("\r\nOV7725����ͷ��ʼ�����\r\n");
	
	Ov7725_vsync = 0;

//  while(Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) != KEY_ON)
//  {
//    /*��ⰴ��*/
//    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON  )
//    {		
//      set_wincc_format(0x02, cam_mode.cam_width, cam_mode.cam_height);
//      
//      /* �˴�Ӧ�������λ����Ӧ���ź� */
//    }
//  }
	
	while(1)
	{
//    if (x % 2 == 0)
//    {
      /* дͼ�����ݵ���λ�� */
      write_rgb_wincc(x % 2, cam_mode.cam_width, cam_mode.cam_height);
      LED3_TOGGLE;
//    }
//    else
//    {
//      /* ���յ���ͼ�������ʾʵʱ��LCD�� */
//      if( Ov7725_vsync == 2 )
//      {
//        frame_count++;
//        FIFO_PREPARE;  			/*FIFO׼��*/
//        ImagDisp(cam_mode.lcd_sx,
//                  cam_mode.lcd_sy,
//                  cam_mode.cam_width,
//                  cam_mode.cam_height);			/*�ɼ�����ʾ*/
//        
//        Ov7725_vsync = 0;
//        LED1_TOGGLE;
//      }
//    }

    /*��ⰴ��*/
    if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON  )
    {		
      set_wincc_format(x % 2, PIC_FORMAT_RGB565, cam_mode.cam_width, cam_mode.cam_height);
      
      while(Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) != KEY_ON);
      /* �˴�Ӧ�������λ����Ӧ���ź� */
    }
		
		/*��ⰴ��*/
//		if( Key_Scan(KEY1_GPIO_PORT,KEY1_GPIO_PIN) == KEY_ON  )
//		{
//			static uint8_t name_count = 0;
//			char name[40];
//      
//      while(Ov7725_vsync != 2);    // �ȴ��ɼ����
//      FIFO_PREPARE;  			/*FIFO׼��*/
//			
//			//�������ý�ͼ���֣���ֹ�ظ���ʵ��Ӧ���п���ʹ��ϵͳʱ����������
//			name_count++; 
//			sprintf(name,"0:/photo2_%dX%d_%d.dat",cam_mode.cam_width, cam_mode.cam_height,name_count);

//			LED_BLUE;
//			printf("\r\n���ڱ��浽�ļ�...");
//			
//			if(write_rgb_file(0, cam_mode.cam_width, cam_mode.cam_height, name) == 0)
//			{
//				printf("\r\n���浽�ļ���ɣ�");
//				LED_GREEN;
//			}
//			else
//			{
//				printf("\r\n���浽�ļ�ʧ�ܣ�");
//				LED_RED;
//			}
//      
//      Ov7725_vsync = 0;		 // ��ʼ�´βɼ�
//		}
    
		/* ��ⰴ���л�ʵʱ��ʾ���߷�����λ�� */
		if( Key_Scan(KEY2_GPIO_PORT,KEY2_GPIO_PIN) == KEY_ON  )
		{
			x++;
    }
   }
}




/*********************************************END OF FILE**********************/

