#ifndef __DELAY_H
#define __DELAY_H

#include <stm32f10x_lib.h>

/******************************************************
ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���
����delay_us, delay_ms
*******************************************************/

void DelayInit(u8 SYSCLK);
void DelayMs(u16 nms);
void DelayUs(u32 nus);

#endif
