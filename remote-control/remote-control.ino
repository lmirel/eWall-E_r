//based on the remote control from www.adeept.com

#include <SPI.h>
#include "RF24.h"

#define DEBUG 0
#define debug_println(...) \
            do { if (DEBUG) Serial.println(__VA_ARGS__); } while (0)
#define debug_print(...) \
            do { if (DEBUG) Serial.print(__VA_ARGS__); } while (0)

RF24 radio(9, 10);            // define the object to control NRF24L01
//byte addresses[5] = "00007";  // define communication address which should correspond to remote control
const uint64_t rf24pipes[1] = { 0xF0F0F0F011LL };

int data[9];                  // define array used to save the communication data
#define BTNA  0x01
#define BTNB  0x02
#define BTNC  0x04
#define BTND  0x08
#define BTNU1 0x10
#define BTNU2 0x20
int btns = 0;                 //will take a mix of buttons above

const int r6Pin = A5;        // define R6
const int r1Pin = A4;        // define R1
const int led1Pin = 6;        // define pin for LED1 which is close to NRF24L01 and used to indicate the state of NRF24L01
const int led2Pin = 7;        // define pin for LED2 which is the mode is displayed in the car remote control mode  
const int led3Pin = 8;        // define pin for LED3 which is the mode is displayed in the car auto mode
const int btnAPin = 2;           // define pin for D2
const int btnBPin = 3;           // define pin for D3
const int btnCPin = 4;           // define pin for D4
const int btnDPin = 5;           // define pin for D5
const int u2ypin = A3;       // define pin for direction x of joystick U2
const int u2xpin = A2;       // define pin for direction X of joystick U2
const int u1ypin = A1;       // define pin for direction Y of joystick U1
const int u1xpin = A0;       // define pin for direction X of joystick U1
const int u1swpin = 1;      // define pin for direction Y of joystick U1
const int u2swpin = 1;      // define pin for direction Y of joystick U1

void setup() 
{
  if (DEBUG)
  {
    Serial.begin (115200); // initialize serial port
    while (!Serial); //delay for Leonardo
  }
  debug_println ("i:wire config");
  radio.begin();                      // initialize RF24
  if (radio.isChipConnected())
  {
    radio.setRetries(0, 15);            // set retries times
    radio.setDataRate (RF24_250KBPS); //Fast enough.. Better range
    radio.setChannel (108); //2.508 Ghz - Above most Wifi Channels
    radio.setPALevel(RF24_PA_LOW);      // set power
    radio.openWritingPipe(rf24pipes[0]);   // open delivery channel
    radio.stopListening();              // stop monitoring
    // optionally, reduce the payload size.  seems to
    // improve reliability
    //radio.setPayloadSize(8);
  }
  pinMode(led1Pin, OUTPUT);   // set led1Pin to output mode
  pinMode(led2Pin, OUTPUT);   // set led2Pin to output mode
  pinMode(led3Pin, OUTPUT);   // set led3Pin to output mode
  pinMode(btnAPin, INPUT_PULLUP);   // set APin to output mode
  pinMode(btnBPin, INPUT_PULLUP);   // set BPin to output mode
  pinMode(btnCPin, INPUT_PULLUP);   // set CPin to output mode
  pinMode(btnDPin, INPUT_PULLUP);   // set DPin to output mode  
  debug_println ("i:setup done");
}

void loop() 
{
  // put the values of rocker, switch and potentiometer into the array
  data[0] = analogRead (u1xpin); //left joy, horizontal
  data[1] = analogRead (u1ypin); //left joy, vertical
  data[2] = analogRead (u2xpin); //right joy, horizontal
  data[3] = analogRead (u2ypin); //right joy, vertical
  data[4] = analogRead (r1Pin);  //left pot
  data[5] = analogRead (r6Pin);  //right pot
  //
  data[6] = 0x0;
  if(digitalRead (btnAPin)==LOW)
  {
    data[6] |= BTNA;
  }
  if( digitalRead(btnBPin)==LOW)
  {
    data[6] |= BTNB;
  }
  if(digitalRead(btnCPin)==LOW)
  {
    data[6] |= BTNC;
  }
  if(digitalRead(btnDPin)==LOW)
  {
    data[6] |= BTND;
  }
  data[7] = 0;
  data[8] = 0;
  // send array data. If the sending succeeds, open signal LED
  digitalWrite(led1Pin,HIGH);
  debug_print ("#i:<data ");
  for (int i = 0; i < sizeof (data) / sizeof (int); i++)
  {
    debug_print (", ");
    debug_print (data[i], DEC);
  }
  debug_println ();
  //
  radio.write (data, sizeof (data));
  // delay for a period of time, then turn off the signal LED for next sending
  delay(2);
  digitalWrite(led1Pin,LOW);
}
