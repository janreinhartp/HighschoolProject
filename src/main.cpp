#include <Arduino.h>
#include "control.h"
#include <ezButton.h>

ezButton sensor2(9);   // create ezButton object that attach to pin 6;
ezButton sensor1(8);   // create ezButton object that attach to pin 7;
ezButton btnStart(13); // create ezButton object that attach to pin 7;

bool sensor1State = false;
bool sensor2State = false;
bool btnStartState = false;

Control conveyor(2);
Control linearDoorGrinder(4);
Control linearDoorDrier(5);
Control linearArmMetal(6);
Control grinder(11);

Control openMetalArmTimer(100);
Control dumpMetalTimer(100);
Control closeMetalArmTimer(100);

Control dumpTimer(100);
Control heaterTimer(100);

char *secondsToHHMMSS(int total_seconds)
{
  int hours, minutes, seconds;

  hours = total_seconds / 3600;         // Divide by number of seconds in an hour
  total_seconds = total_seconds % 3600; // Get the remaining seconds
  minutes = total_seconds / 60;         // Divide by number of seconds in a minute
  seconds = total_seconds % 60;         // Get the remaining seconds

  // Format the output string
  static char hhmmss_str[7]; // 6 characters for HHMMSS + 1 for null terminator
  sprintf(hhmmss_str, "%02d%02d%02d", hours, minutes, seconds);
  return hhmmss_str;
}
int parametersTimer[6] = {8, 8, 8, 5, 60};

void setTimers()
{
  openMetalArmTimer.setTimer(secondsToHHMMSS(parametersTimer[0]));
  dumpMetalTimer.setTimer(secondsToHHMMSS(parametersTimer[1]));
  closeMetalArmTimer.setTimer(secondsToHHMMSS(parametersTimer[2]));
  dumpTimer.setTimer(secondsToHHMMSS(parametersTimer[3]));
  heaterTimer.setTimer(secondsToHHMMSS(parametersTimer[4]));
  grinder.setTimer(secondsToHHMMSS(60));
}

void runSensors()
{
  sensor1.loop();
  sensor2.loop();
  btnStart.loop();

  sensor1State = !sensor1.getState();
  sensor2State = !sensor2.getState();
  btnStartState = btnStart.getState();
  // Serial.print("Sensor 1: ");
  // Serial.println(sensor1State);
  // Serial.print("Sensor 2: ");
  // Serial.println(sensor2State);
}

void stopAll()
{
  conveyor.stop();
  linearDoorGrinder.stop();
  linearDoorDrier.stop();
  linearArmMetal.stop();
  grinder.stop();

  openMetalArmTimer.stop();
  dumpMetalTimer.stop();
  closeMetalArmTimer.stop();

  dumpTimer.stop();
  heaterTimer.stop();
}
bool runAutoFlag = false;
int runAutoStatus = 0;
/*
runAutoStatus
1 = conveyor run while waiting for sensors
  if metal sensor is sense jump to 2
  if sensor 2 is sense jump to

2 = conveyor stop and open linear arm metal until said time
3 = conveyor run while linear arm metal is on until said time
4 = conveyor stop and close linear arm metal until said time

5 = continue motor run until said time
6 = close the heater lid and simulatate heater until said time
7 = run the grinder and open the grinder door until said time

8 = reset all motors
*/

