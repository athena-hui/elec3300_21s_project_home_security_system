/**
  ******************************************************************************
  * @file    bsp_key.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   bmp�ļ���������ʾbmpͼƬ�͸�Һ����ͼ
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-ָ���� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
  
#include "ff.h"
#include "./lcd/bsp_ili9341_lcd.h"
#include "./bmp/bsp_bmp.h"
#include "./usart/bsp_usart.h"

#define RGB24TORGB16(R,G,B) ((unsigned short int)((((R)>>3)<<11) | (((G)>>2)<<5)	| ((B)>>3)))

BYTE pColorData[960];			/* һ�����ɫ���ݻ��� 320 * 3 = 960 */
FIL bmpfsrc, bmpfdst; 
FRESULT bmpres;

/* �������Ҫ��ӡbmp��ص���ʾ��Ϣ,��printfע�͵�����
 * ��Ҫ��printf()���轫���������ļ���������
 */
#define BMP_DEBUG_PRINTF(FORMAT,...)  printf(FORMAT,##__VA_ARGS__)	 

/* ��ӡBMP�ļ���ͷ��Ϣ�����ڵ��� */
static void showBmpHead(BITMAPFILEHEADER* pBmpHead)
{
    BMP_DEBUG_PRINTF("λͼ�ļ�ͷ:\r\n");
    BMP_DEBUG_PRINTF("�ļ���С:%ld\r\n",(*pBmpHead).bfSize);
    BMP_DEBUG_PRINTF("������:%d\r\n",(*pBmpHead).bfReserved1);
    BMP_DEBUG_PRINTF("������:%d\r\n",(*pBmpHead).bfReserved2);
    BMP_DEBUG_PRINTF("ʵ��λͼ���ݵ�ƫ���ֽ���:%ld\r\n",(*pBmpHead).bfOffBits);
		BMP_DEBUG_PRINTF("\r\n");	
}

/* ��ӡBMP�ļ���ͷ��Ϣ�����ڵ��� */
static void showBmpInforHead(tagBITMAPINFOHEADER* pBmpInforHead)
{
    BMP_DEBUG_PRINTF("λͼ��Ϣͷ:\r\n");
    BMP_DEBUG_PRINTF("�ṹ��ĳ���:%ld\r\n",(*pBmpInforHead).biSize);
    BMP_DEBUG_PRINTF("λͼ��:%ld\r\n",(*pBmpInforHead).biWidth);
    BMP_DEBUG_PRINTF("λͼ��:%ld\r\n",(*pBmpInforHead).biHeight);
    BMP_DEBUG_PRINTF("biPlanesƽ����:%d\r\n",(*pBmpInforHead).biPlanes);
    BMP_DEBUG_PRINTF("biBitCount������ɫλ��:%d\r\n",(*pBmpInforHead).biBitCount);
    BMP_DEBUG_PRINTF("ѹ����ʽ:%ld\r\n",(*pBmpInforHead).biCompression);
    BMP_DEBUG_PRINTF("biSizeImageʵ��λͼ����ռ�õ��ֽ���:%ld\r\n",(*pBmpInforHead).biSizeImage);
    BMP_DEBUG_PRINTF("X����ֱ���:%ld\r\n",(*pBmpInforHead).biXPelsPerMeter);
    BMP_DEBUG_PRINTF("Y����ֱ���:%ld\r\n",(*pBmpInforHead).biYPelsPerMeter);
    BMP_DEBUG_PRINTF("ʹ�õ���ɫ��:%ld\r\n",(*pBmpInforHead).biClrUsed);
    BMP_DEBUG_PRINTF("��Ҫ��ɫ��:%ld\r\n",(*pBmpInforHead).biClrImportant);
		BMP_DEBUG_PRINTF("\r\n");
}





/**
 * @brief  ����ILI9341�Ľ�ȡBMPͼƬ
 * @param  x ����ɨ��ģʽ1�½�ȡ��������X���� 
 * @param  y ����ɨ��ģʽ1�½�ȡ��������Y���� 
 * @param  pic_name ��BMP��ŵ�ȫ·��
 * @retval ��
 */
