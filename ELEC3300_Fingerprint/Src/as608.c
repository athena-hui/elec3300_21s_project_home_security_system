#include "as608.h"
#include "rgb_led.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"

uint32_t AS608Addr = 0XFFFFFFFF; //Ĭ��
char str2[6] = {0};
//���ڽ��ջ����� 	
uint8_t USART1_RX_BUF[USART1_MAX_RECV_LEN]; 				//���ջ���,���USART1_MAX_RECV_LEN���ֽ�.
uint8_t USART1_TX_BUF[USART1_MAX_SEND_LEN]; 			  //���ͻ���,���USART1_MAX_SEND_LEN�ֽ�

//ͨ���жϽ�������2���ַ�֮���ʱ������10ms�������ǲ���һ������������.
//���2���ַ����ռ������10ms,����Ϊ����1����������.Ҳ���ǳ���10msû�н��յ�
//�κ�����,���ʾ�˴ν������.
//���յ�������״̬
//[15]:0,û�н��յ�����;1,���յ���һ������.
//[14:0]:���յ������ݳ���
__IO uint16_t USART1_RX_STA=0;



//���ڷ���һ���ֽ�
static void MYUSART_SendData(uint8_t data)
{
  while((USART1->SR & 0X40) == 0);
  USART1->DR = data;
}
//���Ͱ�ͷ
static void SendHead(void)
{
  MYUSART_SendData(0xEF);
  MYUSART_SendData(0x01);
}
//���͵�ַ
static void SendAddr(void)
{
  MYUSART_SendData(AS608Addr >> 24);
  MYUSART_SendData(AS608Addr >> 16);
  MYUSART_SendData(AS608Addr >> 8);
  MYUSART_SendData(AS608Addr);
}
//���Ͱ���ʶ,
static void SendFlag(uint8_t flag)
{
  MYUSART_SendData(flag);
}
//���Ͱ�����
static void SendLength(int length)
{
  MYUSART_SendData(length >> 8);
  MYUSART_SendData(length);
}
//����ָ����
static void Sendcmd(uint8_t cmd)
{
  MYUSART_SendData(cmd);
}
//����У���
static void SendCheck(uint16_t check)
{
  MYUSART_SendData(check >> 8);
  MYUSART_SendData(check);
}
//�ж��жϽ��յ�������û��Ӧ���
//waittimeΪ�ȴ��жϽ������ݵ�ʱ�䣨��λ1ms��
//����ֵ�����ݰ��׵�ַ
static uint8_t *JudgeStr(uint16_t waittime)
{
  char *data;
  uint8_t str[8];
  str[0] = 0xef;
  str[1] = 0x01;
  str[2] = AS608Addr >> 24;
  str[3] = AS608Addr >> 16;
  str[4] = AS608Addr >> 8;
  str[5] = AS608Addr;
  str[6] = 0x07;
  str[7] = '\0';
//  USART1_RX_STA = 0;
		//HAL���д��
	HAL_UART_Receive(&huart1,(uint8_t *)USART1_RX_BUF,USART1_MAX_RECV_LEN,waittime/4);//��������������
	
	if(!memcmp(str,USART1_RX_BUF,7))//�ȶ�����
	{
		  data = strstr((const char*)USART1_RX_BUF, (const char*)str);
      if(data)
        return (uint8_t*)data;
	}
	//
//  while(--waittime)
//  {
//    HAL_Delay(1);
//    if(USART1_RX_STA & 0X8000) //���յ�һ������
//    {
//      USART1_RX_STA = 0;
//      data = strstr((const char*)USART1_RX_BUF, (const char*)str);
//      if(data)
//        return (uint8_t*)data;
//    }
//  }
  return 0;
}
//¼��ͼ�� PS_GetImage
//����:̽����ָ��̽�⵽��¼��ָ��ͼ�����ImageBuffer��
//ģ�鷵��ȷ����
uint8_t PS_GetImage(void)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x01);
  temp =  0x01 + 0x03 + 0x01;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//�������� PS_GenChar
