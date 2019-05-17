#include <M5Stack.h>
#include <M5StackUpdater.h>
 
#include "BLEMouse.h"
 
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
}
 
void loop() {
  M5.update();
	if(M5.BtnA.isPressed()) Mouse.sendReport(0,1,1);
}