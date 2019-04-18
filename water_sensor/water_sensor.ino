int analogPin = A0;
int val = 0;

void setup() {

  Serial.begin(9600);
}

void loop() {

  val = analogRead(analogPin);
  Serial.print(F("sensor : "));
  Serial.println(val);

  delay(100);
}
