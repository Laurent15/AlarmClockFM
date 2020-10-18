// TFT Kuman K60 - size = 320x240 pixel
#include <Adafruit_GFX.h>    // Core graphics library
// #include <Fonts/Org_01.h>
// #include <Fonts/FreeSans9pt7b.h>
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

// RTC - Version: 1.4.1
#include "RTClib.h"

// Radio
// #include <radio.h>
// #include <SI4703.h>
#include <SI470X.h>
#include <RDSParser.h>

#include <EEPROM.h>

// ----- Define TFT Screen Size -----
#define center_y   140
#define center_x   160
#define max_width  320
#define max_height 240

// ----- Radio -----
#define FIX_BAND     RADIO_BAND_FM   //< The band that will be tuned by this sketch is FM.
#define FIX_STATION  10230            //< The station that will be tuned by this sketch is 105.50 MHz.
#define FIX_VOLUME   4               //< The volume that will be set by this sketch is level 4.

#define MAX_VOLUME      14
#define MIN_FREQ        8750
#define MAX_FREQ        10800
#define eeprom_vol      0
#define eeprom_freq_int 1   // Integer part of the freauency (i.e. 105 for 105.50 MHz)
#define eeprom_freq_dec 2   // Decimal part of the freauency (i.e. 50 for 105.50 MHz)

// ----- Define Modes ----
#define MODE_CLOCK          0
#define MODE_SETTING_GLOBAL 1
#define MODE_SETTING_CLOCK  2
#define MODE_SETTING_ALARM  3
#define MODE_SETTING_RADIO  4

// ----- Define some colour values -----
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

#define GRAY        0x2222
#define LIGHT_GRAY  0xBDF7
#define DARK_GRAY   0x7BEF
#define DARKBLUE    0x18EB //0x298E

#define BG      0xEEE
#define GRS     0xEEEF

#define MAIN_COLOR LIGHT_GRAY

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin

// ---- Touch screen -----
#define TS_MINX 120
#define TS_MAXX 1160
#define TS_MINY 322
#define TS_MAXY 910
 
#define YP A3
#define XM A2
#define YM 9
#define XP 8
 
#define MINPRESSURE 10
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 364);
Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

RTC_DS3231 rtc;

SI470X radio; 
#define RESET_PIN 52 // On Arduino Atmega328 based board, this pin is labeled as A0 (14 means digital pin instead analog)

// ----- Icons -----
// generated from http://javl.github.io/image2cpp/
// 'settings', 20x20px
const unsigned char setting_bmp [] PROGMEM = {
	0x00, 0xf0, 0x00, 0x00, 0x90, 0x00, 0x1d, 0x9b, 0x80, 0x37, 0x0e, 0xc0, 0x20, 0x00, 0x40, 0x30, 
	0x00, 0xc0, 0x11, 0xf8, 0x80, 0x33, 0x0c, 0xc0, 0xe2, 0x04, 0x70, 0x82, 0x04, 0x10, 0x82, 0x04, 
	0x10, 0xe2, 0x04, 0x70, 0x33, 0x0c, 0xc0, 0x11, 0xf8, 0x80, 0x30, 0x00, 0xc0, 0x20, 0x00, 0x40, 
	0x37, 0x0e, 0xc0, 0x1d, 0x9b, 0x80, 0x00, 0x90, 0x00, 0x00, 0xf0, 0x00
};

// 'bell alarm 1', 20x20px
const unsigned char bell_1_bmp [] PROGMEM = {
	0x00, 0x60, 0x00, 0x00, 0xf0, 0x00, 0x03, 0xfc, 0x00, 0x07, 0x0e, 0x00, 0x0e, 0x07, 0x00, 0x0c, 
	0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0xe3, 0x80, 0x18, 0x21, 0x80, 0x18, 0x21, 
	0x80, 0x18, 0x21, 0x80, 0x38, 0xf1, 0xc0, 0x30, 0xf0, 0xc0, 0x70, 0x00, 0x60, 0xc0, 0x00, 0x30, 
	0xff, 0xff, 0xf0, 0x7f, 0xff, 0xf0, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00
};

