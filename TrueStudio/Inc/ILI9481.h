#ifndef  ILI9481_H
#define ILI9481_H

#include <string.h>

#include "main.h"
#include "fatfs.h"
#include "font.h"
#include "pins.h"

#include "wav.h"


int LCD_WIDTH = 320;
int LCD_HEIGHT = 480;

#define LCD_CMD_SET_COL_ADDR        0x2A
#define LCD_CMD_SET_PAGE_ADDR       0x2B
#define LCD_CMD_WRITE_MEM_START     0x2C
#define LCD_CMD_DISPLAY_ON          0x29
#define LCD_CMD_DISPLAY_OFF         0x28
#define LCD_CMD_SET_ADDRESS_MODE    0x36

#define BLACK 		0x0000
#define BLUE 			0x001F
#define RED 			0xF800
#define GREEN 		0x07E0
#define CYAN 			0x07FF
#define MAGENTA		0xF81F
#define YELLOW 		0xFFE0
#define WHITE 		0xFFFF

#define LEFT	0
#define CENTER	1
#define RIGHT	2

#define TOP		0
#define BOTTOM	2




#define DATA_PORT     GPIOE //FOR GPIOA:
//  D0  |  D1  |  D2  |  D3  |  D4  |  D5  |  D6  |  D7
//  PA0 |  PA1 |  PA2 |  PA3 |  PA4 |  PA5 |  PA6 |  PA7

#define CONTROL_PORT  GPIOC
//ALL CONTROL_PORT PINS NEED TO BE OF THIS PORT
//ANY PIN FROM CONTROL_PORT

#define LCD_RD    0
#define LCD_WR    1
#define LCD_RS    2
#define LCD_CS    3
#define LCD_RST   13




const int TFT_RD_MASK_HIGH = 1 << LCD_RD;
const int TFT_WR_MASK_HIGH = 1 << LCD_WR;
const int TFT_RS_MASK_HIGH = 1 << LCD_RS;
const int TFT_CS_MASK_HIGH = 1 << LCD_CS;
const int TFT_RST_MASK_HIGH = 1 << LCD_RST;

const int TFT_RD_MASK_LOW = 1 << (LCD_RD + 16);//LOW low
const int TFT_WR_MASK_LOW = 1 << (LCD_WR + 16);
const int TFT_RS_MASK_LOW = 1 << (LCD_RS + 16);
const int TFT_CS_MASK_LOW = 1 << (LCD_CS + 16);
const int TFT_RST_MASK_LOW = 1 << (LCD_RST + 16);

#define RD_LOW      CONTROL_PORT->BSRR = TFT_RD_MASK_LOW
#define RD_HIGH     CONTROL_PORT->BSRR = TFT_RD_MASK_HIGH
#define WR_LOW      CONTROL_PORT->BSRR = TFT_WR_MASK_LOW
#define WR_HIGH     CONTROL_PORT->BSRR = TFT_WR_MASK_HIGH
#define RS_LOW      CONTROL_PORT->BSRR = TFT_RS_MASK_LOW
#define RS_HIGH     CONTROL_PORT->BSRR = TFT_RS_MASK_HIGH
#define CS_LOW      CONTROL_PORT->BSRR = TFT_CS_MASK_LOW
#define CS_HIGH     CONTROL_PORT->BSRR = TFT_CS_MASK_HIGH
#define RST_LOW     CONTROL_PORT->BSRR = TFT_RST_MASK_LOW
#define RST_HIGH    CONTROL_PORT->BSRR = TFT_RST_MASK_HIGH




void begin(void);
void setDataOutputDirection(void);
//void setDataInputDirection();
void writeToBus(uint16_t d);
void writeCmdData(int c, int d);
void writeCmd(int d);
void writeData(int d);
void setAddress(int x1, int y1, int x2, int y2);
void fillScreen(int c);
void fillRect(int x, int y, int w, int h, int c);
void drawRect(int x, int y, int w, int h, int c);
void drawPixel(int x, int y, int c);
void drawLine(int x0, int y0, int x1, int y1, int color);
void setRotation(int degrees);
void Lcd_Set_Scrolling_Area(int top, int height);
void Lcd_Start_Scrolling(int line);
void drawCircle(int x0, int y0, int r, int color);
void fillCircle(int x0, int y0, int r, int color);
void drawRoundRect(int x, int y, int w, int h, int r, int color);
void fillRoundRect(int x, int y, int w, int h, int r, int color);
void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);
void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color);

