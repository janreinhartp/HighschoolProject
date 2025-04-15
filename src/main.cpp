#include <Arduino.h>
#include "control.h"
#include <ezButton.h>

ezButton sensor2(9); // create ezButton object that attach to pin 6;
ezButton sensor1(8); // create ezButton object that attach to pin 7;

Control conveyor(2);
Control linearDoorGrinder(4);
Control linearDoorDrier(5);
Control linearArmMetal(6);
Control grinder(11);

void runSensors()
{
  sensor1.loop();
  sensor2.loop();
}

void setup()
{
}

void loop()
{
}

void codeSnipper()
{
  int btn1State = button1.getState();
  int btn2State = button2.getState();
  int btn3State = button3.getState();
  Serial.print("button 1 state: ");
  Serial.println(btn1State);
  Serial.print("button 2 state: ");
  Serial.println(btn2State);
  Serial.print("button 3 state: ");
  Serial.println(btn3State);

  if (button1.isPressed())
    Serial.println("The button 1 is pressed");

  if (button1.isReleased())
    Serial.println("The button 1 is released");

  if (button2.isPressed())
    Serial.println("The button 2 is pressed");

  if (button2.isReleased())
    Serial.println("The button 2 is released");

  if (button3.isPressed())
    Serial.println("The button 3 is pressed");

  if (button3.isReleased())
    Serial.println("The button 3 is released");
}
