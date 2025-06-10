#include <Servo.h>

const int right_grab = 2;
const int left_grab = 3;
Servo servo_1;
Servo servo_2;

int raser = 5;
int angle ;
int data;
int hold;

//int trig = 8;
//int echo = 9;

long duration, distance;

void setup() {

  Serial.begin(115200);

  pinMode(raser, OUTPUT);
  digitalWrite(raser, HIGH);

  //pinMode(trig, OUTPUT);
  //pinMode(echo, INPUT);

  open();

  delay(2000);

  Serial.println("System Initializing Success.");
  Serial.println();

}

void loop() {

  detect();
  //micro();
  check();
  //delay (100);

}

void open() {

  servo_1.attach(right_grab);
  servo_2.attach(left_grab);
  delay(100);

  //angle = 140;
  servo_1.write(50);
  servo_2.write(47);

  delay(300);

  servo_1.detach();
  servo_2.detach();
  Serial.println("OPEN");
}

void close() {

  servo_1.attach(right_grab);
  servo_2.attach(left_grab);
  delay(100);

  //angle = 90;
  servo_1.write(10);
  servo_2.write(90);

  delay(300);
  Serial.println("CLOSE");
}

void detect() {

  data = analogRead(A0);
  Serial.print("Razer value : ");
  Serial.println(data);
  
  delay (100);

  if (data >= 150) {
    delay (100);
    close();
    hold = 1;
  }
  else {
    if (data <= 120) {
      open();
      Serial.println("Error!");
    }
  }

}

void check() {
  if (hold == 1) {
    if (distance >= 150) {
      open ();
    }
  }
}