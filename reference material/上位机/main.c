#include <STC_CPU.H>
#include <INTRINS.H>
#include <main.h>
#include <uart.h>
#include <AD.h>
#include <String.h>
#include <PWM_NEW.h>
#define uchar unsigned char
#define uint unsigned int
#define uint32 unsigned long


BYTE Lfastcounter,Lslowcounter,Rfastcounter,Rslowcounter;
bit Lfastflag,Lslowflag,Rfastflag,Rslowflag;
uint Lslowfq,Lstime,Lfastfq,Lftime;
uint Rslowfq,Rstime,Rfastfq,Rftime;

void IAP_READ(void);
void IAP_WRITE(void);
void IAP_CLEAR(void);
void Jiaohuan(u16,uint32);
void  savecombuf(BYTE* buf);
BYTE i=0;
BYTE timer1_flag=0;
BYTE timer2_flag=0;
BYTE rec_buf[20];
BYTE Set_cfg[20];
BYTE rpy_addr[5],snd_addr[5],timer1_counter,timer2_counter,t1_flag,t2_flag;
BYTE addr,addrlast;
BYTE timer3_counter,timer4_counter;
extern BYTE g_byBegT1buf,g_byEndT1buf,g_byBegR1buf,g_byEndR1buf;
extern BYTE g_byT1buf[T1BUF_MAX_LEN];
extern BYTE g_byR1buf[R1BUF_MAX_LEN],g_byR2buf[R2BUF_MAX_LEN];
extern BYTE rcv_flag,snd_flag,rcv_length,rcv_flag2,snd_flag2,rcv_length2;
BYTE snd_flag3;//����2���ͱ�־λ
BYTE save_flag;//�����������дָ��ʱ������д��һ������ֹ�ظ����ó��ִ���
u16 spd_val;
extern BYTE Receive[20],Receive2[20];
extern void rcv_handle(BYTE rcv);
extern void rcv_handle2(BYTE rcv);
extern void Pwm_Init(void);
extern void AD_init();
void UartACK2(BYTE status);
void DealReadRegister(uchar addr,uchar len);
unsigned int crc16(unsigned char *puchMsg, unsigned int usDataLen); 
extern void Send1(BYTE* buf, BYTE len);
extern BYTE check_modbus(void);
extern unsigned int AD_get(BYTE channel);
extern BYTE t1_rcvtimeout,t2_rcvtimeout;


sbit REL = P2^0;
sbit DEL = P2^1;
sbit RER = P2^2;
sbit DER = P2^3;

uint32 temp_cl1,temp_cl2,temp_clz,cl_j,cl_y,cl_b,cl_d;
BYTE command[11][6];     //��Ŵ���������������Ҫ����λ������ָ����һλΪ1ʱ��һ���ֽڣ�Ϊ2ʱ�������ֽڣ�Ϊ4ʱ��4���ֽڣ�                         
//u16 g_usRegister[32];  //�ñ������ö�Ӧ��MODBUS��ַ40001-40032
u16 g_usRegister[66]; 
//�������Ƶ��	40001,��������Ƶ��	40002,�������ʱ��	40003,��������ʱ��	40004
//���泤������	40005,����̽ɴѡ��	40007
//�������Ƶ��	40008,��������Ƶ��	40009,�������ʱ��	40010,��������ʱ��	40011
//���泤������	40012,����̽ɴѡ��	40014
//�װ���Ʋ���	40015,�Ұ���Ʋ���	40017,������Ʋ���	40019,������Ʋ���	40021
//�ۼ��ܲ���	40023,���ұ���ѡ��	40031
//��ߵ�ǰ�����ٶ�	40025,���浱ǰ�����ٶ�	40026
//��λ��ͳ����������	40027	
//���㰴ť	40028
//�������ָʾ	40029,�������ָʾ	40030,������λ���ĸ���40032


/***************��ʱ�ӳ���****************************/
void delay(char a)//
{
	uchar n;//
	uchar b;//
	for(b=a;b>0;b--)
	{
		n=210;
		while(n!=0) --n;
	}
}