// 'bell alarm 2', 20x20px
const unsigned char bell_2_bmp [] PROGMEM = {
	0x00, 0x60, 0x00, 0x00, 0xf0, 0x00, 0x03, 0xfc, 0x00, 0x07, 0x0e, 0x00, 0x0e, 0x07, 0x00, 0x0c, 
	0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0xf3, 0x00, 0x0c, 0x33, 0x80, 0x18, 0x31, 0x80, 0x18, 0x21, 
	0x80, 0x18, 0x41, 0x80, 0x38, 0xf9, 0xc0, 0x30, 0x00, 0xc0, 0x70, 0x00, 0x60, 0xc0, 0x00, 0x30, 
	0xff, 0xff, 0xf0, 0x7f, 0xff, 0xf0, 0x00, 0xf0, 0x00, 0x00, 0x70, 0x00
};

// ----- icon & UI position -----
#define header_footer_line_pos 25

#define setting_x max_width-22
#define setting_y max_height-22
#define bell_1_x 2
#define bell_1_y max_height-22
#define bell_2_x 102
#define bell_2_y max_height-22

// ----- Define labels -----
#define setting_clock "Date / Heure"
#define setting_alarm "Alarm"
#define setting_radio "Radio"

// ----- Global variables. -----
uint16_t        alarm_1_color, alarm_2_color;
unsigned long   ms_time_display;
int             mode = MODE_CLOCK;   // Mode 0 = Clock / Mode 1 = Setting / Mode 2 = tbd
int             previous_mode = mode;
int             edit_hour, edit_minute, edit_day, edit_month, edit_year, edit_freq;
int             edit_alarm_hour, edit_alarm_minute; 
void setup() 
{
    alarm_1_color = DARK_GRAY;
    alarm_2_color = DARK_GRAY;

    int station = 1023;

    
    
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        // Serial.println("RTC lost power, lets set the time!");
        // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        rtc.adjust(DateTime(2020, 1, 1, 0, 0, 0));
    }
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    ms_time_display = millis();

    Init_Display();
    Clock_UI();

    radio.setup(RESET_PIN, 20);
    radio.setVolume(6);  
    radio.setFrequency(station * 10);
    radio.setMono(true);


    // radio.init();
    // radio.setVolume(10);
    // radio.setBandFrequency(RADIO_BAND_FM, station * 10);
    // radio.setFrequency(station * 10);
    // radio.setMono(true);
    // radio.setMute(true);
    // radio.setSoftMute(true);
    // radio.checkRDS() ;
}

void loop()
{
    TSPoint p = ts.getPoint();
    
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) 
    {
        TSPoint c = p; 
        c.x = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0); 
        c.y = map(p.x, TS_MINX, TS_MAXX, 0, tft.width()); 

        switch (mode)
        {
        case MODE_CLOCK:
            if(touchin(c, 0, max_height - 20, 80, 20)) 
                btn_setting(); 
            break;

        case MODE_SETTING_GLOBAL:
            if(touchin(c, max_width-80, max_height - 20, 80, 20))
                btn_back();

            if(touchin(c, 0, 60, max_width, 25))
                btn_setting_clock();

            if(touchin(c, 0, 130, max_width, 25))
                btn_setting_alarm();
            break;

        case MODE_SETTING_CLOCK:
            if(touchin(c, max_width-80, max_height - 20, 80, 20))
                btn_back();
            if(touchin(c, 0, max_height - 20, 80, 20))
            {
                rtc.adjust(DateTime(edit_year, edit_month, edit_day, edit_hour, edit_minute, 0));
                btn_back();
            }

            // Hours + 1
            if(touchin(c, max_width/2 + 20, 35, 55, 20))
            {    edit_hour++; 
                if (edit_hour > 23) 
                    edit_hour = 0;
                Display_edit_clock_update(MAIN_COLOR);
            }
            // Hours - 1
            if(touchin(c, max_width/2 + 20, 93, 55, 20))
            {
                edit_hour--; 
                if (edit_hour < 0) 
                    edit_hour = 23;
                Display_edit_clock_update(MAIN_COLOR);
            }
            // Minutes + 1
            if(touchin(c, max_width/2 - 55, 35, 55, 20))
            {
                edit_minute++; 
                if (edit_minute > 59) 
                    edit_minute = 0;
                Display_edit_clock_update(MAIN_COLOR);   
            }         

            //  -25 / 60 / +33
            // Minutes - 1
            if(touchin(c, max_width/2 - 55, 93, 55, 20))
            {
                edit_minute--; 
                if (edit_minute < 0) 
                    edit_minute = 59;
                Display_edit_clock_update(MAIN_COLOR);
            }
            
            // 125 / 150 / 183
            // Day + 1
            if(touchin(c, max_width/2 + 90, 125, 55, 20))
            {
                edit_day++; 
                if (edit_day > 31) 
                    edit_day = 31;
                Display_edit_date_update(MAIN_COLOR);   
            }         
            // Day - 1
            if(touchin(c, max_width/2 + 90, 183, 55, 20))
            {
                edit_day--; 
                if (edit_day < 1) 
                    edit_day = 1;
                Display_edit_date_update(MAIN_COLOR);
            }

            // Month + 1
            if(touchin(c, max_width/2 + 10, 125, 55, 20))
            {
                edit_month++; 
                if (edit_month > 12) 
                    edit_month = 12;
                Display_edit_date_update(MAIN_COLOR);   
            }         
            // Month - 1
            if(touchin(c, max_width/2 + 10, 183, 55, 20))
            {
                edit_month--; 
                if (edit_month < 1) 
                    edit_month = 1;
                Display_edit_date_update(MAIN_COLOR);
            }
            // Year + 1
            if(touchin(c, max_width/2 - 120, 125, 110, 20))
            {
                edit_year++; 
                Display_edit_date_update(MAIN_COLOR);   
            }         
            // Year - 1
            if(touchin(c, max_width/2 - 120, 183, 110, 20))
            {
                edit_year--; 
                if (edit_year < 2020) 
                    edit_year = 2020;
                Display_edit_date_update(MAIN_COLOR);
            }

            break;

        case MODE_SETTING_ALARM:
            if(touchin(c, max_width-80, max_height - 20, 80, 20))
                btn_back();
            break;

        default:
            break;
        }
    }

    // Clock Mode - @1s refresh rate
    if (millis() - ms_time_display > 1000) 
    { 
        if (mode == MODE_CLOCK) 
        {
            Display_rtc(MAIN_COLOR);
            ms_time_display = millis();
        }
        else
        {
            Display_header_hour(MAIN_COLOR);
            ms_time_display = millis();
        }
    }
}


