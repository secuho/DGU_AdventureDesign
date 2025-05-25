#define TXD1 19
#define RXD1 21

HardwareSerial arduSerial(1);

//char data[2];
int data;

void setup() {
  Serial.begin(115200);
  arduSerial.begin(9600, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println("ESP32 UART Transmitter");
}

void loop() {




  if (Serial.available()) {
    data = Serial.read();
    
      if (data != '\n') {

        arduSerial.write(data);
        Serial.print("Data Sended : ");
        Serial.println(data);
      }
    }

    if (arduSerial.available()) {
    Serial.write(arduSerial.read());
  }

}