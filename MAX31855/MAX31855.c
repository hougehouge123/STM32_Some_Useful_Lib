 
/**
  ******************************************************************************
  * @file           : max31855.c
  * @brief          :MAX31855��˲����ȵ�ż������ת��������
  ******************************************************************************
  *���ڣ�2021.1.14
  * 
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "MAX31855.h"
#include "usart.h"
 
/* ˽�б��� ---------------------------------------------------------*/
float TCTemp = 0, CJTemp = 0; //�ȵ�ż���� �ο��˱���
bool TCErrorFlag = false;		//�ȵ�ż���ϱ�־λ����0                       
 
/* MAX31855 functions */
/*�ȵ�ż�˲�������*/
static float ThermalCoupleTemp(uint16_t data)
{
  float fraction = 0.0;	//С������
//  float temp = 0.0;
  uint16_t intpart = 0;	//��������
 
  
  /* �ȵ�żС������λ: D19 D18 */
//  if(data&0x0008)    fraction += 0.5;
//  if(data&0x0004)    fraction += 0.25;
 
	/*����С������*/
	fraction = ((data&0x000F) >> 2)*0.25;	//С������
	intpart = (data>>4)&0x7FF;		//�Ƚ�D16��D19λ�Ƶ�����ȥ���ţ�ȡ�������֡�
  /*�ж������¶Ȳ����� */
	  if(data&0x8000)	//���D31λΪ1�����ʾ�Ǹ��¶�
	  {int i;
		TCTemp=((2047-intpart)+(1-fraction));
		  TCTemp=intpart+fraction;
//						i=TCTemp*10;
//	printf("x3.val=-%d\xff\xff\xff",i);
//		printf("�ȵ�ż���¶ȣ�-%0.2f��\r\n",TCTemp);
	  }
	  else //����Ϊ���¶ȣ�ֱ��ȡ���ݼ���
	  {int i;
		TCTemp=intpart+fraction; //�Ƚ�D31����λȥ�����ٽ�D16 D17λȥ���������ȵ�ż�¶����ݡ�
//			i=TCTemp*10;
//	printf("x3.val=%d\xff\xff\xff",i);
//		printf("�ȵ�ż���¶ȣ�%0.2f��\r\n",TCTemp);
	  }
	return TCTemp;
}
 
/*�ο��˲�������*/
static float ColdJunctionTemp(uint16_t data)
{
  float fraction = 0.0;
//  float temp = 0.0;
  uint16_t intpart = 0;
  
  /*�ڲ��¶�С������: D7 D6 D5 D4*/
//  if(data&0x0080)        fraction += 0.5;
//  if(data&0x0040)        fraction += 0.25;
//  if(data&0x0020)        fraction += 0.125;
//  if(data&0x0010)        fraction += 0.0625;
	
  fraction = ((data&0x00FF) >> 4)*0.0625;  //С������
  intpart = (data>>8)&0X7F;	//�Ƚ�D0��D7λ�Ƶ���ȥ���ţ�ȡ�������� 
  /*�ж������¶Ȳ����� */	
  if(data&0x8000)	//D15λΪ����λ��Ϊ1��ʾ���¶�
  {int i;
	CJTemp=(127-intpart)+(1-fraction);
//		i=CJTemp*10;
//	printf("x3.val=-%d\xff\xff\xff",i);	
//	printf("�ο��ˣ�-%0.2f��\r\n",CJTemp);	  
  }
  else	//����Ϊ���¶�
  {int i;
	CJTemp=intpart+fraction;
//		i=CJTemp*10;
//	printf("x3.val=%d\xff\xff\xff",i);	
//	printf("�ο��ˣ�%0.2f��\r\n",CJTemp);
  }
  return CJTemp;
}
 