void setTextColor(int Color);
void setTextSize(int size);
void print(char * txt);
void textAlign(int h, int v);
void text(const char * txt, int x, int y);
void setCursor(int x, int y);
	
void Hex_To_RGB(unsigned short hexValue, unsigned char *r, unsigned char *g, unsigned char *b);
uint16_t color565(unsigned char r, unsigned char g, unsigned char b);

void drawHorizontalLine(int x, int y, int l, int c);
void drawVerticalLine(int x, int y, int h, int c);
void writeLine(int x0, int y0, int x1, int y1, int color);
void fillCircleHelper(int x0, int y0, int r, unsigned char cornername, int delta, int color);
void drawCircleHelper( int x0, int y0, int r, unsigned char cornername, int color);
void drawChar(int x, int y, int Char, int color, int Size);
int isKthBitSet(int n, int k);
int constrain(int var, int MIN, int MAX);
uint16_t color565(unsigned char r, unsigned char g, unsigned char b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3);
}


void begin(void)
{
  //__HAL_RCC_GPIOC_CLK_ENABLE();
  //__HAL_RCC_GPIOE_CLK_ENABLE();
	
  setDataOutputDirection();
  RD_HIGH;
  WR_HIGH;
  RS_HIGH;
  CS_HIGH;
  RST_HIGH;

  /**
     Bring up the LCD
  */
  RST_HIGH;
  RST_LOW;
  RST_HIGH;

  CS_HIGH;
  WR_HIGH;
  CS_LOW;

  writeCmd(0x11); // exit sleep mode

  writeCmd(0xD0); // power settings
  writeData(0x07);
  writeData(0x42);
  writeData(0x18);

  writeCmd(0xD1); // vcom control
  writeData(0x00);
  writeData(0x07);
  writeData(0x10);

  writeCmd(0xD2); // power settings for normal mode
  writeData(0x01);
  writeData(0x02);

  writeCmd(0xC0); // panel driving setting
  writeData(0x10);
  writeData(0x3B);
  writeData(0x00);
  writeData(0x02);
  writeData(0x11);

  writeCmd(0xC5); // frame rate & inversion control
  writeData(0x03); // 72Hz
  //writeData(0x02); // 85Hz, default
	
	
	writeCmd(0xC8);//gamma correct 0,0xf3,0,0xbc,0x50,0x1f,0,7,0x7f,0x7,0xf,0
	writeData(0x00);
	writeData(0xf3);
	writeData(0x00);
	writeData(0xbc);
	writeData(0x50);
	writeData(0x1f);
	writeData(0x00);
	writeData(0x07);
	writeData(0x7f);
	writeData(0x07);
	writeData(0x0f);
	writeData(0x00);

  writeCmd(LCD_CMD_SET_ADDRESS_MODE); // set address mode
  writeData(0x0A); // page-address-order | page/column-selection |  horizontal flip

  writeCmd(0x3A); // set pixel format
  writeData(0x55);

  writeCmd(LCD_CMD_SET_COL_ADDR);
  writeData(0x00);
  writeData(0x00);
  writeData(0x01);
  writeData(0x3F);

  writeCmd(LCD_CMD_SET_PAGE_ADDR);
  writeData(0x00);
  writeData(0x00);
  writeData(0x01);
  writeData(0xE0);

  writeCmd(LCD_CMD_DISPLAY_ON); // set display on
	
	writeCmd(0x21);//invert display
	//writeData(0x0);
}
void setDataOutputDirection(void)
{
  //DATA_PORT-> MODER |= (1 << 0);
  //DATA_PORT-> MODER |= (1 << 2);
  //DATA_PORT-> MODER |= (1 << 4);
  //DATA_PORT-> MODER |= (1 << 6);
  //DATA_PORT-> MODER |= (1 << 8);
  //DATA_PORT-> MODER |= (1 << 10);
  //DATA_PORT-> MODER |= (1 << 12);
  //DATA_PORT-> MODER |= (1 << 14);
	
  //pinMode(PA0, OUTPUT);
  //pinMode(PA1, OUTPUT);
  //pinMode(PA2, OUTPUT);
  //pinMode(PA3, OUTPUT);
  //pinMode(PA4, OUTPUT);
  //pinMode(PA5, OUTPUT);
  //pinMode(PA6, OUTPUT);
  //pinMode(PA7, OUTPUT);

  //CONTROL_PORT-> MODER |= (1 << LCD_RD * 2);
  //CONTROL_PORT-> MODER |= (1 << LCD_WR * 2);
  //CONTROL_PORT-> MODER |= (1 << LCD_RS * 2);
  //CONTROL_PORT-> MODER |= (1 << LCD_CS * 2);
  //CONTROL_PORT-> MODER |= (1 << LCD_RST * 2);
	DATA_PORT-> MODER = 0x55555555;
	//CONTROL_PORT-> MODER |= 0x155;
	
	pinMode(LCD_RD, CONTROL_PORT, OUTPUT);
	pinMode(LCD_WR, CONTROL_PORT, OUTPUT);
	pinMode(LCD_RS, CONTROL_PORT, OUTPUT);
	pinMode(LCD_CS, CONTROL_PORT, OUTPUT);
	pinMode(LCD_RST, CONTROL_PORT, OUTPUT);
	
}
void writeToBus(uint16_t d)
{
  WR_HIGH;

  //DATA_PORT->BSRR = ((~d)<<16) | (d);//((~d)<<16) | (d);
	DATA_PORT->ODR = d;

  WR_LOW;

	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles
	__asm__("nop\n\t"); // kill 2 cpu cycles

  WR_HIGH;
}

