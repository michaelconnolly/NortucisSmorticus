    // Import external libraries
#include <Adafruit_ssd1306syp.h>
#include <IRremote.h>
#include "SoftwareSerial.h"
#include "WString.h"

// Constants: I/O Pins
#define PIN_MOTOR_A_ENABLE  5
#define PIN_MOTOR_B_ENABLE  6
#define PIN_MOTOR_A_INPUT1  7
#define PIN_MOTOR_A_INPUT2  8
#define PIN_MOTOR_B_INPUT1  11
#define PIN_MOTOR_B_INPUT2  4
#define PIN_LED             10
#define PIN_I2C_SDA         A4
#define PIN_I2C_SCL         A5
#define PIN_IR_RECEIVER     12

// More Constants
#define AUTO_SHUTOFF_TIME 30000
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define COMMAND_NONE '?'

// The various states of our bluetooth connection.
enum BluetoothState {
  BLUETOOTH_DISCONNECTED,
  BLUETOOTH_CONNECTED,
  BLUETOOTH_ABANDONDED
};


// Global Variables: Run state
boolean dead = false;
bool autoShutOff = false;
unsigned long startTime = 0; 
const char build_timestamp[] =  __DATE__ " " __TIME__;

// Global Variables: the OLED Display, connected via I2C interface
Adafruit_ssd1306syp display(PIN_I2C_SDA, PIN_I2C_SCL);

// Global Variables: motor control
int velocity = 100;  

// Global Variables: Bluetooth command Queue
SoftwareSerial bluetooth(2, 3);
BluetoothState bluetoothState = BLUETOOTH_DISCONNECTED;
char command = 'S';
char prevCommand = 'A';
unsigned long timeLastCommand = 0;  //Stores the time when the last command was received from the phone

// Global Variables: Infrared
IRrecv ir_receiver(PIN_IR_RECEIVER);
char last_ir_command = COMMAND_NONE;

/*** DISPLAY ***/


/**
 * Main output for status while in main sequence. This is called once per loop.
 */
void displayStatus(String line1, String line2, String line3, String line4) {  

  // If there is no display, bail.
//  if (display == NULL) {
//    Serial.println("bailing");
//    return;
//  }

 Serial.println(line1);
 //display.display();
  Serial.println("draw screen - b1");
  
  display.clear();

 Serial.println("draw screen - b2");
 
  
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0,0);
  display.println(line1);


  Serial.println("draw screen - c");
  

  // Fun little animation to prove that we are not locked up.
  const int circleRadius = 5;
  const int circleOffset = 0;
  const int maxRight = (SCREEN_WIDTH - 1) - circleRadius;
  const int maxLeft = 82;
  int numFrames = maxRight - maxLeft;
  int numFramesDouble = numFrames * 2;
  int timePerFrame = 3000 / numFramesDouble;
  int currentFrame = (millis() / timePerFrame) % numFramesDouble;
  int circleCenterX = (currentFrame < numFrames) ? (maxRight - currentFrame) : (maxLeft + (currentFrame - numFrames));
  display.drawCircle(circleCenterX, circleRadius + circleOffset, circleRadius, WHITE);


  Serial.println("draw screen - d");
  

  display.setCursor(0,10);
  display.println("config: despicable (mc)");
  
  display.setCursor(0,20);
  display.println(line2);
    
  display.setCursor(0,30);
  display.println(line3);

  display.setCursor(0,40);
  display.println(line4);

  // Print the timestamp of when we built this code.
  const int stampHeight = 10;
  int beginStamp = 55;
  display.drawLine(0, beginStamp, SCREEN_WIDTH - 1, beginStamp, WHITE); // top line
  display.drawLine(0, beginStamp, 0, beginStamp + stampHeight, WHITE); // left line
  display.drawLine(SCREEN_WIDTH - 1, beginStamp, SCREEN_WIDTH - 1, beginStamp + stampHeight, WHITE); // right line
  display.setCursor(4, beginStamp + 2);
  display.println(build_timestamp);

  display.update();

  
  Serial.println("draw screen - z");
  
}

/**
 * Used for error messages to the OLED display.
 */
void displayMessage(String line1, String line2) {
  display.clear();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(0,2);
  display.println(line1);
  
  display.setCursor(0,34);
  display.println(line2);
  
  display.update();
}


/**
 * Entrypoint: called once when the program first starts, just to initialize all the sub-components.
 */
