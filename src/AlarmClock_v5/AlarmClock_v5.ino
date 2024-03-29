// Alarm Clock v5
// Replaced the touch screen by push button
//
//
// Red Button = On/Off Radio
// Blue Button = Menu
// 2 White Button = Alarm 1 & 2
// Rotary button = Acknowledge and Volume button

// RTC - Version: 1.13.0
#include "RTClib.h"

// Radio
#include "SI470X.h"
#include <EEPROM.h>


#define DISPLAY_TIME false

// EEPROM Mapping
#define eeprom_INIT_0   0       // Must be 'Y'
#define eeprom_INIT_1   1       // Must be 'E'
#define eeprom_INIT_2   2       // Must be 'S'
#define eeprom_vol      10      // Max 15
#define eeprom_freq     12      // Radio default freauency (i.e. 10550 for 105.50 MHz)

// ----- Radio -----
#define FIX_BAND     RADIO_BAND_FM   //< The band that will be tuned by this sketch is FM.
#define FIX_STATION  10230            //< The station that will be tuned by this sketch is 105.50 MHz.
#define FIX_VOLUME   4               //< The volume that will be set by this sketch is level 4.

#define MAX_VOLUME      14
#define MIN_FREQ        8750
#define MAX_FREQ        10800

RTC_DS3231 rtc;

SI470X radio; 
#define RESET_PIN 52 // On Arduino Atmega328 based board, this pin is labeled as A0 (14 means digital pin instead analog)


unsigned long   ms_time_display;

// set pin numbers:
const int pb_On_Off_Pin = 22;     // the number of the pushbutton pin
const int rotary_pin_SW = 30;
const int rotary_pin_DT = 31;
const int rotary_pin_CLK = 32;

// On/Off switch variable
int pb_On_Off_State;             // the current reading from the input pin
int pb_On_Off_lastState = LOW;   // the previous reading from the input pin

// Rotary switch variable
int         rotary_Position = 0; 
int         rotation;
boolean     leftRight;
int         pb_rotary_state;
int         pb_rotary_last_state = LOW;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;     // the last time the output pin was toggled
unsigned long debounceDelay = 50;       // the debounce time; increase if the output flickers

// Radio variables
bool radio_State = false;
unsigned int radio_volume = 7;
unsigned long radio_frequency = 10550;

void setup() 
{
    char msg[100];
    char eeprom_init[4];
    
    Serial.begin(9600);

    Serial.println("Setup");

    Serial.println("Init EEPROM");
   
    EEPROM.get(eeprom_INIT_0, eeprom_init);
    Serial.println(eeprom_init);

    if( (eeprom_init[0] == 'Y') && (eeprom_init[1] == 'E') && (eeprom_init[2] == 'S') && (eeprom_init[3] == '\0')){
        EEPROM.get(eeprom_vol, radio_volume);
        Serial.println(radio_volume);
        EEPROM.get(eeprom_freq, radio_frequency);
        Serial.println(radio_frequency);

    }
    else{
        EEPROM.put(eeprom_INIT_0, "YES\0");
        EEPROM.put(eeprom_vol, (unsigned int)(FIX_VOLUME));
        EEPROM.put(eeprom_freq, (unsigned long)(FIX_STATION));
    }
 
    Serial.println("Init RTC");

    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }
    else {
        Serial.println("RTC started");
    }

    if (rtc.lostPower()) {
        // Serial.println("RTC lost power, lets set the time!");
        // If the RTC have lost power it will sets the RTC to the date & time this sketch was compiled in the following line
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    ms_time_display = millis();
    
    Serial.println("Configure push button");

    // initialize the pushbutton and rotary pins as an input:
    pinMode(pb_On_Off_Pin, INPUT);
    pinMode(rotary_pin_CLK, INPUT);
    pinMode(rotary_pin_DT, INPUT);
    pinMode(rotary_pin_SW, INPUT);
    rotation = digitalRead(rotary_pin_CLK);
    Serial.println(rotation);

    Serial.println("Init Radio");

    radio.setup(RESET_PIN, 20 /* SDA pin  for Arduino ATmega328 */);
    // radio.setMute(true);
    radio.setVolume(radio_volume);
    radio.setFrequency(radio_frequency);
    radio.setMono(false);

    sprintf(msg, "Set radio frequency to %d", radio_frequency);
    Serial.println(msg);

    Serial.println("Init completed!");

}

void loop() 
{
    // put your main code here, to run repeatedly:

    char        time_str[30];
    DateTime    now;

    // Clock Mode - @1s refresh rate
    if (millis() - ms_time_display > 1000) 
    { 
        ms_time_display = millis();

        now = rtc.now();
        if(DISPLAY_TIME == true){
            sprintf(time_str, "Time %02d/%02d/%04d %02d:%02d:%02d", now.day(), now.month(), now.year(), 
                                                                now.hour(), now.minute(), now.second());
            Serial.println(time_str);
        }
    }

    bool pb_state_change = push_button_get_state(pb_On_Off_Pin, &pb_On_Off_State, &pb_On_Off_lastState);
    if(pb_state_change == true) {
        if (pb_On_Off_State == HIGH){
            if (radio_State == true){
                Serial.println("Shutdown Radio");
                // radio.setVolume(0);
                radio_State = false;
            }
            else{
                Serial.println("Start Radio");
                radio.setVolume(7);
                radio_State = true;
            }
        }
    }

    detect_rotary_button();   
    
}

int detect_rotary_button()
{
    int rotary_value = digitalRead(rotary_pin_CLK);

    // we use the DT pin to find out which way we turning.
    if (rotary_value != rotation){ 
        if (digitalRead(rotary_pin_DT) != rotary_value) {  // Clockwise
            rotary_Position ++;
            leftRight = true;
        } 
        else { //Counterclockwise
            leftRight = false;
            rotary_Position--;
        }
        // turning right will turn on red led.
        if (leftRight){ 
            Serial.println ("clockwise");
            /* TODO: Add function to manage increase position */
        }
        else{        // turning left will turn on green led.   
            Serial.println("counterclockwise");
            /* TODO: Add function to manage deacrese position */
        }
        Serial.print("Encoder RotPosition: ");
        Serial.println(rotary_Position);
        // this will print in the serial monitor.
    } 
    rotation = rotary_value;

    push_button_get_state(rotary_pin_SW, &pb_rotary_state, &pb_rotary_last_state);
}

boolean push_button_get_state(int pb_pin, int *pb_state, int *pb_last_state)
{
    // read the state of the pushbutton value:
    int reading = digitalRead(pb_pin);

    // If the switch changed, due to noise or pressing:
    if (reading != *pb_last_state) {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

        // if the button state has changed:
        if (reading != *pb_state) {
            *pb_state = reading;
            Serial.println("Button pressed");
            return(true);
        }
    }
    *pb_last_state = reading;
    return(false);

}

