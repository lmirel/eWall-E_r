
#include <SPI.h>
#include "RF24.h"
//#include <Servo.h>

#define DEBUG 1
#define debug_println(...) \
            do { if (DEBUG) Serial.println(__VA_ARGS__); } while (0)
#define debug_print(...) \
            do { if (DEBUG) Serial.print(__VA_ARGS__); } while (0)

RF24 radio (2, 4);              //define the object to control NRF24L01
//byte addresses[6] = "00007";  // define communication address which should correspond to remote control
const uint64_t rf24pipes[1] = { 0xF0F0F0F011LL }; // define communication address which should correspond to remote control
const int actPin = 13;           //activity pin
char have_radio = 0;
//
int lightMode = 0;              //0 off, 1 on, 2 alternate left/right, 3 alternate on/off
#define ACTMAX  50
int actKnt  = 0;
int actRate = ACTMAX;
//
#define BTNA  0x01
#define BTNB  0x02
#define BTNC  0x04
#define BTND  0x08
#define BTNU1 0x10
#define BTNU2 0x20
const int tonePin = 3;        //tone pin
//
#define SERVO_MAX   2000
#define SERVO_MID   1500
#define SERVO_MIN   1000
#define SERVO_RNG   500
#define SERVO_OFF   0
//
#define THR_MAX     1023
#define THR_MIN     0
#define ADC_MAX     1023
#define ADC_MID     511
#define ADC_MIN     0
int dirv = SERVO_MID;
int dirt = THR_MAX;             //direction throttle
const int dirServoPin = 5;      //dir servo pin
//Servo dirServo;                 //define servo to control turning of smart car
//
int powv = SERVO_MID;
int powt = THR_MAX;             //power throttle
const int powServoPin = 6;      //power servo pin
//Servo powServo;                 //define servo to control turning of smart car
//
int data[9];                    //define array used to save the communication data
//
// wired connections: RIGHT motor
#define HG7881_A_IA 5 // D5 --> Motor A Input A PWM --> MOTOR A +
#define HG7881_A_IB 7 // D4 --> Motor A Input B --> MOTOR A -
 
// functional connections
#define MOTOR_A_PWM HG7881_A_IA // Motor A PWM Speed
#define MOTOR_A_DIR HG7881_A_IB // Motor A Direction

// wired connections: LEFT motor
#define HG7881_B_IA 6 // D6 --> Motor B Input A PWM --> MOTOR B +
#define HG7881_B_IB 8 // D7 --> Motor B Input B --> MOTOR B -
 
// functional connections
#define MOTOR_B_PWM HG7881_B_IA // Motor B PWM Speed
#define MOTOR_B_DIR HG7881_B_IB // Motor B Direction
int bmdir = 0;
int amdir = 0;
// the actual values for "fast" and "slow" depend on the motor
#define PWM_NONE 0    // arbitrary no speed PWM duty cycle
#define PWM_SLOW 150  // arbitrary slow speed PWM duty cycle
#define PWM_FAST 255  // arbitrary fast speed PWM duty cycle
#define DIR_DELAY 300 // brief delay for abrupt motor changes
//
void bhalt ()
{
  if (bmdir != 0)
  {
    debug_println ("stop B");
    digitalWrite (MOTOR_B_DIR, HIGH); //Motor OFF
    digitalWrite (MOTOR_B_PWM, HIGH);
    delay (DIR_DELAY);
    bmdir = 0;
  }
}

//
void ahalt ()
{
  if (amdir != 0)
  {
    debug_println ("stop A");
    digitalWrite (MOTOR_A_DIR, HIGH); //Motor OFF
    digitalWrite (MOTOR_A_PWM, HIGH);
    delay (DIR_DELAY);
    amdir = 0;
  }
}

void go_forward (int spd)
{
  debug_print ("pow ");
  debug_print (spd);
  debug_println ("going forward");
  //B fw
  if (!bmdir == 1)
    bhalt ();
  bmdir = 1;
  //A fw
  if (!amdir == 1)
    ahalt ();
  amdir = 1;
  // set the motor speed and direction
  //B fw
  digitalWrite (MOTOR_B_DIR, HIGH);
  analogWrite (MOTOR_B_PWM, 255 - spd); // PWM speed = fast
  //A fw
  digitalWrite (MOTOR_A_DIR, LOW);
  analogWrite (MOTOR_A_PWM, spd); // PWM speed = fast
}