void LCD_Show_BMP ( uint16_t x, uint16_t y, char * pic_name )
{
	int i, j, k;
	int width, height, l_width;

	BYTE red,green,blue;
	BITMAPFILEHEADER bitHead;
	BITMAPINFOHEADER bitInfoHead;
	WORD fileType;

	unsigned int read_num;

	bmpres = f_open( &bmpfsrc , (char *)pic_name, FA_OPEN_EXISTING | FA_READ);	
/*-------------------------------------------------------------------------------------------------------*/
	if(bmpres == FR_OK)
	{
		BMP_DEBUG_PRINTF("���ļ��ɹ�\r\n");

		/* ��ȡ�ļ�ͷ��Ϣ  �����ֽ�*/         
		f_read(&bmpfsrc,&fileType,sizeof(WORD),&read_num);     

		/* �ж��ǲ���bmp�ļ� "BM"*/
		if(fileType != 0x4d42)
		{
			BMP_DEBUG_PRINTF("�ⲻ��һ�� .bmp �ļ�!\r\n");
			return;
		}
		else
		{
			BMP_DEBUG_PRINTF("����һ�� .bmp �ļ�\r\n");	
		}        

		/* ��ȡBMP�ļ�ͷ��Ϣ*/
		f_read(&bmpfsrc,&bitHead,sizeof(tagBITMAPFILEHEADER),&read_num);        
		showBmpHead(&bitHead);

		/* ��ȡλͼ��Ϣͷ��Ϣ */
		f_read(&bmpfsrc,&bitInfoHead,sizeof(BITMAPINFOHEADER),&read_num);        
		showBmpInforHead(&bitInfoHead);
	}    
	else
	{
		BMP_DEBUG_PRINTF("���ļ�ʧ��!������룺bmpres = %d \r\n",bmpres);
		return;
	}    
/*-------------------------------------------------------------------------------------------------------*/
	width = bitInfoHead.biWidth;
	height = bitInfoHead.biHeight;

	/* ����λͼ��ʵ�ʿ�Ȳ�ȷ����Ϊ32�ı���	*/
	l_width = WIDTHBYTES(width* bitInfoHead.biBitCount);	

	if(l_width > 960)
	{
		BMP_DEBUG_PRINTF("\n ��ͼƬ̫���޷���Һ��������ʾ (<=320)\n");
		return;
	}
	
	
	/* ��һ��ͼƬ��С�Ĵ���*/
	ILI9341_OpenWindow(x, y, width, height);
	ILI9341_Write_Cmd (CMD_SetPixel ); 

	
	/* �ж��Ƿ���24bit���ɫͼ */
	if( bitInfoHead.biBitCount >= 24 )
	{
		for ( i = 0; i < height; i ++ )
		{
			/*���ļ��ĺ������BMP�ļ���ԭʼͼ����Ϊ���½ǵ����Ͻ�*/
      f_lseek ( & bmpfsrc, bitHead .bfOffBits + ( height - i - 1 ) * l_width );	
			
			/* ��ȡһ��bmp�����ݵ�����pColorData���� */
			f_read ( & bmpfsrc, pColorData, l_width, & read_num );				

			for(j=0; j<width; j++) 											   //һ����Ч��Ϣ
			{
				k = j*3;																	 //һ���е�K�����ص����
				red = pColorData[k+2];
				green = pColorData[k+1];
				blue = 	pColorData[k];
				ILI9341_Write_Data ( RGB24TORGB16 ( red, green, blue ) ); //д��LCD-GRAM
			}            			
		}        		
	}    	
	else 
	{        
		BMP_DEBUG_PRINTF("�ⲻ��һ��24λ���ɫBMP�ļ���");
		return ;
	}
	
	f_close(&bmpfsrc);  
  
}




/**
 * @brief  ����ILI9341�Ľ�ȡBMPͼƬ
 * @param  x ����ȡ��������X���� 
 * @param  y ����ȡ��������Y���� 
 * @param  Width ��������
 * @param  Height ������߶� 
 * @retval ��
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0 :��ͼ�ɹ�
  *     @arg -1 :��ͼʧ��
 */
