const int sensorPin = A0; // temperature pin
const float baselineTemp = 20.0;

const byte interruptPin = 2; // the number of pushbutton pin
const byte ledPin = 7;
volatile byte buttonState = LOW; 

int array[10];
int arrayIndex = 10;

bool read_activate=false;

byte i=0; // iteration variable i (used in the interior for, now the for is in the exterior)
byte j=0; // timer0 of 70Hz => 14ms => the led is on for 14ms*j

#define led_aprins 20 // superior limit for j (ex 20*14=280ms)

void setup() {
// TIMER0 used to keep the LED on as much as we want
// set TIMER0 interrupt at 2kHz
  TCCR0A = 0; // set entire TCCR2A register to 0
  TCCR0B = 0; // same for TCCR2B
  TCNT0  = 0; //initialize counter value to 0
  // set compare match register for 70hz increments
  OCR0A = 222; // = (16*10^6) / (70*1024) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 1024 prescaler
  TCCR0B |= (1 << CS02) | (1 << CS00);   
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);

//TIMER1 is used to make a task of 100ms which activates reading temperature => an average is done after 100ms*arrayIndex
//set TIMER1 interrupt at 100Hz
  TCCR1A = 0; // set entire TCCR0A register to 0
  TCCR1B = 0; // same for TCCR0B
  TCNT1  = 0; //initialize counter value to 0
  // set compare match register for 100hz increments
  OCR1A = 2499; // = (16*10^6) / (100*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR1A |= (1 << WGM12);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR1B |= (1 << CS11) | (1 << CS10);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  Serial.begin(9600);
  for( int pinNumber = 4; pinNumber < 7; pinNumber++)
  {
    pinMode(pinNumber, OUTPUT);
    digitalWrite(pinNumber, LOW);
  }
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), blink, RISING);
  // RISING to trigger when the pin goes from low to high
}

float suma = 0;
void readTemp() {
  array[10]=0;
  
//    for(int i=0; i<arrayIndex; i++)
//    {
//      // read the temperature sensor
//      int sensorVal = analogRead(sensorPin);
//      // convert sensor reading to voltage
//      float voltage = (sensorVal/1024.0)*5.0;
//      //convert the voltage to temperature and send the value to computer
////      Serial.print(", degrees C: ");
//      float temperature = (voltage - 0.5)*100;
////      Serial.print(temperature);
////      Serial.print("\n");
//      array[i]=temperature;
//      suma = suma + temperature;
//      delay(100);
//    }

    // read the temperature sensor
    int sensorVal = analogRead(sensorPin);
    float voltage = (sensorVal/1024.0)*5.0; // convert sensor reading to voltage
    float temperature = (voltage - 0.5)*100; //convert the voltage to temperature
    array[i]=temperature;
    suma = suma + temperature;
  
    if(i==arrayIndex)
    {
      Serial.print("Average temperature: ");
      float avg = suma/10;
      Serial.print(avg);
      Serial.print("\n");

      // turns off LEDs for a low temperature
      if(avg < baselineTemp)
      {
        digitalWrite(4, LOW);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
      }
    
      // turn on one LED for a low temperature
      else if(avg >= baselineTemp+2 && avg < baselineTemp+4){
        digitalWrite(4, HIGH);
        digitalWrite(5, LOW);
        digitalWrite(6, LOW);
      }
    
      // turn on 2 LEDs for a medium temeperature
      else if(avg >= baselineTemp+4 && avg < baselineTemp+6){
        digitalWrite(4, HIGH);
        digitalWrite(5, HIGH);
        digitalWrite(6, LOW);
      }
      else if(avg >= baselineTemp+6){
        digitalWrite(4, HIGH);
        digitalWrite(5, HIGH);
        digitalWrite(6, HIGH);
      }
      i=-1;
      suma = 0;
    }
    i++;
}

void loop() {
  if(read_activate)
  {
    readTemp();
    read_activate=false;
  }
  else
    if(buttonState==HIGH){
      digitalWrite(ledPin, buttonState);
      Serial.print("I have been pressed!");
      Serial.print("\n");
      buttonState=!buttonState;
    }  
}

void blink() {
  buttonState = HIGH;
  TCNT0=0;
  j=0;
  //digitalWrite(ledPin, buttonState);
}

ISR(TIMER1_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for TIMER1
   //interrupt commands here
   read_activate=true;
   
}
ISR(TIMER0_COMPA_vect){  //change the 0 to 1 for timer1 and 2 for TIMER0
   //interrupt commands here
   if(j==led_aprins){
    digitalWrite(ledPin, buttonState);
    j=0;
   }
   j++;
}
