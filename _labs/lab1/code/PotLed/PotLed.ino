int potIPT = A0;
int val = 0;
int PIN_NUMBER = 3;

void setup() {   
  pinMode(PIN_NUMBER, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  val = analogRead(potIPT);     // read the input pin
  analogWrite(PIN_NUMBER, map(val, 0, 1023, 0, 255));
}
