#include <Bounce2.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

//test change for github!!!
// bounce time in ms
int bounceInterval = 1000;
// debouncer for each sensor pin used
Bounce bouncer[30] = Bounce();

// Pin of first sensor
int pinStart = 2;
//  Counter var for how many people are sitting down
int counter = 0;

// Total # Of sensor pins
int pinCount = 30;
//  6 of them are easter eggs
int eEggCount = 6;
//  That leaves us with 24 pins
int dualCount = (pinCount - eEggCount);
//  Pairing them up we have 12
int multiCount = (dualCount / 2);

//  storage vars for button states
int buttonState[30];
int lastButtonState[30];
//  Storage vars for multiState
int lastMultiState[12];
int multiState[12];

//  MIDI CHANNEL DEFS
//  MIDI channel for Lighting Signals
int litChan = 1;
//  MIDI Channel for paired sensor signals
int multiChan = 2;
// MIDI Channel for audio signals
int audChan = 3;
//  MIDI Channel for easter Eggs
int eEggChan = 4;

//  MIDI START NOTES
//  Easter Egg Start MIDI Note
int eEgg = 100;
//  Start MIDI note for lighting signals
int litStart = 60;

void setup()
{
  // Begin MIDI and listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  for (int i = 0; i < pinCount; i++)
  {
    pinMode(i + pinStart, INPUT);
    bouncer[i].attach(i + pinStart, INPUT);
    bouncer[i].interval(bounceInterval);
  }
}

void loop()
{
  for (int i = 0; i < pinCount; i++)
  {
    bouncer[i].update();
   
    if (bouncer[i].changed())
    {
      buttonState[i] = bouncer[i].read();
      // Pin pairing logic (Quantum Enganglement)
      if ((i < multiCount))
      {
        if (bouncer[i].changed() || bouncer[i + multiCount].changed())
        {
          if ((buttonState[i] == buttonState[i + multiCount]) && (buttonState[i] == HIGH))
          {
            // sendNoteX(note, velocity, midiChannel)
            MIDI.sendNoteOn(i + litStart, 120, multiChan);
          }
          else
          {
            MIDI.sendNoteOff(i + litStart, 0, multiChan);
          }
        }
      }

      // If somebody sits down on any sensor thats non easter egg
      // Then send lighting midi note
      if ((i < dualCount) && (buttonState[i] == HIGH))
      {
        MIDI.sendNoteOn(i + litStart, 120, litChan);
        counter++;
      }
      if ((i < dualCount) && (buttonState[i] != HIGH))
      {
        MIDI.sendNoteOff(i + litStart, 0, litChan);
        counter--;
      }

      // Easter egg ON trigger
      if ((i >= dualCount) && (buttonState[i] == HIGH))
      {
        MIDI.sendNoteOn(i + eEgg - dualCount, 120, eEggChan);
        MIDI.sendNoteOff(i + eEgg - dualCount, 0, eEggChan);
      }

      // Easter egg OFF trigger
      if ((i >= dualCount) && (buttonState[i] != HIGH))
      {
        MIDI.sendNoteOn(i + eEgg - dualCount + eEggCount, 120, eEggChan);
        MIDI.sendNoteOff(i + eEgg - dualCount + eEggCount, 0, eEggChan);
      }
    }
  }
}