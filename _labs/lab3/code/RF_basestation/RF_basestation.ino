#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x0000000026LL, 0x0000000027LL };

void setup() {
  radio.begin();

  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);

  // base station
  Serial.begin(9600);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  // reset GUI
  Serial.println("reset");

//  radio.printDetails();
}

String convert(unsigned char x, unsigned char y, unsigned char curr){
  String s = String(y)+","+String(x);
  if(bitRead(curr,4)==1){s = s + ",west=true";}
  if(bitRead(curr,5)==1){s = s + ",south=true";}
  if(bitRead(curr,6)==1){s = s + ",east=true";}
  if(bitRead(curr,7)==1){s = s + ",north=true";}
  return s;
}

void loop() {
  // base station
  if ( radio.available() ) {
    unsigned long payload;
    bool done = false;
    while (!done) {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &payload, sizeof(unsigned long) );
      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(20);
    }
    // unpack data (location, walls, etc.) from payload
    unsigned char y = payload & 0xff;
    payload = payload >> 8;
    unsigned char x = payload & 0xff;
    payload = payload >> 8;
    unsigned char curr = payload & 0xff;
    
    // print data to GUI
    Serial.println(convert(x,y, curr));

    // Optionally do an ACK
//      radio.stopListening();
//
//      // Send the final one back.
//      radio.write( &payload, sizeof(unsigned long) );
//
//      // Now, resume listening so we catch the next packets.
//      radio.startListening();
  }
}
