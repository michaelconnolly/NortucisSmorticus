#include <LiquidCrystal.h>
#include "DHT.h"
#include <IRremote.h>


// GLOBALS: Infrared.
#define PIN_IR_RECEIVER 6
IRrecv ir_receiver(PIN_IR_RECEIVER);
decode_results ir_results;
#define COMMAND_NONE '?'
char last_ir_command = COMMAND_NONE;
//int command, prevCommand;


// GLOBALS: Thermostat.
#define PIN_DHT 7
#define DHT_TYPE DHT11
DHT dht(PIN_DHT, DHT_TYPE);


// GLOBALS: LCD display.
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


// GLOBALS: Panel system.
#define PANEL_DASHBOARD 0
#define PANEL_TEMPERATURE 1
#define PANEL_HUMIDITY 2
#define PANEL_INFRARED 3
int panels[] = {PANEL_DASHBOARD, PANEL_TEMPERATURE, PANEL_HUMIDITY, PANEL_INFRARED};
int currentPanel = PANEL_DASHBOARD;


// ***************
// SETUP
// ***************
void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("setup begin");

  // Panel system setup.
  Serial.print("panel count: ");
  Serial.println(panelCount());

  // LCD.
  lcd.begin(16, 2);
  lcd.print("Initializing ...");

  // DHT.
  dht.begin();

  // Infrared.
  setup_ir();

  Serial.println("setup end");
}


// ***************
// LOOP
// ***************
void loop() {

  // If the infrared panel is NOT up, make sure we poll IR in the background.
  if (currentPanel != PANEL_INFRARED) {
    char command = processInfraredBackground();
  }

  switch (currentPanel) {

    case PANEL_DASHBOARD:
      processDashboard();
      break;

    case PANEL_TEMPERATURE:
      processTemperature();
      break;
      
    case PANEL_HUMIDITY:
      processThermostat(currentPanel);
      break;

    case PANEL_INFRARED:
      processInfraredForeground();
      break;

    default:
      Serial.println("bad panel id!");
      processDashboard();
      break;
  }
}


void setup_ir() {
  ir_receiver.enableIRIn();
  ir_receiver.blink13(true);
}


void processDashboard() {

  lcd.clear();
  lcd.print("Sensor Console");
  delay(500);
}


