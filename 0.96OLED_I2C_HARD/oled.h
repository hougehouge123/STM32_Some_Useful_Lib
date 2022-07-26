
#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "oledfront.h"


//操作指令写入		
static void OLED_Write_Cmd(uint8_t cmd)
{
	uint8_t buf[2];
	buf[0] = 0x00;	//control byte
	buf[1] = cmd;
	
	//使用HAL库的API实现
	HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 1000);
}
/**
 * @brief	OLED写入数据
 * @param cmd - 待写入数据
 * @note	移植时，请使用自己的底层API实现 
*/

//具体数据写入
static void OLED_Write_Dat(uint8_t dat)
{
	uint8_t buf[2];
	buf[0] = 0x40; //control byte
	buf[1] = dat;
	
	//使用HAL库的API实现
	HAL_I2C_Master_Transmit(&hi2c1, 0x78, buf, 2, 1000);
}

/**
 * @brief	OLED设置显示位置
 * @param x - X方向位置
 * @param y - Y方向位置
*/
void OLED_Set_Pos(uint8_t x, uint8_t y)
{ 	
	OLED_Write_Cmd(0xb0+y);
	OLED_Write_Cmd(((x&0xf0)>>4)|0x10);
	OLED_Write_Cmd((x&0x0f)|0x01);
}   	  
/**
 * @brief	OLED开启显示
*/ 
void OLED_Display_On(void)
{
	OLED_Write_Cmd(0X8D);  //SET DCDC命令
	OLED_Write_Cmd(0X14);  //DCDC ON
	OLED_Write_Cmd(0XAF);  //DISPLAY ON
}
/**
 * @brief	OLED关闭显示
*/   
void OLED_Display_Off(void)
{
	OLED_Write_Cmd(0X8D);  //SET DCDC命令
	OLED_Write_Cmd(0X10);  //DCDC OFF
	OLED_Write_Cmd(0XAE);  //DISPLAY OFF
}		   			 
/**
 * @brief	OLED清屏函数（清屏之后屏幕全为黑色）
*/ 
void OLED_Clear(void)
{  
	uint8_t i,n;		    
	for(i=0;i<8;i++)  
	{  
		OLED_Write_Cmd(0xb0+i);    //设置页地址（0~7）
		OLED_Write_Cmd(0x00);      //设置显示位置—列低地址
		OLED_Write_Cmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
		{
			OLED_Write_Dat(0);
		}			
	}
}
/**
 * @brief	OLED显示全开（所有像素点全亮）
*/
void OLED_On(void)
{  
	uint8_t i,n;
	for(i=0;i<8;i++)
	{  
		OLED_Write_Cmd(0xb0+i);    //设置页地址（0~7）
		OLED_Write_Cmd(0x00);      //设置显示位置—列低地址
		OLED_Write_Cmd(0x10);      //设置显示位置—列高地址   
		for(n=0;n<128;n++)
		{
			OLED_Write_Dat(1);
		}			
	}
}
/**
 * @brief	在指定位置显示一个ASCII字符
 * @param x - 0 - 127
 * @param y - 0 - 7
 * @param chr  - 待显示的ASCII字符
 * @param size - ASCII字符大小
 * 				字符大小有12(6*8)/16(8*16)两种大小
*/
void OLED_Init(void)
{ 	

	HAL_Delay(500);

	OLED_Write_Cmd(0xAE);//--display off
	OLED_Write_Cmd(0x00);//---set low column address
	OLED_Write_Cmd(0x10);//---set high column address
	OLED_Write_Cmd(0x40);//--set start line address  
	OLED_Write_Cmd(0x81); // contract control
	OLED_Write_Cmd(0xFF);//--128   
	OLED_Write_Cmd(0xA1);//set segment remap 
	OLED_Write_Cmd(0xC8);//Com scan direction
	OLED_Write_Cmd(0xA6);//--normal / reverse
	OLED_Write_Cmd(0xA8);//--set multiplex ratio(1 to 64)
	OLED_Write_Cmd(0x3F);//--1/32 duty
	OLED_Write_Cmd(0xD3);//-set display offset
	OLED_Write_Cmd(0x00);//
	OLED_Write_Cmd(0xD5);//set osc division
	OLED_Write_Cmd(0x80);
	OLED_Write_Cmd(0xD9);//Set Pre-Charge Period
	OLED_Write_Cmd(0xF1);//
	OLED_Write_Cmd(0xDA);//set com pin configuartion
	OLED_Write_Cmd(0x12);//
	OLED_Write_Cmd(0xDB);//set Vcomh
	OLED_Write_Cmd(0x40);//
	OLED_Write_Cmd(0x20);
	OLED_Write_Cmd(0x02);
	OLED_Write_Cmd(0x8D);//set charge pump enable
	OLED_Write_Cmd(0x14);//
	OLED_Write_Cmd(0xA4);
	OLED_Write_Cmd(0xA6);
	OLED_Write_Cmd(0xAF);//--turn on oled panel
	
	OLED_Clear();
	OLED_Set_Pos(0,0);
}

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size)
{   
	uint8_t c=0,i=0;
	
	c = chr-' ';	
	if(x > 128-1)
	{
		x=0;
		y++;
	}
	
	if(size ==16)
	{
		OLED_Set_Pos(x,y);	
		for(i=0;i<8;i++)
		{
			OLED_Write_Dat(F8X16[c*16+i]);
		}
		OLED_Set_Pos(x,y+1);
		for(i=0;i<8;i++)
		{
			OLED_Write_Dat(F8X16[c*16+i+8]);
		}
	}
	else
	{	
		OLED_Set_Pos(x,y);
		for(i=0;i<6;i++)
		{
			OLED_Write_Dat(F6x8[c][i]);
		}
	}
}


