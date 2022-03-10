/**
  ******************************************************************************
  * @file    wildfire_image_assistant.c
  * @version V1.0
  * @date    2020-xx-xx
  * @brief   Ұ������ͷ���ֽӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "./WIA/wildfire_image_assistant.h"
#include "./ov7725/bsp_ov7725.h"
#include "./led/bsp_led.h"

extern uint8_t Ov7725_vsync;

/**
 * @brief   ������� CRC-16.
 *          CRC�ļĴ���ֵ��rcr_init�������������Բ�һ�μ����������ݵĽ��
 *          ������һ֡ͼ����150KB���ڲ�����һ�η�����ô������ݣ���Ҫ�ֶμ��㣩
 * @param   *data:  Ҫ��������ݵ�����.
 * @param   length: ���ݵĴ�С
 * @return  status: ����CRC.
 */
uint16_t calc_crc_16(uint8_t *data, uint16_t length, uint16_t rcr_init)
{
  uint16_t crc = rcr_init;
  
  for(int n = 0; n < length; n++)
  {
    crc = data[n] ^ crc;
    
    for(int i = 0;i < 8;i++)
    {
      if(crc & 0x01)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc = crc >> 1;
      }
    }
  }
  
  return crc;    // �������ֽں͵��ֽ�λ��
}

/**
 * @brief  û�и������ݵ�Ӧ��.
 * @param  timeout: ��ʱʱ��.
 * @return -1����ʱ���أ���-1���յ�Ӧ�𣬷�����λ��Ӧ���ȷ����.
 */
int no_ack_with_data_attached(uint32_t timeout)
{
  uint8_t* data = NULL;
  uint32_t packet_head = 0;
  
  while(get_rx_flag() != 1 && timeout--)
  {
    
  }
  
  if (timeout <= 0)
  {
    return -1;    /* ���س�ʱ���� */
  }
  else
  {
    /* �յ����� */
    data = get_rx_data();
    
    packet_head = *data << 24 | *(data+1) << 16 | *(data+2) << 8 | *(data+3);    // �ϳɰ�ͷ
    
    if (packet_head == FRAME_HEADER)    // У���ͷ�Ƿ���ȷ
    {
      uint32_t pack_len = *(data+5) << 24 | *(data+6) << 16 | *(data+7) << 8 | *(data+8);    // �ϳɳ���
      
      if (calc_crc_16(data, pack_len-2, 0xFFFF) == (*(data+pack_len-1) | *(data+pack_len-2) << 8))    // У�� CRC-16
      {
        reset_rx_data();    // �������ݽ���
        
        return *(data + 10);    // ����ȷ����
      }
    }
    
    return -1;
  }
}

/**
 * @brief  ��������ͼ���ʽ������λ��.
 * @param  type:   ͼ���ʽ.
 * @param  width:  ͼ����.
 * @param  height: ͼ��߶�.
 * @return void.
 */
void set_wincc_format(uint8_t addr, uint8_t type, uint16_t width, uint16_t height)
{
  uint16_t crc_16 = 0xFFFF;
  
  uint8_t packet_head[17] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                             0x00,                       // �豸��ַ
                             0x00, 0x00, 0x00, 0x11,     // ������ (10+1+2+2+2)                
                             0x01,                       // ����ͼ���ʽָ��
                             };
  
  packet_head[4] = addr;    // �޸��豸��ַ
  
  packet_head[10] = type;
                             
  packet_head[11] = width >> 8;
  packet_head[12] = width & 0xFF;
                             
  packet_head[13] = height >> 8;
  packet_head[14] = height & 0xFF;
                             
  crc_16 = calc_crc_16((uint8_t *)&packet_head, sizeof(packet_head) - 2, crc_16);    // �ֶμ���crc��16��У����, �����ͷ��
  
  packet_head[15] = crc_16 & 0x00FF;
  packet_head[16] = (crc_16 >> 8) & 0x00FF;
                             
//  printf("0x%x\r\n", packet_head[15]);
                             
  CAM_ASS_SEND_DATA((uint8_t *)&packet_head, sizeof(packet_head));
}

/**
 * @brief  ����ͼ�����ݰ�����λ��.
 * @param  addr:   �豸��ַ��0 or 1.
 * @param  width:  ͼ����.
 * @param  height: ͼ��߶�.
 * @return 0���ɹ���-1��ʧ��.
 */
