int lightSensor = A0;
int dist = 0;

void setup() {
  // setup serial at 9600 baud
  Serial.begin(9600);   
}

// the loop function runs over and over again forever
void loop() {
  dist = analogRead(lightSensor);         // read the input pin
  Serial.println(dist);             // debug value
  // wait half a second
  delay(500);
}