//����:��ImageBuffer�е�ԭʼͼ������ָ�������ļ�����CharBuffer1��CharBuffer2
//����:BufferID --> charBuffer1:0x01	charBuffer1:0x02
//ģ�鷵��ȷ����
uint8_t PS_GenChar(uint8_t BufferID)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x04);
  Sendcmd(0x02);
  MYUSART_SendData(BufferID);
  temp = 0x01 + 0x04 + 0x02 + BufferID;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//��ȷ�ȶ���öָ������ PS_Match
//����:��ȷ�ȶ�CharBuffer1 ��CharBuffer2 �е������ļ�
//ģ�鷵��ȷ����
uint8_t PS_Match(void)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x03);
  temp = 0x01 + 0x03 + 0x03;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//����ָ�� PS_Search
//����:��CharBuffer1��CharBuffer2�е������ļ����������򲿷�ָ�ƿ�.�����������򷵻�ҳ�롣
//����:  BufferID @ref CharBuffer1	CharBuffer2
//˵��:  ģ�鷵��ȷ���֣�ҳ�루����ָ��ģ�壩
uint8_t PS_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x08);
  Sendcmd(0x04);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(StartPage >> 8);
  MYUSART_SendData(StartPage);
  MYUSART_SendData(PageNum >> 8);
  MYUSART_SendData(PageNum);
  temp = 0x01 + 0x08 + 0x04 + BufferID
         + (StartPage >> 8) + (uint8_t)StartPage
         + (PageNum >> 8) + (uint8_t)PageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    p->pageID   = (data[10] << 8) + data[11];
    p->mathscore = (data[12] << 8) + data[13];
  }
  else
    ensure = 0xff;
  return ensure;
}
//�ϲ�����������ģ�壩PS_RegModel
//����:��CharBuffer1��CharBuffer2�е������ļ��ϲ����� ģ��,�������CharBuffer1��CharBuffer2
//˵��:  ģ�鷵��ȷ����
uint8_t PS_RegModel(void)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x05);
  temp = 0x01 + 0x03 + 0x05;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//����ģ�� PS_StoreChar
//����:�� CharBuffer1 �� CharBuffer2 �е�ģ���ļ��浽 PageID ��flash���ݿ�λ�á�
//����:  BufferID @ref charBuffer1:0x01	charBuffer1:0x02
//       PageID��ָ�ƿ�λ�úţ�
//˵��:  ģ�鷵��ȷ����
uint8_t PS_StoreChar(uint8_t BufferID, uint16_t PageID)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x06);
  Sendcmd(0x06);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(PageID >> 8);
  MYUSART_SendData(PageID);
  temp = 0x01 + 0x06 + 0x06 + BufferID
         + (PageID >> 8) + (uint8_t)PageID;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//ɾ��ģ�� PS_DeletChar
//����:  ɾ��flash���ݿ���ָ��ID�ſ�ʼ��N��ָ��ģ��
//����:  PageID(ָ�ƿ�ģ���)��Nɾ����ģ�������
//˵��:  ģ�鷵��ȷ����
uint8_t PS_DeletChar(uint16_t PageID, uint16_t N)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x07);
  Sendcmd(0x0C);
  MYUSART_SendData(PageID >> 8);
  MYUSART_SendData(PageID);
  MYUSART_SendData(N >> 8);
  MYUSART_SendData(N);
  temp = 0x01 + 0x07 + 0x0C
         + (PageID >> 8) + (uint8_t)PageID
         + (N >> 8) + (uint8_t)N;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//���ָ�ƿ� PS_Empty
