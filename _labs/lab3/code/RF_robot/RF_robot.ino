#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

RF24 radio(9,10);

const int NORTH = 3;
const int EAST  = 2;
const int SOUTH = 1;
const int WEST  = 0;

unsigned char x=0;
unsigned char y=0;
int dir = EAST;

unsigned char maze[3][3] = {
  {0x90, 0x80, 0xc0},
  {0x10, 0x00, 0x40},
  {0x30, 0x20, 0x60}
};

const uint64_t pipes[2] = { 0x000000001CLL, 0x000000001DLL };

void setup() {
  radio.begin();

  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(4);
  
  // put your setup code here, to run once:
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

//  radio.printDetails();
}

// advance the robot's position based on its direction
void adv(){
  switch(dir){
      case NORTH:
        y=y-1;
        break;
      case EAST:
        x=x+1;
        break;
      case SOUTH:
        y=y+1;
        break;
      case WEST:
        x=x-1;
        break;
    }
}

// get the current position's maze data and perform Right hand following
void follow(){
  unsigned char curr = maze[y][x];
  if((bitRead(curr, dir+4)==0) && (bitRead(curr, (dir+1)%4 + 4)==1)){
    adv();
  }
  else if(bitRead(curr, (dir+1)%4 + 4)==0){
    dir = (dir+1)%4;
    adv();
  }
  else if((bitRead(curr, dir+4)==1) && (bitRead(curr, (dir+1)%4 + 4)==1) && (bitRead(curr, (dir+3)%4 + 4)==0)){
    dir = (dir+3)%4;
    adv();
  }
}

void loop() {
  // get current maze information
  unsigned char curr = maze[y][x];   // Single byte with walls, treasures
  
  radio.stopListening();
  
  // pack data into payload
  unsigned long payload = 0;
  payload |= curr;
  payload = payload << 8;
  payload |= x;
  payload = payload << 8;
  payload |= y;
  
  bool ok = false;
  while(!ok) {
    ok = radio.write( &payload, sizeof(unsigned long) );
  }

  // optional
  delay(1000);

  // update position based on maze data
  follow();
  

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
}
