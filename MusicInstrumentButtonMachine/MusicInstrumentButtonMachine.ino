/* MusicInstrumentButtonMachine

   based on code from: 2-12-2011
   Spark Fun Electronics 2011
   Nathan Seidle
   Updated to Arduino 1.01 by Marc "Trench" Tschudin

  Hardware:
  ---------
  Connect 5 buttons to pins 5, 6, 7, 8, & 9
  The first four buttons work as "drum beats" and the last button increments the
  instrument selection.
  
  (Optional - connect the data line on a Serial LCD to pin 10)
*/

#include <SoftwareSerial.h>

SoftwareSerial musicShield(2, 3); //RX, TX -- for the MIDI / Music Instrument Shield
SoftwareSerial serialLCD(2, 10); //RX, TX -- for an external Serial LCD (LCD-09395)

byte note = 0; //MIDI note value to be played
byte resetMIDI = 4; //tied to VS1053 Reset line
byte ledPin = 13; //MIDI traffic inidicator
int  instrument = 0;   //variable used to set the "instrument" setting (command 0xC0)

int buttons[] = {5, 6, 7, 8};    //four push buttons connected to pins 5,6,7,8
int notes[] = {27, 28, 29, 30};  //initialize notes to start here.
int instrumentSelect = 9;

void setup() {
  Serial.begin(57600);

  //Setup soft serial for MIDI control
  musicShield.begin(31250);
  serialLCD.begin(9600);

  //Reset the VS1053
  pinMode(resetMIDI, OUTPUT);
  digitalWrite(resetMIDI, LOW);
  delay(100);
  digitalWrite(resetMIDI, HIGH);
  delay(100);
  talkMIDI(0xB0, 0x07, 120); //0xB0 is channel message, set channel volume to near max (127)

  //setup buttons to be INPUT_PULLUP 
  for (int i = 5; i <= 9; i++)
    pinMode(i, INPUT_PULLUP);
  
  //clear serialLCD display
  serialLCD.write(0xFE);
  serialLCD.write(0x01);
  
  //setup sound bank on the Music Instrument Shield to drums and special sounds
  talkMIDI(0xB0, 0, 0x78); 

   //For this bank 0x78, the instrument does not matter, only the note
  talkMIDI(0xC0, instrument, 0); //Set instrument number. 0xC0 is a 1 data byte command


}

void loop() {
  // cycle through the buttons
  for (int i = 0; i < 4; i++)
  {
    //display to LCD the notes currently mapped to buttons
    serialLCD.write(0xFE);
    serialLCD.write(0x80 + 4 * i);
    serialLCD.print(notes[i]);

    //if button is pressed, play that note 
    if (digitalRead(buttons[i]) == LOW)
    {
      noteOn(0, notes[i], 60);
      delay(100);
      noteOff(0, notes[i], 60);
    }
  }
   
  //if 5th button is pressed, increment the notes
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
     
    //holding trap to wait for the button to be released
    while (digitalRead(instrumentSelect) == LOW)
    {}
    delay(20);   //short debounce delay
  }
}

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
  musicShield.write(cmd);
  musicShield.write(data1);

  //Some commands only have one data byte. All cmds less than 0xBn have 2 data bytes
  //(sort of: http://253.ccarh.org/handout/midiprotocol/)
  if ( (cmd & 0xF0) <= 0xB0)
    musicShield.write(data2);

  digitalWrite(ledPin, LOW);
}