//               绘波专用

uint8_t DataBuffer[8][128];
uint8_t Bef[3];//保存前一个数据的几个参数1.要写在第几页2.0x01要移动几位3.写什么数据
uint8_t Cur[3];//当前前一个数据1.要写在第几页2.0x01要移动几位3.写什么数据

void Before_State_Update(uint8_t y)//根据y的值，求出前一个数据的有关参数
{
	Bef[0] = 7 - y / 8;
	Bef[1] = 7 - y % 8;
	Bef[2] = 1 << Bef[1];
}
void Current_State_Update(uint8_t y)//根据Y值，求出当前数据的有关参数
{
	Cur[0] = 7 - y / 8;//数据写在第几页
	Cur[1] = 7 - y % 8;//0x01要移动的位数
	Cur[2] = 1 << Cur[1];//要写什么数据
}

void OLED_DrawWave(uint8_t x, uint8_t y)
{

	int8_t page_sub;
	uint8_t page_buff, i, j;
	Current_State_Update(y);//根据Y值，求出当前数据的有关参数
	page_sub = Bef[0] - Cur[0];//当前值与前一个值的页数相比较
	//确定当前列，每一页应该写什么数据
	if (page_sub > 0)
	{
		page_buff = Bef[0];
		OLED_Set_Pos(page_buff, x);
		OLED_Write_Dat(Bef[2] - 0x01);
		page_buff--;
		for (i = 0; i < page_sub - 1; i++)
		{
			OLED_Set_Pos(page_buff, x);
			OLED_Write_Dat(0xff);
			page_buff--;
		}
		OLED_Set_Pos(page_buff, x);
		OLED_Write_Dat(0xff << Cur[1]);
	}
	else if (page_sub == 0)
	{
		if (Cur[1] == Bef[1])
		{
			OLED_Set_Pos(Cur[0], x);
			OLED_Write_Dat(Cur[2]);
		}
		else if (Cur[1] > Bef[1])
		{
			OLED_Set_Pos(Cur[0], x);
			OLED_Write_Dat((Cur[2] - Bef[2]) | Cur[2]);
		}
		else if (Cur[1] < Bef[1])
		{
			OLED_Set_Pos(Cur[0], x);
			OLED_Write_Dat(Bef[2] - Cur[2]);
		}
	}
	else if (page_sub < 0)
	{
		page_buff = Cur[0];
		OLED_Set_Pos(page_buff, x);
		OLED_Write_Dat((Cur[2] << 1) - 0x01);
		page_buff--;
		for (i = 0; i < 0 - page_sub - 1; i++)
		{
			OLED_Set_Pos(page_buff, x);
			OLED_Write_Dat(0xff);
			page_buff--;
		}
		OLED_Set_Pos(page_buff, x);
		OLED_Write_Dat(0xff << (Bef[1] + 1));
	}
	Before_State_Update(y);
	//把下一列，每一页的数据清除掉
	for (i = 0; i < 8; i++)
	{
		OLED_Set_Pos(i, x + 1);
		for (j = 0; j < 1; j++)
			OLED_Write_Dat(0x00);
	}
}


	 
#endif