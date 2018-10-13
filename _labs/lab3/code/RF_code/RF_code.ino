#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x0000000026LL, 0x0000000027LL };

#define ROBOT

void setup() {
  radio.begin();

  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);
  
  // put your setup code here, to run once:
  #ifdef ROBOT
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  #else
    // base station
    Serial.begin(9600);
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
    radio.startListening();
  #endif

//  radio.printDetails();
}

void loop() {
  // put your main code here, to run repeatedly:
  #ifdef ROBOT
    // TODO get current maze information
    // pack data into payload
    
    radio.stopListening();
    // TODO set payload
    unsigned long payload = millis();
    bool ok = false;
    while(!ok) {
      ok = radio.write( &payload, sizeof(unsigned long) );
    }

    // optional
    delay(1000);
    
  
    // optionally wait for an ACK
//    radio.startListening();
//    unsigned long started_waiting_at = millis();
//    bool timeout = false;
//    while ( ! radio.available() && ! timeout )
//      if (millis() - started_waiting_at > 200 )
//        timeout = true;
//    unsigned long ACK;
//    radio.read( &ACK, sizeof(unsigned long) );
//    check ACK payload?

  #else
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
      // TODO
      // unpack data (location, walls, etc.) from payload
      // Serial.println(unpacked data);

      // Optionally do an ACK
//      radio.stopListening();
//
//      // Send the final one back.
//      radio.write( &payload, sizeof(unsigned long) );
//
//      // Now, resume listening so we catch the next packets.
//      radio.startListening();
    }
  #endif
}
