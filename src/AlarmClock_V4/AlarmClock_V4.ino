// Alarm Clock V4
// Replaced the touch screen by push button
//
//
// Red Button = On/Off Radio
// Blue Button = Menu
// 2 White Button = Alarm 1 & 2
// Rotary button = Acknowledge and Volume button

// RTC - Version: 1.13.0
#include "RTClib.h"

RTC_DS3231 rtc;

unsigned long   ms_time_display;

void setup() 
{
    Serial.println("Setup");
    
    if (! rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    else
    {
        Serial.println("RTC started");
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
    
}

void loop() {
  // put your main code here, to run repeatedly:

    // Clock Mode - @1s refresh rate
    if (millis() - ms_time_display > 1000) 
    { 
        ms_time_display = millis();
        Serial.println(ms_time_display);

    }
}