//����:  ɾ��flash���ݿ�������ָ��ģ��
//����:  ��
//˵��:  ģ�鷵��ȷ����
uint8_t PS_Empty(void)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x0D);
  temp = 0x01 + 0x03 + 0x0D;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
	{
    ensure = data[9];
	}
  else
    ensure = 0xff;
  return ensure;
}
//дϵͳ�Ĵ��� PS_WriteReg
//����:  дģ��Ĵ���
//����:  �Ĵ������RegNum:4\5\6
//˵��:  ģ�鷵��ȷ����
uint8_t PS_WriteReg(uint8_t RegNum, uint8_t DATA)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x05);
  Sendcmd(0x0E);
  MYUSART_SendData(RegNum);
  MYUSART_SendData(DATA);
  temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  if(ensure == 0)
    printf("\r\n���ò����ɹ���");
  else
    printf("\r\n%s", EnsureMessage(ensure));
  return ensure;
}
//��ϵͳ�������� PS_ReadSysPara
//����:  ��ȡģ��Ļ��������������ʣ�����С��)
//����:  ��
//˵��:  ģ�鷵��ȷ���� + ����������16bytes��
uint8_t PS_ReadSysPara(SysPara *p)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x0F);
  temp = 0x01 + 0x03 + 0x0F;
  SendCheck(temp);
  data = JudgeStr(1000);
  if(data)
  {
    ensure = data[9];
    p->PS_max = (data[14] << 8) + data[15];
    p->PS_level = data[17];
    p->PS_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
    p->PS_size = data[23];
    p->PS_N = data[25];
  }
  else
    ensure = 0xff;
  if(ensure == 0x00)
  {
    printf("\r\nģ�����ָ������=%d", p->PS_max);
    printf("\r\n�Աȵȼ�=%d", p->PS_level);
    printf("\r\n��ַ=%x", p->PS_addr);
    printf("\r\n������=%d", p->PS_N * 9600);
  }
  else
    printf("\r\n%s", EnsureMessage(ensure));
  return ensure;
}
//����ģ���ַ PS_SetAddr
//����:  ����ģ���ַ
//����:  PS_addr
//˵��:  ģ�鷵��ȷ����
uint8_t PS_SetAddr(uint32_t PS_addr)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x07);
  Sendcmd(0x15);
  MYUSART_SendData(PS_addr >> 24);
  MYUSART_SendData(PS_addr >> 16);
  MYUSART_SendData(PS_addr >> 8);
  MYUSART_SendData(PS_addr);
  temp = 0x01 + 0x07 + 0x15
         + (uint8_t)(PS_addr >> 24) + (uint8_t)(PS_addr >> 16)
         + (uint8_t)(PS_addr >> 8) + (uint8_t)PS_addr;
  SendCheck(temp);
  AS608Addr = PS_addr; //������ָ�������ַ
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  AS608Addr = PS_addr;
  if(ensure == 0x00)
    printf("\r\n���õ�ַ�ɹ���");
  else
    printf("\r\n%s", EnsureMessage(ensure));
  return ensure;
}
//���ܣ� ģ���ڲ�Ϊ�û�������256bytes��FLASH�ռ����ڴ��û����±�,
//	�ü��±��߼��ϱ��ֳ� 16 ��ҳ��
//����:  NotePageNum(0~15),Byte32(Ҫд�����ݣ�32���ֽ�)
//˵��:  ģ�鷵��ȷ����
uint8_t PS_WriteNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
  uint16_t temp;
  uint8_t  ensure, i;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(36);
  Sendcmd(0x18);
  MYUSART_SendData(NotePageNum);
  for(i = 0; i < 32; i++)
  {
    MYUSART_SendData(Byte32[i]);
    temp += Byte32[i];
  }
  temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
    ensure = data[9];
  else
    ensure = 0xff;
  return ensure;
}
//������PS_ReadNotepad
//���ܣ�  ��ȡFLASH�û�����128bytes����
//����:  NotePageNum(0~15)
//˵��:  ģ�鷵��ȷ����+�û���Ϣ
uint8_t PS_ReadNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
  uint16_t temp;
  uint8_t  ensure, i;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x04);
  Sendcmd(0x19);
  MYUSART_SendData(NotePageNum);
  temp = 0x01 + 0x04 + 0x19 + NotePageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    for(i = 0; i < 32; i++)
    {
      Byte32[i] = data[10 + i];
    }
  }
  else
    ensure = 0xff;
  return ensure;
}
//��������PS_HighSpeedSearch
//���ܣ��� CharBuffer1��CharBuffer2�е������ļ��������������򲿷�ָ�ƿ⡣
//		  �����������򷵻�ҳ��,��ָ����ڵ�ȷ������ָ�ƿ��� ���ҵ�¼ʱ����
//		  �ܺõ�ָ�ƣ���ܿ�������������
//����:  BufferID�� StartPage(��ʼҳ)��PageNum��ҳ����
//˵��:  ģ�鷵��ȷ����+ҳ�루����ָ��ģ�壩
uint8_t PS_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x08);
  Sendcmd(0x1b);
  MYUSART_SendData(BufferID);
  MYUSART_SendData(StartPage >> 8);
  MYUSART_SendData(StartPage);
  MYUSART_SendData(PageNum >> 8);
  MYUSART_SendData(PageNum);
  temp = 0x01 + 0x08 + 0x1b + BufferID
         + (StartPage >> 8) + (uint8_t)StartPage
         + (PageNum >> 8) + (uint8_t)PageNum;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    p->pageID 	= (data[10] << 8) + data[11];
    p->mathscore = (data[12] << 8) + data[13];
  }
  else
    ensure = 0xff;
  return ensure;
}
//����Чģ����� PS_ValidTempleteNum
//���ܣ�����Чģ�����
//����: ��
//˵��: ģ�鷵��ȷ����+��Чģ�����ValidN
uint8_t PS_ValidTempleteNum(uint16_t *ValidN)
{
  uint16_t temp;
  uint8_t  ensure;
  uint8_t  *data;
  SendHead();
  SendAddr();
  SendFlag(0x01);//�������ʶ
  SendLength(0x03);
  Sendcmd(0x1d);
  temp = 0x01 + 0x03 + 0x1d;
  SendCheck(temp);
  data = JudgeStr(2000);
  if(data)
  {
    ensure = data[9];
    *ValidN = (data[10] << 8) + data[11];
  }
  else
    ensure = 0xff;

  if(ensure == 0x00)
  {
		LCD_Clear(0, 0, 240, 320, WHITE);
		char str[30];
		sprintf(str, "number of models: %d", *ValidN);
		LCD_DrawString(10, 10, str);
  }
  else
    printf("\r\n%s", EnsureMessage(ensure));
  return ensure;
}
//��AS608���� PS_HandShake
//����: PS_Addr��ַָ��
//˵��: ģ�鷵�µ�ַ����ȷ��ַ��
uint8_t PS_HandShake(uint32_t *PS_Addr)
{
  SendHead();
  SendAddr();
  MYUSART_SendData(0X01);
  MYUSART_SendData(0X00);
  MYUSART_SendData(0X00);
  HAL_Delay(200);
  if(USART1_RX_STA & 0X8000) //���յ�����
  {
		
    if(//�ж��ǲ���ģ�鷵�ص�Ӧ���
      USART1_RX_BUF[0] == 0XEF
      && USART1_RX_BUF[1] == 0X01
      && USART1_RX_BUF[6] == 0X07
    )
    {
      *PS_Addr = (USART1_RX_BUF[2] << 24) + (USART1_RX_BUF[3] << 16)
                 + (USART1_RX_BUF[4] << 8) + (USART1_RX_BUF[5]);
      USART1_RX_STA = 0;
      return 0;
    }
    USART1_RX_STA = 0;
  }
  return 1;
}
//ģ��Ӧ���ȷ������Ϣ����
//���ܣ�����ȷ���������Ϣ������Ϣ
//����: ensure
const char *EnsureMessage(uint8_t ensure)
{
  const char *p;
  switch(ensure)
  {
  case  0x00:
    p = "       OK       ";
    break;
  case  0x01:
    p = " ���ݰ����մ��� ";
    break;
  case  0x02:
    p = "��������û����ָ";
    break;
  case  0x03:
    p = "¼��ָ��ͼ��ʧ��";
    break;
  case  0x04:
    p = " ָ��̫�ɻ�̫�� ";
    break;
  case  0x05:
    p = " ָ��̫ʪ��̫�� ";
    break;
  case  0x06:
    p = "  ָ��ͼ��̫��  ";
    break;
  case  0x07:
    p = " ָ��������̫�� ";
    break;
  case  0x08:
    p = "  ָ�Ʋ�ƥ��    ";
    break;
  case  0x09:
    p = " û��������ָ�� ";
    break;
  case  0x0a:
    p = "   �����ϲ�ʧ�� ";
    break;
  case  0x0b:
    p = "��ַ��ų�����Χ";
  case  0x10:
    p = "  ɾ��ģ��ʧ��  ";
    break;
  case  0x11:
    p = " ���ָ�ƿ�ʧ�� ";
    break;
  case  0x15:
    p = "������������Чͼ";
    break;
  case  0x18:
    p = " ��дFLASH����  ";
    break;
  case  0x19:
    p = "   δ�������   ";
    break;
  case  0x1a:
    p = "  ��Ч�Ĵ�����  ";
    break;
  case  0x1b:
    p = " �Ĵ������ݴ��� ";
    break;
  case  0x1c:
    p = " ���±�ҳ����� ";
    break;
  case  0x1f:
    p = "    ָ�ƿ���    ";
    break;
  case  0x20:
    p = "    ��ַ����    ";
    break;
  default :
    p = " ����ȷ�������� ";
    break;
  }
  return p;
}

