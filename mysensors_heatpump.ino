#include <MySensor.h>
#include <SPI.h>
#include <Arduino.h>
#include "LedPixels.h"
#include <Bounce2.h>

#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>

MySensor gw;
MyMessage powerMsg(POWER_ID, V_STATUS);
MyMessage modeMsg(MODE_ID, V_HVAC_FLOW_STATE);
MyMessage fanMsg(FAN_ID, V_PERCENTAGE);
MyMessage tempMsg(TEMP_ID, V_TEMP);
MyMessage vdirMsg(VDIR_ID, V_VAR1);
MyMessage hdirMsg(HDIR_ID, V_VAR2);
MyMessage strengthMsg(STRENGTH_ID, V_STATUS);

IRSenderPWM irSender(3);       // IR led on Arduino digital pin 3, using Arduino PWM

HeatpumpIR *heatpumpIR = new PanasonicNKEHeatpumpIR();

// Instantiate a Bounce object
Bounce debouncer = Bounce();

//Some global variables to hold the states
int POWER_STATE;
int TEMP_STATE;
int FAN_STATE;
int MODE_STATE;
int VDIR_STATE;
int HDIR_STATE;
bool STRENGTH_STATE;


void loop() {
  debouncer.update();
  int value = debouncer.read();
  // Turn on or off the LED as determined by the state :
  if ( value == HIGH) {
    setLed(blue);
    sendHeatpumpCommand();
    turnOffLeds();
    gw.wait(1000);
  }
  gw.process();
}

void handlePowerMessage(bool newState) {
  if (newState) {
    POWER_STATE = POWER_ON;
  }
  else {
    POWER_STATE = POWER_OFF;
  }
  gw.saveState(POWER_ID, newState);
}

void handleModeMessge(int newMode) {
  switch(newMode) {
    case 0:
      MODE_STATE = MODE_HEAT; break;
    case 1:
      MODE_STATE = MODE_COOL; break;
    case 2:
      MODE_STATE = MODE_AUTO; break;
    case 3:
      MODE_STATE = MODE_FAN; break;
     case 4:
      MODE_STATE = MODE_DRY; break;
  }
  MODE_STATE = newMode;
  gw.saveState(MODE_ID, newMode);
}

void handleFanMessage(int newFan) {
  if (newFan > 5) newFan=5;
  switch(newFan) {
//    case -1:
//      FAN_STATE = FAN_OFF; break;
    case 0:
      FAN_STATE = FAN_AUTO; break;
    case 1:
      FAN_STATE = FAN_1; break;
    case 2:
      FAN_STATE = FAN_2; break;
    case 3:
      FAN_STATE = FAN_3; break;
    case 4:
      FAN_STATE = FAN_4; break;
    case 5:
      FAN_STATE = FAN_5; break;
    default:
      FAN_STATE = FAN_AUTO; break;
  }
  FAN_STATE = newFan;
  gw.saveState(FAN_ID, newFan);
}

void handleTempMessage(int newTemp) {
  TEMP_STATE = newTemp;
  gw.saveState(TEMP_ID, newTemp);
}

void handleVdirMessage(int newVDir) {
  //TODO
}

void handleHDirMessage(int newHDir) {
  // TODO
}

void handleStrengthMessage(bool newStrength) {
  STRENGTH_STATE = newStrength;
//  gw.saveState(newStrength);
}

void sendHeatpumpCommand() {
  Serial.println("Power = " + (String)POWER_STATE);
  Serial.println("Mode = " + (String)MODE_STATE);
  Serial.println("Fan = " + (String)FAN_STATE);
  Serial.println("Temp = " + (String)TEMP_STATE);

  boolean powerful; boolean quiet;
  if (STRENGTH_STATE) {
   powerful = true;
   quiet = false;
  }
  else {
    powerful= false;
    quiet = true;
  }

  heatpumpIR->send(irSender, POWER_STATE, MODE_STATE, FAN_STATE, TEMP_STATE, VDIR_AUTO, HDIR_AUTO);
}

void incomingMessage(const MyMessage &message) {
  setLed(magenta);
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }
   Serial.print("Incoming change for sensor:");
   Serial.print(message.sensor);
   Serial.print(", New status: ");
   Serial.println(message.getBool());

   switch(message.sensor) {
    case POWER_ID: {
      bool newState = message.getBool();
      handlePowerMessage(newState);
      break;
    }
    case MODE_ID: {
      int newMode = message.getInt();
      handleModeMessge(newMode);
      break;
    }
    case FAN_ID: {
      int newFan = message.getInt();
      handleFanMessage(newFan);
      break;
    }
    case TEMP_ID: {
      int newTemp = message.getInt();
      handleTempMessage(newTemp);
      break;
    }
    case VDIR_ID: {
      int newVDir = message.getInt();
      handleVdirMessage(newVDir);
      break;
    }
    case HDIR_ID: {
      int newHDir = message.getInt();
      handleHDirMessage(newHDir);
      break;
    }
    case STRENGTH_ID: {
      int newStrength = message.getBool();
      handleStrengthMessage(newStrength);
      break;
    }
   }
  sendHeatpumpCommand();
  turnOffLeds();
}


void setup()  {

  debouncer.attach(PUSHBUTTON_PIN);

  setupLeds();
  turnOffLeds();

  gw.begin(incomingMessage, AUTO, false);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Heatpump", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(POWER_ID, S_BINARY);
  gw.present(MODE_ID, S_HVAC);
  gw.present(FAN_ID, S_HVAC);
  gw.present(TEMP_ID, S_HVAC);
  gw.present(VDIR_ID, S_CUSTOM);
  gw.present(HDIR_ID, S_CUSTOM);
  gw.present(STRENGTH_ID, S_BINARY);

  // Load our values on start
  POWER_STATE = gw.loadState(POWER_ID);
  TEMP_STATE = gw.loadState(TEMP_ID);
  FAN_STATE = gw.loadState(FAN_ID);
  MODE_STATE = gw.loadState(MODE_ID);
  HDIR_STATE = gw.loadState(HDIR_STATE);
  VDIR_STATE = gw.loadState(VDIR_STATE);
  STRENGTH_STATE = gw.loadState(STRENGTH_ID);

}


