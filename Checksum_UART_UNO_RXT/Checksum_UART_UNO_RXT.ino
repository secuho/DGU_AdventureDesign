#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3); // RX, TX

char buffer[2];
char recent_data;
int count = 0;

void setup() {
  Serial.begin(115200);
  espSerial.begin(115200);

  Serial.println("Arduino UART Receiver");
}


void loop() {
  if (espSerial.available()) {
    char incoming = espSerial.read();
    buffer[count] = incoming;
    count++;

    if (count == 2) {
      if (buffer[0] == buffer[1]) {
        recent_data = buffer[0];

        Serial.print("Received: ");
        Serial.println(recent_data);

        Serial.println("Sending Checksum data...");
        espSerial.write(recent_data);  // 동일한 값 회신

        unsigned long startTime = millis();
        while (!espSerial.available()) {
          if (millis() - startTime > 1500) { // 1초 타임아웃
            Serial.println("ESP response timeout !");
            break;
          }
        }

        if (espSerial.available()) {
          char response = espSerial.read();
          if (response == 'g') {
            Serial.println("ESP : Checksum Successed.");
            Serial.println("Communication Complete : ");
            Serial.print(recent_data);
          } else if (response == 'f') {
            Serial.println("ESP : Checksum Failed.");
            Serial.println("Trying Redata Process...");
            espSerial.write('f');  // 오류 알림
          } else {
            Serial.println("Unexpected response from ESP, SysErr : ");
            Serial.print(response);
          }
        }

      }

      else {
        Serial.println("Receive Process Error");
        Serial.println("Trying Redata Process...");
        espSerial.write('f');  // 오류 알림
      }

      count = 0; // 버퍼 초기화
    }
  }

  if (Serial.available()) {
    espSerial.write(Serial.read());
  }
}