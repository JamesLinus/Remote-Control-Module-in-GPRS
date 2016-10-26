#include <reg52.h>
#include <intrins.h>
#include <stdio.h>
#include <string.h> 
#define uchar unsigned char
#define uint  unsigned int 

#define UART0_BUFFERSIZE 40
uchar UART0_Buffer[UART0_BUFFERSIZE];
uchar UART0_Buffer_Size = 0;
uchar UART0_Input_First = 0;
uchar TX_Ready =1;
uchar key_flag=0;                       //�ж��Ƿ񰴼�ɨ�赽����
static char Byte;


unsigned char TMPH,TMPL; 

code unsigned char ok[]="OK";
code unsigned char newmsg[]="+CMTI:";	
code unsigned char new[]="86"; 
code unsigned char telnum[]="13667684755"; 
code unsigned char led_on[]="led on";	 
code unsigned char led_off[]="led off";
code unsigned char beep_on[]="beep on";
code unsigned char beep_off[]="beep off";
code unsigned char usb_on[]="usb on";
code unsigned char usb_off[]="usb off";


code unsigned char num0[]="000";	  //���չ��ʵ�λ��
code unsigned char num1[]="111";
code unsigned char num2[]="222";
code unsigned char num3[]="333";
code unsigned char num4[]="444";
code unsigned char num5[]="555";
code unsigned char num6[]="666";
code unsigned char num7[]="777";
code unsigned char num8[]="888";
code unsigned char num9[]="999";
													  //����ռ�ձ�					
code uchar num[11][10]={1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};
unsigned char tel[11];	

code uchar a[]={0,10,10,10,10,10,10,10,10,10}; //����ʱ��
code uchar b[]={10,250,90,40,20,10,6,3,2,0};	  //Ϩ��ʱ��

/*LED_Buffer[16]�����洢���ڷ��͵����ݣ�com_dat������¼���ڷ��͵ĸ���*/
code uchar LED_Buffer[16], *q, com_dat;      //�Ӵ��ڽ��յ�����
//���������Ųο�����

sbit E=P2^7;		//1602ʹ������
sbit RW=P2^6;		//1602��д����	
sbit RS=P2^5;		//1602����/����ѡ������

sbit led1=P2^1;
sbit led2=P2^2;
sbit led3=P2^3;
sbit led4=P2^4;
sbit beep=P2^0;
sbit usb =P3^7;
/*
sbit led0=P1^0;
sbit led1=P1^1;		//�����־�������led��
sbit led2=P1^2;
sbit led3=P1^3;
sbit led4=P1^4;
sbit led5=P1^5;
sbit led6=P1^6;
sbit led7=P1^7;
*/
uchar dd=0;	  //��־�Ƿ��һ�ε���������
uchar ff,count=0;��ʱ����
void Delay(uint del)
{
	uint i,j;
	for(i=0; i<del; i++)
	for(j=0; j<1827; j++);
}

/********************************************************************
* ���� : delay()
* ���� : ��ʱ,��ʱʱ����Ϊ140US��
* ���� : ��
* ��� : ��
***********************************************************************/
void delay()
{
	int i,j;
	for(i=0; i<=10; i++)
	for(j=0; j<=2; j++);
}
void delay1(uint z)
{
	uint x,y;
	for(x=z;x>0;x--)
		for(y=110;y>0;y--);
}




/********************************************************************
* ���� : enable(uchar del)
* ���� : 1602�����
* ���� : ���������ֵ
* ��� : ��
***********************************************************************/
void enable(uchar del)
{
	P0 = del;
	RS = 0;
	RW = 0;
	E = 0;
	delay();
	E = 1;
	delay();
}

/********************************************************************
* ���� : wcmd(uchar del)
* ���� : 1602�����
* ���� : ���������ֵ
* ��� : ��
***********************************************************************/
void wcmd(uchar del)
{
	while(Busy());
	RS = 0;
	RW = 0;
	E = 0;
	delay();
	P0 = del;
	delay();
	E = 1;
	delay();
	E = 0;
}