void writeCmdData(int c, int d) {//not used
  writeCmd(c);
  writeData(d);
}
void writeCmd(int d)
{
  //digitalWrite(LCD_RS, LOW);
  //GPIOE->BSRR = (1<<25);
  RS_LOW;
  writeToBus(d);
}

void writeData(int d)
{
  RS_HIGH;
  //GPIOE->BSRR = (1<<9);
  writeToBus(d);
}


void setAddress(int x1, int y1, int x2, int y2)
{
  writeCmd(LCD_CMD_SET_COL_ADDR);
  writeData(x1 >> 8);
  writeData(x1);
  writeData(x2 >> 8);
  writeData(x2);

  writeCmd(LCD_CMD_SET_PAGE_ADDR);
  writeData(y1 >> 8);
  writeData(y1);
  writeData(y2 >> 8);
  writeData(y2);

  writeCmd(LCD_CMD_WRITE_MEM_START);
}

void fillScreen(int c)
{
  fillRect(0, 0, LCD_WIDTH, LCD_HEIGHT, c);
}

void fillRect(int x, int y, int w, int h, int c) {
  int i, k;

  //digitalWrite(LCD_CS, LOW);
  CS_LOW;

  setAddress(x, y, constrain(x + w - 1, 0, LCD_WIDTH - 1), constrain(y + h - 1, 0, LCD_HEIGHT - 1));

  //digitalWrite(LCD_RS, HIGH);
  RS_HIGH;
  WR_HIGH;
  //int c1 = c >> 8;

  for (i = 0; i < h; i++)
  {
    for (k = 0; k < w; k++)
    {
      //writeData(c1);
      //writeData(c);
			
			DATA_PORT->ODR = c;

			WR_LOW;

			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles
			__asm__("nop\n\t"); // kill 2 cpu cycles

			WR_HIGH;
    }
  }

  //digitalWrite(LCD_CS, HIGH);
  CS_HIGH;
}

FATFS fs;  // file system
//FIL fil; // File
//FILINFO fno;
//FRESULT fresult;  // result
//UINT br, bw;  // File read/write count

#define BUFFER_LEN		4096
int pixelColor = BLACK;