BYTE Lfastcounter,	Lslowcounter,Rfastcounter,	Rslowcounter;
bit Lfastflag,Lslowflag,Rfastflag,Rslowflag;
uint Lslowfq,Lstime,Lfastfq,Lftime;
uint Rslowfq,Rstime,Rfastfq,Rftime;
extern void AD_work( BYTE val,BYTE channel);
sbit AOUT0 = P1^0;
sbit AOUT1 = P1^1;
sbit PWM0 = P1^3;
sbit PWM1 = P1^4;
sbit REL0 = P2^4;
sbit REL1 = P2^5;

												        

/************T1�жϴ���*****************/
void T1_IRQ(void) interrupt 3//10ms�ж�
{
 	TF1=0;
	TH1=0xdc;
	TL1=0x00;
	if(Lfastflag==1)
	{
		if(Lfastcounter>0)
			Lfastcounter--;
	}
	if(Lslowflag==1)
	{
		if(Lslowcounter>0)
			Lslowcounter--;
	}
	if(Rfastflag==1)
	{
		if(Rfastcounter>0)
			Rfastcounter--;
	}
	if(Rslowflag==1)
	{
		if(Rslowcounter>0)
			Rslowcounter--;
	}
	if (t2_rcvtimeout>0) t2_rcvtimeout--;
}
	

void T0_IRQ(void)interrupt 1 //��ʱ���70ms
{
	TF0=0;
	TH0=0x04;
	TL0=0x00;
	if(snd_flag3==0)//�����Ѿ�������
	{
	 	if(command[0][0]!=0x00)
		{
			Send2(command[0],command[0][4],command[0][5]); //�Ӵ���2�������泤��,0��ʾ���棬1��ʾ����
			snd_flag3=1;
			timer2_counter=5;//350ms
			t2_flag=1;	
		}
	}	
	if(t2_flag==1)
	{
		if(timer2_counter>0)
			timer2_counter--;
		if(timer2_counter==0)
		{ 
			snd_flag3=0;
			timer2_flag++;
		}
	}
	if(timer2_flag>4)//���4�κ���δ�յ����ݣ��������ָ�����ִ������ָ�
	{
		memset(command[0],0,6);//�������,���������;
		for(i=0;i<10;i++)
		{
			memcpy(command[i],command[i+1],6);
		}	  
		snd_flag3=0;
		timer2_flag=0;
		t2_flag=0;
	}
	if(timer3_counter>0)
		timer3_counter--;
	if(timer3_counter==0)//����ַλָ��ÿ21�뷢һ���ȷ������ٷ�����
	{
		snd_addr[0]=0x01;
		snd_addr[1]=0x00;
		snd_addr[4]=2;
		snd_addr[5]=0;
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			save_flag=0;
		}
		//snd_addr[0]=0x01;
		//snd_addr[1]=0x00;
		//snd_addr[4]=2;
		snd_addr[5]=1; //left-right
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			save_flag=0;
		}
		timer3_counter=200; 	
	} 
	if(timer4_counter>0)
		timer4_counter--;

	if(timer4_counter==0)//������ָ��ÿ7�뷢һ��
	{
		snd_addr[0]=0x04;
		snd_addr[4]=1;
		snd_addr[5]=0;
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			save_flag=0;
		}
		//snd_addr[0]=0x04;
		//snd_addr[4]=1;
		snd_addr[5]=1;
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			save_flag=0;
		}
		timer4_counter=100; 	
	}		
	  

}

/********************************************************************************************************************
�ú�����Ҫʵ�ֽ��յ���MODBUS�����ּĴ���ָ��Ĵ���
//���������͵������У�01 03 00 00 00 06 XX XX  ; 01 03 00 06 00 06 XX XX ; 
01 03 00 10 00 05 XX XX; 01 03 00 15 00 02 XX XX;  ��ָ��
����������д����ʱ�����͵������ʽΪ��01 10 00 04 00 01 02 00 05 XX XX ������11���ֽ� 
���ⴥ���������ܷ���01 01 00 60 00 10 3D D8�������Ȧָ�����������Բ���
g_byR1bufΪ���������������յ�������MODBUS����󣬽����ջ��������Ƶ���������
������addr���ּĴ����ĵ�ַ
********************************************************************************************************************/
void DealReadRegister(uchar addr,uchar len)
{
	
//	uchar  g_usRegister1[32];	   //32������g_usRegister��������������ȵ���Ϣ��Ӧ40001-40032
	uchar data1[80];
	uint crc ;

	memset(data1,0,80);
	data1[0] = 0x01; //��λ����Ӧ��MODBUS��ַ��
	data1[1] = 0x03; //˵����ǰ�ǶԶ��Ĵ����������Ӧ
	data1[2] = len + len ;

	memcpy((char *)(data1+3),(char *)&g_usRegister[addr],data1[2]);  //���ص����ݳ���Ϊ�Ĵ������ȵ�����

	//����CRC
	crc = crc16(data1,data1[2] + 3);
	memcpy(data1 + data1[2] + 3,&crc,2);//��CRC��У����Ҳ�ŵ����ͻ�����

	//���￪ʼ׼����������������Ӧ�ˡ�
	Send1(data1,data1[2] + 5) ;  
}

