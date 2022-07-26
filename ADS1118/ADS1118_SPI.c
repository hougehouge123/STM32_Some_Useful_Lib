#include "main.h"
#include "ADS1118_SPI.h"
uint16_t Conversion;		// �洢��оƬ��ȡ������
uint16_t Conversion1;		// �洢��оƬ��ȡ������
float Voltage;    			// �洢��Ҫ��ʾ�Ĳ�����ѹֵ
float BaseV;						// �ɼ���ѹ��ѹ��
uint8_t firstflag;			// ��һ�ν����־
uint8_t collect;				// ÿ�βɼ�������λ��
float DP[8];						// ��ʾ�����İ�ͨ������
ConfigDef Config;




/**
  * �������ƣ�ReadAds1118
  * �������ܣ�ͨ��SPI��Ads1118������
  * ��   �Σ���Ҫ���õ�adc�Ĵ�������
  * ����ֵ ���ɼ�����16λ����
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
  * �������ƣ�ADS1118Init
  * �������ܣ���ʼ�����üĴ���ֵ
  * ��   �Σ�����ת��������ģʽ���������ʣ�����ʹ�ܣ���������
  * ����ֵ ����
  */
void ADS1118Init(uint8_t ss,uint8_t mode ,uint8_t dr,uint8_t pue,uint8_t nop)
{
	Config.ConfigDef_T.SS = ss;						// ����Ϊ��Ч��
	Config.ConfigDef_T.MODE = mode;				// ����Ϊ����ת��ģʽ
	Config.ConfigDef_T.DR = dr;						// ����ת������Ϊ128 SPS
	Config.ConfigDef_T.PULL_UP_EN = pue;	// ����DOUT����ʹ��
	Config.ConfigDef_T.NOP = nop;					// ������Ч���ݣ��������üĴ���
	Config.ConfigDef_T.CNV_RDY_FL = 0x01;	// ����λ��ʼ��д1

	Conversion = 0;
	Voltage = 0;
	BaseV = 0;
	firstflag = 0;
}

/**
  * �������ƣ�GetData
  * �������ܣ�ͨ��SPI���üĴ���ֵ�������ɼ����������ƽ��ֵ
  * ��   �Σ�ͨ��ѡ�񣬷Ŵ������棬����ģʽ
  * ����ֵ ����
  */
float GetData(uint8_t mux,uint8_t pga,uint8_t tsmode)
{
	float FV[20];			// �洢���������ת������
	float displayData;

	Config.ConfigDef_T.MUX = mux;				// ����ΪAIN0��GND
	Config.ConfigDef_T.PGA = pga;				// ����FSR=��4.096V
	Config.ConfigDef_T.TS_MODE = tsmode;// �����¶ȴ�����ģʽΪADCģʽ

	switch (pga)
	{
		case 0:
			BaseV = 187.5;						// ѹ����λΪuV
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
		if(DOUT==0||firstflag == 0)						// CS��Ҫ����������������Ƿ����µ����ݲ���(���INPUT�����Ƿ��е͵�ƽ)
		{
			Conversion = ReadAds1118(Config.Bytes);
			Conversion1 = Conversion;
			Voltage = (BaseV*Conversion)/1000000;			// ת����λ��uV��V
			Conversion = 0;														// ������ʾ֮������
			firstflag = 1;
		}

		
		HAL_Delay(20);															// �ӳ�ʱ�䲻�ܵ���15ms
	
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
