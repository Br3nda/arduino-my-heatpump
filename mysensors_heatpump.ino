#define MY_RADIO_NRF24
#define MY_DEBUG

#include "SensorConfig.h"
#include <MySensors.h>
#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>


MyMessage powerMsg(POWER_ID, V_STATUS);
MyMessage modeMsg(MODE_ID, V_HVAC_FLOW_STATE);
MyMessage fanMsg(FAN_ID, V_PERCENTAGE);
MyMessage tempMsg(TEMP_ID, V_TEMP);
MyMessage vdirMsg(VDIR_ID, V_VAR1);
MyMessage hdirMsg(HDIR_ID, V_VAR2);
MyMessage strengthMsg(STRENGTH_ID, V_STATUS);

MyMessage msgHVACSetPointC(CHILD_ID_HVAC, V_HVAC_SETPOINT_COOL);
MyMessage msgHVACSpeed(CHILD_ID_HVAC, V_HVAC_SPEED);
MyMessage msgHVACFlowState(CHILD_ID_HVAC, V_HVAC_FLOW_STATE);

IRSenderPWM irSender(3);       // IR led on Arduino digital pin 3, using Arduino PWM
HeatpumpIR *heatpumpIR = new PanasonicNKEHeatpumpIR();

//Some global variables to hold the states
int POWER_STATE;
int TEMP_STATE;
int FAN_STATE;
int MODE_STATE;
int VDIR_STATE;
int HDIR_STATE;
bool STRENGTH_STATE;

void presentation() {
  present(POWER_ID, S_DOOR);
  present(CHILD_ID_HVAC, S_HVAC, "Thermostat");

  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Heatpump", "2.0");

  present(POWER_ID, S_BINARY);
  present(MODE_ID, S_HVAC);
  present(FAN_ID, S_HVAC);
  present(TEMP_ID, S_HVAC);
  present(VDIR_ID, S_CUSTOM);
  present(HDIR_ID, S_CUSTOM);
  present(STRENGTH_ID, S_BINARY);

  // Load our values on start
  POWER_STATE = loadState(POWER_ID);
  TEMP_STATE = loadState(TEMP_ID);
  FAN_STATE = loadState(FAN_ID);
  MODE_STATE = loadState(MODE_ID);
  HDIR_STATE = loadState(HDIR_STATE);
  VDIR_STATE = loadState(VDIR_STATE);
  STRENGTH_STATE = loadState(STRENGTH_ID);
}

void loop() {
}

void handlePowerMessage(bool newState) {
  if (newState) {
    POWER_STATE = POWER_ON;
  } else {
    POWER_STATE = POWER_OFF;
  }
  saveState(POWER_ID, newState);
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
  saveState(MODE_ID, newMode);
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
  saveState(FAN_ID, newFan);
}

void handleTempMessage(int newTemp) {
  TEMP_STATE = newTemp;
  saveState(TEMP_ID, newTemp);
}

void handleVdirMessage(int newVDir) {
  //TODO
}

void handleHDirMessage(int newHDir) {
  // TODO
}

void handleStrengthMessage(bool newStrength) {
  STRENGTH_STATE = newStrength;
  saveState(STRENGTH_STATE, newStrength);
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
  } else {
    powerful= false;
    quiet = true;
  }

  heatpumpIR->send(irSender, POWER_STATE, MODE_STATE, FAN_STATE, TEMP_STATE, VDIR_AUTO, HDIR_AUTO);
}

void incomingMessage(const MyMessage &message) {

  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
     return;
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
}
