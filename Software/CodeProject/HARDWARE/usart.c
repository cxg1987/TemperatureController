#include "usart.h"
#include "..\user\option.h"

extern u16	temp[CHANNEL];	               //�ⲿ��������
extern u8	curt_cycle;
extern u16	comp_temp;				      //ȫ�ڲ����¶�ֵ0.1C�� 
extern u16	each_seg_comp[CHANNEL];	      //���ڲ����¶�ֵ1C��
extern u16	set_temp;					  //����趨�¶�ֵ1C��
extern u8	duty_ratio_shadow[CHANNEL];	  //���ڼ̵������ռ�ձȣ�0~WORK_CYCLE��
extern u8	frame_space_time;			  //֡����ʱ��


u8  rx_buf[9];				//��������
u8  tx_buf[30]; 			//��������
u8  recv_count=0;
u8  send_count=0;	 
/*******************************************************************
* ����1 ���Ժ���
* �������ڳ�ʼ�����������жϺ���
* ֧����Ӧ��ͬƵ���µĴ��ڲ��������ã������˴��ڽ��������
********************************************************************/
void Usart1_Init(u32 pclk2, u32 bound)
{												   
	float temp;
	u16 mantissa;
	u16 fraction;
	temp = (float)(pclk2*1000000)/(bound*16);  //temp=USARTDIV,  Tx / Rx ������ �� fCK / (16* USARTDIV)
 	mantissa = temp;                      //�õ���������
	fraction = (temp-mantissa)*16;		  //�õ�С������
	mantissa <<= 4;
	mantissa += fraction;

	RCC->APB2ENR |= 1<<3; 				  //ʹ��PORTB��,
	RCC->APB2ENR |= 1<<14;                //USART1ʱ��
	RCC->APB2ENR |= 1<<0;				  //AFIOʱ��ʹ��

	GPIOB->CRL &=0x000FFFFF;
	GPIOB->CRL |=0x4B100000;             //PB5(485_ctr),PB6(Tx1),PB7(RX1)״̬����
	GPIOB->BRR |= GPIO_Pin_5;			 //set 485_ctr for RX
	AFIO->MAPR |= 0x00000004;			 //USART1 remap


	RCC->APB2RSTR |= 1<<14;	    		  //��λUSART1
	RCC->APB2RSTR &= ~(1<<14);			  //ֹͣ��λ

	USART1->BRR = mantissa;				  //���ò�����
	USART1->CR1 |= (1<<12 | 2<<9 );       //1λֹͣλ��У��ʹ�ܣ�żУ�飬9������λ��8+1У��λ��
	USART1->CR1 |= (1<<13 | 3<<2 );		  //USART,TE,REʹ��,8λ����


    USART1->SR  &= ~(1<<5);				 //���־λRXNE
//  USART1->CR1 |= 1<<8;				 //PE��żУ���ж�ʹ��
	USART1->CR1 |= 1<<5;				 //���ջ������ǿ�RXNEIE�ж�ʹ��
	MY_NVIC_Init(3,1,USART1_IRQChannel,2);	 //��2������ȼ�
}

/*******************************************************************
* ���ͺ���
* ����8λ���ݣ�16λ���ַ������ݵķ��ͺ���
********************************************************************/
/*����һ���ֽڵ����� */
void USART1_TXByte(u8 data)
{
    GPIOB->BSRR |= GPIO_Pin_5;			//set 485_ctr for TX

	while (!(USART1->SR&0x80));		//��ѯSR�е�TXE
	USART1->DR = (data & 0xFF);
	while (!(USART1->SR&0x40));		//��ѯSR�е�TC
	while (!(USART1->SR&0x80));		//��ѯSR�е�TXE
	while (!(USART1->SR&0x40));		//��ѯSR��TC��־

	GPIOB->BRR |= GPIO_Pin_5;
}

/*����һ��16λ���� */
void USART1_TXWord(u16 data)
{	 
	USART1_TXByte(data/256);
	USART1_TXByte(data%256);       //�ȷ��͸��ֽڣ��󷢵��ֽ�	 
}

/*����һ���ַ���   */
void USART1_SendString(u8*string, u16 len)
{
	u16 i;
	for(i=0; i<len; i++)
	{	
		USART1_TXByte(string[i]);
	}
}

