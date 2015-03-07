#include <stm32f10x_lib.h>
#include "../SYSTEM/sys.h"
#include "../SYSTEM/delay.h"
#include "../HARDWARE/init.h" 
#include "../HARDWARE/usart.h"

#include "option.h"

u16 set_temp;					//�����趨�¶�ֵ1C��
u16 comp_temp;					//ȫ�ֲ����¶�ֵ0.1C��
u16 each_seg_comp[CHANNEL];		//���ڲ����¶�ֵ1C��
u8  duty_ratio_shadow[CHANNEL];	//���ڼ̵������ռ�ձ�
u8  duty_ratio[CHANNEL];		//���ڼ̵������ռ�ձȣ�0~WORK_CYCLE��
u8  frame_space_time;			
u16	temp[CHANNEL];				//��Ÿ�ͨ��ת������¶�ֵ0.1C��


u16	ad_value[CHANNEL][SAMP_TIME+1]; //���ADת�����
u8	channel_num=0;				     //��־��ǰ��ģ�⿪��ͨ��ֵ
u8	curt_samp=0;				     //��־��ǰͨ���ĵ�ǰ�������� EOC�жϼ���
u8	curt_cycle=0;				     //��־��ʱ���жϴ���		 Timer�жϼ���
u8	samp_ok=0;					     //��־AD�������

u16	param[PARAM_NUM];				//���籣��ʱ����Ĳ���

#if _DEBUG
	u16  sample_time=0;				//��־ͨ��SAMP_TIME�����ݲɼ��Ĺ���ʱ��
	u8   com_time=0;				//��־��������ƹ��̽���
#endif

																													
uc16 table[] = { 			  		//Rt�ֶȱ�����ֵ����100����Ϊ����

/*0*/	10000, 10039, 10078, 10117, 10156, 10195, 10234, 10273, 10312, 10351,   
/*10*/	10390, 10429, 10468, 10507, 10546, 10585, 10624, 10663, 10702, 10740,	  
/*20*/	10779, 10818, 10857, 10896, 10935, 10973, 11012, 11051, 11090, 11129,	  
/*30*/	11167, 11206, 11245, 11283, 11322, 11361, 11400, 11438, 11477, 11515,	  
/*40*/	11554, 11593, 11631, 11670, 11708, 11747, 11786, 11824, 11863, 11901,   
/*50*/	11940, 11978, 12017, 12055, 12094, 12132, 12171, 12209, 12247, 12286,   
/*60*/	12324, 12363, 12401, 12439, 12478, 12516, 12554, 12593, 12631, 12669,
/*70*/	12708, 12746, 12784, 12822, 12861, 12899, 12937, 12975, 13013, 13052, 
/*80*/	13090, 13128, 13166, 13204, 13242, 13280, 13318, 13357, 13395, 13433,
/*90*/	13471, 13509, 13547, 13585, 13623, 13661, 13699, 13737, 13775, 13813,
/*100*/	13851, 13888, 13926, 13964, 14002, 14040, 14078, 14116, 14154, 14191,
/*110*/	14229, 14267, 14305, 14343, 14380, 14418, 14456, 14494, 14531, 14569,
/*120*/	14607, 14644, 14682, 14720, 14757, 14795, 14833, 14870, 14908, 14946,
/*130*/	14983, 15021, 15058, 15096, 15133, 15171, 15208, 15246, 15283, 15321,
/*140*/	15358, 15396, 15433, 15471, 15508, 15546, 15583, 15620, 15658, 15695,
/*150*/	15733, 15770, 15807, 15845, 15882, 15919, 15956, 15994, 16031, 16068,
/*160*/	16105, 16143, 16180, 16217, 16254, 16291, 16329, 16366, 16403, 16440,
/*170*/	16477, 16514, 16551, 16589, 16626, 16663, 16700, 16737, 16774, 16811,
/*180*/	16848, 16885, 16922, 16959, 16996, 17033, 17070, 17107, 17143, 17180,
/*190*/	17217, 17254, 17291, 17328, 17365, 17402, 17438, 17475, 17512, 17549,
/*200*/	17586, 17622, 17659, 17696, 17733, 17769, 17806, 17843, 17879, 17916,
/*210*/	17953, 17989, 18026, 18063, 18099, 18136, 18172, 18209, 18246, 18282,
/*220*/	18319, 18355, 18392, 18428, 18465, 18501, 18538, 18574, 18611, 18647, 
/*230*/	18684, 18720, 18756, 18793, 18829, 18866, 18902, 18938, 18975, 19011, 
/*240*/	19047, 19084, 19120, 19156, 19192, 19229, 19265, 19301, 19337, 19374, 
/*250*/	19410, 19446, 19482, 19518, 19555, 19591, 19627, 19663, 19699, 19735
} ;																 


/***************************** �������˲�����(��ͷȥβȡƽ��ֵ) *******************************/