/********************************************************************************************************************
�ú�����Ҫʵ�ֽ��յ���MODBUS�����ּĴ���ָ��Ĵ���
//���������͵������У�01 03 00 00 00 06 XX XX  ; 01 03 00 06 00 06 XX XX ; 
01 03 00 10 00 05 XX XX; 01 03 00 15 00 02 XX XX;  ��ָ��
����������д����ʱ�����͵������ʽΪ��01 10 00 04 00 01 02 00 05 XX XX ������11���ֽ� 
���ⴥ���������ܷ���01 01 00 60 00 10 3D D8�������Ȧָ�����������Բ���
g_byR1bufΪ���������������յ�������MODBUS����󣬽����ջ��������Ƶ���������
������addr���ּĴ����ĵ�ַ
********************************************************************************************************************/
void DealWriteRegister(uchar addr,uchar val[4],uchar len)
{
	uchar data1[34];  //
	uint crc ;

	memset(data1,0,34);
	data1[0] = 0x01; //��λ����Ӧ��MODBUS��ַ��
	data1[1] = 0x10; //˵����ǰ�ǶԶ��Ĵ����������Ӧ
	data1[2] = 0x00 ;
	data1[3] = addr;
	data1[4] = 0x00;
	data1[5] = 0x00;
	//����CRC
	crc = crc16(data1,6);
	memcpy(data1 + 6,&crc,2);//��CRC��У����Ҳ�ŵ����ͻ�����

	//���￪ʼ׼����������������Ӧ�ˡ�
	Send1(data1,8) ; 
	 

	//��д������ԣ�����Ҫ����λ������д���
		//�������Ƶ��	40001,��������Ƶ��	40002,�������ʱ��	40003,��������ʱ��	40004
		//���泤������	40005,����̽ɴѡ��	40007
		//�������Ƶ��	40008,��������Ƶ��	40009,�������ʱ��	40010,��������ʱ��	40011
		//���泤������	40012,����̽ɴѡ��	40014
		//�װ���Ʋ���	40015,�Ұ���Ʋ���	40017,������Ʋ���	40019,������Ʋ���	40021
		//�ۼ��ܲ���	40023,���ұ���ѡ��	40031
		//��ߵ�ǰ�����ٶ�	40025,���浱ǰ�����ٶ�	40026
		//������λ��ͳ�Ƹ���	40027	  ������λ��ͳ�Ƹ��� 40034
		//���㰴ť	40028
		//�������ָʾ	40029,�������ָʾ	40030,
		//����������λ���ĸ���40032	 ����������λ���ĸ���  40033
	if(addr == 0 )//�յ��������Ƶ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Lfastfq=(uint)val[1];
	}
	else if(addr == 1 )//�յ���������Ƶ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Lslowfq=(uint)val[1];
	}
	else if(addr == 2 )//�յ��������ʱ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	 Lftime=((uint)val[0]<<8)|((uint)val[1]);
	}
	else if(addr == 3 )//�յ���������ʱ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Lstime=((uint)val[0]<<8)|((uint)val[1]);
	}		
	else if (addr == 4 )//���泤������
	{
	//	memcpy(g_usRegister+addr,val,4); 
		snd_addr[0]=0x02;
		snd_addr[1]=val[3];//
		snd_addr[2]=val[0];//
		snd_addr[3]=val[1];//
		snd_addr[4]=4;//�������ݳ���
		snd_addr[5]=0;//0��ʾ���棬1��ʾ����
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			memcpy(g_usRegister+addr,val,4); 
			save_flag=0;
		} 
	}
	else if(addr == 7 )	//�յ��������Ƶ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Rfastfq=(uint)val[1];
	}
	else if(addr == 8 )//�յ���������Ƶ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Rslowfq=(uint)val[1];
	}
	else if(addr == 9 )	//�յ��������ʱ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	 Rftime=((uint)val[0]<<8)|((uint)val[1]);
	}
	else if(addr == 10 )//�յ���������ʱ��
	{
		memcpy(g_usRegister+addr,val,2);
	 	Rstime=((uint)val[0]<<8)|((uint)val[1]);
	}
	else if (addr == 11 )//���泤������
	{
		memcpy(g_usRegister+addr,val,4); 
		snd_addr[0]=0x02;
		snd_addr[1]=val[3];//
		snd_addr[2]=val[0];//
		snd_addr[3]=val[1];//
		snd_addr[4]=4;
		snd_addr[5]=1;//0��ʾ���棬1��ʾ����	
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			save_flag=0;
		} 	
	}
 
	else if(addr == 6 )//�յ������趨̽˿��
	{
	//	memcpy(g_usRegister+addr,val,2);
	 	snd_addr[0]=0X03;
		snd_addr[1]=val[1];
		snd_addr[4]=2;
		snd_addr[5]=0;
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			memcpy(g_usRegister+addr,val,2);
			save_flag=0;
		}
	}
 	else if(addr == 13)//�յ������趨̽˿��
	{
	//	memcpy(g_usRegister+addr,val,2);		 
	 	snd_addr[0]=0X03;
		snd_addr[1]=val[1];
		snd_addr[4]=2;
		snd_addr[5]=1;
		if(save_flag==0)
		{	
			save_flag=1;
			savecombuf(snd_addr);
			memcpy(g_usRegister+addr,val,2);
			save_flag=0;
		}
	}
			 
	else if(addr == 31)
	{
		memcpy(g_usRegister+addr,val,2);
	}
	else if	(addr == 32)
	{
		memcpy(g_usRegister+addr,val,2); 
	}  
}




