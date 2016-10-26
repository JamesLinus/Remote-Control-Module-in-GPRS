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
uchar key_flag=0;                       //判断是否按键扫描到数字
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


code unsigned char num0[]="000";	  //接收功率档位数
code unsigned char num1[]="111";
code unsigned char num2[]="222";
code unsigned char num3[]="333";
code unsigned char num4[]="444";
code unsigned char num5[]="555";
code unsigned char num6[]="666";
code unsigned char num7[]="777";
code unsigned char num8[]="888";
code unsigned char num9[]="999";
													  //控制占空比					
code uchar num[11][10]={1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0};
unsigned char tel[11];	

code uchar a[]={0,10,10,10,10,10,10,10,10,10}; //点亮时间
code uchar b[]={10,250,90,40,20,10,6,3,2,0};	  //熄灭时间

/*LED_Buffer[16]用来存储串口发送的数据，com_dat用来记录串口发送的个数*/
code uchar LED_Buffer[16], *q, com_dat;      //从串口接收的数据
//这三个引脚参考资料

sbit E=P2^7;		//1602使能引脚
sbit RW=P2^6;		//1602读写引脚	
sbit RS=P2^5;		//1602数据/命令选择引脚

sbit led1=P2^1;
sbit led2=P2^2;
sbit led3=P2^3;
sbit led4=P2^4;
sbit beep=P2^0;
sbit usb =P3^7;
/*
sbit led0=P1^0;
sbit led1=P1^1;		//定义标志开发板的led灯
sbit led2=P1^2;
sbit led3=P1^3;
sbit led4=P1^4;
sbit led5=P1^5;
sbit led6=P1^6;
sbit led7=P1^7;
*/
uchar dd=0;	  //标志是否第一次到三个引号
uchar ff,count=0;延时函数
void Delay(uint del)
{
	uint i,j;
	for(i=0; i<del; i++)
	for(j=0; j<1827; j++);
}

/********************************************************************
* 名称 : delay()
* 功能 : 延时,延时时间大概为140US。
* 输入 : 无
* 输出 : 无
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
* 名称 : enable(uchar del)
* 功能 : 1602命令函数
* 输入 : 输入的命令值
* 输出 : 无
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
* 名称 : wcmd(uchar del)
* 功能 : 1602命令函数
* 输入 : 输入的命令值
* 输出 : 无
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
* 名称 : wdata(uchar del)
* 功能 : 1602写数据函数
* 输入 : 需要写入1602的数据
* 输出 : 无
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
* 名称 : write(uchar del)
* 功能 : 1602写数据函数
* 输入 : 需要写入1602的数据
* 输出 : 无
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
* 名称 : L1602_init()
* 功能 : 1602初始化，请参考1602的资料
* 输入 : 无
* 输出 : 无
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
* 名称 : L1602_string(uchar hang,uchar lie,uchar *p)
* 功能 : 改变液晶中某位的值，如果要让第一行，第五个字符开始显示"ab cd ef" ，调用该函数如下
	 	 L1602_string(1,5,"ab cd ef;")
* 输入 : 行，列，需要输入1602的数据
* 输出 : 无
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
* 名称 : L1602_char(uchar hang,uchar lie,char sign)
* 功能 : 改变液晶中某位的值，如果要让第一行，第五个字符显示"b" ，调用该函数如下
		 L1602_char(1,5,'b')
* 输入 : 行，列，需要输入1602的数据
* 输出 : 无
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





串口通信程序
/********************************************************************
* 名称 : Com_Init()
* 功能 : 串口初始化，晶振11.0592,波特率9600，使能了串口中断
* 输入 : 无
* 输出 : 无
***********************************************************************/
void Com_Init(void)
{
     TMOD = 0x20;   //定时器工作在定时器1的方式2
     PCON = 0x00;   //不倍频
     SCON = 0x50;	//串口工作在方式1，并且启动串行接收	
     TH1 = 0xFd;    //设置波特率 9600
     TL1 = 0xFd;
     TR1 = 1;		//启动定时器1
	 ES = 1;		//开串口中断
	 EA = 1;		//开总中断		
}

/****************************************************************
*
*发送一个字符
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
*发送字符串
*****************************************************************/
void UART0_send_string(uchar *Buf)
{
 	while(*Buf!='\0') 
	{	   
		UART0_send_byte(*Buf++);
	}
}
 /*
 数组清0
 */
 void arry_to_zero()
 {	int i;
 	for (i=0;i<UART0_BUFFERSIZE;i++)
	{
	  UART0_Buffer[i] =0;
	
	}
   UART0_Buffer_Size=0;
 
 }
//接收数据判断