void USART1_Print(u8*str)
{
	while(*str != '\0')
	{	
		USART1_TXByte(*str++);
	}
}

/*******************************************************************
* ���պ���
* ��־λ��ⷽʽ �� �жϽ��պ���
********************************************************************/

/*�����ж� */
void USART1_IRQHandler(void)
{											
	u8 t;

	t = USART1->DR;		 //��USART_DR �Ķ������ɽ�RXNE��־λ����
	frame_space_time = curt_cycle;

	if( recv_count < 8 )	
	{
		rx_buf[recv_count] = t;
		recv_count++;
	}

}

/*******************************************************************
*  ��    �ܣ� ����CRC��
*  ��ڲ���:  ����CRC�������
*  ��    �أ� framcrc[2]
********************************************************************/
u16 CalCrc( u8* msg, u8 N )
{
	u8 i,j,t;
	u16 crc = 0xFFFF;
	
   for(i=0; i<N; i++)
   {
   		crc ^= (u16)msg[i];
		for(j=0; j<8; j++)
		{
			t = crc & 1;
			crc = crc >> 1;		 //����һλ
			crc = crc & 0x7fff;	 //���λ����
            if (t == 1)			 //���Ƴ���LSBΪ1��crc^0xa001 ,��LSB=0��crc����
            crc = crc ^ 0xa001;
            crc = crc & 0xffff;
		}
   }
   return crc;
} 

