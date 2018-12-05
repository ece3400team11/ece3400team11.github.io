#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

const uint64_t pipes[2] = { 0x000000001CLL, 0x000000001DLL };

void initRadio() {
  radio.begin();

  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);
  
  // put your setup code here, to run once:
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
}

void sendData() {
  radio.stopListening();
  
  // pack data into payload
  unsigned long payload = get_maze_data_payload();
  
  bool ok = false;
  while(!ok) {
    ok = radio.write( &payload, sizeof(unsigned long) );
  }
}
