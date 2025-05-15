#define TXD1 19
#define RXD1 21

HardwareSerial mySerial(1);

int counter = 0;

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600, SERIAL_8N1, RXD1, TXD1);
  
  Serial.println("ESP32 UART Transmitter");
}

void loop() {
  
  // Send message over UART
  mySerial.println(String(counter));
  
  Serial.println("Sent: " + String(counter));
  
  // increment the counter
  counter++;
  
  delay(1000); 
}