int drawImage(int x, int y, char * filename) {
	FIL fil; // File
	
	unsigned char BUFFER[BUFFER_LEN];
	int preTime = HAL_GetTick();
	SDState = BUSY;
	if (f_open(&fil, filename, FA_READ) != FR_OK) {
		return 0;
	}
	//int aftTime = HAL_GetTick();
	SDState = IDLE;

	UINT d;
	SDState = BUSY;
	if (f_read(&fil, BUFFER, 4, &d) != FR_OK) {
		SDState = IDLE;
		return 0;
	}
	SDState = IDLE;

	int w = (BUFFER[0] << 8) + BUFFER[1];
	int h = (BUFFER[2] << 8) + BUFFER[3];
	
	h = constrain(h, 0, LCD_HEIGHT - y);

  CS_LOW;
  setAddress(x, y, x + w - 1, y + h - 1);
  RS_HIGH;


	int count = 0;
  for (int i = 0; i < w * h; i += BUFFER_LEN / 2)
  {
  	SDState = BUSY;
		if (f_read(&fil, BUFFER, BUFFER_LEN, &d) != FR_OK) {
			SDState = IDLE;
			return 0;
		}
		SDState = IDLE;
		if (interuptFlagWaiting == true && wavState == BUSY) {//complete interrupt operation if needed
			HAL_Delay(1);//wait to let SD read interrupt execute
		}
		pixelColor = (BUFFER[0] << 8) + BUFFER[1];
		count += BUFFER_LEN / 2;

		if (count > w * h) {
			for (int j = 0; j < w * h - i; j++) {
				  WR_HIGH;

					//DATA_PORT->BSRR = ((~d)<<16) | (d);//((~d)<<16) | (d);
					DATA_PORT->ODR = (BUFFER[j * 2] << 8) + BUFFER[j * 2 + 1];

					WR_LOW;

					for (int i = 0; i < 3; i++) {
						__asm__("nop\n\t"); // kill 2 cpu cycles
					}
					WR_HIGH;
			}
		}
		else {
			for (int j = 0; j < BUFFER_LEN / 2; j++) {

					//DATA_PORT->BSRR = ((~d)<<16) | (d);//((~d)<<16) | (d);
					DATA_PORT->ODR = (BUFFER[j * 2] << 8) + BUFFER[j * 2 + 1];

					WR_LOW;

					for (int i = 0; i < 3; i++) {
						__asm__("nop\n\t"); // kill 2 cpu cycles
					}
					WR_HIGH;
			}
		}
  }

  CS_HIGH;
  SDState = BUSY;
	if (f_close(&fil) != FR_OK) {
		Error_Handler();
	}
	SDState = IDLE;
	return HAL_GetTick() - preTime;
}

void drawPixel(int x, int y, int c)
{

  //digitalWrite(LCD_CS, LOW);
  CS_LOW;

  setAddress(x, y, x, y);
  //writeData(c >> 8);
  writeData(c);

  //digitalWrite(LCD_CS, HIGH);
  CS_HIGH;
}
void drawHorizontalLine(int x, int y, int l, int c)
{
  int i;

  //digitalWrite(LCD_CS, LOW);
  CS_LOW;

  setAddress(x, y, x + l, y);

  for (i = 0; i < l; ++i)
  {
    //writeData(c >> 8);
    writeData(c);
  }

  //digitalWrite(LCD_CS, HIGH);
  CS_HIGH;
}


void drawVerticalLine(int x, int y, int h, int c)
{
  int i;

  //digitalWrite(LCD_CS, LOW);
  CS_LOW;

  setAddress(x, y, x, y + h);

  for (i = 0; i < h; ++i)
  {
    //writeData(c >> 8);
    writeData(c);
  }

  //digitalWrite(LCD_CS, HIGH);
  CS_HIGH;
}
#define _swap_int16_t(a, b) { int t = a; a = b; b = t; }

