#define TXD1 19
#define RXD1 21

HardwareSerial arduSerial(1);

char data[2];

void setup() {
  Serial.begin(115200);
  arduSerial.begin(115200, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println("ESP32 UART Transmitter");
}

void loop() {




  if (Serial.available()) {
    data[0] = Serial.read();
    if(data[0] == '\n')
    {
      data[0] = 1;
    }
      if (data[0] != '\n') {

        arduSerial.write(data[0]);
        Serial.print("Data Sended : ");
        Serial.println(data[0]);
      }
    }

   if (arduSerial.available()) {
    
    data[1] = arduSerial.read();
    Serial.print("arduSerial.read: "); Serial.println(data[1]);
    if (data[1] == 'f') { //Transmitt Error
      Serial.println("Transmitt Error.");
      Serial.println("Try again Trans The data.");
          arduSerial.write(data[0]);
          delay(500);
        }
    else if (data[0]==data[1]) {
        Serial.println("Communication Complete : ");
        Serial.print("data[0]: ");Serial.println(data[0]);
        Serial.print("data[1]: ");Serial.println(data[1]);
        //Serial.print(data[1]);
        arduSerial.write('g'); // g = good
      }

      else {
        Serial.println("Checksum Failed.");
        Serial.println("Sendding Checksum Fail Signal...");
        arduSerial.write('f');
        }
      }
}