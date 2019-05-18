#include <M5Stack.h>
#include <M5StackUpdater.h>
#include "utility/MPU9250.h"
 
#include "BLEMouse.h"

MPU9250 IMU;
 
void setup() {
  M5.begin();
  Wire.begin();

  if(digitalRead(BUTTON_A_PIN) == 0) {
    Serial.println("Will Load menu binary");
    updateFromFS(SD);
    ESP.restart();
  }

  Serial.begin(9600);
  Mouse.begin();
  if (0x71 != IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250)) {
    M5.Lcd.println("MPU9250 not connected...");
    delay(2000);
  }
  IMU.initMPU9250();
  if ( 0x48 != IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963)) {
      M5.Lcd.println("AK8963 not connected...");
      delay(2000);
    }
  IMU.initAK8963(IMU.magCalibration);

  if(digitalRead(BUTTON_C_PIN) == 0) {
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);
  }
}
 
void loop() {
  //delay(16);
  //if(!Mouse.isConnected) return;
  M5.update();
	// if(M5.BtnA.isPressed()) {
  //   M5.Lcd.print("Left");
  //   Mouse.sendReport(1,0,0);//Left Click, 1, 1
  // }
  // else if(M5.BtnA.wasReleased()) {
  //   M5.Lcd.print("Released");
  //   Mouse.sendReport(0,0,0);
  // }
  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {
    IMU.readGyroData(IMU.gyroCount);
    IMU.getGres();
    IMU.gx = (float)IMU.gyroCount[0]*IMU.gRes;
    IMU.gy = (float)IMU.gyroCount[1]*IMU.gRes;
    IMU.gz = (float)IMU.gyroCount[2]*IMU.gRes;
    Mouse.sendReport(M5.BtnA.isPressed(),-(int8_t)(IMU.gy*0.6),-(int8_t)(IMU.gx * 0.6));
    //Serial.printf("%d %d %d \n",(int8_t)IMU.gx,(int8_t)IMU.gy,(int8_t)IMU.gz);
  }
}