void writeLine(int x0, int y0, int x1, int y1, int color) {
  int steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    _swap_int16_t(x0, y0);
    _swap_int16_t(x1, y1);
  }

  if (x0 > x1) {
    _swap_int16_t(x0, x1);
    _swap_int16_t(y0, y1);
  }

  int dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int err = dx / 2;
  int ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      drawPixel(y0, x0, color);
    } else {
      drawPixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

void drawLine(int x0, int y0, int x1, int y1, int color) {

  if (x0 == x1) {
    if (y0 > y1) _swap_int16_t(y0, y1);
    drawVerticalLine(x0, y0, y1 - y0 + 1, color);
  } else if (y0 == y1) {
    if (x0 > x1) _swap_int16_t(x0, x1);
    drawHorizontalLine(x0, y0, x1 - x0 + 1, color);
  } else {
    writeLine(x0, y0, x1, y1, color);
  }
}


void drawRect(int x, int y, int w, int h, int c)
{
  x = constrain(x, 0, LCD_WIDTH);
  y = constrain(y, 0, LCD_HEIGHT);
  w = constrain(w - 1, 0, LCD_WIDTH - 1);
  h = constrain(h - 1, 0, LCD_HEIGHT - 1);
  drawHorizontalLine(x, y, w + 1, c);
  drawHorizontalLine(x, y + h, w + 1, c);
  drawVerticalLine(x, y, h, c);
  drawVerticalLine(x + w, y, h, c);
}

void Hex_To_RGB(unsigned short hexValue, unsigned char *r, unsigned char *g, unsigned char *b)
{

  unsigned char red = ((hexValue >> 11) & 0x1F);  // Extract the 5 R bits
  unsigned char green = ((hexValue >> 5) & 0x3F);   // Extract the 6 G bits
  unsigned char blue = ((hexValue) & 0x1F);        // Extract the 5 B bits

  (*r) = ((red * 255) / 31);
  (*g) = ((green * 255) / 63);
  (*b) = ((blue * 255) / 31);
}

int _Rotation = 0;
void setRotation(int degrees)
{
  unsigned char cfg = 0xA;
  LCD_WIDTH = 320;
  LCD_HEIGHT = 480;
	
	_Rotation = degrees;

  switch (degrees) {
    case 1:
      cfg = 0x28;
      LCD_WIDTH = 480;
      LCD_HEIGHT = 320;
      break;
    case 2:
      cfg = 0x9;
      LCD_WIDTH = 320;
      LCD_HEIGHT = 480;
      break;
    case 3:
      cfg = 0x2B;
      LCD_WIDTH = 480;
      LCD_HEIGHT = 320;
      break;
  }
	
  CS_LOW;
  writeCmd(LCD_CMD_SET_ADDRESS_MODE);
  writeData(cfg);
  CS_HIGH;
}




void Lcd_Set_Scrolling_Area(int top, int height)
{
  CS_LOW;
  writeCmd(0x33);
  //writeData(top >> 8);
  writeData(top);
  //writeData(height >> 8);
  writeData(height);
  //writeData((top + height) >> 8);
  writeData(top + height);
  CS_HIGH;
}
void Lcd_Start_Scrolling(int line)
{
  CS_LOW;
  writeCmd(0x37);
  //writeData(line >> 8);
  writeData(line);
  CS_HIGH;
}

void drawCircle(int x0, int y0, int r, int color) {
  int f = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x = 0;
  int y = r;


  drawPixel(x0  , y0 + r, color);
  drawPixel(x0  , y0 - r, color);
  drawPixel(x0 + r, y0  , color);
  drawPixel(x0 - r, y0  , color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    drawPixel(x0 + x, y0 + y, color);
    drawPixel(x0 - x, y0 + y, color);
    drawPixel(x0 + x, y0 - y, color);
    drawPixel(x0 - x, y0 - y, color);
    drawPixel(x0 + y, y0 + x, color);
    drawPixel(x0 - y, y0 + x, color);
    drawPixel(x0 + y, y0 - x, color);
    drawPixel(x0 - y, y0 - x, color);
  }

}
void fillCircle(int x0, int y0, int r, int color) {
  drawVerticalLine(x0, y0 - r, 2 * r + 1, color);
  fillCircleHelper(x0, y0, r, 3, 0, color);
}

// Used to do circles and roundrects
void fillCircleHelper(int x0, int y0, int r, unsigned char cornername, int delta, int color) {

  int f     = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x     = 0;
  int y     = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      drawVerticalLine(x0 + x, y0 - y, 2 * y + 1 + delta, color);
      drawVerticalLine(x0 + y, y0 - x, 2 * x + 1 + delta, color);
    }
    if (cornername & 0x2) {
      drawVerticalLine(x0 - x, y0 - y, 2 * y + 1 + delta, color);
      drawVerticalLine(x0 - y, y0 - x, 2 * x + 1 + delta, color);
    }
  }
}
void drawCircleHelper( int x0, int y0, int r, unsigned char cornername, int color) {
  int f     = 1 - r;
  int ddF_x = 1;
  int ddF_y = -2 * r;
  int x     = 0;
  int y     = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;
    if (cornername & 0x4) {
      drawPixel(x0 + x, y0 + y, color);
      drawPixel(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      drawPixel(x0 + x, y0 - y, color);
      drawPixel(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      drawPixel(x0 - y, y0 + x, color);
      drawPixel(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      drawPixel(x0 - y, y0 - x, color);
      drawPixel(x0 - x, y0 - y, color);
    }
  }
}
void drawRoundRect(int x, int y, int w, int h, int r, int color) {

  drawHorizontalLine(x + r  , y    , w - 2 * r, color); // Top
  drawHorizontalLine(x + r  , y + h - 1, w - 2 * r, color); // Bottom
  drawVerticalLine(x    , y + r  , h - 2 * r, color); // Left
  drawVerticalLine(x + w - 1, y + r  , h - 2 * r, color); // Right

  drawCircleHelper(x + r    , y + r    , r, 1, color);
  drawCircleHelper(x + w - r - 1, y + r    , r, 2, color);
  drawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(x + r    , y + h - r - 1, r, 8, color);

}

void fillRoundRect(int x, int y, int w, int h, int r, int color) {


  fillRect(x + r, y, w - 2 * r, h, color);

  fillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
  fillCircleHelper(x + r    , y + r, r, 2, h - 2 * r - 1, color);
}

void drawTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color) {
  drawLine(x0, y0, x1, y1, color);
  drawLine(x1, y1, x2, y2, color);
  drawLine(x2, y2, x0, y0, color);
}
void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, int color) {

  int a, b, y, last;
  if (y0 > y1) {
    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
  }
  if (y1 > y2) {
    _swap_int16_t(y2, y1); _swap_int16_t(x2, x1);
  }
  if (y0 > y1) {
    _swap_int16_t(y0, y1); _swap_int16_t(x0, x1);
  }

  if (y0 == y2) {
    a = b = x0;
    if (x1 < a)      a = x1;
    else if (x1 > b) b = x1;
    if (x2 < a)      a = x2;
    else if (x2 > b) b = x2;
    drawHorizontalLine(a, y0, b - a + 1, color);
    return;
  }

  int
  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1;
  int
  sa   = 0,
  sb   = 0;

  if (y1 == y2) last = y1;
  else         last = y1 - 1;

  for (y = y0; y <= last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b) _swap_int16_t(a, b);
    drawHorizontalLine(a, y, b - a + 1, color);
  }

  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b) _swap_int16_t(a, b);
    drawHorizontalLine(a, y, b - a + 1, color);
  }
}


