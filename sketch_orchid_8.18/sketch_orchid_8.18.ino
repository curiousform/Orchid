#include <Bounce2.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

// bounce time in ms
int bounceInterval = 200;
// debouncer for each sensor pin used
Bounce bouncer[30] = Bounce();

//  number of ms delay for on and off switch
int delTime = 50;

// Pin of first sensor
int pinStart = 2;

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
//  Start MIDI note for audio signals
int audStart = 40;

//  Counter var for how many people are sitting down
int counter = 0;
int lastCounter = 0;
int counterIncrement = 1000;

//  COUNTER THRESHOLDS FOR ADDING MORE AUDIO LAYERS
int layer[] = {2, 4, 8, 12, 18, 24}; // 24 is for party mode
int layerCount = sizeof(layer) / sizeof(int);
int trend = 0;

long layerTimer[layerCount];
int deferredAudMidiNote[layerCount]

long lastRandomize = millis();
int randomizeInterval = 5000
int randomOffset = 0;

void setup()
{
  // Begin MIDI and listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  for (int i = 0; i < pinCount; i++)
  {
    pinMode(i + pinStart, INPUT);
    bouncer[i].attach(i + pinStart, INPUT);
    bouncer[i].interval(bounceInterval);
    buttonState[i] = bouncer[i].read();
    if ((i < dualCount) && (buttonState[i] == HIGH))
    {
      counter++;
    }
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
      else if ((i < dualCount) && (buttonState[i] != HIGH))
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
      else if ((i >= dualCount) && (buttonState[i] != HIGH))
      {
        MIDI.sendNoteOn(i + eEgg - dualCount + eEggCount, 120, eEggChan);
        MIDI.sendNoteOff(i + eEgg - dualCount + eEggCount, 0, eEggChan);
      }
    }
  }

  // Audio layer count trigger
  // Implement nonblocking delay on counter decrement

  if (counter != lastCounter)
  {
    trend = counter - lastCounter;

    for (int i = 0; i < layerCount; i++)
    {
      if (trend > 0 && counter == layer[i])
      {
        // item is increasing and equal to threshold
        MIDI.sendNoteOn(i + audStart, 120, audChan);
        MIDI.sendNoteOff(i + audStart, 0, audChan);
      }
      else if (trend < 0 && counter == (layer[i] - 1))
      {
        // item is decreasing and dropping threshold level
        deferredAudMidiNote[i] = (i + audStart + layerCount);
        layerTimer[i] = millis();
      }
    }
  }

  // Cant run in counter-- block because that code only runs if the sensor changed for that pin
  // cant run uin above block cause that code only runs if counter changed
  // If deferredAudMidiNote is overwritten by another counter decrement before this gets a chance to run the previous one never will
  // TODO! set one of these up for every layer in the loop so each one can know if its waiting to shut off or not.
  for (int i = 0; i < layerCount; i++)
  {
    if (millis() - layerTimer[i] >= counterInterval && deferredAudMidiNote[i] != 0)
    {
      MIDI.sendNoteOn(deferredAudMidiNote[i] + randomOffset, 120, audChan);
      MIDI.sendNoteOff(deferredAudMidiNote[i] + randomOffset, 0, audChan);
      deferredAudMidiNote = 0;
    }
  }

  // Since the layers are exact counter numbers, and we are checking if the counter number equals the int in layer[] itself, randomizing the layer numbers order wont do anything.
  // Do you want the layer ints themselves to randomize? and therefore the thresholds and the midinotes were sending out which are dependent on such layer ints?
  // Instead of that we should just either randomly offset it by 1 or not

  if (millis() - lastRandomize >= randomizeInterval)
  {
    lastRandomize = millis();

    if (trend > 0 && counter < layer[layerCount] || trend == 0)
    {
      randomOffset = random(0, layerCount);
    }
    else
    {
      randomOffset = -random(0, layerCount);
    }
  }

  lastCounter = counter;
}