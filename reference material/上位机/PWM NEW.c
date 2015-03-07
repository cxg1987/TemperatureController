
/* ���桢�������п��ƣ������ٿ��ƣ�������ʱ�䣩 */
#include <stc_cpu.H>
#include <intrins.h>
#include <stdio.h>
#include <PWM_NEW.h>


#define U8 unsigned char
#define U16 unsigned int


// pwm��ʼ��

void  Pwm_Init()
{
	CCON = 0;		   //�������жϱ�־��timer stop running
	CMOD = 0X02;	   //set PCA/PWM timer clock source as SYCCLK/2

	CL = 0;		   // set PCA base timer
	CH = 0;

	//CCAP0H = CCAP0L = 0x80;		 //PWM0 ռ�ձ�50%
	CCAPM0 = 0X42;			 //PWM0 ʹ��

	//CCAP1H = CCAP1L = 0x80;	     //PWM1 ռ�ձ�50%
	CCAPM1 = 0X42;			 //PWM1 ʹ��

    CR = 1;					 //	 start timer

}