//��ʾȷ���������Ϣ
void ShowErrMessage(uint8_t ensure)
{
  //OLED_ShowMixedCH(5,0,(uint8_t*)EnsureMessage(ensure));
}


//¼ָ��
//void Add_FR(void)
//{
//  uint8_t i, ensure, processnum = 0;
//  uint8_t ID_NUM = 0;
//	uint8_t key_num = 0;
//  while(1)
//  {
//    switch (processnum)
//    {
//    case 0:
//      i++;
//			OLED_FullyClear();
//      OLED_ShowMixedCH(0, 16, "    �밴��ָ    ");
//			OLED_RefreshRAM();
//      ensure = PS_GetImage();
//      if(ensure == 0x00)
//      {
//        ensure = PS_GenChar(CharBuffer1); //��������
//        if(ensure == 0x00)
//        {
//          OLED_ShowMixedCH(0, 16, "    ָ������    ");
//          OLED_ShowMixedCH(0, 16, "                ");
//					OLED_RefreshRAM();
//          i = 0;
//          processnum = 1; //�����ڶ���
//        }
//        else ShowErrMessage(ensure);
//      }
//      else ShowErrMessage(ensure);
//      break;

//    case 1:
//      i++;
//      OLED_ShowMixedCH(0, 16, "   ���ٰ�һ��   ");
//      OLED_ShowMixedCH(0, 16, "                ");
//	  	OLED_RefreshRAM();
//      ensure = PS_GetImage();
//      if(ensure == 0x00)
//      {
//        ensure = PS_GenChar(CharBuffer2); //��������
//        if(ensure == 0x00)
//        {
//          OLED_ShowMixedCH(0, 16, "    ָ������    ");
//          OLED_ShowMixedCH(0, 32, "                ");
//					OLED_RefreshRAM();
//          i = 0;
//          processnum = 2; //����������
//        }
//        else ShowErrMessage(ensure);
//      }
//      else ShowErrMessage(ensure);
//      break;

