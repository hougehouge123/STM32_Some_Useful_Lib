 
/**
  ******************************************************************************
  * @file           : max31855.c
  * @brief          :MAX31855冷端补偿热电偶到数字转换器程序。
  ******************************************************************************
  *日期：2021.1.14
  * 
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "MAX31855.h"
#include "usart.h"
 
/* 私有变量 ---------------------------------------------------------*/
float TCTemp = 0, CJTemp = 0; //热电偶变量 参考端变量
bool TCErrorFlag = false;		//热电偶故障标志位，置0                       
 
/* MAX31855 functions */
/*热电偶端采样计算*/
static float ThermalCoupleTemp(uint16_t data)
{
  float fraction = 0.0;	//小数部分
//  float temp = 0.0;
  uint16_t intpart = 0;	//整数部分
 
  
  /* 热电偶小数部分位: D19 D18 */
//  if(data&0x0008)    fraction += 0.5;
//  if(data&0x0004)    fraction += 0.25;
 
	/*先算小数部分*/
	fraction = ((data&0x000F) >> 2)*0.25;	//小数部分
	intpart = (data>>4)&0x7FF;		//先将D16到D19位移掉，再去符号，取整数部分。
  /*判断正负温度并计算 */
	  if(data&0x8000)	//如果D31位为1，则表示是负温度
	  {int i;
		TCTemp=((2047-intpart)+(1-fraction));
		  TCTemp=intpart+fraction;
//						i=TCTemp*10;
//	printf("x3.val=-%d\xff\xff\xff",i);
//		printf("热电偶端温度：-%0.2f℃\r\n",TCTemp);
	  }
	  else //否则为正温度，直接取数据计算
	  {int i;
		TCTemp=intpart+fraction; //先将D31符号位去掉，再将D16 D17位去掉，保留热电偶温度数据。
//			i=TCTemp*10;
//	printf("x3.val=%d\xff\xff\xff",i);
//		printf("热电偶端温度：%0.2f℃\r\n",TCTemp);
	  }
	return TCTemp;
}
 
/*参考端采样计算*/
static float ColdJunctionTemp(uint16_t data)
{
  float fraction = 0.0;
//  float temp = 0.0;
  uint16_t intpart = 0;
  
  /*内部温度小数部分: D7 D6 D5 D4*/
//  if(data&0x0080)        fraction += 0.5;
//  if(data&0x0040)        fraction += 0.25;
//  if(data&0x0020)        fraction += 0.125;
//  if(data&0x0010)        fraction += 0.0625;
	
  fraction = ((data&0x00FF) >> 4)*0.0625;  //小数部分
  intpart = (data>>8)&0X7F;	//先将D0到D7位移掉，去符号，取整数部分 
  /*判断正负温度并计算 */	
  if(data&0x8000)	//D15位为符号位，为1表示负温度
  {int i;
	CJTemp=(127-intpart)+(1-fraction);
//		i=CJTemp*10;
//	printf("x3.val=-%d\xff\xff\xff",i);	
//	printf("参考端：-%0.2f℃\r\n",CJTemp);	  
  }
  else	//否则为正温度
  {int i;
	CJTemp=intpart+fraction;
//		i=CJTemp*10;
//	printf("x3.val=%d\xff\xff\xff",i);	
//	printf("参考端：%0.2f℃\r\n",CJTemp);
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
    /*串行时钟频率最大值 5MHz */
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
  uint8_t RXData[4];//32位数据
  
	SPI2_CS_L;
	HAL_Delay(1);
//	HAL_SPI_TransmitReceive (&hspi2,TXData,RXData, 4, 100);	//普通SPI传输接收
//	HAL_SPI_Receive(&hspi2,RXData,4,100);
	HAL_SPI_Receive_DMA(&hspi2,RXData,4);	//如果开DMA传输，在CubeMX里设置时，必须将SPI中DMA的TX和RX都打开
	HAL_Delay(1);
	SPI2_CS_H;
  
 
  *TC = RXData[0]<<8|RXData[1];	//数据整合起来
  *CJ = RXData[2]<<8|RXData[3];
  
//  printf("RXData[0]=%X\r\n",RXData[0]);
//  printf("RXData[1]=%X\r\n",RXData[1]);
//  printf("RXData[2]=%X\r\n",RXData[2]);
//  printf("RXData[3]=%X\r\n",RXData[3]);
//  printf("TC=%X,CJ=%X\r\n",*TC,*CJ);
#endif
}
 
/*得到热电偶和冷量温度。 */
float Max31855GetTemp(void)
{
  uint16_t TC = 0, CJ = 0;
  
  Max31855ReadData(&TC, &CJ);
  
  //当任何SCV（D2）、SCG（D1）或OC（D0）故障激活时，D16的读数为1
  if(TC&0x0001)	//判断是否故障
  {
	TCErrorFlag = true;
  }
  else 
  {
	TCErrorFlag = false;
  }
  
  if(TCErrorFlag==true)		//当出现故障则进行判断
  {
//	printf("D16位为1，热电偶故障\r\n");
	if((CJ&0x0001)==0x0001)
	{
//		printf("OC热电偶开路\r\n");
	}  
	if((CJ&0x0002)==0x0002)
	{
//		printf("SCG热电偶短路到了GND\r\n");
	}
	if((CJ&0x0004)==0x0004)
	{
//		printf("SCV热电偶短路到了VCC\r\n");
	}	  
  }
  else	//无故障，进行数据转换
  {int i;
//	printf("热电偶无故障\r\n\xff\xff\xff");
	TCTemp = ThermalCoupleTemp(TC);
	CJTemp = ColdJunctionTemp(CJ);
  i=CJTemp*10;
	printf("x3.val=%d\xff\xff\xff",i);
	i=TCTemp*10;
	printf("x2.val=%d\xff\xff\xff",i);
  }
	return TCTemp;
}
 
///* 获取热电偶温度。*/
//float Max31855TCTemp()
//{
//  return TCTemp;
//}
 
///*获取冷态温度。*/
//float Max31855_CJTemp()
//{
//  return CJTemp;
//}
/**********END OF FILE****/
