
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <Servo.h>
#define code_digits 12//11글자의 문자열에 날씨 정보를 포함해서 전달할 것임, 필요에 따라 글자수는 늘리거나 줄일 수2 있음

#include <Adafruit_NeoPixel.h>

#define light 8
#define NUMPIXELS 4
#define bright 255
#define dly 50
Adafruit_NeoPixel neo(NUMPIXELS, light, NEO_GRB + NEO_KHZ800);

//디지털 핀 할당 현황
//2, 3: 집게
//5: 레이저
//6, 7:스피커
//8, 9: ESP->UNO 수신용
//10, 11: 스텝 모터
//12, 13: PIR 센서
//아날로그 핀 할당 현황
//4, 5: 디스플레이
/////////////////////////////////
const int right_grab = 2;
const int left_grab = 3;
const int raser = 4;
SoftwareSerial mySoftwareSerial(6, 7); // RX, TX->오디오 재생용
SoftwareSerial espSerial(8, 9); // RX, TX->UNO통신용
const int dirPin = 10;
const int stepPin = 11;
const int pir1 = 13;
const int pir2 = 12;
LiquidCrystal_I2C lcd(0x27, 20, 4);


DFRobotDFPlayerMini myDFPlayer;

Servo servo_1;
Servo servo_2;

void printDetail(uint8_t type, int value);
char recent_data;
char weather_code[code_digits];
int wc_index = 0;
bool is_today_rainy = 0;
bool is_umbrella_hooked = 0;
int rain_sum = -999;
int temperature = -999;
int weather_id = -999;
int angle;
int data;
int hold;
long duration, distance;
char track[1];
bool pir1state = false;
bool pir2state = false;

bool prevPir1 = false;
bool prevPir2 = false;

int mode = 0;  // 0: Idle, 1: 외출대기, 2: 외출, 3: 귀가대기, 4: 귀가
bool resetPending = false;
bool isOut = false;  // 현재 외출 중인지 상태 추적

bool is_arm_outside = false;

bool has_given_umbrella = false;

  
static bool armHandled = false;

unsigned long eventTime = 0;
const unsigned long resetDelay = 3000;  // 3초 유지


void setup() {
  //시리얼 수신 준비
  Serial.begin(115200);
  espSerial.begin(9600);
  Serial.println("Arduino Bootting...");

  // 네오픽셀 LED 세팅
  neo.begin();
  neo.setBrightness(bright);
  neo.clear();
  neo.show();
  Serial.println("Neopixel Setup Success!");

  ////////////////////////////////////////////////

  // LCD 출력 준비
  lcd.init();
  lcd.backlight();//백라이트 켬
  lcd.setCursor(0,0);
  lcd.print("Weather Auto Sys!");
  lcd.setCursor(0,1);
  lcd.print("Hello World!");

  ////////////////////////////////////////////////

  //오디오 출력 준비
  mySoftwareSerial.begin(9600);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.volume(30);  //Set volume value. From 0 to 30
  ///////////////////////////////////////////////////

  // 로봇 집게 잡기 준비
  pinMode(raser, OUTPUT);
  digitalWrite(raser, HIGH);

  _open();

  _arm_in_force();
  delay(1000);
  _arm_out_force();

  delay(1500);

  Serial.println("Robot Arm Grab System Setup Success!");
  Serial.println();

  ///////////////////////////////////////////////////

  // 스텝 모터 준비
  pinMode ( stepPin, OUTPUT ) ;
  pinMode ( dirPin, OUTPUT ) ;

  ///////////////////////////////////////////////////

  // PIR 센서 준비
  pinMode(pir1, INPUT);
  pinMode(pir2, INPUT);

  ///////////////////////////////////////////////////

  _esp_dummy_for_test();

  Serial.println("System Starting...");
  Serial.println();
}

