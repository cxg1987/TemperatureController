#include "ADC.H"

#define  ADC1_DR_Address    ((u32)0x4001244C)
vu16 ADCConvertedValue;
int AD_value;
/******************************************************
* ADC����
���� AD1��ʼ����DMA��ʼ�����ʹ��ڷ���ת�����
*******************************************************/
/*******************************************************************************
* ������  	: ADC1_Init
* ��������	: ADC1��ʼ��
* ����		: ��
* ���		: ��
* ����		: ��
*******************************************************************************/
//ADC1��ʼ��
#define EN_ADC1_IRQ		//��ADC�ж�
			   
void ADC1_Init()
{
	RCC->APB2ENR |= 1<<2;				//ʹ��PORTAʱ��
	GPIOA->CRL &= 0xFFFFFF0F;
	GPIOA->CRL |= 0x00000000;			//PA1ģ������

	RCC->CFGR |= 2<<14;					//ADCCLK�����14Mh��=PCLK2 6��Ƶ�� 72M/6=12M
	RCC->APB2ENR |= 1<<9;				//ʹ��ADC1ʱ��
	RCC->APB2RSTR |= 1<<9;				//ADC1�ӿڸ�λ
   	RCC->APB2RSTR &= ~(1<<9);			//ֹͣADC1�ӿڸ�λ,
										//���������Խ�ADC1�ļĴ�������λ,
										//��������ʱ��������,��ֱ�Ӹ�ֵ

	ADC1->CR1 |= 1<<8;					//ɨ��ģʽ,
										//������ͨ���Ͻ���ģ�⿴�Ź�
										//��ֹEOCIE

	ADC1->CR2 |= (7<<17 | 1<<20);		//��SWSTART����ת����ʹ���ⲿ����
	ADC1->CR2 |= 1<<8;					//ʹ����DMA
										//����ת��,�Ҷ���

	ADC1->SQR1 |= (10-1)<<20;			//10������ͨ������ת��

	ADC1->SQR3 |= (1<<0 |1<<5 |1<<10);
	ADC1->SQR3 |= (1<<15 | 1<<20 | 1<<25);
	ADC1->SQR2 |= (1<<0 | 1<<5 | 1<<10 | 1<<15);
										//����10��1ͨ���Ĺ��������

	ADC1->SMPR2 |= 4<<3;				//ͨ��1��������41.5��cycle 

    ADC1->CR2 |= 1<<0;					//ADON��0��1��ADת�����ϵ�, ÿ���ϵ�����У׼
	ADC1->CR2 |= 1<<3;					//��λУ׼ֵ
	while( ADC1->CR2 &(1<<3));			//��ѯУ׼�Ĵ����Ƿ��ѳ�ʼ��
	ADC1->CR2 |= 1<<2;					//��ʼУ׼
	while( ADC1->CR2 &(1<<2));			//��ѯУ׼�Ƿ������
	
//#ifdef EN_ADC1_IRQ
//	ADC1->CR1 |=1<<5;		//��EOC�ж�
//	MY_NVIC_Init(3,2,ADC1_2_IRQChannel,2);	 //��2�� ��ռ���ȼ�3����Ӧ���ȼ�3
//#endif
//		   					  
//	ADC1->CR2 |= 5<<20;            //SWSTART��������ͨ��ת��			
}

//��ѯ��ʽ���ADת�����
  int GET_AD_Data()
{
     int data;
  
  	while ((ADC1->SR & (1<<1))==0)	//��ѯSR�е�EOC	λ
	{	     
	     data = ADC1->SR  ;
         USART1_SendByte(data); 
		 USART1_SendByte('M');
	 }
	data = (ADC1->DR&0x0fff);
    USART1_TXword(data);
	return  data ;
} 
/*******************************************************************************
* Function Name  : Time_Inter_Adjust
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  DMA1_Init()

{	
	//��λDMA1_Channal1�ĸ��Ĵ���	 
	DMA1_Channel1->CCR &= ~(1<<0);	 // first Disable the selected DMAy Channelx 
  
  	DMA1_Channel1->CCR  = 0; 	     //Reset DMAy Channelx control register 
  	DMA1_Channel1->CNDTR = 0;  
  	DMA1_Channel1->CPAR  = 0;
  	DMA1_Channel1->CMAR = 0;
	DMA1->IFCR |= 0x0F;	             // Reset interrupt pending bits for DMA1 Channel1 
	DMA1->IFCR &= ~0x0F;
 
  	DMA1_Channel1->CPAR = ADC1_DR_Address ; //&(ADC1->DR);
  	DMA1_Channel1->CMAR = (u32)&ADCConvertedValue;
	DMA1_Channel1->CCR  = 0x00003520 ;	//  |= 1<<10 |=1<<8 |=1<<5 |=3<<12;  Circular Perif to MEM ,Half word,Priority high disable  DIR MINC PINC M2M             
  	DMA1_Channel1->CNDTR = 1; 

//DMAʹ��
	DMA1_Channel1->CCR |=(1<<0);

}
/*******************************************************************************
* Function Name  : Time_Inter_Adjust
* Description    : Adjusts time with interface (time entered by user, using Hyperterminal)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DIS_AD_VAL()
{
         
 	static unsigned long ticks;
	unsigned char Clock1s;
	 while(1)
  	{
		AD_value=ADC1->DR;
   		if (ticks++ >= 9000) 		  /* Set Clock1s to 1 every 1 second    */
		{                  
   		 	ticks   = 0;
    		Clock1s = 1;
  		}
   		/* Printf message with AD value to serial port every 1 second             */
    	if (Clock1s) 
		{
      		Clock1s = 0;
		    USART1_TXword(AD_value);
    	}
  	}
}

void ADC_IRQHandler()
{
   AD_value=ADC1->DR;		 //ת��������־λEOC�ڶ�ADC->DRʱ���
   USART1_TXword(AD_value);
}