boolean touchin(TSPoint p, int x, int y, int w, int h) {
  return (p.x > x && p.x < x+w && p.y > y && p.y < y+h);
}

void btn_setting() 
{
    previous_mode = mode;
    mode = MODE_SETTING_GLOBAL;
    Display_bg(MAIN_COLOR);
    Display_setting_menu(MAIN_COLOR);
}

void btn_setting_clock()
{
    previous_mode = mode;
    mode = MODE_SETTING_CLOCK;
    Display_bg(MAIN_COLOR);
    Display_edit_rtc(MAIN_COLOR);
}

void btn_setting_alarm()
{
    previous_mode = mode;
    mode = MODE_SETTING_ALARM;
    Display_bg(MAIN_COLOR);
    Display_edit_alarm(MAIN_COLOR);
}

void btn_back()
{
    switch (mode)
    {
    case MODE_SETTING_GLOBAL:
        previous_mode = mode;
        mode = MODE_CLOCK;
        Clock_UI();
        break;

    case MODE_SETTING_CLOCK:
        previous_mode = mode;
        mode = MODE_SETTING_GLOBAL;
        Display_bg(MAIN_COLOR);
        Display_setting_menu(MAIN_COLOR);
        break;
    
    case MODE_SETTING_ALARM:
        previous_mode = mode;
        mode = MODE_SETTING_GLOBAL;
        Display_bg(MAIN_COLOR);
        Display_setting_menu(MAIN_COLOR);
        break;

    default:
        break;
    }
}

void Init_Display() 
{
    tft.reset();
    tft.begin(0x9341);
    tft.setRotation(3);
    // tft.setFont(&FreeSans9pt7b);
    setBackLight(10);
}

void Clock_UI()
{
    Display_bg(MAIN_COLOR);
    Display_rtc(MAIN_COLOR);
}

void Display_bg(uint16_t color)
{
    tft.fillScreen(BLACK);
    tft.drawLine(0, header_footer_line_pos, max_width, header_footer_line_pos, color);
    tft.drawLine(0, max_height - header_footer_line_pos, max_width, max_height - header_footer_line_pos, color);
}

void Display_header_title(uint8_t *str_title, uint16_t color)
{
    uint8_t     size_str;
    DateTime    now = rtc.now();

    // Title
    size_str = 2;
    tft.setCursor(0, 5);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str_title);
}