void setup() {  
  
  // Record what time we started.
  startTime = millis();
  
  // Init the serial line; important for debug messages back to the Arduino Serial Monitor, so you can plug
  // the robot into the USB port, and get real time debug messages.
  // Make sure you set the baudrate at 9600 in Serial Monitor as well.
  Serial.begin(9600);
  Serial.println("setup start...");

  // Infrared.
  infraredSetup();
  
  // Init motor control pins.
  pinMode(PIN_MOTOR_A_INPUT1, OUTPUT);
  pinMode(PIN_MOTOR_A_INPUT2, OUTPUT);
  pinMode(PIN_MOTOR_B_INPUT1, OUTPUT);
  pinMode(PIN_MOTOR_B_INPUT2, OUTPUT);
  pinMode(PIN_MOTOR_A_ENABLE, OUTPUT);
  pinMode(PIN_MOTOR_B_ENABLE, OUTPUT);

  // Init LED pin, and initially set it to off.
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  // Init the serial pipe to the bluetooth receiver. NOTE: this can go faster by configuring the HC-05
  // with an AT command. But is that necessary?
  bluetooth.begin(9600);
  
  // Init the OLED display.
  delay(1000);
  display.initialize();

  // Init the rest of our internal state.
  dead = false;
  Serial.println("setup end");
}

void infraredSetup() {
  
  Serial.println("Inside infraredSetup");
  ir_receiver.enableIRIn();
  ir_receiver.blink13(true);
}

char infraredProcess() {

  //Serial.println("Inside infraredProcess");
  char command = COMMAND_NONE;
  decode_results ir_results;

  // Did we get a signal?
  if (ir_receiver.decode(&ir_results)) {

    String debugString = "";
    bool cache_command = true;
    
    switch (ir_results.value) {
  
      case 16738455: // 1 key
        debugString = "1 key";
        break;
      case 16750695: // 2 key
        debugString = "2 key";
        break;
      case 16756815: // 3 key
        debugString = "3 key";
        break;
      case 16724175: // 4 key
        debugString = "4 key";
        break;
      case 16718055: // 5 key
        debugString = "5 key";
        break;
      case 16743045: // 6 key
        debugString = "6 key";
        break;
      case 16716015: // 7 key
        debugString = "7 key";
        break;
      case 16726215: // 8 key
        debugString = "8 key";
        break;
      case 16734885: // 9 key
        debugString = "9 key";
        break;
      case 16728765: // asterix key
        debugString = "asterix key";
        break;
      case 16730805: // 0 key
        debugString = "0 key";
        break;
      case 16732845: // pound key
        debugString = "pound key";
        break;
      case 16736925: // up key
        debugString = "up key";
        command = 'F';
        break;
      case 16720605: // left key
        debugString = "left key";
        command = 'L';
        break;
      case 16712445: // ok key
        debugString = "ok key";
        command = 'S';
        break;
      case 16761405: // right key
        debugString = "right key";
        command = 'R';
        break;
      case 16754775: // down key
        debugString = "down key";
        command = 'B';
        break;
      case 4294967295: // repeat whatever i had before!  Someone is holding a button down.
        debugString = "repeat";
        command = last_ir_command;
        cache_command = false;
        break;
      default: // some command that i don't know about.  WTF!
        debugString = ir_results.value;
        cache_command = false;
        break;
    }
  
    // Debug info.  If things get wonky, uncomment this code to figure out what is going on.
    // Serial.println(debugString);
    
    // cache the last command, so we know what to do if we get a repeat.
    if (cache_command) {
      last_ir_command = command;
    }

     // put the IR receiver back in receiving mode.
    ir_receiver.resume();
  }

  return command;
}


/**
 * Main Loop: called over and over again as the robot runs, 
 */
void loop() {
  
  // If we have marked as dead, be dead and do nothing.
  if (dead) { 
    displayMessage("DEAD", "Deactivated due to auto-shutoff");
    return;
  }

  // Query for any inputs.  First try is IR, then BT.
  char command_received = infraredProcess();
  if (command_received == COMMAND_NONE) {
    command_received = bluetoothProcess();
  }
 // command_received = 'F';

  // Process any input we might have received.
  if (command_received != COMMAND_NONE) {
    //Serial.println(command_received);
    commandProcess(command_received);
  }
 
  // Update the LED screen with our current state.
//  Serial.println("about to draw screen");
//  bool connected = (bluetoothState == BLUETOOTH_CONNECTED);
//  int upSecs = (millis() - startTime) / 1000;
//   Serial.println("draw screen - a");
//  displayStatus(
//    connected ? F("CONNECTED") : F("DISCONNECTED"),
//    "runtime: " + String(upSecs), 
//    "cmd: " + String(command) + "/" + String(prevCommand) + "   v: " + String(velocity),
//    "objective: " + String("FOO"));
//   Serial.println("finished drawing screen");
// 

  // Engage auto-shutoff if it has been enabled.
  if (autoShutOff && (millis() > (startTime + AUTO_SHUTOFF_TIME))) {
    dead = true;
    motorStop();
    displayMessage("Auto-Stop", "Stopped due to shutdown timer");
    delay(3000);
  }
}


