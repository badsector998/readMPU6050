#include<Wire.h>

#define imuRegs 0x68
#define powerRegs 0x6B
#define accRegsX 0x3B
#define accRegsY 0x3D
#define accRegsZ 0x3F
#define gyrRegsX 0x43
#define gyrRegsY 0x45
#define gyrRegsZ 0x47
#define accConfig 0x1B
#define gyrConfig 0x1C

int16_t accRawX,accRawY,accRawZ,gyrRawX,gyrRawY,gyrRawZ;
float accX,accY,accZ,gyrX,gyrY,gyrZ;
float acc_filtered_X,acc_filtered_Y,acc_filtered_Z;
float gyr_filtered_X,gyr_filtered_Y,gyr_filtered_Z;
float accErrorX=0,accErrorY=0;
float gyrErrorX=0,gyrErrorY=0,gyrErrorZ=0;
double time, elapsedTime, timePrev;
float filteredX,filteredY,filteredZ;

void setupSensor(){
  Wire.beginTransmission(imuRegs);
  Wire.write(powerRegs);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(imuRegs);
  Wire.write(accConfig);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.beginTransmission(imuRegs);
  Wire.write(gyrConfig);
  Wire.write(0x00);
  Wire.endTransmission();
}

float readThis(unsigned long int reg){
  int16_t val;
  Wire.beginTransmission(imuRegs);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(imuRegs,2,true);
  val = Wire.read()<<8|Wire.read();
  return val;
}

void calibrateSensor(){
  for(int i=0;i<200;i++){
    accRawX = readThis(accRegsX);
    accRawY = readThis(accRegsY);
    accErrorX = (accErrorX + accRawX)/16384.0;
    accErrorY = (accErrorY + accRawY)/16384.0;
  }
  accErrorX = accErrorX/200.0;
  accErrorY = accErrorY/200.0;

  for(int i=0;i<200;i++){
    gyrRawX = readThis(gyrRegsX)/131.0;
    gyrRawY = readThis(gyrRegsY)/131.0;
    gyrRawZ = readThis(gyrRegsZ)/131.0;
    gyrErrorX = gyrErrorX + gyrRawX;
    gyrErrorY = gyrErrorY + gyrRawY;
    gyrErrorZ = gyrErrorZ + gyrRawZ;
  }
  gyrErrorX = gyrErrorX/200.0;
  gyrErrorY = gyrErrorY/200.0;
  gyrErrorZ = gyrErrorZ/200.0;
  delay(5000);
}

void showCalibrationVal(){
  Serial.println("Initiation value : ");
  Serial.print("Acc X :");
  Serial.print(accErrorX);
  Serial.print("  |Acc Y :");
  Serial.println(accErrorY);
  Serial.print("Gyr X :");
  Serial.print(gyrErrorX);
  Serial.print("  |Gyr Y :");
  Serial.print(gyrErrorY);
  Serial.print("  |Gyr Z :");
  Serial.println(gyrErrorZ);
}

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  setupSensor();
  calibrateSensor();
  Serial.begin(9600);
  showCalibrationVal();
  delay(5000);

  time = millis();
}

void loop() {
  // put your main code here, to run repeatedly:

  timePrev = time;
  time = millis();
  elapsedTime = (time - timePrev) / 1000;
  
  accRawX = readThis(accRegsX);
  accRawY = readThis(accRegsY);
  accRawZ = readThis(accRegsZ);
  accX = (accRawX/16384.0) - accErrorX;
  accY = (accRawY/16384.0) - accErrorY;
  accZ = (accRawZ/16384.0);
  gyrRawX = readThis(gyrRegsX);
  gyrRawY = readThis(gyrRegsY);
  gyrRawZ = readThis(gyrRegsZ);
  gyrX = (gyrRawX/131.0) - gyrErrorX;
  gyrY = (gyrRawY/131.0) - gyrErrorY;
  gyrZ = (gyrRawZ/131.0) - gyrErrorZ;
//  Serial.print("Acc X :");
//  Serial.print(accX);
//  Serial.print("  |Y :");
//  Serial.print(accY);
//  Serial.print("  |Z :");
//  Serial.println(accZ);
//  Serial.print("Gyr X :");
//  Serial.print(gyrRawX);
//  Serial.print("  |Y :");
//  Serial.print(gyrRawY);
//  Serial.print("  |Z :");
//  Serial.println(gyrRawZ);

  acc_filtered_X = (180/3.141592) * atan(accX/sqrt(sq(accY)+sq(accZ)));
  acc_filtered_Y = (180/3.141692) * atan(accY/sqrt(sq(accX)+sq(accZ)));
  acc_filtered_Z = (180/3.141592) * atan(sqrt(sq(accY)+sq(accX))/accZ);
//
//  Serial.print("New Angular Value X: ");
//  Serial.print(acc_filtered_X);
//  Serial.print(" |Y : ");
//  Serial.print(acc_filtered_Y);
//  Serial.print(" |Z : ");
//  Serial.println(acc_filtered_Z);

  gyr_filtered_X = gyrX + (gyrX * elapsedTime);
  gyr_filtered_Y = gyrY + (gyrY * elapsedTime);
  gyr_filtered_Z = gyrZ + (gyrZ * elapsedTime);

//  Serial.print("New Gyro Value X: ");
//  Serial.print(gyr_filtered_X);
//  Serial.print(" |Y : ");
//  Serial.print(gyr_filtered_Y);
//  Serial.print(" |Z : ");
//  Serial.println(gyr_filtered_Z);

  filteredX = (0.04 * gyr_filtered_X) + (0.96 * acc_filtered_X);
  filteredY = (0.04 * gyr_filtered_Y) + (0.96 * acc_filtered_Y);
  filteredZ = (0.04 * gyr_filtered_Z) + (0.96 * acc_filtered_Z);

  Serial.print("New Filtered Value X: ");
  Serial.print(filteredX);
  Serial.print(" |Y : ");
  Serial.print(filteredY);
  Serial.print(" |Z : ");
  Serial.println(filteredZ);

}