u16 Filt(u8 N, u16* a)
{
	u8 i;
	u16  max, min;
	u16  sum, aver;

	max=a[0]; min=a[0]; sum=a[0];
	for(i=1; i<N; i++)
	{	 
	    sum += a[i];

	 	if( a[i] > max )
			max = a[i];
		if( a[i] < min )
			min = a[i];			
	}
	aver = ( sum-max-min ) / (N-2);
	return aver;
}

/***************************** ˳������¶�ֵ*****************************************************/

void TempSearch(u8 N, u16* a)
{
	u8  i2;
	u16 j;
	u16 r[CHANNEL],ad[CHANNEL];
	u32 k;

	for(i2=0; i2<N; i2++)				//CHANNEL
	{
		//�Բ���ֵ�����˲���֮���������ֵ
		//У׼��ĵ���ֵR��AD����ad֮��Ĺ�ϵΪ��R=0.0256573*ad+98.6598
		//�ڴ����б���ʹ�ø���������С��ת��Ϊ����������		
		ad_value[i2][SAMP_TIME] = Filt(SAMP_TIME , &ad_value[i2][0]);   //ȥͷȥβ��ֵ�˲�
	    ad[i2] = ad_value[i2][SAMP_TIME];                               				
		k = (42037*ad[i2]) + 161644184;	//����42307=2.56573*2^14,161644184=9865.98*2^14
		r[i2] = (u16)(k>>13);			//����õ�ʵ�ʵ����2*100��
										//�ֶȱ��еĵ���ֵΪʵ�ʵ���ֵ��100��
										//���Բ�ֵ����ʽ�������������Լ��ټ�����ʧ����

		if(r[i2] < 2*10000)		        //����0�ȵĶ���Ϊ0��
		{								//����ĵõ�����ֵ����ڷֶȱ��е���������������2
			temp[i2]  = 0;
			continue;
		}

		if(r[i2] > 2*19735)		    	//����260�ȵĶ���Ϊ260��
		{								//����ĵõ�����ֵ����ڷֶȱ��е���������������2
			temp[i2] = 2600;
			continue;
		}
		                               	//0~260�ȵ���ֵ��Χ�ڣ���������Բ�ֵ
		for (j=1; j<260; j++)		   
		{
			if(2*table[j]>r[i2])
			{
				//��0.1�ȵ����Բ�ֵ����ʽ����
				//(Temp-10*(j-1))/(r-2*table[j-1]) = (10*j-10*(j-1))/(2*table[j]-2*table[j-1])
				temp[i2] = 10*(j-1) + 5*(r[i2]-(table[j-1]<<1))/(table[j]-table[j-1]);
				break;	
			}	   
	    }
	   temp[i2] -= ( ((short)each_seg_comp[i2]-5)*10 + comp_temp*i2 );
	}
}

/*****************************���ȼ̵����������****************************
*�ú������ݵ�ǰ�¶����趨�¶�֮��Ĳ�ֵ�趨���ȼ̵�������ռ�ձȵ�ֵ
*ռ�ձ�ֵȡֵ��ΧΪ��0~100������ÿ100ms֮��̵������ϵ�ʱ��
*��ע��������Ʋ������ֳ��ⶨ
***************************************************************************/																							  
void HeaterCtr()
{
	u8 i;
	u16 temp_min = (set_temp-1)*10;
	u16 temp_max = (set_temp+1)*10;

	for(i=0; i<CHANNEL; i++)
	{
		duty_ratio_shadow[i] = 50;		//���趨�¶ȵ�һ����Χ֮��ʱ�����б��´���

	    if ( temp[i] > temp_max )		//�����趨�¶�һ����Χʱ�����ͼ̵�������ռ�ձ��Խ���
			duty_ratio_shadow[i] = 20;

		if ( temp[i] < temp_min )		//�����趨�¶�һ����Χʱ������̵�������ռ�ձ��Լ���
			duty_ratio_shadow[i] = 85;
	} 			 
}

/********************************��·���ؿ���  ͨ���л� ********************************************/

void SwitchChannel (u8 key_num)
{  
	if (key_num<16)
	{	
	    GPIOB->BSRR |= 1<<15;	 //��·���� ��ֹ���
	    GPIOB->BRR  |= ( 1<<11 | 1<<12 | 1<<13 | 1<<14);

	   	if( key_num & 0x01 )
		GPIOB->BSRR |= 1<<11;
	    if( key_num	& 0x02 )
		GPIOB->BSRR |= 1<<12;
	    if( key_num	& 0x04 )
		GPIOB->BSRR |= 1<<14;
		if( key_num	& 0x08 )
		GPIOB->BSRR |= 1<<13;
	
		GPIOB->BRR |= 1<<15;     //��������
	}
	else
	return;
}
/******************************  ��ʱ���жϺ���******************************************/

void TIM3_IRQHandler(void)
{
	u16 i=0;   
	TIM3->SR &= ~(1<<0);                //���жϱ�־λ
	curt_cycle++;	

	if (curt_cycle <= CHANNEL)		    //��ʱ��ǰN�����ڶԸ���ͨ������
	{	       						
		ADC1->SR  &= ~(1<<1);           //��EOCת��������־
		ADC1->CR1 |=1<<5;		        //��EOC�ж�
	}

	if(curt_cycle == WORK_CYCLE)		//һ���������ڽ���
	{
		curt_cycle = 0;

		//�����ȼ̵���ռ�ձ�ӳ�������е�ֵװ�ص�ռ�ձ������У�ÿ100msִ��һ��
		for(i = 0;i < CHANNEL;i ++)
		{
			duty_ratio[i] = duty_ratio_shadow[i];
		}
	}
}
								 
