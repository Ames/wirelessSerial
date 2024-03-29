
/*

 Serial1 is connected to the GSM module.
 
 we recieve AT responses and parse them.
 
*/

void serEvent1(){

  while(Serial1.available()){

    // read a char from the module
    char c=Serial1.read();

    // end of line - we might only care about \r
    if(c=='\r' || c=='\n'){

      // if the buffer is empty, bail.
      if(!bufSize1) continue;

      // parse the line.
      doSer1();            

      // 'clear' the buffer
      bufSize1=0;
      continue;
    }

    // ignore invalid chars
    if(c>=127 || c<32) continue;

    // buffer the char
    inBuf1[bufSize1++]=c;

    // buffers can overflow.
    if(bufSize1==2047) Serial.println("inBuf1 overflowed!");

  } // end while(Serial1.available())

} // end serEvent1()


// buffers a line from the GSM Module.
void doSer1(){
  
  // null terminate the string
  inBuf1[bufSize1++]=0;

  // debug
  Serial.println(inBuf1); 

  // many things are in the form COMMAND: DATA
  char* cmd=strtok(inBuf1,":");
  char* dat=strtok(0,":");

  if(dat && cmd){
    dat++;      // there's a space after the colon.
  }else{
    cmd=inBuf1; // this might be the default
    dat=0;
  }
  
  
  ////  now we handle different types of message  ////

  // Ready message - after boot
  if(!strcmp(cmd,"Ready")) gsmReady=true;


  // OK - something succeeded
  if(!strcmp(cmd,"OK")) gsmOK=true;


  // ERROR - something failed
  if(!strcmp(cmd,"ERROR")){
    
    gsmERR=true;
    
    //parse error number, if present
    if(dat) gsmERR=atoi(dat);

    Serial.print("gsmERROR=");
    Serial.println(gsmERR);
  }

  // network registration
  if(!strcmp(cmd,"+CREG")){
    if(dat){
      gsmCREG=atoi(dat);
    }
    else{ // this shouldn't happen
      gsmCREG=0; 
    }
  }
  
  /* CREG values, from docs:
   0 : not registered, ME is not currently searching a new operator to register to
   1 : registered, home network
   2:  not registered, but ME is currently searching a new operator to register to
   3:  registration denied
   4 : unknown
   5:  registered, roaming
  */

  // Receive TCP data
  if(!strcmp(cmd,"+AIPRTCP")){

    // +AIPRTCP: <socket id>, <protocol>, <recv buf len>,<send buf len>,<recv bytes>,<ASCII data stream>

    if(!dat){ //this shouldn't happen
      Serial.println("no dat.");
      return;
    }
    
    // the data is in hex between quotes.
    // we're lazy and just grabbing that part.
    strtok(dat,"\"");
    char* hex=strtok(  0,"\"");

    if(!hex){ // this also shouldn't happen
      Serial.println("no hex.");
      return;
    }

    // decode hex back into inBuf1. it's funny, because it's already there.
    int len=hexDecode(hex,inBuf1);

    // debug
    Serial.print("REMOTE> ");
    for(int i=0;i<len;i++){
      Serial.write(inBuf1[i]);
    }
    Serial.println("");

    // if linkOpen, send the message to the serial port.
    if(linkOpen){
      for(int i=0;i<len;i++){
        Serial2.write(inBuf1[i]);
      }
    }

    // this is a special welcome message that says we're connected.
    if(!strcmp(inBuf1,"welcome.\n")) linkOpen=true;

    return; //we've messed up inBuf1 anyway.
  }


  // socket closed.
  if(!strcmp(cmd,"+AIPCI")){
    Serial.println("TCPIPSockRemoteClosed");
    socketRemClose=true;
  }

  // GSM attatched.
  if(!strcmp(cmd,"+AIPA")){
    gsmA=true;
  }

  // GSM closed.
  if(!strcmp(cmd,"+AIPC")){
    socketOpen=false;
  }

} //end doSer1()

