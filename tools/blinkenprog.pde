#define outPin   10
#define ledAPin   4
#define ledBPin   5
#define keyAPin   2
#define keyBPin   3
#define keyA (!digitalRead(keyAPin))
#define keyB (!digitalRead(keyBPin))

#define bytedelay   20  // ms idle between bytes

boolean dosend = false;

boolean debug = false;

long baseTiming = 50;  // 50 us cycle time of ISR1 in the slave

boolean pinstate = 1;

void out (boolean val) {
  digitalWrite(outPin, val); 
  digitalWrite(ledBPin, val);
  pinstate = val;
}

void toggle () { out(!pinstate); }



// timing values in us
long refbit;
long refbit_low;
long refbit_high;

// -/+ delta for setting refbit_low/high
#define refbit_delta 0.3f


// sets up timing values based on val (in us);
void updaterefbit (long val) {

 refbit = 16*val;
 refbit_low = (long)((float)refbit*(1.0f-refbit_delta));
 refbit_high = (long)((float)refbit*(1.0f+refbit_delta));
 
 if (debug) {
   Serial.println("timings:");
   Serial.print("refbit/H/L/byte = ");
   Serial.print(refbit,DEC);
   Serial.print("us / ");
   Serial.print(refbit_high,DEC);
   Serial.print("us / ");
   Serial.print(refbit_low,DEC);
   Serial.print("us / ");
 }

}


void setup() {
  pinMode(outPin, OUTPUT); 
  pinMode(ledAPin, OUTPUT);
  digitalWrite(ledAPin, HIGH);
  pinMode(ledBPin, OUTPUT);
  pinMode(keyAPin, INPUT); 
  digitalWrite(keyAPin, HIGH);
  pinMode(keyBPin, INPUT); 
  digitalWrite(keyBPin, HIGH);
  
  Serial.begin(9600);
  
  updaterefbit (50);
  if (debug) { Serial.println("ready"); }
  out (LOW);
}



// send one byte, beginning with a high signal
void sendByte(char b) {
            
    toggle();
    int bitnr=0;    
    while (bitnr < 8) {
      if (b & (1<<bitnr)) {                  // 1
        delayMicroseconds(refbit_high);
      } else {                               // 0
        delayMicroseconds(refbit_low);
      }
      toggle();
      bitnr++;
    }

    delayMicroseconds(refbit/2);
    toggle();   
}
  


void loop() {

  Serial.flush();
 
  // wait for serial input
  while (Serial.available() == 0) {}
  out (HIGH);
  delay(bytedelay);

  // init sequence  
  sendByte(0xAA);
  delay(bytedelay);
  sendByte(0xAA);
  delay(bytedelay);

  char c; 
  while (Serial.available() > 0) { 
    c = Serial.read();
    digitalWrite(ledAPin, c == 0x00);
    sendByte(c);
    if (debug) { Serial.print(c); }
    digitalWrite(ledAPin,pinstate);
    delay(bytedelay);
  }
  
  delay(5000);
  out (LOW);
  digitalWrite(ledAPin,LOW);
 
}

