#ifndef __WIA_H
#define	__WIA_H


#include "stm32f10x.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "./usart/bsp_usart.h"

/* ͼ���ʽ�궨�� */
#define PIC_FORMAT_JPEG       0x01u    // ͼ��Ϊ JPEG ͼƬ
#define PIC_FORMAT_BMP        0x02u    // ͼ��Ϊ BMP  ͼƬ
#define PIC_FORMAT_PNG        0x03u    // ͼ��Ϊ PNG  ͼƬ
#define PIC_FORMAT_RGB565     0x04u    // ͼ��Ϊ RGB565 ����
#define PIC_FORMAT_RGB888     0x05u    // ͼ��Ϊ RGB888 ����
#define PIC_FORMAT_WB         0x06u    // ͼ��Ϊ��ֵ������

/* ֡ͷ�궨�� */
#define FRAME_HEADER    0x59485A53u    // ֡ͷ

/* ����궨�� */
#define CMD_ACK          0x00u   // Ӧ���ָ��
#define CMD_FORMAT       0x01u   // ������λ��ͼ���ʽ�����ָ��
#define CMD_PIC_DATA     0x02u   // ����ͼ������ָ��
#define CMD_WRITE_REG    0x10u   // д�Ĵ���ָ��
#define CMD_READ_REG     0x20u   // ���Ĵ���ָ��

/* �������ݽӿ� */
#define CAM_ASS_SEND_DATA(data, len)     debug_send_data(data, len)
//#define CAM_ASS_SEND_DATA(data, len)       send(SOCK_TCPC,data,len);

void set_wincc_format(uint8_t addr, uint8_t type, uint16_t width, uint16_t height);
int write_rgb_wincc(uint8_t addr, uint16_t width, uint16_t height) ;
int write_rgb_file(uint8_t addr, uint16_t width, uint16_t height, char *file_name) ;

#endif /* __WIA_H */

