void init_Sprites(){
  
  spriteBattery.createSprite(SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT);
  spriteBattery.setSwapBytes(true);
  spriteBattery.setTextSize(SPRITE_BATTERY_TEXT_SIZE);
  spriteBattery.setTextFont(SPRITE_BATTERY_FONT);
  
  spriteTime.createSprite(SPRITE_TIME_WIDTH, SPRITE_TIME_HEIGHT);
  spriteTime.setSwapBytes(true);
  spriteTime.setTextSize(SPRITE_TIME_TEXT_SIZE);
  spriteTime.setTextFont(SPRITE_TIME_FONT);

  spriteLora.createSprite(SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT);
  spriteLora.setSwapBytes(true);
  spriteLora.setTextSize(SPRITE_LORA_TEXT_SIZE);
  spriteLora.setTextFont(SPRITE_LORA_FONT);

  spriteGPS.createSprite(SPRITE_GPS_WIDTH, SPRITE_GPS_HEIGHT);
  spriteGPS.setSwapBytes(true);
  spriteGPS.setTextSize(SPRITE_GPS_TEXT_SIZE);
  spriteGPS.setTextFont(SPRITE_GPS_FONT);

  spriteADXL.createSprite(SPRITE_ADXL_WIDTH, SPRITE_ADXL_HEIGHT);
  spriteADXL.setSwapBytes(true);
  spriteADXL.setTextSize(SPRITE_ADXL_TEXT_SIZE);
  spriteADXL.setTextFont(SPRITE_ADXL_FONT);
  spriteADXL.drawString("Fall: NO",0,0);

  spriteSOS.createSprite(SPRITE_SOS_WIDTH, SPRITE_SOS_HEIGHT);
  spriteSOS.setSwapBytes(true);
  spriteSOS.setTextSize(SPRITE_SOS_TEXT_SIZE);
  spriteSOS.setTextFont(SPRITE_SOS_FONT);
}


void init_LCD(){
  display.init();
  display.setRotation(1);
  display.setSwapBytes(true);
  display.fillScreen(TFT_BLACK);  //horiz / vert<> position/dimension
  display.setTextDatum(TC_DATUM);

  //initial Power Check
  getVbatt();
  if(charging==0){
    initPowerCheck();
    display.fillScreen(TFT_BLACK);  //horiz / vert<> position/dimension
  }
  
  //display.setTextFont(1);
  display.setTextSize(2);
  display.setTextColor(TFT_CYAN);
  display.drawString("LORA EXPLORA", 170, 0);
  display.setTextDatum(TL_DATUM);
  init_Sprites();
  updateLCD_Time();
  updateLCD_BattLvl();
  updateLCD_GPS();
  updateLCD_ADXL();
  display.setTextSize(1);
}

void updateLCD_Time(){
  if(timeSynced){  //if(timeSynced){
    if (minute(gpsTime) != minute(myTime)) { //update the display only if the time has changed
        myTime = gpsTime;
        sprintf(currentTime, "%.2d:%.2d", hour(myTime), minute(myTime));
        spriteTime.drawString(currentTime, 0, 0);
    }
  }else{
    spriteTime.drawString("!Sync", 0, 0);
  }

  spriteTime.pushSprite(SPRITE_TIME_X, SPRITE_TIME_Y);
  
}

void updateLCD_BattLvl(){
    getVbatt();
    
    if(!charging){  
      spriteBattery.fillRect(0, 0, SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT, TFT_BLACK);
      spriteBattery.drawString(String(volt / 1000) + "." + String(volt % 1000) + "V", 0, 0);
    }
    else{
        spriteBattery.fillRect(0, 0, SPRITE_BATTERY_WIDTH, SPRITE_BATTERY_HEIGHT, TFT_BLACK);
        progress++;
        if (progress >= 4){
          progress = 0;
          spriteBattery.fillRect(4, 1, 70, 15, TFT_BLACK);
        }
        spriteBattery.drawRoundRect(0, 0, SPRITE_BATTERY_WIDTH-3, SPRITE_BATTERY_HEIGHT, 3, TFT_BLUE);  //rectangle colour
        // progress blocks below here
        for (uint8_t i = 0; i < progress; i++) {
          uint8_t x = i * 23 + 4;  //x location i is increment 20 is a location offset
          spriteBattery.fillRoundRect(x, 2, 22, 13, 3, TFT_GREEN);
        }
      }
  
    spriteBattery.pushSprite(SPRITE_BATT_X, SPRITE_BATT_Y);
}

void updateLCD_GPS(){
  if(locValid){
   spriteGPS.drawString("Latitude: " + String(GPSlat,4),0,0);
   spriteGPS.drawString("Longitude:" + String(GPSlong,4),0,20);
  }else{
   spriteGPS.drawString("Waiting for GPS..." ,0,0);
  }

  spriteGPS.pushSprite(SPRITE_GPS_X, SPRITE_GPS_Y);
}

void updateLCD_ADXL(){
  if((!fallColl)&&(!collDetect)){
   spriteADXL.setTextColor(TFT_WHITE, TFT_BLACK, 0);
   spriteADXL.drawString("NO            ",70,0);
  }else{ 
   spriteADXL.setTextColor(TFT_WHITE, TFT_RED, 1);
   spriteADXL.drawString("DETECTED", 70,0);
  }
  spriteADXL.pushSprite(SPRITE_ADXL_X, SPRITE_ADXL_Y);
}

void updateLCD_SOS(){
  if(!canFall){
   spriteSOS.setTextColor(TFT_YELLOW, TFT_BLACK, 0);
   spriteSOS.drawString("!SOS!",0,0);
   spriteSOS.drawString("SENT",0,15);
  }
  if(canFall){
   spriteSOS.setTextColor(TFT_ORANGE, TFT_BLACK, 0);
   spriteSOS.drawString("SOS",0,0);
   spriteSOS.drawString("CANC",0,15);
  }

  spriteSOS.pushSprite(SPRITE_SOS_X, SPRITE_SOS_Y);
}

void clearLCD_SOS(){
  spriteSOS.fillRect(0,0, SPRITE_SOS_WIDTH, SPRITE_SOS_HEIGHT, TFT_BLACK);
  spriteSOS.pushSprite(SPRITE_SOS_X, SPRITE_SOS_Y);
}
