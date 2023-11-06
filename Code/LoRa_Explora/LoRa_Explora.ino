/* Library used: https://github.com/Bodmer/TFT_eSPI
 * Author: user Bodmer
 * 
 * Library used: https://github.com/PaulStoffregen/Time/tree/master
 * Author: user PaulStoffregen
 */



#include "TFT_eSPI.h"
#include <TimeLib.h>
#include "sprites.h"

#define DEBUG 0 //only for debugging

/*
 *Pinout 
 */
#define PIN_POWER_ON 15  // LCD and battery Power Enable
#define PIN_LCD_BL 38    // BackLight enable pin (see Dimming.txt)
#define LORA_TX 44
#define LORA_RX 43
#define GPS_TX 21
#define GPS_RX 16
#define AXL_X 1
#define AXL_Y 2
#define AXL_Z 3
#define POWER_BUTTON 18
#define FAST_OFF 17
/*
 *LoRa parameters 
 */
#define periodic 0
#define do_retry_join 0
#define msg_time 180000
#define ack 0
#define n_trails 2
#define msg_len 14

//timer interrupt setup------------------------------
#define timerInterval 30000000  //microseconds
volatile int isrTimer;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  isrTimer++;
  portEXIT_CRITICAL_ISR(&timerMux);
}
//button interrupt setup--------------------------------
volatile int btnPressed;
volatile int btnDebounce;
volatile int btnPowerDown;

void IRAM_ATTR onBtnPress(){
  btnPressed++;
}

//------------------------------------------------------

//define object to handle LCD
TFT_eSPI display = TFT_eSPI();
//sprites
TFT_eSprite spriteBattery = TFT_eSprite(& display);     // Battery sprite.
TFT_eSprite spriteTime = TFT_eSprite(& display);        // Time sprite.
TFT_eSprite spriteLora = TFT_eSprite(& display);        // LoRa sprite.
TFT_eSprite spriteGPS = TFT_eSprite(& display);         // GPS sprite.
TFT_eSprite spriteADXL = TFT_eSprite(& display);        // Accelerometer sprite.
TFT_eSprite spriteSOS = TFT_eSprite(& display);         // SOS & Power sprite.


//naming for Serial Comms
auto& comSerial = Serial;
auto& loraSerial = Serial1;
auto& gpsSerial = Serial2; 

//global variables
unsigned long waitTimer, chargeTimer, debounceTimer, pwrTimer, canFallTimer;

//LoRa flags
uint8_t initNwkJoin = 1; 
uint8_t nwkJoined = 0;
uint8_t joinFailed = 0;
uint8_t msgSent = 0;
uint8_t msgCnt = 0;
uint8_t sentFailed = 0;
uint8_t msgSending = 0;
uint8_t readySend = 0;
uint8_t retryJoin = 0;
uint8_t EmrgLvl = 0;

//GPS values
float GPSlat;
float GPSlong;
uint8_t timeValid = 0;
uint8_t timeSynced = 0;
uint8_t locValid = 0;
const int offset = 2;   // UTC+2

//ADXL 
#define FALL_CAN_DELAY 30000
uint8_t fallDetect = 0;
uint8_t fallColl = 0;
uint8_t collDetect = 0;
uint8_t fallToggle = 0;
uint8_t canFall = 1;

//Sending Protocol
uint8_t loraProt[8];
uint8_t neg;
uint8_t negFall[2];

//battery
uint32_t volt;
volatile uint8_t charging = 1; 
uint8_t progress = 0; 

//time
time_t gpsTime = 0;
time_t myTime = 0;
char currentTime[6];

void setup() {

  pinMode(POWER_BUTTON, INPUT_PULLUP);
  
  //enable the OLED screen
  pinMode(PIN_POWER_ON, OUTPUT);  //triggers the LCD backlight
  pinMode(PIN_LCD_BL, OUTPUT);    // BackLight enable pin, tft.init() also handles this and caused an startup issue
  delay(1);
  digitalWrite(PIN_POWER_ON, HIGH);
  digitalWrite(PIN_LCD_BL, HIGH);
  
  //initialize the LCD screen
  init_LCD();

  
  
  //setup UART/Serial Communication
  comSerial.begin(9600);
  loraSerial.begin(9600, SERIAL_8N1, LORA_RX, LORA_TX);
  gpsSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  comSerial.println("Serial Com Established...");

  //timer interrupt
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, timerInterval, true);
  timerAlarmEnable(timer);

  //button interupt
  attachInterrupt(POWER_BUTTON, onBtnPress, FALLING);
  
  //initiate some variables
  waitTimer = millis();
  chargeTimer = millis();

 //------------------------------
//  GPSlat = -33.931460439573506;
//  GPSlong = 18.85629883200975;
//  timeValid = 1;
//  timeSynced = 1;
//  locValid = 1;
//  msgSent = 1;
//  
//  setTime(12,30,0,14,8,2000);
//  sprintf(currentTime, "%.2d:%.2d", hour(myTime), minute(myTime));
//  gpsTime = now();
  //-----------------------------
  
}

void loop() {
  
  if(isrTimer > 0){
    portENTER_CRITICAL(&timerMux);
    isrTimer--;
    portEXIT_CRITICAL(&timerMux);
    if(canFall){
      clearLCD_SOS();
    }
    getVbatt();
    if(!charging){
     updateLCD_BattLvl();
    }
  }

  btnHandler();

  if(charging){
    if(millis() > chargeTimer + 2000){
      chargeTimer = millis();
      updateLCD_BattLvl();
    }
  }

  fallDetection();
  if(fallToggle){
    updateLCD_ADXL();
    fallToggle = 0;
  }

  //updateLCD_GPS();
  getGPS();
  updateLCD_Time();
  
  updateLora();
}

void getVbatt(){
  volt = (analogRead(4) * 2 * 3.3 * 1000) / 4096;

  if(volt < 4200){
    charging = 0;
  }
  else{
    charging = 1;
  }

  if(volt < 3250){
    //battLow();
  }
}

void battLow(){
  display.setTextDatum(TC_DATUM);
  display.fillScreen(TFT_WHITE);
  display.setTextColor(TFT_RED);
  display.drawString("BATTERY LOW", 185,75);
  display.drawString("CHARGE BATTERY", 185,95);
  delay(2000);
  display.fillScreen(TFT_RED);
  display.setTextColor(TFT_WHITE);
  display.drawString("BATTERY LOW", 185,75);
  display.drawString("CHARGE BATTERY", 185,95);
  delay(2000);

  fastPowerDown();
}