//    case 2:
//      OLED_ShowMixedCH(0, 16, "  �Ա�����ָ��  ");
//      OLED_ShowMixedCH(0, 32, "                ");
//			OLED_RefreshRAM();
//      ensure = PS_Match();
//      if(ensure == 0x00)
//      {
//        OLED_ShowMixedCH(0, 16, "    �Աȳɹ�    ");
//        OLED_ShowMixedCH(0, 32, "                ");
//				OLED_RefreshRAM();
//        processnum = 3; //�������Ĳ�
//      }
//      else
//      {
//        OLED_ShowMixedCH(0, 16, "    �Ա�ʧ��    ");
//        OLED_ShowMixedCH(0, 32, "                ");
//				OLED_RefreshRAM();
//        ShowErrMessage(ensure);
//        i = 0;
//        processnum = 0; //���ص�һ��
//      }
//      HAL_Delay(500);
//      break;

//    case 3:
//      OLED_ShowMixedCH(0, 16, "  ����ָ��ģ��  ");
//      OLED_ShowMixedCH(0, 32, "                ");
//			OLED_RefreshRAM();
//      HAL_Delay(500);
//      ensure = PS_RegModel();
//      if(ensure == 0x00)
//      {
//        OLED_ShowMixedCH(0, 16, "����ָ��ģ��ɹ�");
//        OLED_ShowMixedCH(0, 32, "                ");
//				OLED_RefreshRAM();
//        processnum = 4; //�������岽
//      }
//      else
//      {
//        processnum = 0;
//        ShowErrMessage(ensure);
//      }
//      HAL_Delay(1000);
//      break;

