
/*
 serialProxyGSM creates a proxy between a serial port and a remote tcp port using a ADH8066 GSM Module.

 in the main file, we'll:
   set some settings
   define some flags
   do the requisite setup() and loop() routines
   handle input from the debug port

*/

char* serverName="login.sccs.swarthmore.edu";
char* serverPort="8801";

int ledPin = 13;         // LED connected to digital pin 13
int ONKEY = 2;           // GSM  "ON Key"
// it's 3.3 volts, but active low, so we can `push' it by pulling it low


// some flags - these could be done more efficiently
// (why are they all ints when most of them only need to be 1 bit?)

int socketOpen=false;     // the socket is open.
int socketRemClose=false; // the server hung up.
int linkOpen=false;       // serial link open (almost the same as socketOpen)
int gsmReady=false;       // we've recieved the Ready message
int gsmOK=false;          // we've recieved "OK"
int gsmERR=false;         // we've recieved "ERROR"  - set to error number, if present
int gsmCREG=0;            // indicates that we're registered to the cellular network
int gsmA=false;           // attatched? - we set this, but never use it.
int pokeTimer=10;         // how long to wait for response from 'AT' command at boot

int writePending=false;   // we're ready to write
int writeWait=false;      // we just wrote and we're waiting for the OK
int writeReady=false;     // the device is ready to write (we got an OK after the last write)

int counter=0; // for blinking the LED.


#define inBuf1_len 2048
#define inBuf2_len 2048
//#define outBuf_len 2048

char inBuf1[inBuf1_len]; // for buffering input from Serial1, also used to hold the hex-encoded version
char inBuf2[inBuf2_len]; // for buffering input from Serial2
//char outBuf[outBuf_len]; // for holding hex-encoded outgoing data - unneccissary if we encode hex backwards

int bufSize1=0;          //number of buffered chars
int bufSize2=0;        //number of buffered chars


void setup(){

  // open all the serial ports!
  Serial.begin(115200);  // for debugging
  Serial1.begin(115200); // the GSM module
  Serial2.begin(9600);   // local serial port

  // LED
  pinMode(ledPin, OUTPUT);
  
  // don't pull the power button
  pinMode(ONKEY, INPUT);   
  digitalWrite(ONKEY, LOW);

  Serial.println("Oh Hai!");
}

void loop(){

  // blink the heartbeat LED - to show we're looping
  counter=(counter+1)%1000;
  if(counter==  0) digitalWrite(ledPin, HIGH);
  if(counter==500) digitalWrite(ledPin, LOW);

  // run the state machine
  stateMachine();

  // poll for serial activity
  if( Serial.available()) serEvent();  // Arduino
  if(Serial1.available()) serEvent1(); // GSM
  if(Serial2.available()) serEvent2(); // local serial
}


//for now, send input from Arduino to GSM
void serEvent(){
  while(Serial.available()){
    Serial1.write(Serial.read());
  }
}


