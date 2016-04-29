#include <MySensor.h>
#include <SPI.h>
#include <Arduino.h>

#include <PanasonicCKPHeatpumpIR.h>
#include <PanasonicHeatpumpIR.h>

#define POWER_ID 0
#define MODE_ID 1
#define FAN_ID 2
#define TEMP_ID 3
#define VDIR_ID 4
#define HDIR_ID 5


MySensor gw;
MyMessage powerMsg(POWER_ID, V_STATUS); 
MyMessage modeMsg(MODE_ID, V_HVAC_FLOW_STATE);
MyMessage fanMsg(FAN_ID, V_PERCENTAGE);
MyMessage tempMsg(TEMP_ID, V_TEMP);
MyMessage vdirMsg(VDIR_ID, V_VAR1); 
MyMessage hdirMsg(HDIR_ID, V_VAR2); 

IRSenderPWM irSender(3);       // IR led on Arduino digital pin 3, using Arduino PWM

HeatpumpIR *heatpumpIR = new PanasonicNKEHeatpumpIR();

//Some global variables to hold the states
int POWER_STATE;
int TEMP_STATE;
int MODE_STATE;

void setup()  
{  
  gw.begin(incomingMessage, AUTO, true);

  // Send the sketch version information to the gateway and Controller
  gw.sendSketchInfo("Heatpump", "1.0");

  // Register all sensors to gw (they will be created as child devices)
  gw.present(POWER_ID, S_BINARY);
  gw.present(MODE_ID, S_HVAC);
  gw.present(FAN_ID, S_HVAC);
  gw.present(TEMP_ID, S_HVAC);
  gw.present(VDIR_ID, S_CUSTOM);
  gw.present(HDIR_ID, S_CUSTOM);
     
  // Set relay to last known state (using eeprom storage) 
  POWER_STATE = gw.loadState(POWER_ID);
  TEMP_STATE = gw.loadState(TEMP_ID);
  
//  digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
}


/*
*  Example on how to asynchronously check for new messages from gw
*/
void loop() {
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
  }
  MODE_STATE = newMode;
  gw.saveState(MODE_ID, newMode);
}

void handleTempMessage(int newTemp) {
  TEMP_STATE = newTemp;
  gw.saveState(TEMP_ID, newTemp);
}

void sendHeatpumpCommand() {
  Serial.println("Power = " + (String)POWER_STATE);
  Serial.println("Mode = " + (String)MODE_STATE);
  Serial.println("Temp = " + (String)TEMP_STATE);
  heatpumpIR->send(irSender, POWER_STATE, MODE_STATE, FAN_1, TEMP_STATE, VDIR_AUTO, HDIR_AUTO);
}
void incomingMessage(const MyMessage &message) {
  // We only expect one type of message from controller. But we better check anyway.
  if (message.isAck()) {
     Serial.println("This is an ack from gateway");
  }
//
//  if (message.type == V_LIGHT) {
//     // Change relay state
//     state = message.getBool();
//     digitalWrite(RELAY_PIN, state?RELAY_ON:RELAY_OFF);
//     // Store state in eeprom
//    
//     // Write some debug info
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
      case TEMP_ID: {
        int newTemp = message.getInt();
        handleTempMessage(newTemp);
        break;
      }
     }
  sendHeatpumpCommand();
}