int Screen_Shot( uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, char * filename)
{
	/* bmp  �ļ�ͷ 54���ֽ� */
	unsigned char header[54] =
	{
		0x42, 0x4d, 0, 0, 0, 0, 
		0, 0, 0, 0, 54, 0, 
		0, 0, 40,0, 0, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 
		0, 0, 0
	};
	
	int i;
	int j;
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned int mybw;
	unsigned int read_data;
	char kk[4]={0,0,0,0};
	
	uint8_t ucAlign;//
	
	
	/* ��*�� +������ֽ� + ͷ����Ϣ */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		

	/* �ļ���С 4���ֽ� */
	header[2] = (unsigned char)(file_size &0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	width=Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) &0x000000ff;
	header[20] = (width >> 16) &0x000000ff;
	header[21] = (width >> 24) &0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	height = Height;
	header[22] = height &0x000000ff;
	header[23] = (height >> 8) &0x000000ff;
	header[24] = (height >> 16) &0x000000ff;
	header[25] = (height >> 24) &0x000000ff;
		
	/* �½�һ���ļ� */
	bmpres = f_open( &bmpfsrc , (char*)filename, FA_CREATE_ALWAYS | FA_WRITE );
	
	/* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
	f_close(&bmpfsrc);
		
	bmpres = f_open( &bmpfsrc , (char*)filename,  FA_OPEN_EXISTING | FA_WRITE);

	if ( bmpres == FR_OK )
	{    
		/* ��Ԥ�ȶ���õ�bmpͷ����Ϣд���ļ����� */
		bmpres = f_write(&bmpfsrc, header,sizeof(unsigned char)*54, &mybw);		
			
		ucAlign = Width % 4;
		
		for(i=0; i<Height; i++)					
		{
			for(j=0; j<Width; j++)  
			{					
				read_data = ILI9341_GetPointPixel ( x + j, y + Height - 1 - i );	    // ��һ��RGB565����				
				
        /* ���Ϊ R G B */
				r =  GETR_FROM_RGB16(read_data);
				g =  GETG_FROM_RGB16(read_data);
				b =  GETB_FROM_RGB16(read_data);

        /* д���ļ� */
				bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);
			}
				
			if( ucAlign )				/* �������4�ֽڶ��� */
				bmpres = f_write ( & bmpfsrc, kk, sizeof(unsigned char) * ( ucAlign ), & mybw );

		}/* ������� */

		f_close(&bmpfsrc); 
		
		return 0;
		
	}	
	else/* ����ʧ�� */
		return -1;
}

#include "./ov7725/bsp_ov7725.h"

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

/* ����ͷ�ṹ�� */
typedef __packed struct
{
  uint32_t head;
  uint8_t addr;
  uint32_t len;
  uint8_t cmd;
}packet_head_t;

int write_rgb_file(uint16_t width, uint16_t height, char * filename) 
{
  uint16_t i, j; 
  uint16_t crc_16 = 0xFFFF;
	uint16_t Camera_Data[700];
  unsigned int mybw;

//  packet_head_t packet_head =
//  {
//    .head = 0x59485A53,     // ��ͷ
//    .addr = 0x00,           // �豸��ַ
//    .len  = 0x00000011,     // ������ (10+320*240*2+2)
//    .cmd  = 02,             // ͼ����������
//  };
  
  uint8_t packet_head[10] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                             0x00,                       // �豸��ַ
                             0x00, 0x02, 0x58, 0x0C,     // ������ (10+320*240*2+2)
                             0x02                        // ͼ����������
                             };
  
  uint32_t data_len = 10 + width * height * 2 + 2;
                             
  packet_head[5] = data_len >> 24;
  packet_head[6] = data_len >> 16;
  packet_head[7] = data_len >> 8;
  packet_head[8] = data_len;
  
  memset(Camera_Data, 0xDD, sizeof(Camera_Data));
  	/* �½�һ���ļ� */
	bmpres = f_open( &bmpfsrc , (char*)filename, FA_CREATE_ALWAYS | FA_WRITE );
	
	/* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
	f_close(&bmpfsrc);
		
	bmpres = f_open( &bmpfsrc , (char*)filename,  FA_OPEN_EXISTING | FA_WRITE);

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
    
    f_close(&bmpfsrc);       // �ر��ļ�
  }
  else
  {
    f_close(&bmpfsrc);     // �ر��ļ�
    return -1;    // ����ʧ��
  }
  return 0;    // ���سɹ�
}

void set_upper_computer_format(uint8_t type, uint16_t width, uint16_t height)
{
  uint16_t crc_16 = 0xFFFF;
  
  uint8_t packet_head[17] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                             0x00,                       // �豸��ַ
                             0x00, 0x00, 0x00, 0x11,     // ������ (10+1+2+2+2)                
                             0x01,                       // ����ͼ���ʽָ��
                             };
  
  packet_head[10] = type;
                             
  packet_head[11] = width >> 8;
  packet_head[12] = width & 0xFF;
                             
  packet_head[13] = height >> 8;
  packet_head[14] = height & 0xFF;
                             
  crc_16 = calc_crc_16((uint8_t *)&packet_head, sizeof(packet_head) - 2, crc_16);    // �ֶμ���crc��16��У����, �����ͷ��
  
  packet_head[15] = crc_16 & 0x00FF;
  packet_head[16] = (crc_16 >> 8) & 0x00FF;
                             
//  printf("0x%x\r\n", packet_head[15]);
                             
  debug_send_data((uint8_t *)&packet_head, sizeof(packet_head));
}

int write_rgb_usart(uint16_t width, uint16_t height) 
{
  uint16_t i, j; 
  uint16_t crc_16 = 0xFFFF;
	uint16_t Camera_Data[700];

//  packet_head_t packet_head =
//  {
//    .head = 0x59485A53,     // ��ͷ
//    .addr = 0x00,           // �豸��ַ
//    .len  = 0x00000011,     // ������ (10+320*240*2+2)
//    .cmd  = 02,             // ͼ����������
//  };
  
  uint8_t packet_head[10] = {0x59, 0x48, 0x5A, 0x53,     // ��ͷ
                             0x00,                       // �豸��ַ
                             0x00, 0x02, 0x58, 0x0C,     // ������ (10+320*240*2+2)
                             0x02                        // ͼ����������
                             };
  
  uint32_t data_len = 10 + width * height * 2 + 2;
                             
  packet_head[5] = data_len >> 24;
  packet_head[6] = data_len >> 16;
  packet_head[7] = data_len >> 8;
  packet_head[8] = data_len;
  
  memset(Camera_Data, 0xDD, sizeof(Camera_Data));

  /* ����ͷ */
  debug_send_data((uint8_t *)&packet_head, sizeof(packet_head));
  crc_16 = calc_crc_16((uint8_t *)&packet_head, sizeof(packet_head), crc_16);    // �ֶμ���crc��16��У����, �����ͷ��

  /* ����ͼ������ */
  for(i = 0; i < width; i++)
  {
    for(j = 0; j < height; j++)
    {
      READ_FIFO_PIXEL(Camera_Data[j]);		// ��FIFO����һ��rgb565���ص�Camera_Data����
    }

    debug_send_data((uint8_t *)Camera_Data, j*2);
    crc_16 = calc_crc_16((uint8_t *)Camera_Data, j*2, crc_16);    // �ֶμ���crc��16��У���룬����һ��ͼ������
  }

  /* ����У������ */
  crc_16 = ((crc_16&0x00FF)<<8)|((crc_16&0xFF00)>>8);    //  �������ֽں͵��ֽ�λ��
  debug_send_data((uint8_t *)&crc_16, 2);

  return 0;    // ���سɹ�
}