/********************************************************************
* 名称 : Com_Int()
* 功能 : 串口中断子函数
* 输入 : 无
* 输出 : 无
***********************************************************************/
void UART0_Interrupt (void) interrupt 4
{
	uchar i=0;
	if(RI == 1)   //当硬件接收到一个数据时，RI会置位
	{
		if( UART0_Buffer_Size == 0)         // If new word is entered
		{   
        	 UART0_Input_First = 0;         
		 }		
	   				//	LED_Buffer[com_dat] = SBUF; 		//把从串口读出的数存到数组
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
			for(i=0;i<11;i++)							   //显示手机号
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




键盘扫描程序
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
uchar key()						//扫描键盘
{
	uchar Key_Value;  //读出的键值
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
			Delay_1ms(20);	//按键消抖
			if(P1 != 0xf0)
			{
				Delay_1ms(20);	//按键消抖
				if(P1 != 0xf0)
				{
					Key_Value = Keyscan();
					key_flag=1;
					count++;
//					L1602_char(2,15,Key_Value / 10 + 48);	 //十位
					L1602_char(2,m,Key_Value % 10 + 48);	 //个位	
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




控制通断及功率程序
uchar compare_num()	            //判断是否收到调整功率的指令
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
											else if(UART0_receive_compare(num9))				//没收到信息
											{
												return	10;
											}
}


void compare_led()				//判断短信里是否有控制灯的信息
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

void compare_beep()					//判断短信里是否有控制蜂鸣器的信息
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

void compare_usb()					//判断短信里是否有控制USB口的信息
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
* 名称 : read_bit()
* 功能 : 从DS18B20读一个位值
* 输入 : 无
* 输出 : 从DS18B20读出的一个位值
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
* 名称 : write_bit()
* 功能 : 向DS18B20写一位
* 输入 : bitval（要对DS18B20写入的位值）
* 输出 : 无
***********************************************************************/
void write_bit(uchar bitval)
{
DQ=0;if(bitval==1)
DQ=1;
delay(5);
DQ=1;
}

/********************************************************************
* 名称 : read_byte()
* 功能 : 从DS18B20读一个字节
* 输入 : 无
* 输出 : 从DS18B20读到的值
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
* 名称 : write_byte()
* 功能 : 向DS18B20写一个字节
* 输入 : val（要对DS18B20写入的命令值）
* 输出 : 无
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

	while( !UART0_receive_compare	(ok))  //确认收到‘ok’     
   	{
		UART0_send_string("AT\r\n");	   //发送‘AT’指令，判断信号强弱
	}
	
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=1\r\n");				  //删除短信1
	}
	while(! UART0_receive_compare	(ok)); //确认收到‘ok’   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=2\r\n");				  //删除短信2
	}
	while(! UART0_receive_compare	(ok)); //确认收到‘ok’   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=3\r\n");				  //删除短信3
	}
	while(! UART0_receive_compare	(ok)); //确认收到‘ok’   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=4\r\n");				  //删除短信4
	}
	while(! UART0_receive_compare	(ok)); //确认收到‘ok’   
	arry_to_zero();

	do
	{
		UART0_send_string("AT+CMGD=5\r\n");				  //删除短信5
	}
	while(! UART0_receive_compare	(ok)); //确认收到‘ok’   
	arry_to_zero();
				 
	while( !UART0_receive_compare	(ok))
	{													//确认收到‘ok’     
		arry_to_zero();
   		UART0_send_string("AT+CMGF=1\r\n");			    //设置短信格式
	}
	
	while( !UART0_receive_compare	(ok)); 			//确认收到OK					
	arry_to_zero();
 	
	key_flag=0;
//	led0=0;


}主函数
void main (void)	//主函数	'
{
	initial();
	while (1) 
    {
		arry_to_zero();
		while(!UART0_receive_compare (newmsg));
//   	while (!((UART0_receive_compare(newmsg))&&(key()==0)))		//判断是否有新信息
//		if(UART0_receive_compare(newmsg)==1)
//		{
//			led7=0;
//			delay1(800);
//			led7=1;
			arry_to_zero();
      		UART0_send_string("AT+CMGR=1\r\n");					//读短息

			while(!UART0_receive_compare (new));			   //收短信内容
			delay1(200);
			compare_led();
			compare_beep();
			compare_usb();
//			led6=0;
//			delay1(800);
//			led6=1;
			flag1=0;
			L1602_string(1,4," ");							   //显示电话号码
			L1602_string(2,4," ");							   //显示电话号码
			L1602_string(1,4,telnum);						   //显示电话号码
			L1602_string(1,15,"  ");						   //显示电话号码
			key();
			count=0;
            temperature();
            L1602_string(2,15,temp);                   //显示温度
      		UART0_send_string("ATD");					//拨号
            UART0_send_string(telnum);
      		UART0_send_string("\r\n");		
//		}
		continue;
		}
}