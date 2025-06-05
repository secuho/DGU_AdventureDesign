const int pirPin1 = 7;
const int pirPin2 = 8;

bool pir1State = false;
bool pir2State = false;
int mode = 0;
//0: 대기, 1:외출대기, 2:외출, 3:귀가대기, 4:귀가
unsigned long pir1TriggerTime = 0;
unsigned long pir2TriggerTime = 0;
const unsigned long activeDuration = 3000;  // 3초 유지

void setup() {
  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  Serial.begin(9600);
}

void loop() {
  unsigned long currentTime = millis();

  // 센서 1 감지 처리
  if (digitalRead(pirPin1) == HIGH && !pir1State) {
    pir1State = true;
    pir1TriggerTime = currentTime;
    Serial.println("센서 1 감지됨");
    if(mode == 0){
      mode = 1;
    }
    if(mode == 3){
      mode = 4;
      Serial.println("귀가");
    }
  }

  // 센서 1 상태 리셋
  if (pir1State && (currentTime - pir1TriggerTime >= activeDuration)) {
    pir1State = false;
    Serial.println("센서 1 상태 초기화됨");
    mode = 0;
  }

  // 센서 2 감지 처리
  if (digitalRead(pirPin2) == HIGH && !pir2State) {
    pir2State = true;
    pir2TriggerTime = currentTime;
    Serial.println("센서 2 감지됨");
    if(mode == 0){
      mode = 3;
    }
    if(mode == 1){
      mode = 2;
      Serial.println("외출");
    }
  }

  // 센서 2 상태 리셋
  if (pir2State && (currentTime - pir2TriggerTime >= activeDuration)) {
    pir2State = false;
    Serial.println("센서 2 상태 초기화됨");
    mode = 0;
  }

  delay(100);  // 센서 체크 간격
}