void processTemperature() {

   //float t = dht.readTemperature();
   float f = dht.readTemperature(true);

  // if (isnan(t) || isnan(f)) {
   if (isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //float hif = dht.computeHeatIndex(f, h);
  //float hic = dht.computeHeatIndex(t, h, false);

  lcd.clear();
  lcd.print("Temperature");
  lcd.setCursor(0,1);
  lcd.print(f);
  lcd.print(" F");
  delay(250);
}

void processThermostat(int panelId) {

  float h = dht.readHumidity();
 
  if (isnan(h)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

//  Serial.print(F("Humidity: "));
//  Serial.print(h);
//  Serial.print(F("%  Temperature: "));
//  Serial.print(t);
//  Serial.print(F("°C "));
//  Serial.print(f);
//  Serial.print(F("°F  Heat index: "));
//  Serial.print(hic);
//  Serial.print(F("°C "));
//  Serial.print(hif);
//  Serial.println(F("°F"));

  lcd.clear();
  lcd.print("Humidity");
  lcd.setCursor(0,1);
  lcd.print(h);
  delay(250);
}

void processInfraredForeground() {

  char command = processInfraredBackground();
  if (command == COMMAND_NONE) command = ' ';

  lcd.clear();
  lcd.print("Infrared");
  lcd.setCursor(0,1);
  lcd.print(command);
  delay(250);
}

char processInfraredBackground() {

  char command = COMMAND_NONE;

  // bail if we don't have any new information.
  if (ir_receiver.decode(&ir_results)) {

    //String debugString = "";
    bool cache_command = true;
    
    switch (ir_results.value) {
  
      case 16738455: // 1 key
        command = '1';
        break;
      case 16750695: // 2 key
        command = '2';
        break;
      case 16756815: // 3 key
        command = '3';
        break;
      case 16724175: // 4 key
        command = '4';
        break;
      case 16718055: // 5 key
        command = '5';
        break;
      case 16743045: // 6 key
        command = '6';
        break;
      case 16716015: // 7 key
        command = '7';
        break;
      case 16726215: // 8 key
        command = '8';
        break;
      case 16734885: // 9 key
        command = '9';
        break;
      case 16728765: // asterix key
        command = '*';
        break;
      case 16730805: // 0 key
        command = '0';
        break;
      case 16732845: // pound key
        command = '#';
        break;
      case 16736925: // up key
        command = 'F';
        gotoNextPanel();
        break;
      case 16720605: // left key
        command = 'L';
        break;
      case 16712445: // ok key
        command = 'S';
        break;
      case 16761405: // right key
        command = 'R';
        break;
      case 16754775: // down key
        command = 'B';
        gotoPreviousPanel();
        break;
      case 4294967295: // repeat whatever i had before!  Someone is holding a button down.
        command = last_ir_command;
        cache_command = false;
        break;
      default: // some command that i don't know about.  WTF!
        Serial.print("Uknown IR code: ");
        Serial.println(ir_results.value);
        cache_command = false;
        break;
    }
  
    // Debug info.  If things get wonky, uncomment this code to figure out what is going on.
     if (command != COMMAND_NONE) {
      Serial.println(command);
    }
    
    // cache the last command, so we know what to do if we get a repeat.
    if (cache_command) {
      last_ir_command = command;
    }

     // put the IR receiver back in receiving mode.
    ir_receiver.resume();
  }

  return command;
}

//char processInfraredBackground() {
//
//  //Serial.println("Inside processInfrared");
//
//  char command = '?';
//
//  // bail if we don't have any new information.
//  if (ir_receiver.decode(&ir_results)) {
//
//    String debugString = "";
//    bool cache_command = true;
//    
//    switch (ir_results.value) {
//  
//      case 16738455: // 1 key
//        debugString = "1 key";
//        break;
//      case 16750695: // 2 key
//        debugString = "2 key";
//        break;
//      case 16756815: // 3 key
//        debugString = "3 key";
//        break;
//      case 16724175: // 4 key
//        debugString = "4 key";
//        break;
//      case 16718055: // 5 key
//        debugString = "5 key";
//        break;
//      case 16743045: // 6 key
//        debugString = "6 key";
//        break;
//      case 16716015: // 7 key
//        debugString = "7 key";
//        break;
//      case 16726215: // 8 key
//        debugString = "8 key";
//        break;
//      case 16734885: // 9 key
//        debugString = "9 key";
//        break;
//      case 16728765: // asterix key
//        debugString = "asterix key";
//        break;
//      case 16730805: // 0 key
//        debugString = "0 key";
//        break;
//      case 16732845: // pound key
//        debugString = "pound key";
//        break;
//      case 16736925: // up key
//        debugString = "up key";
//        command = 'F';
//
//        gotoNextPanel();
//        
//        break;
//      case 16720605: // left key
//        debugString = "left key";
//        command = 'L';
//        break;
//      case 16712445: // ok key
//        debugString = "ok key";
//        command = 'S';
//        break;
//      case 16761405: // right key
//        debugString = "right key";
//        command = 'R';
//        break;
//      case 16754775: // down key
//        debugString = "down key";
//        command = 'B';
//
//        gotoPreviousPanel();
//        
//        break;
//      case 4294967295: // repeat whatever i had before!  Someone is holding a button down.
//        debugString = "repeat";
//        command = last_ir_command;
//        cache_command = false;
//        break;
//      default: // some command that i don't know about.  WTF!
//        debugString = ir_results.value;
//        cache_command = false;
//        break;
//    }
//  
//    // Debug info.  If things get wonky, uncomment this code to figure out what is going on.
//     if (command != '?') {
//      Serial.println(command);
//    }
//    
//    // cache the last command, so we know what to do if we get a repeat.
//    if (cache_command) {
//      last_ir_command = command;
//    }
//
//     // put the IR receiver back in receiving mode.
//    ir_receiver.resume();
//  }
//
//  return command;
//}



//
//void commandProcess(char command) {
//  
//  // Process the known commands. Note that we change pin mode only if new command is different from previous.  
//  if (command != prevCommand) {
//    Serial.println("cmd: " + String(command));
//    switch (command){
//    case 'F': 
//      //motorForward();
//      break;
//    case 'B': 
//      motorReverse();
//      break;
//    case 'L': 
//      motorTurnLeft();
//      break;
//    case 'R':
//      motorTurnRight();
//      break;
//    case 'S': 
//      motorStop();
//      break;
//    case 'I':
//      motorTurnRightForward();
//      break;
//    case 'J':
//     motorTurnRightReverse();
//      break;       
//    case 'G':
//      motorTurnLeftForward();
//      break;
//    case 'H':
//      motorTurnLeftReverse();
//      break;
//    case 'W':  
//      //Front Lights ON
//      digitalWrite(PIN_LED, HIGH);
//      break;
//    case 'w':  
//      //Front Lights OFF
//      digitalWrite(PIN_LED, LOW);
//      break;
//    case 'U':  //Back ON
//      //digitalWrite(pinbackLights, HIGH);
//      break;
//    case 'u':  //Back OFF
//      //digitalWrite(pinbackLights, LOW);
//      break;
//    case 'D':  //Everything OFF
//      digitalWrite(PIN_LED, LOW);
//      //digitalWrite(pinbackLights, LOW);
//      motorStop();
//      break;        
//    default:  //Get velocity
//      if(command=='q'){
//        velocity = 255;  //Full velocity
//        //yellowCar.SetSpeed_4W(velocity);
//      }
//      else{
//        //Chars '0' - '9' have an integer equivalence of 48 - 57, accordingly.
//        if((command >= 48) && (command <= 57)){
//          //Subtracting 48 changes the range from 48-57 to 0-9.
//          //Multiplying by 25 changes the range from 0-9 to 0-225.
//          velocity = (command - 48)*25;      
//          //yellowCar.SetSpeed_4W(velocity);
//        }
//      }
//    }
//  }
// }

int panelCount() {
  return (sizeof(panels) / 2);
}

void gotoNextPanel() {

  if (currentPanel < (panelCount() - 1)) {
    currentPanel++;
  }
  else {
    currentPanel = 0;
  }

  Serial.print("New panel id: ");
  Serial.println(currentPanel);
}

void gotoPreviousPanel() {

  if (currentPanel > 0) {
    currentPanel--;
  }
  else {
    currentPanel = panelCount() - 1;
  }

  Serial.print("New panel id: ");
  Serial.println(currentPanel);
}
