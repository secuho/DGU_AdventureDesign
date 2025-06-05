const int stepPin = 11 ;
const int dirPin = 10 ;
 
void setup ( ) {
pinMode ( stepPin, OUTPUT ) ;
pinMode ( dirPin, OUTPUT ) ;

digitalWrite ( dirPin, LOW ) ; // 회전 방향을 변경합니다.
// 2 회 전체 순환을 위해 400 펄스를 만듭니다.
for ( int x = 0 ; x < 200*4 ; x ++ ) {
digitalWrite ( stepPin, HIGH ) ;
delayMicroseconds ( 500 ) ;
digitalWrite ( stepPin, LOW ) ;
delayMicroseconds ( 500 ) ;
}

delay(1000);

digitalWrite ( dirPin, HIGH ) ; // 모터가 특정 방향으로 움직일 수있게합니다.
// 하나의 전체 사이클 회전을 만들기 위해 200 개의 펄스를 만듭니다.
for ( int x = 0 ; x < 200*4 ; x ++ ) {
digitalWrite ( stepPin, HIGH ) ;
delayMicroseconds ( 500 ) ;
digitalWrite ( stepPin, LOW ) ;
delayMicroseconds ( 500 ) ;
}

delay ( 1000 ) ; // 1 초 지연

}
void loop ( ) {




}