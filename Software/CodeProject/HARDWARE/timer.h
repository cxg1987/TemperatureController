#ifndef __TIMER_H
#define __TIMER_H

#include <stm32f10x_lib.h>
#include "../SYSTEM/sys.h"
#include "led.h"
#include "../SYSTEM/delay.h"

/******************************************************
* ��ʱ����������
*******************************************************/

void Timerx_Init(u16 arr, u16 psc);
void TIM3_IRQHandler(void);

#endif