/********************************************************************
* ���� : wdata(uchar del)
* ���� : 1602д���ݺ���
* ���� : ��Ҫд��1602������
* ��� : ��
***********************************************************************/
void wdata(uchar del)
{
	while(Busy());
	RS = 1;
	RW = 0;
	E = 0;
	delay();
	P0 = del;
    delay();
	E = 1;
	delay();
	E = 0;
}

/********************************************************************
* ���� : write(uchar del)
* ���� : 1602д���ݺ���
* ���� : ��Ҫд��1602������
* ��� : ��
***********************************************************************/
void write(uchar del)
{
	P0 = del;
	RS = 1;
	RW = 0;
	E = 0;
	delay();
	E = 1;
	delay();
}

/********************************************************************
* ���� : L1602_init()
* ���� : 1602��ʼ������ο�1602������
* ���� : ��
* ��� : ��
***********************************************************************/
void L1602_init(void)
{
	enable(0x01);
	enable(0x38);
	enable(0x0c);
	enable(0x06);
	enable(0xd0);
}

/********************************************************************
* ���� : L1602_string(uchar hang,uchar lie,uchar *p)
* ���� : �ı�Һ����ĳλ��ֵ�����Ҫ�õ�һ�У�������ַ���ʼ��ʾ"ab cd ef" �����øú�������
	 	 L1602_string(1,5,"ab cd ef;")
* ���� : �У��У���Ҫ����1602������
* ��� : ��
***********************************************************************/
void L1602_string(uchar hang,uchar lie,uchar *p)
{
	uchar a;
	if(hang == 1) a = 0x80;
	if(hang == 2) a = 0xc0;
	a = a + lie - 1;
	enable(a);
	while(1)
	{
		if(*p == '\0') break;
		write(*p);
		p++;
	}
}

/********************************************************************
* ���� : L1602_char(uchar hang,uchar lie,char sign)
* ���� : �ı�Һ����ĳλ��ֵ�����Ҫ�õ�һ�У�������ַ���ʾ"b" �����øú�������
		 L1602_char(1,5,'b')
* ���� : �У��У���Ҫ����1602������
* ��� : ��
***********************************************************************/
void L1602_char(uchar hang,uchar lie,char sign)
{
	uchar a;
	if(hang == 1) a = 0x80;
	if(hang == 2) a = 0xc0;
	a = a + lie - 1;
	wcmd(a);
	wdata(sign);
}





����ͨ�ų���
/********************************************************************
* ���� : Com_Init()
* ���� : ���ڳ�ʼ��������11.0592,������9600��ʹ���˴����ж�
* ���� : ��
* ��� : ��
***********************************************************************/
void Com_Init(void)
{
     TMOD = 0x20;   //��ʱ�������ڶ�ʱ��1�ķ�ʽ2
     PCON = 0x00;   //����Ƶ
     SCON = 0x50;	//���ڹ����ڷ�ʽ1�������������н���	
     TH1 = 0xFd;    //���ò����� 9600
     TL1 = 0xFd;
     TR1 = 1;		//������ʱ��1
	 ES = 1;		//�������ж�
	 EA = 1;		//�����ж�		
}

/****************************************************************
*
*����һ���ַ�
*****************************************************************/
void UART0_send_byte(uchar dat)
{
	 SBUF=dat;
	 delay1(1) ;
	 while(TI==0);
	 TI=0;
}
/****************************************************************
*
*�����ַ���
*****************************************************************/
void UART0_send_string(uchar *Buf)
{
 	while(*Buf!='\0') 
	{	   
		UART0_send_byte(*Buf++);
	}
}
 /*
 ������0
 */
 void arry_to_zero()
 {	int i;
 	for (i=0;i<UART0_BUFFERSIZE;i++)
	{
	  UART0_Buffer[i] =0;
	
	}
   UART0_Buffer_Size=0;
 
 }
//���������ж�

/********************************************************************
* ���� : Com_Int()
* ���� : �����ж��Ӻ���
* ���� : ��
* ��� : ��
***********************************************************************/
void UART0_Interrupt (void) interrupt 4
{
	uchar i=0;
	if(RI == 1)   //��Ӳ�����յ�һ������ʱ��RI����λ
	{
		if( UART0_Buffer_Size == 0)         // If new word is entered
		{   
        	 UART0_Input_First = 0;         
		 }		
	   				//	LED_Buffer[com_dat] = SBUF; 		//�ѴӴ��ڶ��������浽����
	  			    //	SBUF = LED_Buffer[com_dat];
		RI = 0;
		Byte = SBUF;                       // Read a character from UART
 						                   // Clear interrupt flag
  		if (UART0_Buffer_Size < UART0_BUFFERSIZE)
     	{
     	    UART0_Buffer[UART0_Input_First] = Byte; // Store in array
     	   	UART0_Buffer_Size++;             // Update array's size
   	     	UART0_Input_First++;             // Update counter			
		}
	
		else if	((UART0_Buffer_Size == UART0_BUFFERSIZE)&&(flag1==0))
		{
			for(dd=38;dd<39;dd++)
			{
				ff=dd-38;
				UART0_Buffer[ff]=UART0_Buffer[dd];
			}
			for(i=0;i<11;i++)							   //��ʾ�ֻ���
			{
				tel[i]=UART0_Buffer[i+3];
			}
			UART0_Buffer_Size=UART0_Buffer_Size-38;
			UART0_Input_First=UART0_Input_First-38;
			UART0_Buffer[UART0_Input_First] = Byte; // Store in array
     	   	UART0_Buffer_Size++;             // Update array's size
   	     	UART0_Input_First++;             // Update counter
			flag1==1;
		}
		else if((UART0_Buffer_Size == UART0_BUFFERSIZE)&&(flag1==1)) 
		{
//			arry_to_zero();
			UART0_Buffer_Size=UART0_Buffer_Size-38;
			UART0_Input_First=UART0_Input_First-38;
		}
	}	
}
 
int UART0_receive_compare(uchar *p)	    
{     uchar *ptr;
	  ptr = strstr(UART0_Buffer,p );
	  if (ptr!=NULL)
	  	return  1;
	  else 
		return  0;
} 




����ɨ�����
uchar Keyscan(void)
{
	uchar i,j, temp, Buffer[4] = {0xfe, 0xfd, 0xfb, 0xf7};
	for(j=0; j<4; j++)
	{
		P1 = Buffer[j];
		temp = 0x10;                                                                                                                                                                    
		for(i=0; i<4; i++)
		{
			if(!(P1 & temp)) 
			{
				return (i+j*4);
			}
			temp <<= 1;
		}	
	}
}
uchar key()						//ɨ�����
{
	uchar Key_Value;  //�����ļ�ֵ
	uchar m=5,n=0;
	key_flag=0;
	P1 = 0xf0;
		while(1)
	{
		if(count==11)
			break;
		P1 = 0xf0;
		if(P1 != 0xf0)
		{
			Delay_1ms(20);	//��������
			if(P1 != 0xf0)
			{
				Delay_1ms(20);	//��������
				if(P1 != 0xf0)
				{
					Key_Value = Keyscan();
					key_flag=1;
					count++;
//					L1602_char(2,15,Key_Value / 10 + 48);	 //ʮλ
					L1602_char(2,m,Key_Value % 10 + 48);	 //��λ	
					Delay_1ms(250);
					if(m<=16)
						m++;
					else
						m=5;
				}
			}
		}
	}
	return key_flag;			
}




