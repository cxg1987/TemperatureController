#include <STC_CPU.H>
#include"intrins.h"
#include"math.h"
#include <String.h>
#include"uart.h"
#include"AD.h"
void AD_init();
void AD_work( BYTE val,BYTE channel);
sbit REL0 = P2^4;
sbit REL1 = P2^5;
extern u16 g_usRegister[66]; 
BYTE g_ucChn; // ��ǰ�ɼ���ͨ��

//------------------------------------------------------------------------------
void AD_init()
{
	P1ASF=0x03;         //P1.0��P1.1����Ϊģ�⹦��A/Dʹ��
    AUXR1|=0x04;
	ADC_RES=0;         //����ת������Ĵ�����2λ
	ADC_RESL=0;       //����ת������Ĵ�����8λ
	ADC_CONTR=0x80;  //����AD��Դ
	IE |= 0xa0;      //����AD�ж�
	g_ucChn = 0;
	delay(2);       //�ȴ�1ms����AD��Դ�ȶ�
	ADC_CONTR = 0x88 | g_ucChn;
}	

void adc_isr() interrupt 5 using 1
{
 	ADC_CONTR &= !ADC_FLAG;
	g_usRegister[24+g_ucChn]= (u16)ADC_RES<<10+(u16)ADC_RESL;
    if (g_ucChn == 0 ) g_ucChn = 1;
	else g_ucChn = 0;
	ADC_CONTR = 0x88| g_ucChn;
}

/*unsigned int AD_get(BYTE channel)
{
   ADC_CONTR=0x88|channel;    //����ADת��1000 1000 ��POWER SPEED1 SPEED0 ADC_FLAG   ADC_START CHS2 CHS1 CHS0 
   _nop_(); 
   _nop_(); 
   _nop_();
   _nop_();                  //Ҫ����4��CPUʱ�ӵ���ʱ����ֵ���ܹ���֤�����ý�ADC_CONTR �Ĵ���
   while(!(ADC_CONTR&0x10));    //�ȴ�ת�����
   ADC_CONTR&=0xe7;      //�ر�ADת����ADC_FLAGλ�������0
   return((u16)ADC_RES<<10+(u16)ADC_RESL);   //����ADת����ɵ�10λ����(16����)
}
   */
void AD_work( BYTE val,BYTE channel)
{
	u16 AD_val;
	u16 js_vol,chazhi;
	AD_val=g_usRegister[24+channel];//
	js_vol= val >> 2;
	if(AD_val>js_vol)							
	chazhi=AD_val-js_vol;
	else
	chazhi=js_vol-AD_val;

	if(chazhi>100)
	{
		 REL0=0;
	}
}

