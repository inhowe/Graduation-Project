/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   USART printf example
  ******************************************************************************
  * @attention
    δ��ɣ�
    ����Ĺ㲥
    ִ�����㲥
    ������
    ��λ�����ͽ���
    ִ��������
  *
  ******************************************************************************
  */

/*Include Files*/
#include "including.h"

/*Extern Variable*/

/*Global Variable*/
TIME_TypeDef GetTime,SetTime;
u8 CANFlag=0;
u8 GetCAN[8];
uint32_t CAN_ID;//��ʶ��
u8 SecAlarm=0;//��
u8 Ctrl_ID=0xC0;//����������ʶ
u8 tempflag=0;
u8 Token=0xD1;//����
DetectorMsg Detector_1F[4]={0},Detector_2F[4]={0},Detector_3F[4]={0};//�����±�=̽�������,0��Ӧ���ǿյģ�
ActuatorMsg Fan;
ActuatorMsg Door;
ActuatorMsg Pump;
ActuatorMsg Alarm;
FiredMsg FiredDetector[9];
u32 FiredNum=0;
u8 Fired[9]={0};
u8 RestFire=0;//��������� 
u8 LocalFire=0;//�������
u8 Reset=0;
u8 FiredSended=0;
/*Extern Function*/
void NVIC_Config(void);

void ReadID(void)
{
  
  GPIO_InitTypeDef 			 GPIO_InitStructure;
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(GPIOC, &GPIO_InitStructure);    
  
  Ctrl_ID=Ctrl_ID+(GPIO_ReadInputData(GPIOC)&0x0F);
}
 /**
  * @brief  ������
  * @param  ��
  * @retval ��
  */
int main(void)
{
  
	NVIC_Config();
	USART1_Config();
	
	CAN_Config();
	DS3231_Config();
  ActuatorInit();
  ReadID();
  
//  SetTime.year =0x17;
//  SetTime.month =0x05;
//  SetTime.date =0x19;
//  SetTime.hour =0x14;
//  SetTime.min =0x11;
//  SetTime.sec =0x00;
//  DS3231_WriteTime(&SetTime);
  
	LED_STATE_Config();
	SysTick_Init();
  Ctrl_ID=0xC1;
  //�������߻�������
  if(Ctrl_ID==0xC1)//�����������õĹ���
  {
    Screen_Config();
    Delay_ms(3000);
    SC_SendID();//Ҫ��2�Σ������ǵ�һ�η�����©��һ������
    SC_SendID();
//    TCPS_Config();
  }
	
  tick=0;
  printf("START!\r\n");
  LocalFire=RestFire=0;
  Door.ID=1;
  Door.State=0;
	while(1)
	{
    LED_Flash();
    
    if( dhcp_ok&&(Ctrl_ID==0xC1) )
      do_tcp_server();
    
    if(LocalFire||RestFire)//�л��飬��ʼִ�з�������
    {
      DealActuator();
    }
//    DealDoor();
    if(CANFlag!=0)
    {
      CANFlag=0;
      #ifdef DEBUG
      printf("CAN1 get data!\r\n");
      #endif
    }
    if(pulse)
    {
      pulse=0;
      CAN_TokenCtl();
      CheckFire();
    }
    if(SecAlarm)
    {
      #ifdef DEBUG
      //������Ϊ���ڵ���ʹ���߷�æ
      printf("Time:%x:%x:%x\r\n",GetTime.hour ,GetTime.min ,GetTime.sec );
      #endif
      tempflag=1;//TCP���ͱ�־
      SecAlarm=0;
      if(Ctrl_ID==0xC1)//���������ſ�����Ļ
      {
        SC_SendTime();
        SC_SendFirePoint();
      }
//    PwrTokenCtrl();//�����ز����ƿ���
//    CAN_Broadcast();//CAN���߹㲥
    }
	}
}

void NVIC_Config(void)
{	
	NVIC_InitTypeDef NVIC_InitStructure; 	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	/* Enable CAN1 RX0 interrupt IRQ channel */
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable the USART2 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

  /*Enable the EXTI4 Interrupt(used for DS3231_INT)*/
  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
	/*Enable the TIM2 Interrupt(used for w5500)*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}
/*********************************************END OF FILE**********************/