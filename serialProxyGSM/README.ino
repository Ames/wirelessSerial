/*

serialProxy creates a proxy between a serial port and a remote tcp port using a ADH8066 GSM Module.

An Arduino MEGA is used because it has multible serial ports and plenty of memory.

The GSM module is connected to Serial1.
The Serial device is on Serial2.


The "ON Key" of the GSM is connected to digital pin 2.


To talk to a server, the module must:
 -boot
 -register to the network (CREG) (happens automatically after boot)
 -define a PDP context (AIPDCONT)
 -attatch (AIPA)
 -open the socket (AIPO)
then we can write (AIPW) and read (AIPRTCP - happens automatically by default)



The program works as follows:
 -we start by checking if the module is on by repeatedly emitting the 'AT' command and waiting for 'OK'.
   -if we don't get a response, press the "ON Key" until 'Ready'
   -if we do, emit 'CREG?' to check if it's registered to the network
 -wait for registration (CREG)
 -define the PDP context (AIPDCONT), wait for OK
 -attach GPRS (AIPA)
   -if error, detatch and try again
 -open the socket (AIPO) to the server
   -if error, try disconnecting and try again
 -send "hello.", wait for "welcome."
 -now we're in CONNECTED mode.
   -anything written to the serial port is sent to the server
   -anything recieved from the server is written to the serial port
   -if the server disconnects, we close the socket and try to connect again.

The above functionality is implemented using a state machine.
This design allows for the event-driven nature of the system.

A seperate program runs on the server and allows two clients to connect and talk to each other.






 
 TODO:
   - handle more errors, including GMS module reset (SystemInit)
   - when should we send data from serial? right now it sends as soon as it reads, but perhaps buffering a little before sending would be more efficient.
   - yuck.
   - maybe we want some kind of hearbeat signal, so we know if the socket is disconnected
   - alternatively, we could call or SMS the device to wake it up and tell it to connect.
   - can we busyloop in low-power mode?
 */