void go_backward (int spd)
{
  debug_print ("pow ");
  debug_print (spd);
  debug_println ("going backward");
  //B bw
  if (!bmdir == -1)
    bhalt ();
  bmdir = -1;
  //A bw
  if (!amdir == -1)
    ahalt ();
  amdir = -1;
  // set the motor speed and direction
  //B bw
  digitalWrite (MOTOR_B_DIR, LOW); // direction = forward
  analogWrite (MOTOR_B_PWM, spd); // PWM speed = fast
  //A bw
  digitalWrite (MOTOR_A_DIR, HIGH); // direction = forward
  analogWrite (MOTOR_A_PWM, 255 - spd); // PWM speed = fast
} 

//B - left
//A - right
//B bw, A fw
void go_left (int spd)
{
  debug_print ("pow ");
  debug_print (spd);
  debug_println ("going left");
  //B bw
  if (!bmdir == -1)
    bhalt ();
  bmdir = -1;
  //A fw
  if (!amdir == 1)
    ahalt ();
  amdir = 1;
  // set the motor speed and direction
  //B bw
  digitalWrite (MOTOR_B_DIR, LOW); // direction = forward
  analogWrite (MOTOR_B_PWM, spd); // PWM speed = fast
  //A fw
  digitalWrite (MOTOR_A_DIR, LOW); // direction = forward
  analogWrite (MOTOR_A_PWM, spd); // PWM speed = fast
}

//B - left
//A - right
//B fw, A bw
void go_right (int spd)
{
  debug_print ("pow ");
  debug_print (spd);
  debug_println ("going right");
  //B fw
  if (!bmdir == 1)
    bhalt ();
  bmdir = 1;
  //A bw
  if (!amdir == -1)
    ahalt ();
  amdir = -1;
  // set the motor speed and direction
  //B fw
  digitalWrite (MOTOR_B_DIR, HIGH);
  analogWrite (MOTOR_B_PWM, 255 - spd); // PWM speed = fast
  //A bw
  digitalWrite (MOTOR_A_DIR, HIGH); // direction = forward
  analogWrite (MOTOR_A_PWM, 255 - spd); // PWM speed = fast
}
//
void setup ()
{
  pinMode (actPin, OUTPUT);   // set led1Pin to output mode
  digitalWrite (actPin, HIGH);
  //pinMode (A0, OUTPUT);
  //digitalWrite (A0, LOW);
  //pinMode (A1, OUTPUT);
  //digitalWrite (A1, LOW);
  if (DEBUG)
  {
    Serial.begin (115200); // initialize serial port
    while (!Serial); //delay for Leonardo
  }
  debug_println ("i:wire config");
  radio.begin ();                      // initialize RF24
  if (radio.isChipConnected ())
  {
    radio.setRetries (0, 15);            // set retries times
    radio.setDataRate (RF24_250KBPS); //Fast enough.. Better range
    radio.setChannel (108); //2.508 Ghz - Above most Wifi Channels
    radio.setPALevel (RF24_PA_LOW);      // set power
    radio.openReadingPipe (1, rf24pipes[0]);// open delivery channel
    radio.startListening ();             // start monitoring
    // optionally, reduce the payload size.  seems to
    // improve reliability
    //radio.setPayloadSize(8);
    debug_println ("i:wire ready");
    have_radio = 1;
  }
  else
  {
    debug_println ("w:wire not available");
  }
  //
  pinMode (MOTOR_A_DIR, OUTPUT );
  pinMode (MOTOR_A_PWM, OUTPUT );
  digitalWrite (MOTOR_A_DIR, LOW ); //Motor start OFF
  digitalWrite (MOTOR_A_PWM, LOW );
  //
  pinMode (MOTOR_B_DIR, OUTPUT );
  pinMode (MOTOR_B_PWM, OUTPUT );
  digitalWrite (MOTOR_B_DIR, LOW ); //Motor start OFF
  digitalWrite (MOTOR_B_PWM, LOW );
  //dirServo.attach (dirServoPin);  // attaches the servo on servoDirPin to the servo object
  //powServo.attach (powServoPin);  // attaches the servo on servoDirPin to the servo object
  //alow for servos move
  debug_println ("i:setup done");
  delay (200);
  digitalWrite (actPin, LOW);
}