/** MOTOR CONTROL **/

#define TURN_SCALER 0.40

void motorForward() {
  Serial.println("Inside motorForward.");
  digitalWrite(PIN_MOTOR_A_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_A_INPUT2, LOW);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
  
  digitalWrite(PIN_MOTOR_B_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_B_INPUT2, LOW);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorReverse() {
  digitalWrite(PIN_MOTOR_A_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_A_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
    
  digitalWrite(PIN_MOTOR_B_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_B_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorTurnLeft() {
  digitalWrite(PIN_MOTOR_A_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_A_INPUT2, LOW);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
  
  digitalWrite(PIN_MOTOR_B_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_B_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorTurnLeftForward() {
  digitalWrite(PIN_MOTOR_A_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_A_INPUT2, LOW);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
  
  digitalWrite(PIN_MOTOR_B_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_B_INPUT2, LOW);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity * TURN_SCALER); 
}

void motorTurnLeftReverse() {
  digitalWrite(PIN_MOTOR_A_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_A_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
    
  digitalWrite(PIN_MOTOR_B_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_B_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity * TURN_SCALER); 
}

void motorTurnRight() {
  digitalWrite(PIN_MOTOR_A_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_A_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity);
  
  digitalWrite(PIN_MOTOR_B_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_B_INPUT2, LOW);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorTurnRightForward() {
  digitalWrite(PIN_MOTOR_A_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_A_INPUT2, LOW);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity * TURN_SCALER);
  
  digitalWrite(PIN_MOTOR_B_INPUT1, HIGH);
  digitalWrite(PIN_MOTOR_B_INPUT2, LOW);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorTurnRightReverse() {
  digitalWrite(PIN_MOTOR_A_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_A_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_A_ENABLE, velocity * TURN_SCALER);
    
  digitalWrite(PIN_MOTOR_B_INPUT1, LOW);
  digitalWrite(PIN_MOTOR_B_INPUT2, HIGH);
  analogWrite(PIN_MOTOR_B_ENABLE, velocity); 
}

void motorStop() {
  analogWrite(PIN_MOTOR_A_ENABLE, 0); 
  analogWrite(PIN_MOTOR_B_ENABLE, 0); 
}


/**** BLUETOOTH ****/

/**
 * 
 */
char bluetoothProcess() {
  
  // If no commands, then nothing to do.
  if (bluetooth.available() <= 0) {
    return COMMAND_NONE;
  }
  // A little state management. Since we have data ready to read, that means we are connected.
  bluetoothState = BLUETOOTH_CONNECTED;

  // Record when the last command was processed.
  timeLastCommand = millis();  

  // Read the actual command from the bluetooth buffer.
  prevCommand = command;
  command = bluetooth.read();
  
  if (command != 'S') {
    Serial.println("cmd2new: " + String(command) + "/" + String(prevCommand));
  }

  return command;
}


void commandProcess(char command) {
  
  // Process the known commands. Note that we change pin mode only if new command is different from previous.  
  if (command != prevCommand) {
    Serial.println("cmd2: " + String(command));
    switch (command){
    case 'F': 
      motorForward();
      break;
    case 'B': 
      motorReverse();
      break;
    case 'L': 
      motorTurnLeft();
      break;
    case 'R':
      motorTurnRight();
      break;
    case 'S': 
      motorStop();
      break;
    case 'I':
      motorTurnRightForward();
      break;
    case 'J':
     motorTurnRightReverse();
      break;       
    case 'G':
      motorTurnLeftForward();
      break;
    case 'H':
      motorTurnLeftReverse();
      break;
    case 'W':  
      //Front Lights ON
      digitalWrite(PIN_LED, HIGH);
      break;
    case 'w':  
      //Front Lights OFF
      digitalWrite(PIN_LED, LOW);
      break;
    case 'U':  //Back ON
      //digitalWrite(pinbackLights, HIGH);
      break;
    case 'u':  //Back OFF
      //digitalWrite(pinbackLights, LOW);
      break;
    case 'D':  //Everything OFF
      digitalWrite(PIN_LED, LOW);
      //digitalWrite(pinbackLights, LOW);
      motorStop();
      break;        
    default:  //Get velocity
      if(command=='q'){
        velocity = 255;  //Full velocity
        //yellowCar.SetSpeed_4W(velocity);
      }
      else{
        //Chars '0' - '9' have an integer equivalence of 48 - 57, accordingly.
        if((command >= 48) && (command <= 57)){
          //Subtracting 48 changes the range from 48-57 to 0-9.
          //Multiplying by 25 changes the range from 0-9 to 0-225.
          velocity = (command - 48)*25;      
          //yellowCar.SetSpeed_4W(velocity);
        }
      }
    }
  }
 }
 