//날씨 코드 설명
//weather_code[0] : @를 고정적으로 입력받아, 문자열의 시작을 알리는 공간
//weather_code[1] : 그날의 날씨를 간단하게 표현할 수 있는 코드
//weather_code[2] : 비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 십의 자리.
//weather_code[3] :  비가 온다면, 24시간 강수량총합/24를 통해 하루 평균 강수량을 받는 곳, 하루 평균 강수량의 일의 자리.
//weather_code[4] : 일평균 기온의 부호
//weather_code[5] : 일평균 기온의 십의 자리
//weather_code[6] : 일평균 기온의 일의 자리
//weather_code[7] : 날씨 코드 백의 자리
//weather_code[8] : 날씨 코드 십의 자리
//weather_code[9] : 날씨 코드 일의 자리
//weather_code[10] : #을 고정적으로 입력받아, 문자열의 종료를 알리는 공간. #을 입력받으면 시리얼에서 제대로 입력받았는지 방금 입력받은 weather_code string을 확인차 출력시켜주자

void loop() {
  _esp_communication();
  _pir_sensing();
  _detect();
  _umbrella_control();


  static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {  // 1초마다 한번만 찍음
      Serial.print("Current MODE: ");
      Serial.println(mode);
      Serial.print("isOut: ");
      Serial.println(isOut);
      Serial.print("Razer Value: ");
      Serial.println(analogRead(A0));
      Serial.print("Robot Arm Status: ");
      
      if (is_arm_outside == 1) {
        Serial.println("Out");
      } else if (is_arm_outside == 0) {
        Serial.println("In");
      }

      Serial.println();
      lastPrint = millis();
    }
}

void green() {
  for (int i = 0; i < NUMPIXELS; i++) {
    neo.setPixelColor(i, 0, 255, 0);
    neo.show();
    delay(dly);
  }
}

void red() {
  for (int i = 0; i < NUMPIXELS; i++) {
    neo.setPixelColor(i, 255, 0, 0);
    neo.show();
    delay(dly);
  }
}

void _esp_dummy_for_test(){
  is_today_rainy = 1;
  weather_id = 500;
}
//우산을 줄지, 그냥 날씨만 알려줄지, 사람은 언제오는지, 귀가하는 상황인지, 외출하는 상황인지 총체적으로 구분하는 함수

void _umbrella_control() {

  // 외출 완료 : 우산 전달 한번만 실행
  if (mode == 2 && !has_given_umbrella)
  {
    if (is_today_rainy == 1 && is_umbrella_hooked == 1)
    {
      Serial.println("----- 1번 조건 진입 (우산 전달) -----");
      _playing_weather_audio_and_lcd_print();
      _arm_out();
      _open();
      delay(3000);
      _arm_in();
    }
    else if (is_today_rainy == 0)
    {
      Serial.println("----- 2번 조건 진입 (우산 필요 없음) -----");
      _playing_weather_audio_and_lcd_print();
    }
    has_given_umbrella = true;  // 1회만 실행
  }

  // 귀가 완료 : 우산 걸었을 때 자동 팔 넣기
  else if (mode == 4)
{
  Serial.println("----- 3번 조건 진입 (귀가 완료) -----");

  
  if (!armHandled) {
    if (is_umbrella_hooked == 0)
    {
      _arm_out();
    }
    else if (is_umbrella_hooked == 1)
    {
      _arm_in();
    }
    armHandled = true;
  }
}

}


void _pir_sensing() {
  pir1state = digitalRead(pir1);
  pir2state = digitalRead(pir2);
  bool pir1Triggered = (!prevPir1 && pir1state);
  bool pir2Triggered = (!prevPir2 && pir2state);

  if (resetPending) {
    if (!pir1state && !pir2state) {
      resetPending = false;
      Serial.println("센서 초기화 완료, Idle 상태로 복귀");
    } 
    prevPir1 = pir1state;
    prevPir2 = pir2state;
    delay(50);
    return;
  }

  switch(mode) {
    case 0:
      if (pir2Triggered) {
        mode = 3;
        Serial.println("센서 2번 감지 : 귀가 대기");
      }
      else if (pir1Triggered) {
        mode = 1;
        Serial.println("센서 1번 감지 : 외출 대기");
      }
      break;

    case 3:
      if (pir1Triggered) {
        mode = 4;
        Serial.println("센서 1번 감지 : 귀가 완료");
        eventTime = millis();
      }
      break;

    case 4:
      if (millis() - eventTime >= resetDelay) {
        mode = 0;
        resetPending = true;
        armHandled = false;
        Serial.println("초기화 (귀가 완료)");
      }
      break;

    case 1:
      if (pir2Triggered) {
        mode = 2;
        Serial.println("센서 2번 감지 : 외출 완료");
        eventTime = millis();
        has_given_umbrella = false;  // 외출 완료 진입시 플래그 초기화
      }
      break;

    case 2:
      if (millis() - eventTime >= resetDelay) {
        mode = 0;
        resetPending = true;
        Serial.println("초기화 (외출 완료)");
      }
      break;
  }

  prevPir1 = pir1state;
  prevPir2 = pir2state;
  delay(50);
}