/***********************  ADC���˴�ֻ��PA1����ͨ��ת�����жϺ���*******************/

void ADC_IRQHandler()
{
	ad_value[channel_num][curt_samp] = ADC1->DR;//ת��������־λEOC�ڶ�ADC->DRʱ���
    curt_samp+=1;						     //����EOC�жϴ�������־��ǰͨ����������

    if(curt_samp == SAMP_TIME)			   	 //��ǰͨ����n�β�������
    {	
   	    curt_samp = 0; 
		channel_num = (channel_num+1)%CHANNEL;

		ADC1->CR1 &=~(1<<5);		         //��EOC�ж�
		ADC1->SR &= ~(1<<1);                 //��EOCת��������־
		SwitchChannel (channel_num);         //�л�ͨ��

		if(channel_num == 0)
		samp_ok = 1;

#if _DEBUG
	   	sample_time = TIM3->CNT;
#endif

    }
}
/***********************  ���籣���ⲿ�жϺ���***********************************/
 void EXTI9_5_IRQHandler (void)
{
	u8 i=0;
	if(  EXTI->PR&(1<<8)  )
	{
		EXTI->PR |= 1<<8;	            //д1����
		FLASH_Unlock();	  				//���ڲ�FLASH��������Ҫ�Ƚ���

		//��д���쳣��־��Ȼ��������
		FLASH_ProgramHalfWord((PARAM_ADDR+(PARAM_NUM<<1)),0x1234);
		for(i=0;i<PARAM_NUM;i++)		//����������Flash��
		{
			//����д����,������д��Flash��
			FLASH_ProgramHalfWord((PARAM_ADDR+(i<<1)),param[i]);
		}
		FLASH_Lock();					//������֮������
	}
} 
/**********************************������************************************/
int main(void)
{
	u8 i=0;	
	Stm32ClockInit(9);                      //ϵͳʱ�ӳ�ʼ����PLLCLK=8M*9=72M 
	DelayInit(72);

	GPIOConfig();
	EXTI8_Init();
	Usart1_Init(72,BAUDRATE);				//USART1��ʼ�� USART1:pclk2
	Timer3_Init(800*SAMP_TIME, 8);			//72M/(8+1)=8Mhz�ļ���Ƶ�� eg:������8000Ϊ8000/8M=1ms
	ADC1_Init();
   	SwitchChannel (0); 
	ADC1->CR2 |= 5<<20;						//SWSTART��������ͨ��ת�� ADת�����ڶ�ʱ���ж��￪

	for(i=0;i<PARAM_NUM;i++)
	{
		param[i] = 0xffff;	
	}

	if(*(vu16*)(PARAM_ADDR+(PARAM_NUM<<1)) != 0xffff)
	{
		if(*(vu16*)(PARAM_ADDR+(PARAM_NUM<<1)) == 0x1234)
		{
			for(i=0;i<PARAM_NUM;i++)
			{
				param[i] = *(vu16*)(PARAM_ADDR + (i<<1));	
			}
		}

		FLASH_Unlock();	  				//���ڲ�FLASH��������Ҫ�Ƚ���
		i = FLASH_ErasePage((u32)PARAM_ADDR);
		FLASH_Lock();					//������֮������

#if	_DEBUG
		if(i != FLASH_COMPLETE)
		{
			USART1_Print("Flash erase error!\n");
		}
#endif
	}

#if	_DEBUG
	for(i=0;i<PARAM_NUM;i++)
	{
		param[i] = param[i]+2*i+1;	
	}
#endif

	comp_temp = 0;							//�����¶ȳ�ֵ
	i = 0;
	while(i<CHANNEL)
    {
		each_seg_comp [i] = 5;				//���ڲ����¶ȳ�ֵ0x05=0C��
		duty_ratio_shadow[i]= WORK_CYCLE ;	//���ڼ̵������ռ�ձȳ�ֵEachDutyRatio[i]/WORK_CYCLE=1
		duty_ratio[i] = duty_ratio_shadow[i];
		i++;
	}

	while(1)
	{
		for(i=0; i<CHANNEL; i++)	  
		{
			if( curt_cycle < duty_ratio[i] )
				GPIOC->BSRR |= 1<<i;
			else
				GPIOC->BRR  &= ~(1<<i);
		}

		if ( samp_ok == 1 )	               //�����������󼸸����ڲ���AD���������ת���¶�ֵ����̵��������ͨѶ
		{
			samp_ok = 0;
			TempSearch(CHANNEL,temp);
			HeaterCtr();

#if _DEBUG
	    	com_time = curt_cycle;
#endif

			ComunicatProcess();
		}		  
	}
}

					