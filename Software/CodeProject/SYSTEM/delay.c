 #include <stm32f10x_lib.h>
 #include "delay.h"

 /******************************************************
* ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���
* ����delay_us, delay_ms
*******************************************************/

static u8  fac_us=0;   //us��ʱ������
static u16 fac_ms=0;   //ms��ʱ������

 /******************************************************
* ��ʼ����ʱ����
* SYSTICK��ʱ�ӹ̶�ΪHCLCʱ�ӵ�1/8
* SYSCLK: ϵͳʱ��Ƶ�ʣ���λΪHZ
*******************************************************/

void DelayInit(u8 SYSCLK)
{
	 SysTick->CTRL&=0xfffffffb;  //bit2��0��ѡ���ⲿʱ�� HCLK/8
	 fac_us=SYSCLK/8;
	 fac_ms=(u16)fac_us*1000;
}

 /******************************************************
* ��ʱnus����
*******************************************************/

void DelayUs(u32 nus)
{
	u32 temp;
	SysTick->LOAD=nus*fac_us;  //���ؼ���ֵ
	SysTick->VAL=0x00;		   //��ռ�����
	SysTick->CTRL=0x01;		   //��ʼ����
	
	do
	{
	   temp=SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&1<<16)); //�ȴ�ʱ�䵽��
	SysTick->CTRL=0x00;		  //�رռ�����
	SysTick->VAL =0x00;		  //��ռ�����
}

 /******************************************************
* ��ʱnms����
* ע��nms�ķ�Χ
* SysTick->LOADΪ24λ�Ĵ���,����,�����ʱΪ:
* nms<=0xffffff*8/SYSCLK*1000
* SYSCLK��λΪHz,nms��λΪms
* ��72M������,nms<=1864 
*******************************************************/

void DelayMs(u16 nms)
{
	u32 temp;
	SysTick->LOAD=(u32)nms*fac_ms;	//����ֵ����(SysTick->LOADΪ24bit)
	SysTick->VAL=0x00;			    //��ռ�����
	SysTick->CTRL=0x01;				//��ʼ����
	do
	{
		temp=SysTick->CTRL;
	}
	while((temp&0x01)&&!(temp&(1<<16)));//�ȴ�ʱ�䵽��   
	SysTick->CTRL=0x00;       //�رռ�����
	SysTick->VAL =0X00;       //��ռ�����				   
}