//로봇 팔 펴기
void _arm_in(){
  if (!is_arm_outside) {
    // 이미 안에 들어가 있으면 굳이 다시 안 넣는다
    return;
  }

  digitalWrite ( dirPin, LOW ) ;
  for ( int x = 0 ; x < 500 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
  }
  
  is_arm_outside = false; // 로봇팔 안으로 들어감 상태 업데이트
}


void _arm_in_force(){
  digitalWrite ( dirPin, LOW ) ;
  for ( int x = 0 ; x < 500 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
  }
  is_arm_outside = false;
}

//로봇 팔 접기

void _arm_out(){
  if (is_arm_outside) {
    // 이미 나와있으면 다시 안 내민다
    return;
  }

  digitalWrite ( dirPin, HIGH ) ;
  for ( int x = 0 ; x < 500 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
  }
  
  is_arm_outside = true; // 로봇팔 밖으로 나감 상태 업데이트
}

//집게 열기

void _arm_out_force(){
  digitalWrite ( dirPin, HIGH ) ;
  for ( int x = 0 ; x < 500 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
  }
  is_arm_outside = true;
}


void _open() {

  servo_1.attach(right_grab);
  servo_2.attach(left_grab);
  delay(100);

  
  servo_1.write(50);
  servo_2.write(47);

  delay(300);

  servo_1.detach();
  servo_2.detach();
}
//집게 닫기
void _close() {

  servo_1.attach(right_grab);
  servo_2.attach(left_grab);
  delay(100);

  servo_1.write(9);
  servo_2.write(89);

  delay(300);
}
//집게 감지 하기
void _detect() {

  data = analogRead(A0);
  //Serial.print("Razer value : ");
  //Serial.println(data);
  
  delay (70);

  if (data >= 150) {
    delay (100);
    //Serial.println("CLOSE");
    _close();
    is_umbrella_hooked = 1;
    hold = 1;
    green();
  }
  else {
    if (data <= 120) {
      _open();
      is_umbrella_hooked = 0;
      red();
      //Serial.println("OPEN");
      //Serial.println("Error!");
    }
  }

}
void _esp_communication()
{
  if (espSerial.available())
  {
    recent_data = espSerial.read();
    //Serial.print("Received: ");
    //Serial.println(recent_data);

    if(recent_data == '@')//문자 '@'를 받으면 입력이 시작된 것으로 간주한다.
    {
      wc_index = 0;
    }

    weather_code[wc_index] = recent_data;
    wc_index++;
    
    if(recent_data == '#')//문자 '#'을 받으면 입력이 종료된 것으로 간주하고, weather_code를 확인차 한 번 출력한다.
    {
      weather_code[code_digits] = 0;
      //Serial.print("weather_code: ");
      //Serial.println(weather_code);
      /////////////////////////////////////////////////////////////
      //입력 종료 후 수신한 정보에 따라 정보 가공
      //강수량이 0보다 크면 비가 오는 것으로 간주
      rain_sum = (weather_code[2] - '0') * 10 + (weather_code[3] - '0');
      if(rain_sum == 0)
      {
        is_today_rainy = 0;
      }
      else
      {
        is_today_rainy = 1;
      }
      //평균 온도 가공
      temperature = (weather_code[5] - '0') * 10 + (weather_code[6] - '0');
      if(weather_code[4] - '0' == 1)//부호 비트가 1이라면: 기온이 영하라면
      {
        temperature *= -1;
      }
      //날씨 id 가공
      weather_id = (weather_code[7] - '0') * 100 + (weather_code[8] - '0') * 10 + (weather_code[9] - '0');
      //////////////////////////////////////////////////////////////
      //Serial.print("rain_sum: ");Serial.println(rain_sum);
      //Serial.print("is_today_rainy: ");Serial.println(is_today_rainy);
      //Serial.print("temperature: ");Serial.println(temperature);
      //Serial.print("weather_id: ");Serial.println(weather_id);
      //_playing_weather_audio_and_lcd_print();
    }

    //ESP에서 가져온 정보를 무시하고 임시로 
    //테스트용 데이터 삽입
    _esp_dummy_for_test();
  }
}
void _playing_weather_audio_and_lcd_print()
{
  //가공한 weather_id에 따라서 재생할 오디오를 고르고 디스플레이에 출력하는 함수
  switch(weather_id)
  {
    case 200:
    case 201:
    case 202:
    case 210:
    case 211:
    case 212:
    case 221:
    case 230:
    case 231:
    case 232:
      //오늘의 날씨는 천둥번개를 동반한 비바람입니다
      _lcdprint(0);
      myDFPlayer.play(11);
      break;
    case 300:
    case 301:
    case 302:
    case 310:
    case 311:
    case 312:
    case 313:
    case 314:
    case 321:
      //오늘의 날씨는 이슬비입니다
      _lcdprint(1);
      myDFPlayer.play(10);
      break;
    case 500:
    case 501:
    case 502:
    case 503:
    case 504:
    case 511:
    case 520:
    case 521:
    case 522:
    case 531:
      //오늘의 날씨는 비입니다 우산을 가져가세요
      _lcdprint(2);
      myDFPlayer.play(9);
      break;
    case 600:
    case 601:
    case 602:
    case 611:
    case 612:
    case 613:
    case 615:
    case 616:
    case 620:
    case 621:
    case 622:
      //오늘의 날씨는 눈입니다 우산을 가져가세요
      _lcdprint(3);
      myDFPlayer.play(7);
      break;
    case 701:
    case 711:
    case 721:
    case 741:
      //오늘의 날씨는 안개가 많이 끼어있습니다
      _lcdprint(4);
      myDFPlayer.play(1);
      break;
    case 731:
    case 751:
    case 761:
      //오늘의 날씨는 황사가 심합니다 마스크를 쓰세요
      _lcdprint(5);
      myDFPlayer.play(5);
      break;
    case 762:
      //오늘의 날씨는 화산재가 날리는 날입니다 마스크를 쓰세요
      _lcdprint(6);
      myDFPlayer.play(4);
      break;
    case 771:
      //오늘은 열대 지방의 스콜 소나기가 올지도 모릅니다 우산을 가져가세요
      _lcdprint(7);
      myDFPlayer.play(2);
      break;
    case 781:
      //오늘은 토네이도가 발생할지도 모릅니다 바깥 활동에 주의하세요
      _lcdprint(8);
      myDFPlayer.play(3);//오디오 이슈로 일단 3번 대신 11번으로 재생
      break;
    case 800:
      //오늘의 날씨는 맑음입니다
      _lcdprint(9);
      myDFPlayer.play(8);
      break;
    case 801:
    case 802:
    case 803:
    case 804:
    //오늘의 날씨는 구름과 함께 맑음입니다
      _lcdprint(10);
      myDFPlayer.play(6);
      break;
    default:
      //weather_id error
      break;
  }
}
void _lcdprint(int menu)
{
  switch(menu)
  {
    case 0:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Thunderstorm!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Be careful!");
      break;
    case 1:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Drizzle!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Rain!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Snow!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 4:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Foggy!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Be careful of sight");
      break;
    case 5:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Dust!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your mask!");
      break;
    case 6:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Volcanic Ash!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your mask!");
      break;
    case 7:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Squall!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Take your umbrella!");
      break;
    case 8:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Tornado!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Don't go outside!");
      break;
    case 9:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Clear!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Have a nice day!");
      break;
    case 10:
      lcd.setCursor(0,0);
      lcd.print("                ");
      lcd.setCursor(0,0);
      lcd.print("Today's Clouds!");
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      lcd.print("Have a nice day!");
      break;
  }
}