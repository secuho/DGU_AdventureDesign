#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3); // RX, TX

char recent_data[2];

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);

  Serial.println("Arduino UART Receiver");
}


void loop() {

  if (espSerial.available()) {

    recent_data[0] = espSerial.read();

      Serial.print("Received: ");
      Serial.println(recent_data[0]);

      Serial.println("Sending Checksum data...");
      espSerial.write(recent_data[0]);

      delay(3000);

    recent_data[1] = espSerial.read();

    if (recent_data[1] == 'g') {
      Serial.println("ESP : Checksum Successed.");
      Serial.print("Communication Complete : ");
      Serial.println(recent_data[0]);
    }
    else if (recent_data[1] == 'f') {
      Serial.println("ESP : Checksum Failed.");
      Serial.println("Trying Redata Process...");
      espSerial.write('f');  // 오류 알림
    }

  }

  /*if (Serial.available()) {
    espSerial.write(Serial.read());
  }*/

}