//    case 4:
//			OLED_FullyClear();
//      OLED_ShowMixedCH(0, 0, " ��K4��,��K2�� ");
//      OLED_ShowMixedCH(0, 16, "    ��K3����    ");
//      OLED_ShowMixedCH(0, 32, "  0=< ID <=99   ");
//			OLED_RefreshRAM();
//      while(key_num != KEYENTER_PRES)
//      {
//        key_num = key_scan();
//        if(key_num == KEYDOWN_PRES)
//        {
//          key_num = 0;
//          if(ID_NUM > 0)
//            ID_NUM--;
//        }
//        if(key_num == KEYUP_PRES)
//        {
//          key_num = 0;
//          if(ID_NUM < 99)
//            ID_NUM++;
//        }
//				show_ID(40,48,ID_NUM);
//      }
//      key_num = 0;
//      ensure = PS_StoreChar(CharBuffer2, ID_NUM); //����ģ��
//      if(ensure == 0x00)
//      {
//        OLED_FullyClear();
//        OLED_ShowMixedCH(0, 16, "  ¼��ָ�Ƴɹ�  ");
//        OLED_ShowMixedCH(0, 32, "                ");
//				OLED_RefreshRAM();
//        HAL_Delay(1500);
//        OLED_FullyClear();
//        OLED_ShowMixedCH(0, 0, "ָ��ģ����Գ���");
//        OLED_ShowMixedCH(16, 16, "K1�����ָ��");
//        OLED_ShowMixedCH(16, 32, "K3��ɾ��ָ��");
//        OLED_ShowMixedCH(16, 48, "K5����ָ֤��");
//				OLED_RefreshRAM();
//        return ;
//      }
//      else
//      {
//        OLED_FullyClear();
//        processnum = 0;
//        ShowErrMessage(ensure);
//      }
//      break;
//    }
//    HAL_Delay(400);
//    if(i == 10) //����5��û�а���ָ���˳�
//    {
//      break;
//    }
//  }
//}

