#include <SoftwareSerial.h>

SoftwareSerial mySerial(13, 12); // RX, TX

void setup() {
  mySerial.begin(4800);
  Serial.begin(9600);
}

void loop() {
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
}