/**
  * @brief  ������ʾλ��
	* @param  sx:x��ʼ��ʾλ��
	* @param  sy:y��ʼ��ʾλ��
	* @param  width:��ʾ���ڿ��,Ҫ���OV7725_Window_Set�����е�widthһ��
	* @param  height:��ʾ���ڸ߶ȣ�Ҫ���OV7725_Window_Set�����е�heightһ��
  * @param  pic_name:�ļ�·��
  * @retval ��
  */
void FileImagDisp(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height, char *pic_name)
{
	uint16_t i, j; 
	uint16_t color_data[1];
  unsigned int read_num;
	
	ILI9341_OpenWindow(sx,sy,width,height);
	ILI9341_Write_Cmd ( CMD_SetPixel );	
  
  bmpres = f_open( &bmpfsrc , (char *)pic_name, FA_OPEN_EXISTING | FA_READ);	

  if ( bmpres == FR_OK )    // �ļ��򿪳ɹ�
  {
    /* ����ǰ���ͷ��ֱ�Ӵ��������ݿ�ʼ�� */
    f_lseek ( & bmpfsrc, 10);
    
    for ( i = 0; i < height; i ++ )
		{
			for(j=0; j<width; j++) 											   //һ����Ч��Ϣ
			{
        /* ��ȡһ��RGB565�� color_data */
        f_read ( & bmpfsrc, color_data, 2, &read_num);	
        
				ILI9341_Write_Data(color_data[0]);    // д�� LCD �Դ�
			}            			
		}
    
    f_close(&bmpfsrc);       // �ر��ļ�
  }
  else
  {
    f_close(&bmpfsrc);     // �ر��ļ�
  }
}

/**
 * @brief  ��rgbת����bmp
 * @param  x �����X���� 
 * @param  y �����Y���� 
 * @param  Width ��������
 * @param  Height ������߶� 
 * @param  pic_name ��bmp�ļ��� 
 * @param  filename ��rgb�����ļ��� 
 * @retval 
  *   �ò���Ϊ����ֵ֮һ��
  *     @arg 0 :ת���ɹ�
  *     @arg -1 :ת��ʧ��
 */
