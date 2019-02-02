void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    char rx_byte = Serial.read();

    if (rx_byte == '1') {
      Serial.println("im here");
    }
  }

}
