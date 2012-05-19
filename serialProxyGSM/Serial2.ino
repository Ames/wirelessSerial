
/*

 Serial1 is connected to the local device.
 
 we recieve data and, if linked, send it to GSM module.
 
 */

void serEvent2(){

  while(Serial2.available()){

    char c=Serial2.read();


    if(c>255 || c<0) // invalid char range
      continue;

    // debug
    Serial.print(c);

    // buffer the char
    inBuf2[bufSize2++]=c;

    // BAD!
    if(bufSize2==2047) Serial.println("inBuf2 overflowed!");

    // it's unclear when we should buffer until. It might vary.

    if(bufSize2){
      writePending=true;
    }

  } // end while

} // end serEvent1


// send data to GSM.
// called by the state machine
void doSer2(){

  //debug
  Serial.print("LOCAL> ");
  Serial.println(inBuf2); // prob. stupid

  // encode as hex
  hexEncode(inBuf2,inBuf2,bufSize2);

  // send the message
  Serial1.print("AT+AIPW=1,\"");
  Serial1.print(inBuf2);
  Serial1.print("\"\r");

  bufSize2=0;

}

