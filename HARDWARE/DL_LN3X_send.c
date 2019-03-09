#include "lee.h"
#include "led.h"
#include "package.h"
#include "usart.h"
#include "sys.h"
#include "usart3.h"
u8 uartSendNextByte(void);




//�������͵İ�ת���������ʽͳһ����
typedef struct sPkgBase__
{
	u8 length;
	u8 data[63];
}sPkgBase;


//���ͼ�����,�����������¼����һ��Ҫ���͵�������data�����е�ƫ����
#define SS_IDLE 0XFF//�������Ϊ0xff��ʾĿǰ���Ϳ���
#define SS_HEAD 0XA0//�������Ϊ0xA0��ʾ�ոշ����˰�ͷ��0xfe�ȴ����Ͱ�����
static u8 Send_counter = SS_IDLE;


//���Ͱ�����,�����͵İ���������������ṹ��
static sPkgBase Send_temp;

/********************************************************
*���Ͱ�����,�������������һ����
*����:	
*	sPkg* pkg		�����͵İ�,���뽫��ת��ΪsPkg��ʽ���з���
*����ֵ:	����fail��ʾ����ʧ��,ͨ������Ϊ�������ڷ���һ����
*		����done��ʾ�����ɹ�.done��fail����lee.h�ж���ĺ�
*********************************************************/		
u8 sendPkg(sPkg* pkg)
{
	u16 i;
	u8 data;
  sPkgBase* pb = (sPkgBase*)pkg;
	if(Send_counter != SS_IDLE)
	{
		return fail;
	}

	Send_temp.length = pb->length;
	for( i = 0;i<Send_temp.length;i++)
	{
		Send_temp.data[i] = pb->data[i];	
	}
	Send_counter = SS_HEAD;
	//uartSendFirstByte(0xfe);
	
  while((USART3->SR&0X40)==0);//�ȴ����ͽ���
	USART3->DR=0xfe;
	
	do{
	
	data=uartSendNextByte();
	
  while((USART3->SR&0X40)==0);//�ȴ����ͽ���
	USART3->DR=data;
		
	}while(data != 0xff);
	
	return done;
}

/********************************************************
*��ȡ��һ�������͵�����,���ڷ��ͻ�����жϻ�����������,
*��������᷵�ر�������һ�������͵�����.
*
*����:	��
*����ֵ:	�����͵���һ������,����0xff������������һ������.
*********************************************************/
u8 uartSendNextByte(void)
{
	switch (Send_counter)
	{
	case SS_HEAD:
		Send_counter = 0;
		return Send_temp.length;
	case SS_IDLE:
		return 0xff;
	default:
		if(Send_counter == Send_temp.length)
		{
			Send_counter = SS_IDLE;
			return 0xff;
		}
		else
		{
			if(Send_temp.data[Send_counter] >= 0xfe)
			{
				Send_temp.data[Send_counter]-=2;
				return 0xfe;
			}
			else
			{
				u8 temp = Send_temp.data[Send_counter];
				Send_counter++;
				return temp;
			}
		}
	}
}
