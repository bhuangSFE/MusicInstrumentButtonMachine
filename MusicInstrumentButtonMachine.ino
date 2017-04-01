/*
   MusicInstrumentButtonMachine

   based on code from: 2-12-2011
   Spark Fun Electronics 2011
   Nathan Seidle
   Updated to Arduino 1.01 by Marc "Trench" Tschudin

  To play "Sticks" (31):
  talkMIDI(0xB0, 0, 0x78); //Bank select: drums
  talkMIDI(0xC0, 5, 0); //Set instrument number
  //Play note on channel 1 (0x90), some note value (note), middle velocity (60):
  noteOn(0, 31, 60);

*/

#include <SoftwareSerial.h>

SoftwareSerial mySerial(2, 3); // RX, TX
SoftwareSerial serialLCD(2, 10); // RX, TX

byte note = 0; //The MIDI note value to be played
byte resetMIDI = 4; //Tied to VS1053 Reset line
byte ledPin = 13; //MIDI traffic inidicator
int  instrument = 0;

int buttons[] = {5, 6, 7, 8};
//int notes[] = {30, 31, 32, 33};
int notes[] = {27, 28, 29, 30};
int instrumentSelect = 9;

void setup() {
  Serial.begin(57600);

  //Setup soft serial for MIDI control
  mySerial.begin(31250);
  serialLCD.begin(9600);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)
  for (int i = 5; i <= 9; i++)
    pinMode(i, INPUT_PULLUP);
  serialLCD.write(0xFE);
  serialLCD.write(0x01);
}

void loop() {
  // Serial.println("Demo Fancy Sounds");
  talkMIDI(0xB0, 0, 0x78); //Bank select drums

  //For this bank 0x78, the instrument does not matter, only the note

  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

  //Play fancy sounds from 'High Q' to 'Open Surdo [EXC 6]'
  for (int i = 0; i < 4; i++)
  {
    serialLCD.write(0xFE);
    serialLCD.write(0x80 + 4 * i);
    serialLCD.print(notes[i]);
    if (digitalRead(buttons[i]) == LOW)
    {
      noteOn(0, notes[i], 60);
      delay(100);
      noteOff(0, notes[i], 60);
    }
  }
  if (digitalRead(instrumentSelect) == LOW)
  {
    Serial.println("New Notes!");
    for (int i = 0; i < 4; i++)
    {
      notes[i] = notes[3] + i + 1;
      Serial.println(notes[i]);
    }
    // reset notes if go beyond limit
    if (notes[3] > 87)
    {
      for (int i = 0; i < 4; i++)
      {
        notes[i] = 27 + i;
      }
    }
    while (digitalRead(instrumentSelect) == LOW)
    {}
    delay(50);
  }
}


/*
  //Demo Melodic
  //=================================================================
  Serial.println("Demo Melodic? Sounds");
  talkMIDI(0xB0, 0, 0x79); //Bank select Melodic
  //These don't sound different from the main bank to me

  //Change to different instrument
  for(instrument = 27 ; instrument < 87 ; instrument++) {

    Serial.print(" Instrument: ");
    Serial.println(instrument, DEC);

    talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command

    //Play notes from F#-0 (30) to F#-5 (90):
    for (note = 30 ; note < 40 ; note++) {
      Serial.print("N:");
      Serial.println(note, DEC);

      //Note on channel 1 (0x90), some note value (note), middle velocity (0x45):
      noteOn(0, note, 60);
      delay(50);

      //Turn off the note with a given off/release velocity
      noteOff(0, note, 60);
      delay(50);
    }

    delay(100); //Delay between instruments
  }
*/

//Send a MIDI note-on message.  Like pressing a piano key
//channel ranges from 0-15
void noteOn(byte channel, byte note, byte attack_velocity) {
  talkMIDI( (0x90 | channel), note, attack_velocity);
}

//Send a MIDI note-off message.  Like releasing a piano key
void noteOff(byte channel, byte note, byte release_velocity) {
  talkMIDI( (0x80 | channel), note, release_velocity);
}

//Plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that data values are less than 127
void talkMIDI(byte cmd, byte data1, byte data2) {
  digitalWrite(ledPin, HIGH);
  mySerial.write(cmd);
  mySerial.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if ( (cmd & 0xF0) <= 0xB0)
    mySerial.write(data2);

  digitalWrite(ledPin, LOW);
}
