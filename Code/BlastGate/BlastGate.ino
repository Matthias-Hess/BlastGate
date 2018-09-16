#include <Servo.h>
#include "EmonLib.h"

EnergyMonitor emon1;

Servo servo;

enum status
{
  automatic,
  off, 
  on,
  error
};

const int maxTimeForGateOperation = 4000;
const int pin_current_sensor   =  0;
const int pin_openSwitch       =  2;
const int pin_closeSwitch      =  3;
const int pin_button           =  4;
const int pin_servo            =  5;
const int pin_relais           =  6;
const int pin_led_blue         =  9;
const int pin_led_green        = 10;
const int pin_led_red          = 11;

int lastServoSpeed   =-1;
status theStatus;

void setup()
{
  theStatus = automatic;  
  emon1.current ( pin_current_sensor, 111.1);             // Current: input pin, calibration.
  pinMode(pin_openSwitch, INPUT_PULLUP);
  pinMode(pin_closeSwitch, INPUT_PULLUP);
  pinMode(pin_button, INPUT_PULLUP);
  servo.attach(pin_servo);
  pinMode(pin_relais, OUTPUT);
  pinMode(pin_led_blue, OUTPUT);
  pinMode(pin_led_green, OUTPUT);
  pinMode(pin_led_red, OUTPUT);
  Serial.begin(9600);
}

void loop()
{  
  handleButton();
  setLed();
  
  switch(theStatus)
  {
    case automatic:
      if(isMachineRunning())
        closeGate();
      else
        openGate();
      break;
      
    case off:
      closeGate();
      break;
  
    case on:
      openGate();
      break;
  }
}

void closeGate()
{
  unsigned long startTime;
  startTime=millis();
  
  while(!isGateFullyClosed() && millis() < (startTime + maxTimeForGateOperation))
  {
    servoClose();
    if (handleButton())
    {
      servoStop();
      return;
    }
  }
  
  servoStop();

  if(millis() > (startTime + maxTimeForGateOperation))
  {
    theStatus=error;
    return;
  }
}

void openGate()
{
  unsigned long startTime;
  startTime=millis();
  
  while(!isGateFullyOpen() && millis() < (startTime + maxTimeForGateOperation))
  {
    servoOpen();
    if (handleButton())
    {
      servoStop();
      return;
    }
  }
  
  servoStop();

  if(millis() > (startTime + maxTimeForGateOperation))
  {
    theStatus=error;
    return;
  }
}

bool handleButton()
{
  if(isButtonPressed())
  {
    theStatus = toggle(theStatus);
    return true;
  }
  return false;
}

bool isButtonPressed()
{
  if(digitalRead(pin_button) == HIGH)
  {
    return false;
  }

  while(digitalRead(pin_button) == LOW)
  {
    // wait until user releases button
    delay(50);
  }

  return true;
}

status toggle(status theStatus)
{
  switch(theStatus)
  {
    case automatic:
      return off;
      break;
      
    case off:
      return on;
      break;
  
    case on:
      return automatic;
      break;

    case error:
      return automatic;
      break;
  }
}

bool isMachineRunning()
{
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only
  Serial.println(Irms);             // Irms

  bool isRunning= (Irms > 15);
  if(isRunning)
    Serial.print("Is Running");
    else
    Serial.print("NOT Running");

    return isRunning;
}

bool isGateFullyOpen()
{
  return (digitalRead(pin_openSwitch) == LOW);
}

bool isGateFullyClosed()
{
  return (digitalRead(pin_closeSwitch) == LOW);
}

void turnDustExtractorOn()
{
  digitalWrite(pin_relais, HIGH);
}

void turnDustExtractorOff()
{
  digitalWrite(pin_relais, LOW);
}


void servoOpen()
{
    servo.write(180);
}

void servoStop()
{
    servo.write(90);
    delay(100); // wait for current to settle
}

void servoClose()
{
    servo.write(0);
}

void setLed()
{
  int r;
  int g;
  int b;

  switch(theStatus)
  {
    case off:
      r=HIGH;
      g=LOW;
      b=LOW;
      break;

    case on:
      r=LOW;
      g=HIGH;
      b=LOW;
      break;

    case automatic:
      r=LOW;
      g=LOW;
      b=HIGH;
      break;

    case error:
      r=HIGH;
      g=HIGH;
      b=LOW;
      break;
  }
  
  digitalWrite(pin_led_red,   r);
  digitalWrite(pin_led_green, g);
  digitalWrite(pin_led_blue,  b);  
}



