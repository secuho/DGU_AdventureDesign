#include <Servo.h>

Servo servo_1;
Servo servo_2;

int raser = 5;
int angle ;
int data;
int hold;

int trig = 8;
int echo = 9;

long duration, distance;

void setup() {

  Serial.begin(115200);

  pinMode(raser, OUTPUT);
  digitalWrite(raser, HIGH);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);

  open();

  delay(2000);

  Serial.println("System Initializing Success.");
  Serial.println();

}

void loop() {

  detect();
  micro();
 // check();
  delay (100);

}

void open() {

  servo_1.attach(2);
  servo_2.attach(3);
  delay(100);

  angle = 140;
  servo_1.write(angle);
  servo_2.write(180-angle);

  delay(300);

  servo_1.detach();
  servo_2.detach();
  
}

void close() {

  servo_1.attach(2);
  servo_2.attach(3);
  delay(100);

  angle = 90;
  servo_1.write(angle);
  servo_2.write(180-angle);

  delay(300);

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
    if (data <= 100) {
      open();
      Serial.println("Error!");
    }
  }

}

void micro() {

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  duration = pulseIn(echo, HIGH);
  distance = ((float)(340*duration) / 1000) / 2 ;

  Serial.print("Distance : ");
  Serial.println(distance);
  Serial.println();

}

void check() {
  if (hold == 1) {
    if (distance >= 150) {
      open ();
    }
  }
}