/*********************************************************************************************************************
���ǹ㣺���ϲ��������ٲ�������Щ������λ��û�õġ�
**********************************************************************************************************************/

//	PWM_clock(2);      // PCA/PWMʱ��ԴΪ��ʱ��0�����
//	PWM_start(2,0); // ģ��0,����ΪPWM���,���ж�,��ʼռ������Ϊ25%
/*********************************************************************************************************************
���ǹ㣺��һ���ֿ���PWM�������Ҫ������Щ�������á�
**********************************************************************************************************************/
   /************T0�жϴ���*****************/


void main(void) 
{ 
	uchar buf[4];
	REL=0; 
	DEL=0;
	RER=0;
	DER=0; 
	InitUart();
	InitUart2();
	Pwm_Init();
	AD_init();        //A/Dת����ʼ��
	delay(100);
	TMOD =0x11; /* timer 0 mode 1: 8-Bit reload */ 
//	TH0=0x00;
//	TL0=0x3f;
	TH0=0x04;
	TL0=0x00;
	ET0=1;
	IT0=1;
	TF0=0;
	TR0=1;
//	TH1=0x0d;
//	TL1=0xbf;
	TH1=0xdc;
	TL1=0x00;
	ET1=1;
	IT1=1;
	TF1=0;
	TR1=1;  
	EA=1;    
	Lfastflag=1;
	Lslowflag=0; 
	Lfastfq=0;
	Lslowfq=0;
	Lftime=0;
	Lstime=0;
	Rfastflag=1;
	Rslowflag=0;		  
	Rfastfq=0;
	Rslowfq=0;
	Rftime=0;
	Rstime=0;
	REL0=0;
	REL1=0;
	temp_cl1=0;
	temp_cl2=0;
	temp_clz=0;
	cl_j=cl_y=cl_b=cl_d=0;
	rcv_flag=rcv_flag2=0;
	snd_flag=snd_flag2=0;
	snd_flag3=0;/////////////////////////////////////////////////////////////////////////
	save_flag=0;
	spd_val=0;
	timer3_counter=timer4_counter=0;
	memset(g_usRegister,0,132);
//	for(i=0;i<11;i++)
	{
		 memset(&command[0][0],0,66); 
	}
	 
	while(1) 
	{  
		if(Lfastflag==1)
		{
			if(Lfastcounter==0)	//��߿���Ƶ������ʱ�䵽
			{
				CCAP0H =0xFF*(Lslowfq/10);	   //ռ�ձ�����
				Lslowflag=1;
				Lfastflag=0;
				Lslowcounter=Lstime;
		   		AD_work( CCAP0H,0);
			} 					
		}
		if(Lslowflag==1)
		{
			if(Lslowcounter==0)//�������Ƶ������ʱ�䵽
			{
				CCAP0H =0xFF*(Lfastfq/10);
				Lfastflag=1;
				Lslowflag=0;
				Lfastcounter=Lftime;
				AD_work( CCAP0H,0);
			}
					
		}
		if(Rfastflag==1)
		{
			if(Rfastcounter==0)//�ұ߿���Ƶ������ʱ�䵽
			{
				CCAP1H = 0xFF*(Rslowfq/10);
				Rslowflag=1;
				Rfastflag=0;
				Rslowcounter=Rstime;
				AD_work( CCAP1H,1);
			}
					
		}
		if(Rslowflag==1)
		{
			if(Rslowcounter==0)//�ұ�����Ƶ������ʱ�䵽
			{
				CCAP1H =0xFF*(Rfastfq/10);
				Rfastflag=1;
				Rslowflag=0;
				Rfastcounter=Rftime;
				AD_work( CCAP1H,1);
			}
				
		}
/*********************************************************
��һ����д�����ٶ�
*******************************************************/				 
	   //spd_val=AD_get(0);
	   //memcpy(&g_usRegister[24],&spd_val,2);
	   //spd_val=AD_get(1);
	   //memcpy(&g_usRegister[25],&spd_val,2);

/********************************************************************************************************************
	//����1�봥�������ӡ�
	//���������͵������У�01 03 00 00 00 06 XX XX  ; 01 03 00 06 00 06 XX XX ; 
	                      01 03 00 10 00 05 XX XX; 01 03 00 15 00 02 XX XX;  ��ָ��
	����������д����ʱ�����͵������ʽΪ��01 10 00 04 00 01 02 00 05 XX XX ������11���ֽ� 
	                                      01 10 00 04 00 02 04 00 00 00 00 XX XX									
	���ⴥ���������ܷ���01 01 00 60 00 10 3D D8�������Ȧָ�����������Բ���
*********************************************************************************************************************/
		if (rcv_flag > 0)
		{
			//����1�յ��Ĵ�����������������
			//�ж��Ƿ�Ϊһ��������MODBUS��������������MODBUS��������ݴӽ��ջ������ŵ������������д���ͬʱRcv_flag��0�����ջ�������λ��
			snd_flag = check_modbus(); 
		}

		if(snd_flag == 1)	 //g_byR1bufΪ���������������յ�������MODBUS����󣬽����ջ��������Ƶ���������
		{
			//����������������Ƕ������Ӵ���1����ǰ�ļĴ���ֵ���͵�������
			switch (g_byR1buf[1])  //
			{
			case 0x03:  //�����ּĴ����������������ȡ���λ����������Ϣ
				//����Ҫ������		
				DealReadRegister(g_byR1buf[3],g_byR1buf[5]);   //Ҫ���ı��ּĴ����ĵ�ַ
				break;
			case 0x01:  //����Ȧ
				//������Ӧ�ÿ��Բ�������
				break; 
		    case 0x10:  //д�Ĵ�������Ϊд����ʱ��һ����Ҫ��UART1����������Ӧ����һ��Ҫ�Ӵ���2����λ������ǰ�ļĴ�������
				//����Ҫ����Ҫд�������ֽڣ�Ƶ�ʣ��������ֽڣ����ȣ�
				if (g_byR1buf[5] == 0x01)
				{
					buf[0] = g_byR1buf[7];
					buf[1] = g_byR1buf[8];
					buf[2] = 0x00;
					buf[3] = 0x00;
					DealWriteRegister(g_byR1buf[3],buf,1);  //����϶���дһ���Ĵ��������Ǿͼ򵥴���ɡ�
				}
				if (g_byR1buf[5] == 0x02)
				{
					buf[0] = g_byR1buf[7];
					buf[1] = g_byR1buf[8];
					buf[2] = g_byR1buf[9];
					buf[3] = g_byR1buf[10];
					DealWriteRegister(g_byR1buf[3],buf,2);  //����϶���дһ���Ĵ��������Ǿͼ򵥴���ɡ�
				}
				break; 
			default:	//����������ǾͲ��������ˣ�������Ӧ�ò��ᷢ��
				break; 
			}
			snd_flag = 0;  //�ϴ������Ѿ�������ˡ�
		}

		if(rcv_flag2 > 0)  //����յ�
		{
			if (Receive2[rcv_flag2-1] == 0xfe)
				rcv_handle2(Receive2[0]);
			if (t2_rcvtimeout == 0)
			{
				memset(Receive2,0,20);
				rcv_flag2 = 0;
			}
		 }
		
		if(snd_flag2 == 1)	   //�յ���λ���Ľ��������	 �����g_byR2buf
		{
			if(g_byR2buf[0]!=0x09)
			{
				if (g_byR2buf[0] != 0x00)
					UartACK2(g_byR2buf[0]);//�Ȼظ�ACK,�ٴ�������;
				t2_flag=0;
				if(g_byR2buf[0]==0x08)//���ط��Ͳ���
				{
					if(command[0][5]==0)
						temp_cl1=((uint32)g_byR2buf[1]<<24)|((uint32)g_byR2buf[2]<<16)|((uint32)g_byR2buf[3]<<8)|(uint32)g_byR2buf[4];
		  			else
				   	   temp_cl2=((uint32)g_byR2buf[1]<<24)|((uint32)g_byR2buf[2]<<16)|((uint32)g_byR2buf[3]<<8)|(uint32)g_byR2buf[4];
					if (g_usRegister[30] == 0x01) //�װ�
					{
						cl_j = temp_cl1 + temp_cl2; //�ۼƲ���
						Jiaohuan(14,cl_j);
					}
					else if (g_usRegister[30] == 0x02)//�Ұ�
					{
						cl_y = temp_cl1 + temp_cl2; //�ۼƲ���
						Jiaohuan(16,cl_j);
					}
					else if (g_usRegister[30] == 0x03)
					{
						cl_b = temp_cl1 + temp_cl2; //�ۼƲ���
						Jiaohuan(18,cl_j);
					}
					else if (g_usRegister[30] == 0x04)
					{
						cl_d = temp_cl1 + temp_cl2; //�ۼƲ���
						Jiaohuan(20,cl_j);
					}	
						temp_clz = cl_j+cl_y+cl_b+cl_d;
						Jiaohuan(22,cl_j);
						//temp_cl1 = 0;
					//	temp_cl2 = 0;
					}
					if(g_byR2buf[0]==0x05)   //���ص�ַ�趨
					{
			 			if(command[0][5]==0)
						{
							g_usRegister[26] = g_byR2buf[1];
							if(g_usRegister[26]!=g_usRegister[31])
							g_usRegister[28]=0x0001;
							else 
							g_usRegister[28]=0x0000;
						}
						else
						{
							g_usRegister[33] = g_byR2buf[1];
							if(g_usRegister[33]!=g_usRegister[32])
							g_usRegister[29]=0x0001;
							else 
							g_usRegister[29]=0x0000;
						}
					}
					memset(command[0],0,6);//�������,���������;
						for(i=0;i<10;i++)
						{
							memcpy(command[i],command[i+1],6);
						}
						snd_flag3=0;
					
				}	
				if(g_byR2buf[0]==0x09)//�յ�ACK
				{
					//if(g_byR2buf[1]==command[0][0]) //////////////////////////////////////////////////////////
					t2_flag=0;	
					memset(g_byR2buf,0,30);			
				}
		   	snd_flag2=0;
			}	 
	}
}
void UartACK2(BYTE status)
{
	BYTE ack[2];
	ack[0]=0x09;
	ack[1]=status;
    Send2(ack,2,command[0][5]);
}
void Jiaohuan(u16 addr,uint32 in)
{
	u16 in1=0,in2=0;
	in2=(u16)(in&0x0000ffff);
	in1=(u16)in>>16;
	in=(uint32)((((uint32)in2)<<16)+in1);
	memcpy((char*)(&g_usRegister[addr]),(char*)&in,4);
}
void  savecombuf(BYTE* buf)
{
 	BYTE i=0, j=0;
	for(i=0;i<10;i++)
	{
	
		if((command[i][0]==0x00)||((buf[0]==command[i][0])&&(buf[5]==command[i][5]))) //���ǰ���Ѿ��и�ָ��򸲸ǣ�û�еĻ������
		{	
			for(j=0;j<6;j++)
			{
				command[i][j]=buf[j];	
			}
		  break;
		}
	}
}