static void Max31855ReadData(uint16_t *TC, uint16_t *CJ)
{
#if SPI_SIM
  uint32_t data = 0;
  
  HAL_GPIO_WritePin(MAX_SPI_GPIO_Port, MAX_SPI_CS_Pin|MAX_SPI_SCK_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);        // Delay 1us
  
  for(int i=0; i<32; i++)
  {
    HAL_GPIO_WritePin(MAX_SPI_GPIO_Port, MAX_SPI_SCK_Pin, GPIO_PIN_SET);    
    /*����ʱ��Ƶ�����ֵ 5MHz */
    HAL_Delay(1);         // Delay 1us
 
    data <<= 1;
    if(HAL_GPIO_ReadPin(MAX_SPI_GPIO_Port, MAX_SPI_MISO_Pin) == GPIO_PIN_SET)
    {
      data |= 1U;      // Bit set
    }
    else
    {
      //data &= ~1U;   // Bit clear
    }
    
    HAL_GPIO_WritePin(MAX_SPI_GPIO_Port, MAX_SPI_SCK_Pin, GPIO_PIN_RESET);    
    HAL_Delay(1);        // Delay 1us
  }
  
  HAL_GPIO_WritePin(MAX_SPI_GPIO_Port, MAX_SPI_CS_Pin, GPIO_PIN_SET);
  
  *TC = data >> 16;
  *CJ = data & 0x0000FFFF;
#else
//  uint8_t TXData[4];
  uint8_t RXData[4];//32λ����
  
	SPI2_CS_L;
	HAL_Delay(1);
//	HAL_SPI_TransmitReceive (&hspi2,TXData,RXData, 4, 100);	//��ͨSPI�������
//	HAL_SPI_Receive(&hspi2,RXData,4,100);
	HAL_SPI_Receive_DMA(&hspi2,RXData,4);	//�����DMA���䣬��CubeMX������ʱ�����뽫SPI��DMA��TX��RX����
	HAL_Delay(1);
	SPI2_CS_H;
  
 
  *TC = RXData[0]<<8|RXData[1];	//������������
  *CJ = RXData[2]<<8|RXData[3];
  
//  printf("RXData[0]=%X\r\n",RXData[0]);
//  printf("RXData[1]=%X\r\n",RXData[1]);
//  printf("RXData[2]=%X\r\n",RXData[2]);
//  printf("RXData[3]=%X\r\n",RXData[3]);
//  printf("TC=%X,CJ=%X\r\n",*TC,*CJ);
#endif
}
 
/*�õ��ȵ�ż�������¶ȡ� */
float Max31855GetTemp(void)
{
  uint16_t TC = 0, CJ = 0;
  
  Max31855ReadData(&TC, &CJ);
  
  //���κ�SCV��D2����SCG��D1����OC��D0�����ϼ���ʱ��D16�Ķ���Ϊ1
  if(TC&0x0001)	//�ж��Ƿ����
  {
	TCErrorFlag = true;
  }
  else 
  {
	TCErrorFlag = false;
  }
  
  if(TCErrorFlag==true)		//�����ֹ���������ж�
  {
//	printf("D16λΪ1���ȵ�ż����\r\n");
	if((CJ&0x0001)==0x0001)
	{
//		printf("OC�ȵ�ż��·\r\n");
	}  
	if((CJ&0x0002)==0x0002)
	{
//		printf("SCG�ȵ�ż��·����GND\r\n");
	}
	if((CJ&0x0004)==0x0004)
	{
//		printf("SCV�ȵ�ż��·����VCC\r\n");
	}	  
  }
  else	//�޹��ϣ���������ת��
  {int i;
//	printf("�ȵ�ż�޹���\r\n\xff\xff\xff");
	TCTemp = ThermalCoupleTemp(TC);
	CJTemp = ColdJunctionTemp(CJ);
  i=CJTemp*10;
	printf("x3.val=%d\xff\xff\xff",i);
	i=TCTemp*10;
	printf("x2.val=%d\xff\xff\xff",i);
  }
	return TCTemp;
}
 
///* ��ȡ�ȵ�ż�¶ȡ�*/
//float Max31855TCTemp()
//{
//  return TCTemp;
//}
 
///*��ȡ��̬�¶ȡ�*/
//float Max31855_CJTemp()
//{
//  return CJTemp;
//}
/**********END OF FILE****/
