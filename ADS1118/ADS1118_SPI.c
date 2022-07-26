#include "main.h"
#include "ADS1118_SPI.h"
uint16_t Conversion;		// 存储从芯片读取的数据
uint16_t Conversion1;		// 存储从芯片读取的数据
float Voltage;    			// 存储需要显示的测量电压值
float BaseV;						// 采集电压的压基
uint8_t firstflag;			// 第一次进入标志
uint8_t collect;				// 每次采集的数据位置
float DP[8];						// 显示处理后的八通道数据
ConfigDef Config;




/**
  * 函数名称：ReadAds1118
  * 函数功能：通过SPI给Ads1118发数据
  * 形   参：需要配置的adc寄存器数据
  * 返回值 ：采集到的16位数据
  */
uint16_t ReadAds1118(uint16_t Data)
{
	uint8_t i;
	uint16_t returnData = 0x0000;
	HAL_Delay(1);
	SCLK_L;
	DIN_L;
	CS_H;
	CS_L;
	HAL_Delay(1);
	for(i = 0; i < 16;i++)
	{ 
		
		SCLK_H; 
		returnData<<=1;
		if(DOUT == 1)
		returnData |= 0x0001;
		if(Data & 0x8000)
			DIN_H;
		else
			DIN_L;
		Data<<=1;
		SCLK_L;

		

	}
	CS_H;
	SCLK_L;
	DIN_L;
	printf("%d\n",returnData);
	return returnData;
	
}



/**
  * 函数名称：ADS1118Init
  * 函数功能：初始化配置寄存器值
  * 形   参：单次转换，工作模式，传输速率，上拉使能，更新数据
  * 返回值 ：无
  */
void ADS1118Init(uint8_t ss,uint8_t mode ,uint8_t dr,uint8_t pue,uint8_t nop)
{
	Config.ConfigDef_T.SS = ss;						// 设置为无效果
	Config.ConfigDef_T.MODE = mode;				// 设置为连续转换模式
	Config.ConfigDef_T.DR = dr;						// 设置转换速率为128 SPS
	Config.ConfigDef_T.PULL_UP_EN = pue;	// 设置DOUT上拉使能
	Config.ConfigDef_T.NOP = nop;					// 设置有效数据，更新配置寄存器
	Config.ConfigDef_T.CNV_RDY_FL = 0x01;	// 保留位，始终写1

	Conversion = 0;
	Voltage = 0;
	BaseV = 0;
	firstflag = 0;
}

/**
  * 函数名称：GetData
  * 函数功能：通过SPI配置寄存器值并连续采集五次数据求平均值
  * 形   参：通道选择，放大器增益，工作模式
  * 返回值 ：无
  */
float GetData(uint8_t mux,uint8_t pga,uint8_t tsmode)
{
	float FV[20];			// 存储连续的五次转换数据
	float displayData;

	Config.ConfigDef_T.MUX = mux;				// 设置为AIN0和GND
	Config.ConfigDef_T.PGA = pga;				// 设置FSR=±4.096V
	Config.ConfigDef_T.TS_MODE = tsmode;// 设置温度传感器模式为ADC模式

	switch (pga)
	{
		case 0:
			BaseV = 187.5;						// 压基单位为uV
			break;
		case 1:
			BaseV = 125;
			break;
		case 2:
			BaseV = 62.5;
			break;
		case 3:
			BaseV = 31.25;
			break;
		case 4:
			BaseV = 15.625;
			break;
		case 5:
			BaseV = 7.8125;
			break;
	}

		HAL_Delay(1);
		if(DOUT==0||firstflag == 0)						// CS需要周期性拉低来检测是否有新的数据产生(检测INPUT引脚是否有低电平)
		{
			Conversion = ReadAds1118(Config.Bytes);
			Conversion1 = Conversion;
			Voltage = (BaseV*Conversion)/1000000;			// 转换单位：uV→V
			Conversion = 0;														// 数据显示之后清零
			firstflag = 1;
		}

		
		HAL_Delay(20);															// 延迟时间不能低于15ms
	
	displayData = Voltage;
	firstflag = 0;
	switch(mux)
	{
		case ADS1118_MUX_0G:
			DP[0] = displayData;
			break;
		case ADS1118_MUX_1G:
			DP[1] = displayData;
			break;
		case ADS1118_MUX_2G:
			DP[2] = displayData;
			break;
		case ADS1118_MUX_3G:
			DP[3] = displayData;
			break;
	}
	

//	printf("%f\n",DP[0]);
  return DP[0];
} 