SysPara AS608Para;//ָ��ģ��AS608����
//ˢָ��
void press_FR(void)
{
  SearchResult seach;
  uint8_t ensure;
  char str[20];
	uint8_t key_num = 0;
  while(1 /*key_num != KEYBACK_PRES*/)
  {
    //key_num = key_scan();
    ensure = PS_GetImage();
    if(ensure == 0x00) //��ȡͼ��ɹ�
    {
      ensure = PS_GenChar(CharBuffer1);
      if(ensure == 0x00) //���������ɹ�
      {
        ensure = PS_HighSpeedSearch(CharBuffer1, 0, 99, &seach);
        if(ensure == 0x00) //�����ɹ�
        {
					LCD_Clear(0, 0, 240, 320, WHITE);
          LCD_DrawString(10, 10, "Verification succeeded.\0");
					LCD_Clear(0, 0, 240, 320, WHITE);
          sprintf(str, " ID:%d score:%d ", seach.pageID, seach.mathscore);
          LCD_DrawString(10, 10, str);
          HAL_Delay(1500);
          HAL_Delay(1500);
					return;
        }
        else
        {
					LCD_Clear(0, 0, 240, 320, WHITE);
          LCD_DrawString(10, 10, "Verification failed.\0");
          HAL_Delay(1500);
					return;
        }
      }
      else
			{
				LCD_Clear(0, 0, 240, 320, WHITE);
				LCD_DrawString(10, 10, "Cannot communicate with the module!\0");
			}
      LCD_Clear(0, 0, 240, 320, WHITE);
      LCD_DrawString(10, 10, "Please press your finger.\0");
    }
		else
		{
			LCD_Clear(0, 0, 240, 320, WHITE);
			LCD_DrawString(10, 10, "No response!\0");
		}
  }
  LCD_Clear(0, 0, 240, 320, WHITE	);

}

//ɾ��ָ��
//void Del_FR(void)
//{
//  uint8_t  ensure;
//  uint16_t ID_NUM = 0;
//	uint8_t key_num = 0;
//	OLED_FullyClear();
//  OLED_ShowMixedCH(0, 16, "K4�� K2�� K3ȷ��");
//  OLED_ShowMixedCH(0, 32, "K1���� 0=<ID<=99");
//	OLED_RefreshRAM();
//  while(key_num != KEYENTER_PRES)
//  {
//    key_num = key_scan();
//    if(key_num == KEYDOWN_PRES)
//    {
//      key_num = 0;
//      if(ID_NUM > 0)
//        ID_NUM--;
//    }
//    if(key_num == KEYUP_PRES)
//    {
//      key_num = 0;
//      if(ID_NUM < 99)
//        ID_NUM++;
//    }
//    if(key_num == KEYBACK_PRES)
//      goto MENU ; //������ҳ��
//		show_ID(40,48,ID_NUM);
//  }
//  ensure = PS_DeletChar(ID_NUM, 1); //ɾ������ָ��
//  if(ensure == 0)
//  {
//    OLED_FullyClear();
//    OLED_ShowMixedCH(0, 16, "  ɾ��ָ�Ƴɹ�  ");
//		OLED_RefreshRAM();
//  }
//  else
//    ShowErrMessage(ensure);
//  HAL_Delay(1500);
//MENU:
//  OLED_FullyClear();
//  OLED_ShowMixedCH(0, 0, "ָ��ģ����Գ���");
//  OLED_ShowMixedCH(16, 16, "K1�����ָ��");
//  OLED_ShowMixedCH(16, 32, "K3��ɾ��ָ��");
//  OLED_ShowMixedCH(16, 48, "K5����ָ֤��");
//	OLED_RefreshRAM();
//  key_num = 0;
//}

void delete_ALL(void)
{
		uint8_t ensure;
   ensure = PS_Empty(); //���ָ�ƿ�
   if(ensure == 0)
   {
			LCD_Clear(0, 0, 240, 320, WHITE);
			LCD_DrawString(10, 10, "Deleted all models\0");
   }
   else
	 {
      ShowErrMessage(ensure);
      HAL_Delay(1500);
   }
}