����ͨ�ϼ����ʳ���
uchar compare_num()	            //�ж��Ƿ��յ��������ʵ�ָ��
{
	if(UART0_receive_compare(num0)) 
	{
		return 0; 
	}
		else if	(UART0_receive_compare(num1))
		{
			return 	1;
		}
			else if	(UART0_receive_compare(num2))
			{
				return 	2;
			}
				else if	(UART0_receive_compare(num3))
				{
					return 	3;
				}
					else if	(UART0_receive_compare(num4))
					{
						return 	4;
					}
						else if	(UART0_receive_compare(num5))
						{
							return 	5;
						}
							else if	(UART0_receive_compare(num6))
							{
								return 	6;
							}
								else if	(UART0_receive_compare(num7))
								{
									return 	7;
								}
									else if	(UART0_receive_compare(num8))
									{
										return 	8;
									}
										else if	(UART0_receive_compare(num9))
										{
											return 	9;
										}
											else if(UART0_receive_compare(num9))				//û�յ���Ϣ
											{
												return	10;
											}
}


void compare_led()				//�ж϶������Ƿ��п��ƵƵ���Ϣ
{
	uchar i,j,k;
	if(UART0_receive_compare(led_on))
	{
		i=compare_num();
		for(k=0;k<100;k++)
		{
			for(j=0;j<10;j++)
			{
				led1=num[i][j];
				led2=num[i][j];
				led3=num[i][j];
				led4=num[i][j];
			}
		}
//			led1=0;
//			j=compare_num();
//		for(j=0;j<10;j++)
		{
			
		}
//		if(j<10)
//			proportion(j);
	}
	else if(UART0_receive_compare(led_off))
	{
		led1=1;
		led2=1;
		led3=1;
		led4=1;
//		led1=1;
	}  
}

void compare_beep()					//�ж϶������Ƿ��п��Ʒ���������Ϣ
{
	uchar i,j,k;
	if(UART0_receive_compare(beep_on))
	{
		i=compare_num();
		for(k=0;k<100;k++)
		{
			for(j=0;j<10;j++)
			{
				led1=num[10-i][j];
				led2=num[10-i][j];
				led3=num[10-i][j];
				led4=num[10-i][j];
			}
		}
///		led2=0;
//		j=compare_num();
//		if(j<10)
//			proportion(j);
	}
	else if(UART0_receive_compare(beep_off))
	{
		beep=1;
//		led2=1;
	}	
}

void compare_usb()					//�ж϶������Ƿ��п���USB�ڵ���Ϣ
{
	uchar i,j,k;
	if(UART0_receive_compare(usb_on))
	{
		i=compare_num();
		for(k=0;k<100;k++)
		{
			for(j=0;j<10;j++)
			{
				led1=num[i][j];
				led2=num[i][j];
				led3=num[i][j];
				led4=num[i][j];
			}
		}
///		led3=0;
//		j=compare_num();
//		if(j<10)
//			proportion(j);
	}
	else if(UART0_receive_compare(usb_off))
	{
		usb=0;
//		led3=1;
	}	
}



uchar Reset(void)
{
	uchar deceive_ready;
	DQ = 0;
	delay(29);
	DQ = 1;
	delay(3);
	deceive_ready = DQ;
	delay(25);
	return(deceive_ready);
}

/********************************************************************
* ���� : read_bit()
* ���� : ��DS18B20��һ��λֵ
* ���� : ��
* ��� : ��DS18B20������һ��λֵ
***********************************************************************/
uchar read_bit(void)
{
	uchar i;
	DQ = 0;
	DQ = 1;
	for(i=0; i<3; i++);
	return(DQ);
}

/********************************************************************
* ���� : write_bit()
* ���� : ��DS18B20дһλ
* ���� : bitval��Ҫ��DS18B20д���λֵ��
* ��� : ��
***********************************************************************/
void write_bit(uchar bitval)
{
DQ=0;if(bitval==1)
DQ=1;
delay(5);
DQ=1;
}

/********************************************************************
* ���� : read_byte()
* ���� : ��DS18B20��һ���ֽ�
* ���� : ��
* ��� : ��DS18B20������ֵ
***********************************************************************/
uchar read_byte(void)
{
	uchar i,m,receive_data;
	m = 1;
	receive_data = 0;
	for(i=0; i<8; i++)
	{
		if(read_bit())
		{
			receive_data = receive_data + (m << i);
		}
		delay(6);
	}
	return(receive_data);
}