void runAuto()
{
  switch (runAutoStatus)
  {
  case 1:
    if (sensor1State == true)
    {
      conveyor.relayOff();
      openMetalArmTimer.start();
      runAutoStatus = 2;
      Serial.println("Metal Sensor Triggered");
    }

    if (sensor2State == true)
    {
      conveyor.relayOff();
      dumpTimer.start();
      runAutoStatus = 5;
      Serial.println("Material Sensor Triggered");
    }

    conveyor.relayOn();

    break;

  case 2:
    openMetalArmTimer.run();
    if (openMetalArmTimer.isTimerCompleted() == true)
    {
      runAutoStatus = 3;
      dumpMetalTimer.start();
      Serial.println("Open Metal Arm Done");
    }
    else
    {
      conveyor.relayOff();
      linearArmMetal.relayOn();
    }
    break;

  case 3:
    dumpMetalTimer.run();
    if (dumpMetalTimer.isTimerCompleted() == true)
    {
      Serial.println("Dump Metal Done");
      conveyor.relayOff();
      closeMetalArmTimer.start();
      runAutoStatus = 4;
    }
    else
    {

      conveyor.relayOn();
    }
    break;

  case 4:
    closeMetalArmTimer.run();
    if (closeMetalArmTimer.isTimerCompleted() == true)
    {
      Serial.println("Metal Arm Closing Done");
      runAutoStatus = 1;
    }
    else
    {
      linearArmMetal.relayOff();
    }
    break;

  case 5:
    dumpTimer.run();
    if (dumpTimer.isTimerCompleted() == true)
    {
      Serial.println("Move to Heater Done");
      conveyor.relayOff();
      heaterTimer.start();
      runAutoStatus = 6;
      Serial.println("Heater Start");
    }
    else
    {
      linearDoorDrier.relayOff();
    }
    break;

  case 6:
    heaterTimer.run();
    if (heaterTimer.isTimerCompleted() == true)
    {
      Serial.println("Heater Done");
      grinder.start();
      runAutoStatus = 7;
      Serial.println("Grinder Start");
    }
    else
    {
      linearDoorDrier.relayOn();
    }
    break;

  case 7:
    grinder.run();
    if (grinder.isTimerCompleted() == true)
    {
      Serial.println("Grinder Done");
      runAutoStatus = 8;
    }
    else
    {
      linearDoorGrinder.relayOn();
    }
    break;

  case 8:
    stopAll();
    runAutoFlag = false;
    runAutoStatus = 0;
    break;

  default:
    break;
  }
}

void readButtons()
{
  if (runAutoFlag == true)
  {
    if (btnStart.isReleased())
    {
      runAutoFlag = false;
      stopAll();
      runAutoStatus = 0;
    }
  }
  else
  {
    if (btnStart.isReleased())
    {
      runAutoFlag = true;
      runAutoStatus = 1;
    }
  }
}

void setup()
{
  setTimers();
  sensor2.setDebounceTime(50);
  sensor1.setDebounceTime(50);
  btnStart.setDebounceTime(100);
  Serial.begin(9600);
}
unsigned long currentMillisRunAuto;
unsigned long previousMillisRunAuto;
unsigned long intervalRunAuto = 1000;

void printRunAutoStatus()
{
  Serial.print("Run Auto Flag : ");
  Serial.println(runAutoFlag);
  Serial.print("Run Auto Status : ");
  Serial.println(runAutoStatus);
  switch (runAutoStatus)
  {
  case 1:
    Serial.println("Waiting for Sensor");
    break;
  case 2:
    Serial.print("Open Metal Arm - ");
    Serial.println(openMetalArmTimer.getTimeRemaining());
    break;
  case 3:
    Serial.print("Dump Metal - ");
    Serial.println(dumpMetalTimer.getTimeRemaining());
    break;
  case 4:
    Serial.print("Close Metal Arm- ");
    Serial.println(closeMetalArmTimer.getTimeRemaining());
    break;
  case 5:
    Serial.print("Dump to Heater- ");
    Serial.println(dumpMetalTimer.getTimeRemaining());
    break;
  case 6:
    Serial.print("Drying - ");
    Serial.println(heaterTimer.getTimeRemaining());
    break;
  case 7:
    Serial.print("Grinding - ");
    Serial.println(grinder.getTimeRemaining());
    break;
  default:
    break;
  }
}

void loop()
{
  runSensors();
  readButtons();
  if (runAutoFlag == true)
  {
    runAuto();
  }

  unsigned long currentMillisRunAuto = millis();
  if (currentMillisRunAuto - previousMillisRunAuto >= intervalRunAuto)
  {
    previousMillisRunAuto = currentMillisRunAuto;
    printRunAutoStatus();
  }
}