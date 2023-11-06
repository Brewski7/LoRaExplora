void readLora(){
  while (loraSerial.available()) {
        comSerial.write(loraSerial.read());
  }
}

void sendCmdLora(){
  while (comSerial.available()) {
        loraSerial.write(comSerial.read());
  }
}

void handleLoraSerial()                 //https://forum.arduino.cc/t/reading-strings-through-uart/164480/8
{
    const int LORA_BUFF_SIZE = 70; // make it big enough to hold your longest command
    static char buffer[LORA_BUFF_SIZE+1]; // +1 allows space for the null terminator
    static int length = 0; // number of characters currently in the buffer

    if(loraSerial.available())
    {
        char c = loraSerial.read();
        if((c == '\r') || (c == '\n'))
        {
            // end-of-line received
            if(length > 0)
            {
                handleLoraMessage(buffer);
            }
            length = 0;
        }
        else
        {
            if(length < LORA_BUFF_SIZE)
            {
                buffer[length++] = c; // append the received character to the array
                buffer[length] = 0; // append the null terminator
            }
            else
            {
                // buffer full - discard the received character
                comSerial.println("LoRa buffer full, byte discarded");
            }
        }
    }
}

void handleLoraMessage(char *msg)   //https://forum.arduino.cc/t/reading-strings-through-uart/164480/8
{
  comSerial.write(msg);
  comSerial.println();
  
//  if(strncmp(msg, "+CJOIN:FAIL", 11) == 0)
//  {
//    comSerial.println("Test Worked");
//  }
  if(strncmp(msg, "Joined", 6) == 0)
  {
    nwkJoined = 1;
    joinFailed = 0;
  }
  if(strncmp(msg, "+CJOIN:FAIL", 11) == 0)
  {
    joinFailed = 1;
    nwkJoined = 0;
  }
  else if(strncmp(msg, "OK+SEND", 7) == 0)
  {
    msgSending = 1;
  }
  else if(strncmp(msg, "OK+SENT", 7) == 0)
  {
    msgSent = 1;
    msgSending = 0;
    readySend = 1;
  }
  else if(strncmp(msg, "ERR+SEND", 8) == 0)
  {
    readySend = 1;
  }
  
  
  
}

void compileMsg(){

  if((GPSlat < 0)&&(GPSlong < 0)){
      neg = 3;   //both coordinates is negative
  }else if(GPSlat < 0){
      neg = 1;     //latitude is negative
  }else if(GPSlong < 0){
      neg = 2;    //longitude is negative
  }

  //fall = 1;
  negFall[0] = fallDetect << 4 | neg;
  

  int32_t sentGPSlat = sqrt(pow((GPSlat * 10000), 2));   //unsigned integers used as bit shifting performed not suitable for negative ints.
  int32_t sentGPSlong = sqrt(pow((GPSlong * 10000),2));
  uint8_t bytesLat[4];
  uint8_t bytesLong[4];
  
  bytesLat[0] = (sentGPSlat >> 16) & 0xFF;
  bytesLat[1] = (sentGPSlat >> 8) & 0xFF;
  bytesLat[2] = (sentGPSlat) & 0xFF;

  bytesLong[0] = (sentGPSlong >> 16) & 0xFF;
  bytesLong[1] = (sentGPSlong >> 8) & 0xFF;
  bytesLong[2] = (sentGPSlong) & 0xFF;

  strcpy((char*)loraProt, (char*)bytesLat);
  strcpy((char*)loraProt + 3, (char*)bytesLong);
  strcpy((char*)loraProt + 6, (char*)negFall);

}

void sendLora(){
  compileMsg();
  char msg[32];
  sprintf(msg,"AT+DTRX=%d,%d,%d,%.2X%.2X%.2X%.2X%.2X%.2X%.2X\n", ack, n_trails, msg_len, loraProt[0], loraProt[1], loraProt[2], loraProt[3], loraProt[4], loraProt[5], loraProt[6]);
  comSerial.print(msg);
  loraSerial.write(msg);
}

void updateLora(){
  #if periodic
    if(readySend && locValid){
    //#if periodic
      if(millis() > waitTimer + msg_time){
        comSerial.println("Sending Message");
        //loraSerial.write("AT+DTRX=1,1,14,052D7202E09501\n");
        sendLora();
        waitTimer = millis();
      }
    }
  #else 
    sendCmdLora(); 
  #endif  

//  const char *test = "HELLO WORLD\n";
//  comSerial.write(test);
//  comSerial.println();
//
//  if(strncmp(test, "HELLO", 5) == 0){
//    comSerial.println("Test Worked");
//  }
  
  //readLora();
  handleLoraSerial();

  if(initNwkJoin){
    spriteLora.fillRect(0,0, SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT, TFT_BLACK);
    spriteLora.drawString("JOINING NETWORK", 0, 0);
    initNwkJoin = 0;
  }

  if(nwkJoined){
    comSerial.println("Joined Network Successfully");
    spriteLora.fillRect(0,0, SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT, TFT_BLACK);
    spriteLora.drawString("NETWORK JOINED", 0, 0);
    readySend = 1;
    nwkJoined = 0;
    waitTimer = millis();
  }

  if(joinFailed){
    comSerial.println("Network Join Failed. Retry in 10s");
    spriteLora.fillRect(0,0, SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT, TFT_BLACK);
    spriteLora.drawString("NETWORK JOIN FAILED. RETRY IN 10s ", 0, 0);
    joinFailed = 0;
    retryJoin = 1;
    waitTimer = millis();
  }

  if(retryJoin && do_retry_join){
     if(millis() > waitTimer + 10000){
      comSerial.println("Retry to Join Network");
      spriteLora.fillRect(0,0, SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT, TFT_BLACK);
      spriteLora.drawString("RETRY JOINING NETWORK ", 0, 0);
      comSerial.println("AT+CJOIN=1,1,10,3");
      loraSerial.write("AT+CJOIN=1,1,10,3\n");
      waitTimer = millis();
      retryJoin = 0;
    }
  }
  
  if(msgSent){
    comSerial.println("Message Sent Successfully");
    spriteLora.fillRect(0,0, SPRITE_LORA_WIDTH, SPRITE_LORA_HEIGHT, TFT_BLACK);
    spriteLora.drawString("MESSAGE SENT SUCCESSFULLY " + String(currentTime), 0, 0);
    msgSent = 0;
    msgCnt++;
  }

  spriteLora.pushSprite(SPRITE_LORA_X, SPRITE_LORA_Y);
  
}
