#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "timer.h"
#include "OLED.h"
//#include "bmp.h"
#include "includes.h"
#include "lee.h"
//#include "dht11.h" 	
#include "package.h"
#include "DL_LN3X.h"
#include "usart3.h"			 	 
#include "string.h"	  
#include <stdio.h> 


//START 任务
//设置任务优先级
#define START_TASK_PRIO			10  ///开始任务的优先级为最低
//设置任务堆栈大小
#define START_STK_SIZE			128
//任务任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *pdata);

//LED0任务
//设置任务优先级
#define LED0_TASK_PRIO			7
//设置任务堆栈大小
#define LED0_STK_SIZE			64
//任务堆栈
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//任务函数
void led0_task(void *pdata);

//LED1任务
//设置任务优先级
#define OLED_TASK_PRIO			6
//设置任务堆栈大小
#define OLED_STK_SIZE			128
//任务堆栈
OS_STK LED1_TASK_STK[OLED_STK_SIZE];
//任务函数
void OLED_task(void *pdata);

//浮点测试任务
#define GETT_TASK_PRIO			5
//设置任务堆栈大小
#define GETT_STK_SIZE			128
//任务堆栈
//如果任务中使用printf来打印浮点数据的话一点要8字节对齐
__align(8) OS_STK GETT_TASK_STK[GETT_STK_SIZE]; 
//任务函数
void GETT_task(void *pdata);


//温湿度采集实验
void recievePkg(sPkg* pkg);
void loopReceive(void);
void loopAll(void);
void initAll(void);
extern void uartRevieveByte(u8 data);

	u8 temperature;  	    
	u8 humidity;  
//newPkg(num)是一个宏,这个宏展开后是一个包结构体,包的数据部分长度是num
//使用下面的语句可以在RAM中生成一个带有3个数据的包
//newPkg(3) redPkg={	
//	.length = 7,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x0000,
//	.data = {0,10,20}
//};

newPkg(3) redPkg={7,0x90,0x32,0x00,0x00,{0,10,20}};

char buf1[7]={0x05,0x90,0x21,0x00,0x00,0x01,0xFF};//读取模块的地址指令,注意最前面的0xFE不加到数组里面，因为sendPkg函数里面自动会发0xfe
char buf2[7]={0x05,0x90,0x21,0x00,0x00,0x02,0xFF};//读取模块的网络ID指令
char buf3[7]={0x05,0x90,0x21,0x00,0x00,0x03,0xFF};//读取模块的信道指令
char buf4[7]={0x05,0x90,0x21,0x00,0x00,0x04,0xFF};//读取模块的波特率指令

//newPkg(1) THPkg={
//	.length = 5,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x000f,
//	.data = {0}
//};
//newPkg(1) THPkg={5,0xa0,0x32,0x0cd0,{0}};
newPkg(1) THPkg={5,0x90,0x20,0x04,0x15,{0}};//温度包格式  目的地址是1504 端口号20
newPkg(1) humiditypkg={5,0x90,0x81,0x04,0x15,{0}};//注意包格式湿度 目的地址是1504 端口号20
void loopAll()
{ 
	u16 i;
	u8 reclen=0;  
	//u8 m=0;
	//OS_CPU_SR cpu_sr=0;
	
	
	while (1)
	{
//		for(i = 0;i<400;i++)
//		{			
//			delay_ms(10);
//			loopReceive();
//		}
		//DHT11_Read_Data(&temperature,&humidity);//读取温湿度值	
		THPkg.dis_port = 0x80;		//温度端口号是80
		THPkg.data[0] = temperature;
	//	THPkg.data[0] = 0xef;
	//	OS_ENTER_CRITICAL();	//进入临界区(关闭中断)
	//	sendPkg((sPkg*)(&THPkg));
	//	OS_EXIT_CRITICAL();		//退出临界区(开中断)

    //LED0=1;
		for(i = 0;i<100;i++)
		{
			delay_ms(10);
			loopReceive();
		}		
		humiditypkg.dis_port = 0x81;
//		humiditypkg.data[0] = humidity;
		humiditypkg.data[0] = humidity;
	//	THPkg.data[0] = m;
	//	humidity++;
	//	if(humidity>=0x3c) humidity=0;
	//	OS_ENTER_CRITICAL();	//进入临界区(关闭中断)
	//	sendPkg((sPkg*)(&humiditypkg));
	//	OS_EXIT_CRITICAL();		//退出临界区(开中断)
		
		//redOff();
		//LED0=0;
		for(i = 0;i<100;i++)
			{
				delay_ms(10);
				loopReceive();
			}
			
				//LED0=!LED0;//闪烁LED,提示系统正在运行.
			
	 // sendPkg((sPkg*)(&buf1)); //用于测试
			
			

		if(USART3_RX_STA&0X8000)			//接收到一次数据了
		 {

		   reclen=USART3_RX_STA&0X7FFF;	//得到数据长度
			 
			 for(i=0;i<reclen;i++)
			 uartRevieveByte(USART3_RX_BUF[i]);

			 USART3_RX_STA=0;	 
		 }

//		for(i = 0;i<100;i++)
//			{
//				delay_ms(10);
//				loopReceive();
//			}		 
 									
	}
}


