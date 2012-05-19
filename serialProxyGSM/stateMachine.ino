/*

 we use a state machine for control flow.

*/

enum states {
  INIT,
  DO_POKE,  WAIT_POKE,
  DO_RESET,
  DO_BOOT,  WAIT_BOOT,
  WAIT_CREG,
  DO_DCONT, WAIT_DCONT,
  DO_D,     WAIT_D,
  DO_A,     WAIT_A,
  DO_OPEN,  WAIT_OPEN,
  DO_HELLO, WAIT_HELLO,
  WAIT_CONNECT,CONNECTED,
  DO_CLOSE,WAIT_CLOSE,
  DO_NOTHING
};

enum states state=INIT; // initial state

void stateMachine(){

  switch(state){

  // initially, we don't know if the device is on or not.
  // we poke it by sending the 'AT' command.
  //   if it's on, it'll reply with 'OK'
  //   otherwise, we want to press the power button
  // ideally, we'd connect a wire to the 'Ready' pin on the GSM
  case INIT:
    delay(2000);   //wait a little before poking.
    state=DO_POKE;
    break;

  // emit a poke
  case DO_POKE: 
    Serial1.write("\r\rAT\r");
    Serial.print("Poking.");
    gsmOK=false;
    state=WAIT_POKE;
    break;

  // wait for OK.
  //   - if we get the OK, ask if we're registered
  //   - if we're out of pokes, we probably need to boot.
  case WAIT_POKE:
    if(gsmOK){
      // it's already on.
      //  we should check what state it's in.
      Serial.println("already on. are we registered?");
      Serial1.write("AT+CREG?\r"); // check creg
      gsmCREG=0;
      state=WAIT_CREG;
      break;
    }
    pokeTimer--;
    if(!pokeTimer){
      state=DO_BOOT;
      Serial.println("");
      break;
    }
    Serial.print(".");
    delay(1000);
    break;

  // send the reset command.
  // we don't use this anymore.
  case DO_RESET:
    Serial.println("Rebooting GSM.");
    Serial1.write("AT+SFUN=RST\r");
    delay(10000);
    state=DO_BOOT;
    break;

  // push the power button
  case DO_BOOT:
    Serial.println("booting GSM...");
    pinMode(ONKEY, OUTPUT);
    state=WAIT_BOOT;
    break;

  // wait for the 'Ready' message, then stop pushing
  case WAIT_BOOT:
    // sometimes it's in a state where the power button doesn't turn it on... perhaps we should stop pressing the button, and then start again
    if(!gsmReady) break;
    Serial.println("Ready indeed.");
    pinMode(ONKEY, INPUT);
    state=WAIT_CREG;
    break;

  // wait for CREG message.
  // this either happens when we boot, or after we emit CREG?
  case WAIT_CREG:
    if(gsmCREG!=1 && gsmCREG!=5)  break;
    Serial.println("We are registered.");
    state=DO_DCONT;
    break;

  // Define PDP context
  case DO_DCONT: 
    delay(5000); //let's just wait a second...
    Serial.println("Defining PDP context...");
    Serial1.write("AT+AIPDCONT=\"wap.cingular\"\r");
    gsmOK=false;
    state=WAIT_DCONT;
    break;

  // wait for an OK
  case WAIT_DCONT:
    if(!gsmOK) break;
    Serial.println("OK.");
    state=DO_A;
    break;

  // detatch from GPRS.
  // sometimes we need to do this
  case DO_D:
    Serial.println("GPRS detatch...");
    Serial1.write("AT+AIPA=0\r");
    state=WAIT_D;
    gsmOK=false;
    gsmERR=false;
    break;

  // after detatching, try to attatch again.
  case WAIT_D:
    if(gsmERR){
      Serial.println("error detatching. giving up.");
      state=DO_NOTHING; //now what?
    }
    if(!gsmOK) break;
    state=DO_A;
    break;

  // attatch GPRS
  case DO_A:
    Serial.println("GPRS attach...");
    Serial1.write("AT+AIPA=1\r");
    gsmOK=false;
    gsmERR=false;
    state=WAIT_A;
    break;

  // wait for OK
  case WAIT_A:
    // on error, try again.
    if(gsmERR){
      // error 301 "Does not connect to GPRS network"
      Serial.println("error. maybe we're already Attatched; we'll try to disconnect.");
      state=DO_D;   // disconnect
      delay(10000); // wait 10 seconds before retrying
      
      if(gsmCREG!=1 && gsmCREG!=5){ // no longer registered
        state=WAIT_CREG;            // maybe we'll get registered again later.
        break;
      }
      break;
    }

    if(!gsmOK) break;

    Serial.println("GPRS attatched.");
    state=DO_OPEN;
    break;

  // try to connect to server
  case DO_OPEN:
    Serial.println("opening socket...");
    
    Serial1.write("AT+AIPO=1,,\"");
    Serial1.write(serverName);
    Serial1.write("\",");
    Serial1.write(serverPort);
    Serial1.write(",0,,1\r");

    gsmOK=false;
    gsmERR=false;
    state=WAIT_OPEN;
    break;

  // wait for OK.
  // if failure, disconnect.
  case WAIT_OPEN:
    if(gsmERR){
      // error 304 "Fail connection at this socket" - should try somthing else after a while.
      Serial.println("error opening socket. perhaps it's already open...");
      gsmOK=true;
      gsmERR=false;
      state=DO_CLOSE;
      break;
    }
    if(!gsmOK) break;
    Serial.println("socket open.");
    socketOpen=true;
    state=DO_HELLO;
    break;

  // once we're conneccted, send the 'hello.' message
  case DO_HELLO:
    Serial.println("saying hello...");
    Serial1.write("AT+AIPW=1,\"68656C6C6F0D0A\"\r"); // "hello.\r\n"
    gsmOK=false;
    gsmERR=false;
    state=WAIT_HELLO;
    break;

  // wait for the message to go through.
  //  if there's an error, disconnect.
  case WAIT_HELLO:
    if(gsmERR){
      Serial.println("error making request.");
      //  we should inverstigate
      state=DO_CLOSE;
    }
    if(!gsmOK) break;
    Serial.println("hello.");
    state=WAIT_CONNECT;
    break;

  // we're connected and we said hello.
  // now we wait for the other end to connect
  case WAIT_CONNECT:
    if(socketRemClose){ // we're discnnected. reconnect?
      Serial.println("oops, looks like we're disconnected.");
      state=DO_CLOSE;
    }
    if(!linkOpen)break;
    Serial.println("the link is open.");
    writeReady=true;
    state=CONNECTED;
    break;

  // both ends are connected!
  // we're forwarding data from the serial port to the GMS, and vice-versa.
  //  we wait for the previous message to go through before sending another.
  case CONNECTED:
  
    // it's like a mini-state machine!

    // the link was closed. close our side.
    if(socketRemClose){ 
      state=DO_CLOSE;
      break;
    }
    
    // we have data to send, and the GSM is ready.
    if(writePending && writeReady){
      writeReady=false;   // it's not ready any more
      writeWait=true;     // we'll be waiting for an OK
      gsmOK=false;        // clear the OK flag
      doSer2();           // write the buffer to the device
      writePending=false; //we don't have data anymore.
    }
    
    // we got an OK, presumably from the last write. we're ready to send again.
    if(writeWait && gsmOK){ 
      writeWait=false;    // we're not waiting any more
      writeReady=true;    // it's ready to write!
    }

    break;

  // close the socket.
  case DO_CLOSE:
    Serial.println("closing socket...");
    Serial1.write("AT+AIPC=1\r"); // close socket 1
    gsmOK=false;
    gsmERR=false;
    state=WAIT_CLOSE;
    socketRemClose=false;
    break;

  // wait for OK, then open again.
  case WAIT_CLOSE:
    if(gsmERR){
      Serial.println("error closing socket. now what? I guess we'll try to connect again?");
      state=DO_OPEN;
    }
    if(!gsmOK)break;
    Serial.println("link closed. now let's try to reconnect. (?)");
    socketOpen=false;
    linkOpen=false;
    state=DO_OPEN;
    break;

  // purgatory
  case DO_NOTHING:
    break;
    
  }
}