void Display_header_hour(uint16_t color)
{
    char        time_str[12];
    int16_t     pos_x;
	uint16_t    len;
    uint8_t     size_str;
    DateTime    now = rtc.now();

    // Hours - Title
    size_str = 2;
    sprintf(time_str, "%02d:%02d", now.hour(), now.minute());
    len = strlen(time_str) * 6 * size_str;		
    pos_x = (max_width - len); 
    tft.setCursor(pos_x, 5);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(time_str);
}

void Display_footer_text_left(uint8_t *str_btn, uint16_t color)
{
    char        time_str[12];
    uint8_t     size_str;
    
    size_str = 2;
    tft.setCursor(0, 220);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str_btn);
}

void Display_footer_text_right(uint8_t *str_btn, uint16_t color)
{
    int16_t     pos_x;
	uint16_t    len;
    uint8_t     size_str;
    DateTime    now = rtc.now();

    size_str = 2;
    len = strlen(str_btn) * 6 * size_str;		
    pos_x = (max_width - len); 
    tft.setCursor(pos_x, 220);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str_btn);
}

void Display_footer_alarm(uint8_t alarm_id, uint16_t color)
{
    if (alarm_id == 1)
        tft.drawBitmap(bell_1_x, bell_1_y, bell_1_bmp, 20, 20, color);
    if (alarm_id == 2)
        tft.drawBitmap(bell_2_x, bell_2_y, bell_2_bmp, 20, 20, color);
}

void Display_btn_up(uint8_t  x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;

    tft.drawRoundRect(x, y, width, height, 3, color);

    size_str = 2;
    sprintf(str, "+");
    size_char = 6 * size_str;
    len = strlen(str) * size_char;		
    pos_x = x + width/2 - size_char/2;
    pos_y = y + height/2 - size_str*7/2;
    tft.setCursor(pos_x, pos_y);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);
}

void Display_btn_down(uint8_t  x, uint8_t y, uint8_t width, uint8_t height, uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;

    tft.drawRoundRect(x, y, width, height, 3, color);

    size_str = 2;
    sprintf(str, "-");
    size_char = 6 * size_str;
    len = strlen(str) * size_char;		
    pos_x = x + width/2 - size_char/2;
    pos_y = y + height/2 - size_str*7/2;
    tft.setCursor(pos_x, pos_y);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);
}

void Display_rtc(uint16_t color)
{
    char        time_str[12];
    int16_t     pos_x;
	uint16_t    len;
    uint8_t     size_str;
    DateTime    now = rtc.now();
    
    // Draw Alarm icon & Setting
    tft.drawBitmap(setting_x, setting_y, setting_bmp, 20, 20, color); 

    Display_footer_alarm(1, alarm_1_color);
    Display_footer_alarm(2, alarm_2_color);
    
    // tft.drawBitmap(bell_1_x, bell_1_y, bell_1_bmp, 20, 20, alarm_1_color); 
    // tft.drawBitmap(bell_2_x, bell_2_y, bell_2_bmp, 20, 20, alarm_2_color);

    // Hours
    size_str = 8;
    sprintf(time_str, "%02d:%02d", now.hour(), now.minute());

    len = strlen(time_str) * 6 * size_str;		
    pos_x = (max_width - len); 
    pos_x /= 2;

    tft.setCursor(pos_x, 60);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(time_str);

    // Date
    size_str = 3;
    sprintf(time_str, "%02d-%02d-%04d", now.day(), now.month(), now.year());
    len = strlen(time_str) * 6 * size_str;		
    pos_x = (max_width - len); 
    pos_x /= 2;
    tft.setCursor(pos_x, 150);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(time_str);
}

void Display_edit_rtc(uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;
    DateTime    now = rtc.now();
    
    edit_hour = now.hour();
    edit_minute = now.minute();
    edit_day = now.day();
    edit_month = now.month();
    edit_year = now.year();

    Display_header_title("Reglage date/heure", color);
    Display_header_hour(color);
    Display_footer_text_right("Sauvegarde", color);
    Display_footer_text_left("Retour", color);

    // Hours
    Display_edit_clock_update(color);

    // Date
    Display_edit_date_update(color);
}

void Display_edit_clock_update(uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;

    pos_y = 60;
    size_str = 4;
    sprintf(str, "%02d:%02d", edit_hour, edit_minute);
    size_char = 6 * size_str;
    len = strlen(str) * size_char;		
    pos_x = (max_width - len) / 2; 
    tft.setCursor(pos_x, pos_y);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);

    // Button Up Hours
    pos_x = max_width/2 - len/2 - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5, 2*6*size_str, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*6*size_str, 20, color);

    // Button Up Minutes
    pos_x = max_width/2 + size_char/2 - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5 , 2*6*size_str, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*6*size_str, 20, color);
}