int rgb_to_bmp( uint16_t x, uint16_t y, uint16_t Width, uint16_t Height, char *pic_name, char * filename)
{
	/* bmp  �ļ�ͷ 54���ֽ� */
	unsigned char header[54] =
	{
		0x42, 0x4d, 0, 0, 0, 0, 
		0, 0, 0, 0, 54, 0, 
		0, 0, 40,0, 0, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 1, 0, 24, 0, 
		0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 
		0, 0, 0
	};
	
	int i;
	int j;
	long file_size;     
	long width;
	long height;
	unsigned char r,g,b;	
	unsigned int mybw;
	char kk[4]={0,0,0,0};
  
  uint16_t color_data[1];
  unsigned int read_num;
	
	uint8_t ucAlign;//
	
	
	/* ��*�� +������ֽ� + ͷ����Ϣ */
	file_size = (long)Width * (long)Height * 3 + Height*(Width%4) + 54;		

	/* �ļ���С 4���ֽ� */
	header[2] = (unsigned char)(file_size &0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	width=Width;	
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) &0x000000ff;
	header[20] = (width >> 16) &0x000000ff;
	header[21] = (width >> 24) &0x000000ff;
	
	/* λͼ�� 4���ֽ� */
	height = Height;
	header[22] = height &0x000000ff;
	header[23] = (height >> 8) &0x000000ff;
	header[24] = (height >> 16) &0x000000ff;
	header[25] = (height >> 24) &0x000000ff;
		
	/* �½�һ���ļ� */
	bmpres = f_open( &bmpfsrc , (char*)pic_name, FA_CREATE_ALWAYS | FA_WRITE );
  if (bmpres == FR_OK)
  {
    printf("���ļ� %s �ɹ�\r\n", pic_name);
  }
  else
  {
    printf("ɾ��ʧ��->(%d)\r\n", bmpres);
  }
  
  bmpres = f_open( &bmpfdst , (char *)filename, FA_OPEN_EXISTING | FA_READ);	    // ��rgb�����ļ�
  if (bmpres == FR_OK)
  {
    printf("���ļ� %s �ɹ�\r\n", filename);
  }
  else
  {
    printf("ɾ��ʧ��->(%d)\r\n", bmpres);
  }
	
	/* �½��ļ�֮��Ҫ�ȹر��ٴ򿪲���д�� */
	f_close(&bmpfsrc);
		
	bmpres = f_open( &bmpfsrc , (char*)pic_name,  FA_OPEN_EXISTING | FA_WRITE);
  if (bmpres == FR_OK)
  {
    printf("���ļ� %s �ɹ�\r\n", pic_name);
  }
  else
  {
    printf("ɾ��ʧ��->(%d)\r\n", bmpres);
  }

	if ( bmpres == FR_OK )
	{    
		/* ��Ԥ�ȶ���õ�bmpͷ����Ϣд���ļ����� */
		bmpres = f_write(&bmpfsrc, header,sizeof(unsigned char)*54, &mybw);		
			
		ucAlign = Width % 4;
    
    /* ����ǰ���ͷ��ֱ�Ӵ��������ݿ�ʼ�� */
    f_lseek ( & bmpfdst, 10);
		
		for(i=0; i<Height; i++)					
		{
			for(j=0; j<Width; j++)  
			{
        /* ��ȡһ��RGB565�� color_data */
        f_read ( & bmpfdst, color_data, 2, &read_num);
				//read_data = ILI9341_GetPointPixel ( x + j, y + Height - 1 - i );	    // ��һ��RGB565����				
				
        /* ���Ϊ R G B */
				r =  GETR_FROM_RGB16(color_data[0]);
				g =  GETG_FROM_RGB16(color_data[0]);
				b =  GETB_FROM_RGB16(color_data[0]);

        /* д���ļ� */
				bmpres = f_write(&bmpfsrc, &b,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &g,sizeof(unsigned char), &mybw);
				bmpres = f_write(&bmpfsrc, &r,sizeof(unsigned char), &mybw);
			}
				
			if( ucAlign )				/* �������4�ֽڶ��� */
				bmpres = f_write ( & bmpfsrc, kk, sizeof(unsigned char) * ( ucAlign ), & mybw );

		}/* ������� */

		bmpres = f_close(&bmpfsrc); 
    if (bmpres == FR_OK)
    {
      printf("�ر��ļ��ɹ�\r\n");
    }
    else
    {
      printf("ɾ��ʧ��->(%d)\r\n", bmpres);
    }
  
    bmpres = f_close(&bmpfdst); 
    if (bmpres == FR_OK)
    {
      printf("�ر��ļ��ɹ�\r\n");
    }
    else
    {
      printf("ɾ��ʧ��->(%d)\r\n", bmpres);
    }
		
		return 0;
		
	}	
	else/* ����ʧ�� */
		return -1;
}
