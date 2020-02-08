#include <IRremote.h>

#define PIN_IR_RECEIVER 12

IRrecv ir_receiver(PIN_IR_RECEIVER);
decode_results ir_results;
char last_ir_command = '?';

void setup(){
  Serial.begin(9600);
  setup_ir();
}

void loop(){
  char command = handle_ir();
  if (command != '?') {
    Serial.println(command);
  }
}
  
void setup_ir() {
  ir_receiver.enableIRIn();
  ir_receiver.blink13(true);
}

char handle_ir() {

  Serial.println("Inside handle_ir");

  char command = '?';

  // bail if we don't have any new information.
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