void Display_edit_date_update(uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;

    // Date
    pos_y = 150;
    size_str = 4;
    sprintf(str, "%02d-%02d-%04d", edit_day, edit_month, edit_year);
    size_char = 6 * size_str;
    len = strlen(str) * size_char;		
    pos_x = (max_width - len); 
    pos_x /= 2;
    tft.setCursor(pos_x, pos_y);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);

    // Button Up Day
    pos_x = max_width/2 - len/2 - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5, 2*size_char, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*size_char, 20, color);

    // Button Up Month
    pos_x = max_width/2 - 2*size_char - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5, 2*size_char, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*size_char, 20, color);

    // Button Up Year
    pos_x = max_width/2 + size_char - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5, 4*size_char, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 4*size_char, 20, color);
}


void Display_edit_alarm(uint16_t color)
{

    edit_alarm_hour = 0;
    edit_alarm_minute = 0;

    Display_header_title("Reglage alarme", color);
    Display_header_hour(color);
    Display_footer_text_right("Sauvegarde", color);
    Display_footer_text_left("Retour", color);

    // Alarm Hours
    Display_edit_alarm_clock_update(color);

}

void Display_edit_alarm_clock_update(uint16_t color)
{
    char        str[20];
    uint8_t     size_str;
    int16_t     pos_x, pos_y;
	uint16_t    len, size_char;

    pos_y = 60;
    size_str = 4;
    sprintf(str, "%02d:%02d", edit_alarm_hour, edit_alarm_minute);
    size_char = 6 * size_str;
    len = strlen(str) * size_char;		
    pos_x = (max_width - len) / 2; 
    tft.setCursor(pos_x, pos_y);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);

    // Button Up Hours
    pos_x = max_width/2 - len/2 - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5, 2*6*size_str, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*6*size_str, 20, color);

    // Button Up Minutes
    pos_x = max_width/2 + size_char/2 - 1;
    Display_btn_up(pos_x, pos_y - 20 - 5 , 2*6*size_str, 20, color);
    Display_btn_down(pos_x, pos_y + 7*size_str + 5 , 2*6*size_str, 20, color);
}

void Display_setting_menu(uint16_t color)
{
    char        str[20];
    int16_t     pos_x;
	uint16_t    len;
    uint8_t     size_str;

    Display_header_title("Reglages", color);
    Display_header_hour(color);
    Display_footer_text_right("", color);
    Display_footer_text_left("Retour", color);

    // Date/Heure
    size_str = 3;
    sprintf(str, setting_clock);
    len = strlen(str) * 6 * size_str;		
    pos_x = (max_width - len); 
    pos_x /= 2;
    tft.setCursor(pos_x, (max_height - 2*header_footer_line_pos) * 1/4 + header_footer_line_pos - 7*size_str/2);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str);

    // Alarm
    size_str = 3;
    sprintf(str, setting_alarm);
    len = strlen(str) * 6 * size_str;		
    pos_x = (max_width - len); 
    pos_x /= 2;
    tft.setCursor(pos_x, (max_height - 2*header_footer_line_pos) * 1/2 + header_footer_line_pos - 7*size_str/2);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str); 

    // Radio
    size_str = 3;
    sprintf(str, setting_radio);
    len = strlen(str) * 6 * size_str;		
    pos_x = (max_width - len); 
    pos_x /= 2;
    tft.setCursor(pos_x, (max_height - 2*header_footer_line_pos) * 3/4 + header_footer_line_pos - 7*size_str/2);
    tft.setTextSize(size_str);
    tft.setTextColor(color, BLACK);
    Print_String(str); 
}









// advanced print string
int Print(uint8_t *st)
{
	int16_t pos;
	uint16_t len;
	const uint8_t * p = st;
	size_t n = 0;

	while(1)
	{
		unsigned char ch = *(p++);//pgm_read_byte(p++);
		if(ch == 0)
			break;
		if(tft.write(ch))
			n++;
		else
			break;
	}	
	return n;
}

//print string
void Print_String(const uint8_t *st)
{
	Print((uint8_t *)st);
}

//print string
void Print_String(uint8_t *st)
{
	Print(st);
}

//print string
void Print_String(String st)
{
	Print((uint8_t *)(st.c_str()));
}

void setBackLight(uint8_t brightness)
{
//   tft.setRegisters8(0x51, brightness);
}