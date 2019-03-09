/*
 * DL_LN3X.c
 *
 * Created: 2017-03-07
 *  Author: mchen_6431
 */ 

#include "lee.h"
#include "led.h"
#include "package.h"
#include "usart.h"
#include "usart3.h"
#include "sys.h"


//���հ�ʹ�õĽṹ��,���˿ں͵�ַ��������data��
typedef struct sPkgBase__
{
	u8 length;
	u8 data[63];
}sPkgBase;

//�յ���ͷ����
static void recvHead(u8 totle);
//�յ���β0xff
static void recvTerminal(void);
//�յ����ݺ���
static void recvData(u8 data);
//�����־����һ���յ���0xfeʱ��ֵΪyes,����Ϊ0
static u8 escape = no;

//�����յ�1byte����ʱ,�ж��е��ô˺���
void uartRevieveByte(u8 data)
{
	switch(data)
	{
		case 0xff://�յ�������
		recvTerminal();
		break;
		case 0xfe://�յ�ת���ַ�
		escape = yes;
		break;
		default://�յ�һ������
		if(escape == yes)
		{//���ǰһ����ת���ַ�
			escape = no;
			if(data <=63)
			{
				recvHead(data);
			}
			else
			{//�յ�ת�����ݵĹ�����0xfe���������+2��ԭ����
				recvData(data+2);
			}
		}
		else
		{//ǰһ������ת���ַ�,ֱ���յ�һ������
			recvData(data);
		}
		break;
	}
}



//���հ�ʹ�õ�˫����,����һ��������ʹ��,��һ������װ����һ����.
static sPkgBase recv_temp[2];
//ȷ��˫�������ĸ����ڽ���,�ĸ���������ı���
static volatile  u8 Loading = 0;
//���ڽ���װ�����ݵĻ���
#define Load_pkg recv_temp[Loading]
//���ڳ�������Ļ���
#define User_pkg recv_temp[(Loading+1)&1]

//���ռ�����,��ʾһ�����յ��˶��ٸ�����
#define RS_IDLE 0XFF//��ʾ����û���յ���ͷ�ͳ���
#define RS_DONE 0xA0//��ʾ���Ѿ�����˽���
static volatile u8 Recv_counter = RS_IDLE;

//���User_pkgװ��һ��δ����İ�,�˱���Ϊ yes
static volatile u8 Received = no;
//����û����ڷ���User_pkg�е����ݴ˱���Ϊ yes
static volatile u8 Locked = no;


//�����������˵����һ�����Ѿ���������,�����Խ���һ����,
//����Ѿ��յ���,�򷵻�һ��ָ�����ָ��,���򷵻�null
sPkg* getNextPkg(void)
{
//	cli();
	sPkg* rev;
	
	Locked = no;

	if(Received == yes)
	{//����������������,receive����yes��������
		Received =no;
		Locked = yes;
		rev = (sPkg*)(&User_pkg);
	}
	else
	{//��û�н���,��ô�Ƿ�Ӧ�ý�����?
		if(Recv_counter == RS_DONE)
		{//�����һ�������Ѿ������˾�Ӧ�ý���
			Loading++;
			Loading&=1;
			Recv_counter = RS_IDLE;	
			Locked = yes;
			rev = (sPkg*)(&User_pkg);			
		}
		else
		{//���򷵻ؿ�,˵��û�а�������
			rev = NULL;
		}		
	}	
	//sei();
	return rev;
}



static void recvHead(u8 totle)
{
	if(Recv_counter == RS_IDLE)
	{
		Load_pkg.length = totle;
		Recv_counter = 0;
	}
}

static void recvData(u8 data)
{	
	if(Recv_counter < Load_pkg.length)
	{		
		Load_pkg.data[Recv_counter] = data;
		Recv_counter++;
	}
	else
	{
		Recv_counter = RS_IDLE;
	}
}

static void recvTerminal(void)
{
	if(Recv_counter == Load_pkg.length)
	{//�յ���ɵİ�
		if(Locked == no)
		{//����������
			Loading++;
			Loading&=1;
			Received = yes;
			Recv_counter = RS_IDLE;
		} 
		else
		{//������������
			Recv_counter = RS_DONE;
		}
	}
	else
	{	//�յ�����ɵİ�,��Ҫ������	
		Recv_counter = RS_IDLE;
	}
}
