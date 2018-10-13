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

//  radio.printDetails();
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
}