/*******************************************************************
*  ��    �ܣ��������ݴ�������CRCУ�飬�������������
*  ��ڲ���: ��
*  ��    �أ���
********************************************************************/
void  Handle()
{	
	u8  reqflag;			//�������־
	u8  except;		        //�쳣��־��0x01,0x02,0x03,0x04��
	u16 framcrc;		    //֡У����
	u8  add;				//�豸��ַ
	u8  i;

	framcrc = CalCrc(rx_buf, 6);
	//��У������ȷ����������
	if( ( rx_buf[7]<<8 | rx_buf[6] ) == framcrc )		
	{
	    add = ( (~ GPIOA->IDR) >>8 ) & 0x0000000F ;				//���豸��ַ
		if ( rx_buf[0] == add )									//��ַ����
		{
		     reqflag=0;
			 except =0;
			 switch( rx_buf[1] )							   
			 {
			   case 6:	if( rx_buf[2] == 0x20 && rx_buf[3]==0x01 )
			          	{ reqflag = 1; break;}	   //�趨�¶���������
						if( rx_buf[2] == 0x20 && rx_buf[3]==0x02 )
						{ reqflag = 2; break;}	   //�趨ȫ���¶Ȳ�������
						if( rx_buf[2] == 0x20 && rx_buf[3]>=0x10 && rx_buf[3]<=0x1B )
						{ reqflag = 3; break;}	   //�趨�����¶Ȳ�������
						if( rx_buf[2] == 0x21 && rx_buf[3]>=0x10 && rx_buf[3]<=0x1B && rx_buf[5]<=100)
						{ reqflag = 5; break;}	   //�趨�������ռ�ձ�����

			   		   	else 
						except = 0x02;

						break;

               case 3:  if( rx_buf[2] == 0x21 && rx_buf[3]==0x00 )
			            {
							 if( rx_buf[4]==0 && (rx_buf[5]&0x0F) <= 12)
							    reqflag = 4;	   //�����¶�����������
							 else
							    except = 0x03;	   //�Ĵ��������쳣   
						}
			   		   	else 
						except = 0x02;			   //��ʼ��ַ�쳣

						break;


			   default:  except = 0x01;			   //�������쳣
			 }
		}
	}
//��������꣬��ʼ��������
	switch( reqflag )
	{
		case 1: //�����趨�¶�����
				set_temp = rx_buf[4]<<8 | rx_buf[5];

				tx_buf[0] = add;
		        tx_buf[1] = 0x06;
				tx_buf[2] = 0x20;
				tx_buf[3] = 0x01;
				tx_buf[4] = 0x01;
				tx_buf[5] = 0x73;
//				framcrc   = CalCrc(tx_buf,6);
//				tx_buf[6] = framcrc & 0xff;															
//				tx_buf[7] = framcrc >> 8;
				tx_buf[6] = rx_buf[6];															
				tx_buf[7] = rx_buf[7];
			    USART1_SendString(tx_buf,8);
				break;
											 
		case 2:	 //����ȫ���¶Ȳ���(1��)
				comp_temp = rx_buf[4]<<8 | rx_buf[5];		        
                tx_buf[0] = add;
		        tx_buf[1] = 0x06;
				tx_buf[2] = 0x20;
				tx_buf[3] = 0x02;
				tx_buf[4] = 0x01;
				tx_buf[5] = 0x73;
//				framcrc   = CalCrc(tx_buf,6);
//				tx_buf[6] = framcrc & 0xff;															
//				tx_buf[7] = framcrc >> 8;
				tx_buf[6] = rx_buf[6];															
				tx_buf[7] = rx_buf[7];
				USART1_SendString(tx_buf, 8);
				break;

		case 3:	 //��������¶Ȳ�����0.1C�㣬50Ϊ��׼50=0C�㣩
				i = rx_buf[3]&0x0F;
				each_seg_comp[i] = rx_buf[4]<<8 | rx_buf[5];		

                tx_buf[0] = add;
		        tx_buf[1] = 0x06;
				tx_buf[2] = 0x20;
				tx_buf[3] = rx_buf[3];
				tx_buf[4] = 0x01;
				tx_buf[5] = 0x73;
//				framcrc   = CalCrc(tx_buf,6);
//				tx_buf[6] = framcrc & 0xff;															
//				tx_buf[7] = framcrc >> 8;
				tx_buf[6] = rx_buf[6];															
				tx_buf[7] = rx_buf[7];
				USART1_SendString(tx_buf, 8);
				break;

		case 4:	 //�������¶�����
                tx_buf[0] = add;
		        tx_buf[1] = 0x03;
				tx_buf[2] = rx_buf[5]*2;
				for(i=0,send_count=3; i<rx_buf[5]; i++,send_count+=2)
				{
					tx_buf[send_count]   = temp[i]>>8;   //עTempΪ�ַ����飬Ҫ���͵��¶���16λ
					tx_buf[send_count+1] = temp[i]&0xff;
				}
				framcrc = CalCrc(tx_buf,send_count);
				tx_buf[send_count++] = framcrc & 0xff;															
				tx_buf[send_count++] = framcrc >> 8;
			//	tx_buf[6] = rx_buf[6];															
			//	tx_buf[7] = rx_buf[7];
				USART1_SendString(tx_buf, send_count);
				break;

		case 5:	 //�����趨�������ռ�ձȣ�0~100������
				i = rx_buf[3]&0x0F;
				duty_ratio_shadow[i] = rx_buf[5];		

                tx_buf[0] = add;
		        tx_buf[1] = 0x06;
				tx_buf[2] = 0x21;
				tx_buf[3] = rx_buf[3];
				tx_buf[4] = 0x01;
				tx_buf[5] = 0x73;
//				framcrc   = CalCrc(tx_buf,6);
//				tx_buf[6] = framcrc & 0xff;															
//				tx_buf[7] = framcrc >> 8;
				tx_buf[6] = rx_buf[6];															
				tx_buf[7] = rx_buf[7];
				USART1_SendString(tx_buf, 8);
				break;

		default: break;
	}
        if( except != 0 )
		{
			tx_buf[0] = add;
			tx_buf[1] = rx_buf[1] | 0x80 ;
			tx_buf[2] = except;	
			framcrc   = CalCrc(tx_buf,3);
			tx_buf[3] = framcrc & 0xff;															
			tx_buf[4] = framcrc >> 8;
			USART1_SendString(tx_buf,5);		
		} 		
}

void ComunicatProcess()
{
	 while( recv_count )		//�����յ�����
	{
		 if( ( frame_space_time < curt_cycle) && (curt_cycle - frame_space_time > 10 )	 //�ж�֡����
		   ||( frame_space_time > curt_cycle) && (frame_space_time - curt_cycle < 90 ) )
		{
			if( recv_count == 8 )
			{
				Handle();
			}
			recv_count = 0;
		}
	}
}

