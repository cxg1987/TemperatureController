#ifndef __ADC_H
#define __ADC_H

#include <stm32f10x_lib.h>
#include "usart.h"
#include "../SYSTEM/sys.h"
#include "../SYSTEM/delay.h"

/******************************************************
* ADC����
���� AD1��ʼ������ת�����
*******************************************************/

#define ADC1_DR_Address    ((u32)0x4001244C)


void ADC1_Init(void);
int GET_AD_Data(void);	 //��ѯ��ʽ���ADת�����

void  DMA1_Init(void);
void DIS_AD_VAL(void);


#endif
