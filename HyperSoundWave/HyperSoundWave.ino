#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
int trigPin = 3;
int echoPin = 4;
int Rled = 12;
int Gled = 13;
int Yled = 11;
int WeatherCode = 0; //0:맑음, 1: 비옴
int JUGI = 0;

void setup() {
  lcd.begin(16, 2);
  lcd.backlight();
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(Rled, OUTPUT);
  pinMode(Gled, OUTPUT);
  pinMode(Yled, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  long duration, distance;
  digitalWrite(trigPin, HIGH);

  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = ((float)(340*duration) / 1000) / 2;

  char buf[20];
  if(WeatherCode == 0)//맑음
  {
    lcd.setCursor(0,0);
    snprintf(buf, sizeof(buf), "Today is Sunny!");
    lcd.print(buf);
    lcd.setCursor(0,1);
    snprintf(buf, sizeof(buf), "               ");
    lcd.print(buf);
    digitalWrite(Yled, LOW);
  }
  if(WeatherCode == 1)//비옴
  {
    lcd.setCursor(0,0);
    snprintf(buf, sizeof(buf), "Take me!       ");
    lcd.print(buf);
    lcd.setCursor(0,1);
    snprintf(buf, sizeof(buf), "It's rainy!    ");
    lcd.print(buf);

    if(JUGI%2 == 0 && distance<=50)
    digitalWrite(Yled, HIGH);
    if(JUGI%2 == 1)
    digitalWrite(Yled, LOW);
  }
  
  //lcd.setCursor(0,0);
  //lcd.print(buf);

  Serial.print("Distance:");
  
  Serial.print(distance);
  Serial.println("mm");
  Serial.println(WeatherCode);
  Serial.println(JUGI);
  if(distance>=50)
  {
    digitalWrite(Rled, HIGH);
    digitalWrite(Gled, LOW);
  }
  else
  {
    digitalWrite(Rled, LOW);
    digitalWrite(Gled, HIGH);
  }
  JUGI++;
  if(JUGI>=10)
  {
    JUGI = 0;
    if(WeatherCode == 0)
    {
      WeatherCode = 1;
    }
    else
    {
      WeatherCode = 0;
    }
  }
  
  delay(500);
}