uint32_t isKthBitSetInt(uint32_t n, uint8_t k) 
{ 
  if (n & (1 << (k - 1))) {
    return true;
  }
  else {
    return false;
  }
}

int textCol = 0xFFFF;
int textSize = 1;

int cursorX = 0;
int cursorY = 0;

float alignHor = LEFT;
float alignVert = TOP;

void setTextColor(int Color) {
 textCol = Color;
}
void setTextSize(int size) {
 textSize = size;
}
void textAlign(int h, int v) {
	alignHor = h;
	alignVert = v;
}
int isKthBitSet(int n, int k)
{
 if (n & (1 << (k - 1))) {
   return true;
 }
 else {
   return false;
 }
}
void drawChar(int x, int y, int Char, int color, int Size) {
	if (Char == ' ') {
		return;
	}
	int c = (int)Char;
	c = c - 32;
	for (int j = 0; j < 5; j++) {  // x advance
		unsigned char Line = font[c * 5 + j];
		for (int i = 1; i < 10; i++) {  // y advance
			if (isKthBitSet(Line, i)) {
				if (Size == 1) {
					drawPixel(x + j, y + i, color);
				}
				if (Size > 1) {
					fillRect(x + (j * Size), y + (i * Size), Size, Size, color);
				}
			}
		}
	}
}

void print(char * txt) {
 int i = 0;
 float offset = strlen(txt) * 6 * textSize * (alignHor / -2);
 cursorX += offset;
 cursorY += (alignVert / -2) * 10;
 while (*txt) {
   drawChar(cursorX + i * 6 * textSize, cursorY, *txt++, textCol, textSize);
   i++;
 }
 cursorX = cursorX + i * 6 * textSize;
}

void text(const char * txt, int x, int y) {
 int i = 0;
 float offset = strlen(txt) * 6 * textSize * (alignHor / -2);
 x += offset;
 y += (alignVert / -2) * (10 * textSize);
 while (*txt) {
   drawChar(x + i * 6 * textSize, y, *txt++, textCol, textSize);
   i++;
 }
}
void setCursor(int x, int y) {
  cursorX = x;
  cursorY = y;
}

int constrain(int var, int MIN, int MAX) {
  if (var < MIN) {
    var = MIN;
  }
  if (var > MAX) {
    var = MAX;
  }
  return var;
}

int CursorY = 5;
char strINT[50];
char * INT_TO_STR(long i) {
	sprintf(strINT, "%d", (int)i);
	return strINT;
}

void LOG(char * str) {
	text(str, 5, CursorY);
	CursorY += 15;
}

#endif
