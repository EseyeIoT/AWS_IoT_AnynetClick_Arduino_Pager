#include <SPI.h>
#include "oled.h"
 
// chipKIT UNO32, socket #1
#define OLED_CS         10
#define OLED_DC         6
#define OLED_RST        A3
 
void oled_init() {
  // put your setup code here, to run once:
  SPI.begin();
 
  // Set OLED pins
  pinMode(OLED_CS, OUTPUT);
  pinMode(OLED_DC, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  OLED_Initialize();
  delay(100);
  OLED_Clear();
  OLED_SetScale(1, 2);
  OLED_Putchar('E');
  OLED_Putchar('S');
  OLED_Putchar('E');
  OLED_Putchar('Y');
  OLED_Putchar('E');
} 
 
// OLED W functions
 
void OLED_Command(uint8_t temp){
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(OLED_CS,LOW);
  digitalWrite(OLED_DC,LOW);
  SPI.transfer(temp);
  digitalWrite(OLED_CS,HIGH);
  SPI.endTransaction();
}
 
void OLED_Data(uint8_t temp){
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(OLED_CS,LOW);
  digitalWrite(OLED_DC,HIGH);
  SPI.transfer(temp);
  digitalWrite(OLED_CS,HIGH);
  SPI.endTransaction();
}
 
void OLED_Initialize(void)
{
    digitalWrite(OLED_RST,LOW);
    delay(1000);
    digitalWrite(OLED_RST,HIGH);
    delay(1000);
    OLED_Command(SSD1306_DISPLAYOFF);             //0xAE  Set OLED Display Off
    OLED_Command(SSD1306_SETDISPLAYCLOCKDIV);     //0xD5  Set Display Clock Divide Ratio/Oscillator Frequency
    OLED_Command(0x80);
    OLED_Command(SSD1306_SETMULTIPLEX);           //0xA8  Set Multiplex Ratio
    OLED_Command(39);
 
    OLED_Command(SSD1306_SETSEGMENTREMAP);        //0xA1  Set Segment Remap Inv
    OLED_Command(SSD1306_COMSCANDEC);             //0xC8  Set COM Output Scan Inv
 
    OLED_Command(SSD1306_SETDISPLAYOFFSET);       //0xD3  Set Display Offset
    OLED_Command(0x00);
    OLED_Command(SSD1306_CHARGEPUMP);             //0x8D  Set Charge Pump
    OLED_Command(0x14);                           //0x14  Enable Charge Pump
    OLED_Command(SSD1306_SETSTARTLINE);           //0x40  Set Display Start Line
    OLED_Command(SSD1306_SETCOMPINS);             //0xDA  Set COM Pins Hardware Configuration
    OLED_Command(0x12);
    OLED_Command(SSD1306_SETCONTRAST);            //0x81   Set Contrast Control
    OLED_Command(0xAF);
    OLED_Command(SSD1306_SETPRECHARGE);           //0xD9   Set Pre-Charge Period
    OLED_Command(0x25);
    OLED_Command(SSD1306_SETVCOMDETECT);          //0xDB   Set VCOMH Deselect Level
    OLED_Command(0x20);
    OLED_Command(SSD1306_DISPLAYALLON_RESUME);    //0xA4   Set Entire Display On/Off
    OLED_Command(SSD1306_NORMALDISPLAY);          //0xA6   Set Normal/Inverse Display
    OLED_Command(SSD1306_DISPLAYON);              //0xAF   Set OLED Display On
} // OLED_Initialize
 
void OLED_SetRow(uint8_t add)
{
    add = 0xB0 | add;
    OLED_Command(add);
}
 
void OLED_SetColumn(uint8_t add)
{
    add += 32;
    OLED_Command((SSD1306_SETHIGHCOLUMN | (add >> 4))); // SET_HIGH_COLUMN
    OLED_Command((0x0f & add));        // SET LOW_COLUMN
}
 
void OLED_PutPicture(const uint8_t *pic)
{
    unsigned char i,j;
    for(i=0; i<5; i++) // 5*8=40 pixel rows (actually 39)
    {
        OLED_SetRow(i);
        OLED_SetColumn(0);
        for(j=0; j<96; j++)  // 96 pixel columns
        {
            OLED_Data(*pic++);
        }
    }
}
 
void OLED_SetContrast(uint8_t temp)
{
    OLED_Command(SSD1306_SETCONTRAST);
    OLED_Command(temp);                  // contrast step 1 to 256
}
 
 
 
uint8_t _x, _y;
uint8_t _sx=1, _sy=1; // scaling factors
 
void OLED_SetScale(uint8_t sx, uint8_t sy){
    _sx = sx; _sy = sy;
}
 
void OLED_Clear(void)
{
    unsigned char i,j;
    for(i=0; i<5; i++) // 5*8=40 pixel rows (actually 39)
    {
        OLED_SetRow(i);
        OLED_SetColumn(0);
        for(j=0; j<96; j++)  OLED_Data(0);
    }
    _x = 0; _y = 0;
    OLED_SetRow(0);
    OLED_SetColumn(0);
}
 
/**  scalable horizontally and vertically
 */
void OLED_Putchar(char ch)
{
    uint8_t i, j, k, byte;
    const uint8_t *f = &font[(ch-' ')*5];
    const uint8_t mask[]={1, 3, 7, 0xf };

    for(i=0; i<6; i++) {
        uint32_t word = 0;
        byte = *f++ << 1;
        if (i==5) byte = 0;
        for(j=0; j<8; j++) { // expand byte to word
            word <<= _sy;
            if (byte & 0x80) word |= mask[_sy-1];
            byte <<= 1;
        }
        for(j=0; j<_sy; j++){ // exp vert
            OLED_SetRow(_y+j) ;
            OLED_SetColumn(_x+i*_sx);
            for(k=0; k<_sx; k++){ // exp horiz
                OLED_Data(word);
            }
            word >>= 8;
        }
    }
 
    _x+= 6 * _sx;
    if (_x >= OLED_WIDTH) { // wrap x
        _x = 0; OLED_SetColumn(0);
        _y += _sy;
        if (_y >= 5-_sy) { // wrap y
            _y = 0;
        }
        OLED_SetRow(_y);
    }
}
 
void OLED_Puts(char x, char y, char *s)
{
    _y = y; _x = x;
    OLED_SetRow(_y);
    OLED_SetColumn(_x);
    while(*s) {
        OLED_Putchar(*s++);
        _x++;
    }
}
