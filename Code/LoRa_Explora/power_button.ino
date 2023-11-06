/*
  Selective code used and adapted from SparkFun Soft Power Switch Example
  In this version manaul SOS is not yet implemented.
*/

long powerPressedStartTime = 0;

#define POWER_ON_DELAY 1000
#define POWER_DOWN_DELAY 3000
#define DEBOUNCE_DELAY 10
#define BTN_DEBUG 1

//Uncomment the following line to turn on shutdown time measurements
#define PRINT_TIMER_OUTPUT

void initPowerCheck(){
  
  //User has pressed the power button to turn on the system
  //Was it an accidental bump or do they really want to turn on?
  //Let's make sure they continue to press for two seconds
  #if(BTN_DEBUG) 
  comSerial.print("Initial power on check.");
  #endif
  display.print("Initial power on check.");
  powerPressedStartTime = millis();
  while (digitalRead(POWER_BUTTON) == LOW)
  {
    //Wait for user to stop pressing button.
    //What if user has left something heavy pressing the power button?
    //The soft power switch will automatically turn off the system! Handy.
    delay(100);
    if (millis() - powerPressedStartTime > POWER_ON_DELAY)
      break;
    #if(BTN_DEBUG)
    comSerial.print(".");
    #endif
    display.print(".");
  }
  #if(BTN_DEBUG)
  comSerial.println();
  #endif

  if (millis() - powerPressedStartTime < POWER_ON_DELAY)
  {
    #if(BTN_DEBUG) 
    comSerial.println("Power button tap. Returning to off state. Powering down.");
    #endif
    display.drawString("Power button tap. Returning to off state.",0,20);
    display.drawString("Powering down.",0,30);
    fastPowerDown();
  }

  #if(BTN_DEBUG) 
  comSerial.println("User wants to turn system on!");
  #endif
  display.fillScreen(TFT_WHITE);
  display.setTextSize(3);
  display.setTextColor(TFT_DARKCYAN);
  display.drawString("LORA EXPLORA",165,55);
  delay(2000);
  powerPressedStartTime = 0; //Reset var to return to normal 'on' state

  //Here we display something to user indicating system is on and running
  //For example an external display or LED turns on
  //pinMode(STAT_LED, OUTPUT);
  //digitalWrite(STAT_LED, HIGH);
  
}

void btnHandler(){
  if(btnPressed){
    btnDebounce = 1;
    btnPowerDown =1;
    debounceTimer = millis();
    pwrTimer = millis();
    btnPressed = 0;
  }
  if(btnDebounce){
    buttonPress();
  }
  if(btnPowerDown){
    pwrDown();
  }
}

void buttonPress(){
  if(millis() - debounceTimer > DEBOUNCE_DELAY){ //debounce check
    if (digitalRead(POWER_BUTTON) == LOW){
      btnDebounce = 0;
      #if(BTN_DEBUG) 
      comSerial.println("Button Pressed");
      #endif
      fallToggle = 1;
      if(!canFall){
        canFall = 1;
        //collDetect = 0;
        //fallColl = 0;
        #if(BTN_DEBUG) 
        comSerial.println("!SOS Cancelled!");
        #endif
        updateLCD_SOS();
      }
    }
  }
}

void pwrDown(){
  if(millis() - pwrTimer > POWER_DOWN_DELAY){ //power down
    if (digitalRead(POWER_BUTTON) == LOW){
      btnPowerDown = 0;
      #if(BTN_DEBUG) 
      comSerial.println("Power Down");
      #endif
      display.fillScreen(TFT_WHITE);
      display.setTextSize(3);
      display.setTextColor(TFT_MAROON);
      display.drawString("POWER DOWN",90,60);
      delay(2000);
      fastPowerDown();
    }
  }
}

//Immediately power down
void fastPowerDown()  //code from Sparkfun Soft Power Switch Example
{
  pinMode(POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, LOW);

  pinMode(FAST_OFF, OUTPUT);
  digitalWrite(FAST_OFF, LOW);

  powerPressedStartTime = millis();
  #if(BTN_DEBUG)
  comSerial.print("Pulling power line low");
  #endif
  
  while (1)
  {
#ifdef PRINT_TIMER_OUTPUT
    comSerial.println(millis() - powerPressedStartTime);
#endif
    delay(1);
  }
}