int write_rgb_wincc(uint8_t addr, uint16_t width, uint16_t height) 
{
  uint16_t i, j; 
  uint16_t crc_16 = 0xFFFF;
	uint16_t Camera_Data[640];
  static uint8_t flag = 1;

//  packet_head_t packet_head =
//  {
//    .head = 0x59485A53,     // ��ͷ
//    .addr = 0x00,           // �豸��ַ
//    .len  = 0x00000011,     // ������ (10+320*240*2+2)
//    .cmd  = 02,             // ͼ����������
//  };
  
  if (flag)    /* ����һ����λ����ͼ���ʽ */
  {
    
    do
    {
      reset_rx_data();    // �������ݽ���
      
      set_wincc_format(addr, PIC_FORMAT_RGB565, width, height);
      flag++;
    }
    while(no_ack_with_data_attached(0x1FFFFFF) != 0 && flag < 10);
    
    if (flag >= 10)
    {
      LED1_ON;
    }
    
    flag = !flag;
  }
  
  
  if (Ov7725_vsync == 2)    // �ɼ����
  {
    FIFO_PREPARE;  			/*FIFO׼��*/
  
    uint8_t packet_head[10] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                               0x00,                       // �豸��ַ
                               0x00, 0x02, 0x58, 0x0C,     // ������ (10+320*240*2+2)
                               0x02                        // ͼ����������
                               };
    
    packet_head[4] = addr;    // �޸��豸��ַ
    
    uint32_t data_len = 10 + width * height * 2 + 2;    // ���������
                               
    /* �޸İ����� */
    packet_head[5] = data_len >> 24;
    packet_head[6] = data_len >> 16;
    packet_head[7] = data_len >> 8;
    packet_head[8] = data_len;
    
    memset(Camera_Data, 0xDD, sizeof(Camera_Data));

    /* ����ͷ */
    CAM_ASS_SEND_DATA((uint8_t *)&packet_head, sizeof(packet_head));
    crc_16 = calc_crc_16((uint8_t *)&packet_head, sizeof(packet_head), crc_16);    // �ֶμ���crc��16��У����, �����ͷ��

    /* ����ͼ������ */
    for(i = 0; i < width; i++)
    {
      for(j = 0; j < height; j++)
      {
        READ_FIFO_PIXEL(Camera_Data[j]);		// ��FIFO����һ��rgb565���ص�Camera_Data����
      }

      CAM_ASS_SEND_DATA((uint8_t *)Camera_Data, j*2);           		// �ֶη�����������
      crc_16 = calc_crc_16((uint8_t *)Camera_Data, j*2, crc_16);    // �ֶμ���crc��16��У���룬����һ��ͼ������
    }

    /*����У������*/
    crc_16 = ((crc_16&0x00FF)<<8)|((crc_16&0xFF00)>>8);    //  �������ֽں͵��ֽ�λ��
    CAM_ASS_SEND_DATA((uint8_t *)&crc_16, 2);
    
    Ov7725_vsync = 0;		 // ��ʼ�´βɼ�
  }

  return 0;    // ���سɹ�
}

#if 1   // ��rgb����д���ļ�������д��λ�����в���
/**
 * @brief  ��ͼ�����ݰ�д���ļ�.
 * @param  addr:   �豸��ַ��0 or 1.
 * @param  width:  ͼ����.
 * @param  height: ͼ��߶�.
 * @return 0���ɹ���-1��ʧ��.
 */

#include "ff.h"

FIL bmpfsrc; 
FRESULT bmpres;

int write_rgb_file(uint8_t addr, uint16_t width, uint16_t height, char *file_name) 
{
  uint16_t i, j; 
  uint16_t crc_16 = 0xFFFF;
	uint16_t Camera_Data[640];
  unsigned int mybw;

//  packet_head_t packet_head =
//  {
//    .head = 0x59485A53,     // ��ͷ
//    .addr = 0x00,           // �豸��ַ
//    .len  = 0x00000011,     // ������ (10+320*240*2+2)
//    .cmd  = 02,             // ͼ����������
//  };
  
  if (Ov7725_vsync == 2)    // �ɼ����
  {
    FIFO_PREPARE;  			/*FIFO׼��*/
  
    uint8_t packet_head[10] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                               0x01,                       // �豸��ַ
                               0x00, 0x02, 0x58, 0x0C,     // ������ (10+320*240*2+2)
                               0x02                        // ͼ����������
                               };
    
    packet_head[4] = addr;
    
    uint32_t data_len = 10 + width * height * 2 + 2;
                               
    packet_head[5] = data_len >> 24;
    packet_head[6] = data_len >> 16;
    packet_head[7] = data_len >> 8;
    packet_head[8] = data_len;
    
    memset(Camera_Data, 0xDD, sizeof(Camera_Data));
                               
    /* �½�һ���ļ� */
    bmpres = f_open( &bmpfsrc , (char*)file_name, FA_CREATE_ALWAYS | FA_WRITE );
    
    /* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
    f_close(&bmpfsrc);
      
    bmpres = f_open( &bmpfsrc , (char*)file_name,  FA_OPEN_EXISTING | FA_WRITE);

    if ( bmpres == FR_OK )    // �ļ��򿪳ɹ�
    {
      /* ����ͷ */
      f_write(&bmpfsrc, (uint8_t *)&packet_head, sizeof(packet_head), &mybw);        // ���Ͱ�ͷ
      crc_16 = calc_crc_16((uint8_t *)&packet_head, sizeof(packet_head), crc_16);    // �ֶμ���crc��16��У����, �����ͷ��

      /* ����ͼ������ */
      for(i = 0; i < width; i++)
      {
        for(j = 0; j < height; j++)
        {
          READ_FIFO_PIXEL(Camera_Data[j]);		// ��FIFO����һ��rgb565���ص�Camera_Data����
        }

        f_write(&bmpfsrc, Camera_Data, j*2, &mybw);        // ����ͼ������
        crc_16 = calc_crc_16((uint8_t *)Camera_Data, j*2, crc_16);    // �ֶμ���crc��16��У���룬����һ��ͼ������
      }

      /*����У������*/
      crc_16 = ((crc_16&0x00FF)<<8)|((crc_16&0xFF00)>>8);    //  �������ֽں͵��ֽ�λ��
      f_write(&bmpfsrc, (uint8_t *)&crc_16, 2, &mybw);       // ����crcУ������
      
      Ov7725_vsync = 0;		 // ��ʼ�´βɼ�
      
      f_close(&bmpfsrc);       // �ر��ļ�
    }
    else
    {
      f_close(&bmpfsrc);     // �ر��ļ�
      return -1;    // ����ʧ��
    }
  }

  return 0;    // ���سɹ�
}
#endif