void loop ()
{
  actKnt++;
  if (actKnt > actRate)
    actKnt = 0;
  //show activity
  if ((actKnt % 2) == 1)
    digitalWrite (actPin, HIGH);
  else
    digitalWrite (actPin, LOW);
  if (!have_radio)
    delay (1000);
  debug_print ("#i");
  debug_print (actKnt);
  debug_println (": wait for data");
  //process commands from remote
  if (have_radio && receiveData ())
  {
    digitalWrite (actPin, HIGH);
    //throttle trim   data[4]
    powt = map (data[4], ADC_MIN, ADC_MAX, ADC_MIN, ADC_MID);
    //direction trim  data[5]
    dirt = map (data[5], ADC_MIN, ADC_MAX, ADC_MIN, ADC_MID);
    //
    //dirv = map (data[2] + dirt - ADC_MID, ADC_MIN, ADC_MAX, SERVO_MIN, SERVO_MAX);
    dirv = data[2];//map (data[2], ADC_MIN, ADC_MAX, ADC_MID - dirt, ADC_MID + dirt);
    powv = data[1];//map (data[1], ADC_MIN, ADC_MAX, ADC_MID - powt, ADC_MID + powt);
#if 1
    debug_print ("#i:dir ");
    debug_print (dirv);
    debug_print (", dirt ");
    debug_print (dirt);
    debug_print (", pow ");
    debug_print (powv);
    debug_print (", powt ");
    debug_print (powt);
    //
    debug_print (", knt ");
    debug_print (actKnt);
    debug_println ();
#endif
    //
    if (powv < 500)
    {
      //go back
      powv = map (powv, ADC_MIN, ADC_MAX, ADC_MID - powt, ADC_MID + powt);
      powv = map (powv, ADC_MIN, ADC_MAX, PWM_NONE, PWM_FAST);
      go_backward (255 - powv);
    }
    else if (powv > 540)
    {
      //go forward
      powv = map (powv, ADC_MIN, ADC_MAX, ADC_MID - powt, ADC_MID + powt);
      powv = map (powv, ADC_MIN, ADC_MAX, PWM_NONE, PWM_FAST);
      go_forward (powv);
    }
    else
    {
      if (dirv < 500)
      {
        //go right
        dirv = map (dirv, ADC_MIN, ADC_MAX, ADC_MID - dirt, ADC_MID + dirt);
        dirv = map (dirv, ADC_MIN, ADC_MAX, PWM_NONE, PWM_FAST);
        go_right (255 - dirv);
      }
      else if (dirv > 540)
      {
        //go left
        dirv = map (dirv, ADC_MIN, ADC_MAX, ADC_MID - dirt, ADC_MID + dirt);
        dirv = map (dirv, ADC_MIN, ADC_MAX, PWM_NONE, PWM_FAST);
        go_left (dirv);
      }
      else// if (dirv == SERVO_MID)
      {
        //stop servos
        ahalt ();
        bhalt ();
      }
    }
  }
  else
  {
    if (have_radio)
    {
      digitalWrite (actPin, LOW);
      //stop servos
      ahalt ();
      bhalt ();
      //dirServo.writeMicroseconds (SERVO_MID);
      //powServo.writeMicroseconds (SERVO_MID);
      //
      debug_print ("#w:");
      debug_print (actKnt);
      debug_println (":remote not available");
    }
  }
}

#define WIRE_TMO  1000
int wire_expect_bytes (byte *ldata, int bts)
{
  int ava = 0, i;
  int c;
  //
  //debug_print ("i:wire read expecting ");
  //debug_println (bts);
  if (bts <= 0)
    return 0;
  //wait for timeout
  while (!radio.available ())
  {
    if (i++ < WIRE_TMO)
      delayMicroseconds (250);
    else
    {
      debug_println ("w:timed out expecting");
      break;
    }
  }
  //
  if (radio.available ())
  {
    //debug_print ("i:wire read available ");
    if (radio.getPayloadSize () >= bts)
    {
      radio.read (ldata, bts);
      //
      //debug_println (bts);
      return bts;
    }
    else
    {
      //debug_println (radio.getPayloadSize ());
    }
  }
  //
  return 0;
}

int receiveData ()
{
  int dl = sizeof (data);
  int res = 0;
  if (!radio.isChipConnected ())
  {
    return res;
  }
  // read all the data
  if (wire_expect_bytes ((byte *)data, dl) == dl)
  {
#if 0
    debug_print ("#i:>data ");
    for (int i = 0; i < dl / sizeof (int); i++)
    {
      debug_print (", ");
      debug_print (data[i], DEC);
    }
    debug_println ();
#endif
    res = 1;
  }
  return res;
}
