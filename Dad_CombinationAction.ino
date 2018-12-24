const int OUT1_STROBE = 2;
const int OUT1_CLOCK = 3;
const int OUT1_DATA = 4;

void setup() {
  pinMode(OUT1_STROBE, OUTPUT);
  pinMode(OUT1_CLOCK, OUTPUT);
  pinMode(OUT1_DATA, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void clockOutput1()
{
    // clock-it-in
    digitalWrite(OUT1_CLOCK, LOW); 
    digitalWrite(OUT1_CLOCK, HIGH);     
}

void sendDataEx(unsigned long iData) {
//  Serial.println(iData);

  // always load MSB...LSB
  const unsigned long MASK = 0x80000000;
  
  for (int i=31; i>=0; i--) 
  {
//    int result = ((iData & MASK) > 0);
//    Serial.print(result);

//    Serial.println(iData);
    
    // possibly deactivate stop; deactivate bit loaded first
    if (iData & MASK) 
      // de-energise
      digitalWrite(OUT1_DATA, LOW); 
      else
      // energise
      digitalWrite(OUT1_DATA, HIGH); 

    clockOutput1();

    // possibly activate stop; activate bit loaded last
    if (iData & MASK) 
      // energise
      digitalWrite(OUT1_DATA, HIGH); 
      else
      // de-energise
      digitalWrite(OUT1_DATA, LOW); 

    clockOutput1();

    // always load MSB...LSB
    iData=iData << 1;
  }

//  Serial.println();
//  Serial.println("end");
}

void setAllOff() {
  // lock output
  digitalWrite(OUT1_STROBE, LOW); 

  for (int i=63; i>=0; i--) {

    // turn off deactivation; deactivate bit loaded first
    digitalWrite(OUT1_DATA, LOW); 

    clockOutput1();

    // turn off activation; activate bit loaded last
    digitalWrite(OUT1_DATA, LOW); 

    clockOutput1();
  }

  // unlock output
  digitalWrite(OUT1_STROBE, HIGH); 
}

void sendData(unsigned long iData1, unsigned long iData2) {
  // lock output
  digitalWrite(OUT1_STROBE, LOW); 

  sendDataEx(iData2);
  sendDataEx(iData1);
  
  // unlock output
  digitalWrite(OUT1_STROBE, HIGH); 

  delay(500);

  setAllOff();
}

// the loop function runs over and over again forever
void loop() {
  Serial.begin(9600);
  
  /*
  bool b=true;
  
  do {
    digitalWrite(OUT1_STROBE, LOW); 

    b=!b;

    if (b) 
      // energise
      digitalWrite(OUT1_DATA, HIGH); 
      else
      // de-energise
      digitalWrite(OUT1_DATA, LOW); 
  
    // clock-it-in
    digitalWrite(OUT1_CLOCK, LOW); 
    digitalWrite(OUT1_CLOCK, HIGH);     

    if (b) 
      // energise
      digitalWrite(OUT1_DATA, LOW); 
      else
      // de-energise
      digitalWrite(OUT1_DATA, HIGH); 
  
    // clock-it-in
    digitalWrite(OUT1_CLOCK, LOW); 
    digitalWrite(OUT1_CLOCK, HIGH);     

    digitalWrite(OUT1_STROBE, HIGH); 

    digitalWrite(LED_BUILTIN, LOW);
    delay(500);                    
    digitalWrite(LED_BUILTIN, HIGH); 
  } while (1); */
  
  unsigned long i=1;

  while (1) {
//    Serial.println(i);
    
    sendData(0xffffffff, 0x00000000);
    delay(1000);
    
    digitalWrite(LED_BUILTIN, LOW);

    sendData(0x00000000, 0xffffffff);
    delay(1000);
    
    digitalWrite(LED_BUILTIN, HIGH); 

//    i=i << 1;*/
  }

  digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
//                                    // but actually the LED is on; this is because 
//                                    // it is active low on the ESP-01)
  delay(1000);                      // Wait for a second
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
//  delay(2000);                      // Wait for two seconds (to demonstrate the active low LED)
}
