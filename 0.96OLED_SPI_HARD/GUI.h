#ifndef __GUI_H
#define __GUI_H
#include "stm32f1xx_hal.h"

u32 oled_pow(u8 m,u8 n);
void OLED_ShowChar(u8 x,u8 y,u8 chr,uint8_t mode);
void  GUI_DrawFont16(u16 x, u16 y, uint8_t *s,uint8_t mode);
void Show_Str(u16 x, u16 y,uint8_t *str,uint8_t size,uint8_t mode);

void OLED_DrawBMP(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);
void Disp_Title(void);

#endif
