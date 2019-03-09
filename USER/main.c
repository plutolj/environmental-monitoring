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


//START ����
//�����������ȼ�
#define START_TASK_PRIO			10  ///��ʼ��������ȼ�Ϊ���
//���������ջ��С
#define START_STK_SIZE			128
//���������ջ
OS_STK START_TASK_STK[START_STK_SIZE];
//������
void start_task(void *pdata);

//LED0����
//�����������ȼ�
#define LED0_TASK_PRIO			7
//���������ջ��С
#define LED0_STK_SIZE			64
//�����ջ
OS_STK LED0_TASK_STK[LED0_STK_SIZE];
//������
void led0_task(void *pdata);

//LED1����
//�����������ȼ�
#define OLED_TASK_PRIO			6
//���������ջ��С
#define OLED_STK_SIZE			128
//�����ջ
OS_STK LED1_TASK_STK[OLED_STK_SIZE];
//������
void OLED_task(void *pdata);

//�����������
#define GETT_TASK_PRIO			5
//���������ջ��С
#define GETT_STK_SIZE			128
//�����ջ
//���������ʹ��printf����ӡ�������ݵĻ�һ��Ҫ8�ֽڶ���
__align(8) OS_STK GETT_TASK_STK[GETT_STK_SIZE]; 
//������
void GETT_task(void *pdata);


//��ʪ�Ȳɼ�ʵ��
void recievePkg(sPkg* pkg);
void loopReceive(void);
void loopAll(void);
void initAll(void);
extern void uartRevieveByte(u8 data);

	u8 temperature;  	    
	u8 humidity;  
//newPkg(num)��һ����,�����չ������һ�����ṹ��,�������ݲ��ֳ�����num
//ʹ���������������RAM������һ������3�����ݵİ�
//newPkg(3) redPkg={	
//	.length = 7,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x0000,
//	.data = {0,10,20}
//};

newPkg(3) redPkg={7,0x90,0x32,0x00,0x00,{0,10,20}};

char buf1[7]={0x05,0x90,0x21,0x00,0x00,0x01,0xFF};//��ȡģ��ĵ�ַָ��,ע����ǰ���0xFE���ӵ��������棬��ΪsendPkg���������Զ��ᷢ0xfe
char buf2[7]={0x05,0x90,0x21,0x00,0x00,0x02,0xFF};//��ȡģ�������IDָ��
char buf3[7]={0x05,0x90,0x21,0x00,0x00,0x03,0xFF};//��ȡģ����ŵ�ָ��
char buf4[7]={0x05,0x90,0x21,0x00,0x00,0x04,0xFF};//��ȡģ��Ĳ�����ָ��

