#define TXD1 19
#define RXD1 21

HardwareSerial arduSerial(1);

char recent_data, now_data;

void setup() {
  Serial.begin(115200);
  arduSerial.begin(115200, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println("ESP32 UART Transmitter");
}

void loop() {

  if (arduSerial.available()) {
    now_data = arduSerial.read();

    if (now_data == 'f') { //Transmitt Error
      Serial.println("Transmitt Error.");
      Serial.println("Try again Trans The data.");
        for (int i=1; i<=2; i++) {
          arduSerial.write(recent_data);
          delay(800);
        }
    }
    
    else {
      if (now_data == recent_data) {
        Serial.println("Communication Complete : ");
        Serial.print(now_data);
        arduSerial.write('g'); // g = good
      }

      else {
        Serial.println("Checksum Failed.");
        Serial.println("Sendding Checksum Fail Signal...");
        arduSerial.write('f');
        }
      }
    }


  if (Serial.available()) {

    recent_data = Serial.read();

    for (int i=1; i<=2; i++) {
    arduSerial.write(recent_data);
    delay(1000);
    }

  Serial.println("Data Sended : ");
  Serial.print(recent_data);
  }

}