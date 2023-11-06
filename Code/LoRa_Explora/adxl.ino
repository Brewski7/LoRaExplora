#define ADXL_CALIBRATE 0
#define ADXL_DEBUG 0

#define LINEAR_FALL_TH  0.2
#define COLLISION_TH    2.5
#define MAJ_COLLISION_TH    5

//Analog read pins
const int xPin = AXL_X;
const int yPin = AXL_Y;
const int zPin = AXL_Z;

//values from specsheet, used for raw data before calibration
float xBias = 1.5;   //V 1.61, 1.64, 1.65
float yBias = 1.5;
float zBias = 1.5;   
float xSens = 0.3;   //mV/g
float ySens = 0.3;
float zSens = 0.3;

//calibration terms
float biasVector[3] = {0.359769, 0.366575, 0.390013};
float calibParam[3][3] = {{ 0.893905, -0.007971, 0.018122},
                          {-0.007971,  0.886442, 0.003972},
                          { 0.018122,  0.003972, 0.914830}};

char buffer[50];
//Declared globally in main.ino
//uint8_t fallDetect = 0;
//uint8_t fallColl = 0;
//uint8_t collDetect = 0;

unsigned long fallTimer;
unsigned long collTimer;


void fallDetection(){

  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  //print the read values - WJ
  #if(ADXL_DEBUG==1)
    sprintf(buffer, "Analog#  x: %d | y: %d | z: %d", xRead, yRead, zRead);
    comSerial.println(buffer);
  #endif

  //Voltage calculation - WJ
  //12 Bit ADC
  float xV = (xRead*3.3)/pow(2,12);
  float yV = (yRead*3.3)/pow(2,12);
  float zV = (zRead*3.3)/pow(2,12);

  //print the caluated voltages
  #if(ADXL_DEBUG==1)
    sprintf(buffer,"Voltage# x: %.5f | y: %.5f | z: %.5f", xV, yV, zV); 
    comSerial.println(buffer);
  #endif

  //ZERO g BIAS LEVEL
  xV -= xBias;
  yV -= yBias;
  zV -= zBias;

  #if(ADXL_DEBUG==1)
    sprintf(buffer,"ZeroV#   x: %.5f | y: %.5f | z: %.5f", xV, yV, zV); 
    comSerial.println(buffer);
  #endif

  //g calculation from SENSITIVITY
  float xG = xV/xSens;
  float yG = yV/ySens;
  float zG = zV/zSens;

  #if(ADXL_DEBUG==1)
    sprintf(buffer,"GForce#  x: %.5f | y: %.5f | z: %.5f\n", xG, yG, zG); 
    comSerial.println(buffer);
  #endif

  //for callibration
  #if(ADXL_CALIBRATE)
  sprintf(buffer,"%.6f, %.6f, %.6f", xG, yG, zG); 
  comSerial.println(buffer);
  #endif

  //involved calibration done with Li’s ellipsoid algorithm [A^-1 * (valVector - biasVector)]
  float valVector[3] = {0.0, 0.0, 0.0};
  float biasCorrection[3] = {(xG - biasVector[0]), (yG - biasVector[1]), (zG - biasVector[2])};
  
  for(int i = 0; i < 3; i++){
    for(int j = 0; j < 3; j++){
      valVector[i] += (calibParam[i][j] * biasCorrection[j]);
    }
  }

  float x_cal = valVector[0];
  float y_cal = valVector[1];
  float z_cal = valVector[2];

  #if((ADXL_DEBUG == 1)||(ADXL_DEBUG == 2))
    sprintf(buffer,"Calibrated G x: %.5f | y: %.5f | z: %.5f\n", x_cal, y_cal, z_cal); 
    comSerial.println(buffer);
  #endif

  if((fallDetect)&&(millis()>fallTimer + 3000)){
    fallDetect = 0;
    fallColl = 0;
  }
  if ((!fallDetect)&&(abs(abs(x_cal) + abs(y_cal) + abs(z_cal)) < LINEAR_FALL_TH)){
    fallDetect = 1;
    fallTimer = millis();
    comSerial.println("\n\n\nFREEFALL DETECTED!!!\n\n");
  }
  if ((!fallColl)&&(fallDetect)&&((abs(x_cal)>COLLISION_TH)||(abs(y_cal)>COLLISION_TH)||(abs(z_cal)>COLLISION_TH))){
    fallColl = 1;
    fallToggle = 1;
    canFall = 0;
    canFallTimer = millis();
    comSerial.println("\n\n\nFALL COLLISION DETECTED!!!\n\n");
  }

  if((collDetect)&&(millis() > collTimer + 3000)){
    collDetect = 0;
  }
  if ((!collDetect)&&((abs(x_cal)>MAJ_COLLISION_TH)||(abs(y_cal)>MAJ_COLLISION_TH)||(abs(z_cal)>MAJ_COLLISION_TH))){
    collDetect = 1;
    fallToggle = 1;
    canFall = 0;
    collTimer = millis();
    canFallTimer = millis();
    comSerial.println("\n\n\nMAJOR COLLISION DETECTED!!!\n\n");
  }

  

  if((!canFall)&&(millis() > canFallTimer + FALL_CAN_DELAY)){
    comSerial.println("!!!SENDING SOS!!!");
    updateLCD_SOS();
    canFall = 1;
  }

  #if((ADXL_DEBUG == 1)||(ADXL_DEBUG == 2))
  delay(50);//just here to slow down the serial output - Easier to read
  #endif
}