//newPkg(1) THPkg={
//	.length = 5,
//	.src_port = 0x90,
//	.dis_port = 0x32,
//	.remote_addr = 0x000f,
//	.data = {0}
//};
//newPkg(1) THPkg={5,0xa0,0x32,0x0cd0,{0}};
newPkg(1) THPkg={5,0x90,0x20,0x04,0x15,{0}};//�¶Ȱ���ʽ  Ŀ�ĵ�ַ��1504 �˿ں�20
newPkg(1) humiditypkg={5,0x90,0x81,0x04,0x15,{0}};//ע�����ʽʪ�� Ŀ�ĵ�ַ��1504 �˿ں�20
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
		//DHT11_Read_Data(&temperature,&humidity);//��ȡ��ʪ��ֵ	
		THPkg.dis_port = 0x80;		//�¶ȶ˿ں���80
		THPkg.data[0] = temperature;
	//	THPkg.data[0] = 0xef;
	//	OS_ENTER_CRITICAL();	//�����ٽ���(�ر��ж�)
	//	sendPkg((sPkg*)(&THPkg));
	//	OS_EXIT_CRITICAL();		//�˳��ٽ���(���ж�)

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
	//	OS_ENTER_CRITICAL();	//�����ٽ���(�ر��ж�)
	//	sendPkg((sPkg*)(&humiditypkg));
	//	OS_EXIT_CRITICAL();		//�˳��ٽ���(���ж�)
		
		//redOff();
		//LED0=0;
		for(i = 0;i<100;i++)
			{
				delay_ms(10);
				loopReceive();
			}
			
				//LED0=!LED0;//��˸LED,��ʾϵͳ��������.
			
	 // sendPkg((sPkg*)(&buf1)); //���ڲ���
			
			

		if(USART3_RX_STA&0X8000)			//���յ�һ��������
		 {

		   reclen=USART3_RX_STA&0X7FFF;	//�õ����ݳ���
			 
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


//���������Ҫ�ڹ����в��ϱ�����,���᳢��һ�ν��հ�,
//������ճɹ��ͽ���recievePkg����,���ٴγ���,ֱ���ղ����µİ�
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

//�յ�һ�����������������,����������ݰ���Ŀ�Ķ˿�ѡ����Ӧ�ĳ�����д���
void recievePkg(sPkg* pkg)
{
	//printf((char*) pkg);//ͨ������1���͸����ԣ����ڲ���
	switch(pkg->dis_port)
	{
		case 0x80:		
		if(pkg->data[0] != 0x00)
		{
		//	greenTog();
		LED0=!LED0;//��˸LED,��ʾϵͳ��������.
			temperature=pkg->data[0];
		//	THPkg.data[0] = temperature;
		//	sendPkg((sPkg*)(&THPkg));
		}
		break;
		case 0x81:		
		if(pkg->data[0] != 0x00)
		{
		//	greenTog();
		LED0=!LED0;//��˸LED,��ʾϵͳ��������.
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

	delay_init();	    	 //��ʱ������ʼ��	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);// �����ж����ȼ�����2
	uart_init(115200);	 //����1��ʼ��Ϊ9600
	usart3_init(115200);//����3��ʼ��Ϊ9600
	LED_Init();		  	 //��ʼ����LED���ӵ�Ӳ���ӿ� 

//	while(DHT11_Init())	//DHT11��ʼ��	
//	{

//	}	
	
	
}


int main(void)
{	
	

	//ϵͳ��ʼ��
	SystemInit();
	delay_init();       //��ʱ��ʼ��
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //�жϷ�������
	OLED_Init();			//��ʼ��OLED  
	//Timer2_Init(0xffff,0);	//������2��ʼ��
	uart_init(115200);    //���ڲ���������
	usart3_init(115200);//����3��ʼ��Ϊ9600
	LED_Init();  	//LED��ʼ��
	OLED_Clear(); 

	LED0=0;
	
	OSInit();  		//UCOS��ʼ��
	OSTaskCreate(start_task,(void*)0,(OS_STK*)&START_TASK_STK[START_STK_SIZE-1],START_TASK_PRIO); //������ʼ����
	OSStart(); 	//��ʼ����
}

//��ʼ����
void start_task(void *pdata)
{
	OS_CPU_SR cpu_sr=0;
	pdata=pdata;
	OSStatInit();  //����ͳ������
	
	OS_ENTER_CRITICAL();  //�����ٽ���(�ر��ж�)
	OSTaskCreate(led0_task,(void*)0,(OS_STK*)&LED0_TASK_STK[LED0_STK_SIZE-1],LED0_TASK_PRIO);//����LED0����
	OSTaskCreate(OLED_task,(void*)0,(OS_STK*)&LED1_TASK_STK[OLED_STK_SIZE-1],OLED_TASK_PRIO);//����OLED����
	OSTaskCreate(GETT_task,(void*)0,(OS_STK*)&GETT_TASK_STK[GETT_STK_SIZE-1],GETT_TASK_PRIO);//���������¶�����
	OSTaskSuspend(START_TASK_PRIO);//����ʼ����
	OS_EXIT_CRITICAL();  //�˳��ٽ���(���ж�)
}
 

//LED0����
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

//OLED����
void OLED_task(void *pdata)
{
//	u8 humi=0;
	while(1)
	{
	//			OLED_showbh(0,BMP2[bmph]);
		//	OLED_ShowNum(64,4,t,3,16);//��ʾASCII�ַ�����ֵ 							
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

//��ȡ����ֵ������Ƶ�ʣ������¶�����
void GETT_task(void *pdata)
{

	//u16 i=0; //����ͳ��ѭ������
//	OS_CPU_SR cpu_sr=0;
	while(1)
	{
//	OS_ENTER_CRITICAL();	//�����ٽ���(�ر��ж�)
	loopAll();
//	OS_EXIT_CRITICAL();		//�˳��ٽ���(���ж�)

	delay_ms(100);
	}
}

