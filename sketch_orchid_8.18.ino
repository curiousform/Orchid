#include <Bounce2.h>
#include <MIDI.h> // Include MIDI Library


MIDI_CREATE_DEFAULT_INSTANCE(); // Create an instance of the midi library


int bounceInterval = 1000;

int pinCount = 30;
int dualCount = 24;
int buttonState [30];
int lastButtonState [30];

int multiCount = 12;
int multiState [12];
int lastMultiState [12];

int eEgg = 100; //easter egg start pin
int eEggCount = 6;

int litChan = 1;
int multiChan = 2;
int audChan = 3;
int eEggChan = 4;

int litStart = 60;

int pinStart = 2; //pin of first sensor

Bounce bouncer[30];


void setup() {
   MIDI.begin(MIDI_CHANNEL_OMNI); // Begin MIDI and listen to all channels
   for (int i = pinStart; i < pinCount+pinStart; i++) {
    pinMode(i, INPUT);
    bouncer[i].attach(i, INPUT);
    bouncer[i].interval(bounceInterval);
   }
  
}
void loop() {
  for (int i = 0; i < pinCount; i++){
    bouncer[i].update();
    buttonState [i] = bouncer[i].read();
    if (bouncer[i].changed()) {  // compare the buttonState to its previous state
     if ((i < dualCount) && (buttonState [i] == HIGH)){ 
      MIDI.sendNoteOn(i+litStart, 120, litChan); // Send note i on with a velocity of 120 and on lighting channel 1
     }
     if ((i < dualCount) && (buttonState [i] != HIGH)) {
      MIDI.sendNoteOff(i+litStart, 0, litChan); 
     }

     
     if ((i >= dualCount) && (buttonState [i] == HIGH)){ //easter egg start 
      MIDI.sendNoteOn(i+eEgg-dualCount, 120, eEggChan);
      MIDI.sendNoteOff(i+eEgg-dualCount, 0, eEggChan);
     }
     if ((i >= dualCount) && (buttonState [i] != HIGH)) {
      MIDI.sendNoteOn(i+eEgg-dualCount+eEggCount, 120, eEggChan);
      MIDI.sendNoteOff(i+eEgg-dualCount+eEggCount, 0, eEggChan);
     }
    }
  }
  
  for (int i = 0; i < multiCount; i++){
   bouncer[i].update();
   bouncer[i+multiCount].update();
   buttonState [i] = bouncer[i].read();
   buttonState [i+multiCount] = bouncer[i+multiCount].read();
   if (bouncer[i].changed() || bouncer[i+multiCount].changed() {
    if ((buttonState [i] == buttonState [i+multiCount]) && (buttonState [i] == HIGH)){
      MIDI.sendNoteOn(i+litStart, 120, multiChan); // Send note i on with a velocity of 120 and on lighting channel 1
      }
     else {                       
      MIDI.sendNoteOff(i+litStart, 0, multiChan); //MIDI.sendNoteOff(i+pinCount, 0, 1); if different note needed
     }
    }
  }
}  