/********************************************************************
* ���� : write_byte()
* ���� : ��DS18B20дһ���ֽ�
* ���� : val��Ҫ��DS18B20д�������ֵ��
* ��� : ��
***********************************************************************/
void write_byte(uchar val)
{
	uchar i,temp;
	for(i=0; i<8; i++)
	{
		temp = val >> i;
		temp = temp & 0x01;
		write_bit(temp);
		delay(5);
	}
}

void temperature ()
{
	uint temp;
	P2 = 0x00;
		Reset();
		write_byte(jump_ROM);
		write_byte(start);
		Reset();
		write_byte(jump_ROM);
		write_byte(read_EEROM);
		TMPL = read_byte();
		TMPH = read_byte();
		temp = TMPL / 16 + TMPH * 16;
		P0 = table[temp/10%10];
		P2 = 0xfd;
		Delay_1ms(5);
		P0 = table[temp%10];
		P2 = 0xfe;
		Delay_1ms(5);
}



void initial()
{
	uchar com_dat = 0;
	L1602_init();
	L1602_string(1,1,"Re:");
	L1602_string(2,1,"In:");
	Delay(100);
	Com_Init();
	P2 = 0x01;
	P1 = 0xff;
	key_flag=0;

	while( !UART0_receive_compare	(ok))  //ȷ���յ���ok��     
   	{
		UART0_send_string("AT\r\n");	   //���͡�AT��ָ��ж��ź�ǿ��
	}
	
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=1\r\n");				  //ɾ������1
	}
	while(! UART0_receive_compare	(ok)); //ȷ���յ���ok��   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=2\r\n");				  //ɾ������2
	}
	while(! UART0_receive_compare	(ok)); //ȷ���յ���ok��   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=3\r\n");				  //ɾ������3
	}
	while(! UART0_receive_compare	(ok)); //ȷ���յ���ok��   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=4\r\n");				  //ɾ������4
	}
	while(! UART0_receive_compare	(ok)); //ȷ���յ���ok��   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=5\r\n");				  //ɾ������5
	}
	while(! UART0_receive_compare	(ok)); //ȷ���յ���ok��   
	arry_to_zero();
				 
	while( !UART0_receive_compare	(ok))
	{													//ȷ���յ���ok��     
		arry_to_zero();
   		UART0_send_string("AT+CMGF=1\r\n");			    //���ö��Ÿ�ʽ
	}
	
	while( !UART0_receive_compare	(ok)); 			//ȷ���յ�OK					
	arry_to_zero();
 	
	key_flag=0;
//	led0=0;


}������
void main (void)	//������	'
{
	initial();
	while (1) 
    {
		arry_to_zero();
		while(!UART0_receive_compare (newmsg));
//   	while (!((UART0_receive_compare(newmsg))&&(key()==0)))		//�ж��Ƿ�������Ϣ
//		if(UART0_receive_compare(newmsg)==1)
//		{
//			led7=0;
//			delay1(800);
//			led7=1;
			arry_to_zero();
      		UART0_send_string("AT+CMGR=1\r\n");					//����Ϣ

			while(!UART0_receive_compare (new));			   //�ն�������
			delay1(200);
			compare_led();
			compare_beep();
			compare_usb();
//			led6=0;
//			delay1(800);
//			led6=1;
			flag1=0;
			L1602_string(1,4," ");							   //��ʾ�绰����
			L1602_string(2,4," ");							   //��ʾ�绰����
			L1602_string(1,4,telnum);						   //��ʾ�绰����
			L1602_string(1,15,"  ");						   //��ʾ�绰����
			key();
			count=0;
            temperature();
            L1602_string(2,15,temp);                   //��ʾ�¶�
      		UART0_send_string("ATD");					//����
            UART0_send_string(telnum);
      		UART0_send_string("\r\n");		
//		}
		continue;
		}
}