//这个函数需要在工作中不断被调用,它会尝试一次接收包,
//如果接收成功就交给recievePkg处理,并再次尝试,直到收不到新的包
void loopReceive(void)
{
	sPkg* pkg;
	pkg = getNextPkg();
	while(pkg != NULL)
	{
		recievePkg(pkg);
		pkg = getNextPkg();
	}
}

//收到一个包后会调用这个函数,这个函数根据包的目的端口选择相应的程序进行处理
void recievePkg(sPkg* pkg)
{
	//printf((char*) pkg);//通过串口1发送给电脑，用于测试
	switch(pkg->dis_port)
	{
		case 0x80:		
		if(pkg->data[0] != 0x00)
		{
		//	greenTog();
		LED0=!LED0;//闪烁LED,提示系统正在运行.
			temperature=pkg->data[0];
		//	THPkg.data[0] = temperature;
		//	sendPkg((sPkg*)(&THPkg));
		}
		break;
		case 0x81:		
		if(pkg->data[0] != 0x00)
		{
		//	greenTog();
		LED0=!LED0;//闪烁LED,提示系统正在运行.
			humidity=pkg->data[0];
		//	humiditypkg.data[0] = humidity;
		//	sendPkg((sPkg*)(&humiditypkg));
		}
		break;
		default:
		break;		
	}
}


void initAll()
{

	delay_init();	    	 //延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// 设置中断优先级分组2
	uart_init(115200);	 //串口1初始化为9600
	usart3_init(115200);//串口3初始化为9600
	LED_Init();		  	 //初始化与LED连接的硬件接口 

//	while(DHT11_Init())	//DHT11初始化	
//	{

//	}	
	
	
}


int main(void)
{	
	

	//系统初始化
	SystemInit();
	delay_init();       //延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	OLED_Init();			//初始化OLED  
	//Timer2_Init(0xffff,0);	//计数器2初始化
	uart_init(115200);    //串口波特率设置
	usart3_init(115200);//串口3初始化为9600
	LED_Init();  	//LED初始化
	OLED_Clear(); 

	LED0=0;
	
	OSInit();  		//UCOS初始化
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //创建开始任务
	OSStart(); 	//开始任务
}

//开始任务
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //开启统计任务
	
	OS_ENTER_CRITICAL();  //进入临界区(关闭中断)
	OSTaskCreate(led0_task,(void*)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);//创建LED0任务
	OSTaskCreate(OLED_task,(void*)0,(OS_STK*)&LED1_TASK_STK[OLED_STK_SIZE-1],OLED_TASK_PRIO);//创建OLED任务
	OSTaskCreate(GETT_task,(void*)0,(OS_STK*)&GETT_TASK_STK[GETT_STK_SIZE-1],GETT_TASK_PRIO);//创建计算温度任务
	OSTaskSuspend(START_TASK_PRIO);//挂起开始任务
	OS_EXIT_CRITICAL();  //退出临界区(开中断)
}
 

//LED0任务
void led0_task(void *pdata)
{
	while(1)
	{
		LED0=0; 
		delay_ms(1000);
		LED0=1;
		delay_ms(1000);
	}
}

//OLED任务
void OLED_task(void *pdata)
{
//	u8 humi=0;
	while(1)
	{
	//			OLED_showbh(0,BMP2[bmph]);
		//	OLED_ShowNum(64,4,t,3,16);//显示ASCII字符的码值 							
			//	show_R(64,4,temperature);
	//	humi=(u8)humidity;
		OLED_ShowCHinese(1,1,0);
		OLED_ShowCHinese(18,1,1);
		OLED_ShowCHinese(35,1,2);
		show_R(44,1,(float)temperature);
		delay_ms(12);	
		OLED_ShowCHinese(1,4,3);
		OLED_ShowCHinese(18,4,4);
		OLED_ShowCHinese(35,4,5);
		show_R(44,4,(float)humidity);
	//			show_R(64,6,flequency);
		delay_ms(1200);
	}
}

//获取计数值，计算频率，计算温度任务
void GETT_task(void *pdata)
{

	//u16 i=0; //用于统计循环次数
//	OS_CPU_SR cpu_sr=0;
	while(1)
	{
//	OS_ENTER_CRITICAL();	//进入临界区(关闭中断)
	loopAll();
//	OS_EXIT_CRITICAL();		//退出临界区(开中断)

	delay_ms(100);
	}
}

