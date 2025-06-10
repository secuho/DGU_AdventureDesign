const int stepPin = 11 ;
const int dirPin = 10 ;
 
void setup ( ) {
pinMode ( stepPin, OUTPUT ) ;
pinMode ( dirPin, OUTPUT ) ;
}

void loop ( ) {
  push();
  delay(1000);
  pull();
  delay(1000);
}


void push() {
  digitalWrite ( dirPin, LOW ) ;
    for ( int x = 0 ; x < 525 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
    }
}


void pull() {
  digitalWrite ( dirPin, HIGH ) ;
    for ( int x = 0 ; x < 525 ; x ++ ) {
    digitalWrite ( stepPin, HIGH ) ;
    delayMicroseconds ( 500 ) ;
    digitalWrite ( stepPin, LOW ) ;
    delayMicroseconds ( 500 ) ;
    }
}