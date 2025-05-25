#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3); // RX, TX

char recent_data[2];

void setup() {
  Serial.begin(115200);
  espSerial.begin(9600);

  Serial.println("Arduino UART Receiver");
}


void loop() {

  if (espSerial.available()) {

    recent_data[0] = espSerial.read();

      Serial.print("Received: ");
      Serial.println(recent_data[0]);
  }

  if (Serial.available()) {
    espSerial.write(Serial.read());
  }

}