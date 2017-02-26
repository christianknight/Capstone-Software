/*--------------------------------------------------------------
  Program:      power_measure

  Description:  Calculates and displays instantaneous voltage,
                current, and power on an LCD, for 6 channels, 
                refreshing at < 1Hz. Voltage calculated based 
                on reading from a voltage divider on analog 
                inputs 0-2, current calculated based on reading 
                from current sense amplifier on analog inputs 
                3-5, power calculated based on voltage and 
                current measurements. External MUX used to do 
                all 12 measurements using the 6 ADC pins.

  Date:         15 December 2016

  Authors:      Christain Knight, Michael Shea
--------------------------------------------------------------*/

// include LCD library and initialize the interface pins
#include <LiquidCrystal.h>
LiquidCrystal lcd(10,9,5,6,7,8);

#define NUM_SAMPLES 90  // number of analog samples to take per reading

unsigned char sample_count = 0;   // current sample number
float sumVoltage[] = {0,0,0,0,0,0};   // sum of samples taken for 6-channel voltage measurement
float sumCurrent[] = {0,0,0,0,0,0};   // sum of samples taken for 6-channel current measurement
float voltage[] = {0,0,0,0,0,0};              // calculated voltage
float current[] = {0,0,0,0,0,0};              // calculated current

byte ADCpins[] = {A0,A0,A1,A1,A2,A2,A3,A3,A4,A4,A5,A5};
const int s0 = 11;    // digital pin 8 used for mux select signal 0
const int s1 = 12;    // digital pin 9 used for mux select signal 1
const int s2 = 13;   // digital pin 10 used for mux select signal 2
int s0State = LOW;
int s1State = LOW;
int s2State = LOW;
volatile byte dispChannel = 0;
volatile byte dispState = 1;
const byte button0 = 2;  // digital pin 0 used for user push button to change display
const byte button1 = 3;  // digital pin 0 used for user push button to change display
int i = 0;

float voltageGain[] = {6.967,6.98,6.95,6.99,6.995,6.995};
float voltageOffset[] = {0.05,0.05,0.05,0.05,0.05,0.05};
float currentGain[] = {2.035,2.035,2.02,2.03,2.045,2.033};
float currentOffset[] = {-0.012,0.01,0.01,-0.01,-0.05,0.02};

// setup routine runs once upon startup/reset:
void setup()  {
  analogReference(EXTERNAL);
  analogRead(0);
  pinMode(button0, INPUT);
  pinMode(button1, INPUT);
  attachInterrupt(digitalPinToInterrupt(button0), setDispMode, HIGH);
  attachInterrupt(digitalPinToInterrupt(button1), setDispChan, HIGH);
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  digitalWrite(s0, s0State);
  digitalWrite(s1, s1State);
  digitalWrite(s2, s2State);
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  lcd.begin(20,4);    // set up the LCD's number of columns and rows
}

void loop()
{
  // take a number of analog samples and add them up
  while (sample_count < NUM_SAMPLES) {
    for (i = 0; i < 6; i++){
      sumVoltage[i] += analogRead(ADCpins[i]);
      sumCurrent[i] += analogRead(ADCpins[i+6]);
      s0State = !digitalRead(s0);
      s1State = !digitalRead(s1);
      s2State = !digitalRead(s2);
      digitalWrite(s0, s0State);
      digitalWrite(s1, s1State);
      digitalWrite(s2, s2State);
      delay(1);
    }
    sample_count++;
    delay(5);
  }

  // calculate voltage and current from analog samples
  for (i = 0; i < 6; i++){
    voltage[i] = ((float)sumVoltage[i] / (float)NUM_SAMPLES * 4.096) / 1023;
    current[i] = ((float)sumCurrent[i] / (float)NUM_SAMPLES * 4.096) / 1023;
  }

  // print to LCD
  lcd.clear();  
  if (dispChannel){
      lcd.setCursor(0,0);
      if (voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1] > 0) {
        lcd.print("V");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print(voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1]);
        lcd.print("V");
      }
      else {
        lcd.print("V");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("V");
        lcd.setCursor(0,1);
        lcd.print("I");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("A");
        lcd.setCursor(0,2);
        lcd.print("P");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("W");
      }
      if (current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1] > 1) {
        lcd.setCursor(0,1);
        lcd.print("I");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print(current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]);
        lcd.print("A");
        lcd.setCursor(0,2);
        lcd.print("P");
        lcd.print(dispChannel);
        lcd.print("=");
        lcd.print((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) * (voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1]));
        lcd.print("W");
      }
      else if ((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) > 0 && (current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) < 1) {
        lcd.setCursor(0,1);
        lcd.print("I");
        lcd.print(dispChannel);
        lcd.print("="); 
        lcd.print((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) * 1e3);
        lcd.print("mA");
        lcd.setCursor(0,2);
        lcd.print("P");
        lcd.print(dispChannel);
        lcd.print("=");
        if ((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) * (voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1]) < 1) {
          lcd.print((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) * (voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1]) * 1e3);
          lcd.print("mW");
        }
        else  {
          lcd.print((current[dispChannel-1] * currentGain[dispChannel-1] + currentOffset[dispChannel-1]) * (voltage[dispChannel-1] * voltageGain[dispChannel-1] + voltageOffset[dispChannel-1]));
          lcd.print("W");
        }
      }
  }
  else if (dispState == 1){
    for (i = 0; i < 3; i++){
      lcd.setCursor(0,i);
      if (voltage[i] * voltageGain[i] + voltageOffset[i] > 0) {
        lcd.print("V");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(voltage[i] * voltageGain[i] + voltageOffset[i]);
        lcd.print("V");
      }
      else {
        lcd.print("V");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("V");
      }
    }
    for (i = 3; i < 6; i++){
      lcd.setCursor(10,i-3);
      if (voltage[i] * voltageGain[i] + voltageOffset[i] > 0) {
        lcd.print("V");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(voltage[i] * voltageGain[i] + voltageOffset[i]);
        lcd.print("V");
      }
      else {
        lcd.print("V");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("V");
      }
    }
  }

  else if (dispState == 2){
    for (i = 0; i < 3; i++){
      lcd.setCursor(0,i);
      if ((current[i] * currentGain[i] + currentOffset[i]) > 1) {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(current[i] * currentGain[i] + currentOffset[i]);
        lcd.print("A");
      }
      else if ((current[i] * currentGain[i] + currentOffset[i]) > 0 && (current[i] * currentGain[i] + currentOffset[i]) < 1) {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("="); 
        lcd.print((current[i] * currentGain[i] + currentOffset[i]) * 1e3);
        lcd.print("m");
      }
      else {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("A");
      }
    }
    
    for (i = 3; i < 6; i++){
      lcd.setCursor(10,i-3);
      if ((current[i] * currentGain[i] + currentOffset[i]) > 1) {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(current[i] * currentGain[i] + currentOffset[i]);
        lcd.print("A");
      }
      else if ((current[i] * currentGain[i] + currentOffset[i]) > 0 && (current[i] * currentGain[i] + currentOffset[i]) < 1) {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("="); 
        lcd.print((current[i] * currentGain[i] + currentOffset[i]) * 1e3);
        lcd.print("m");
      }
      else {
        lcd.print("I");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("A");
      }
    }
  }

    else if (dispState == 3){
    for (i = 0; i < 3; i++){
      lcd.setCursor(0,i);
      if ((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) > 1) {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]));
        lcd.print("W");
      }
      else if ((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) > 0 && (voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) < 1) {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("="); 
        lcd.print((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) * 1e3);
        lcd.print("m");
      }
      else {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("W");
      }
    }
	
    for (i = 3; i < 6; i++){
      lcd.setCursor(10,i-3);
      if ((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) > 1) {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]));
        lcd.print("W");
      }
      else if ((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) > 0 && (voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) < 1) {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("="); 
        lcd.print((voltage[i] * voltageGain[i] + voltageOffset[i]) * (current[i] * currentGain[i] + currentOffset[i]) * 1e3);
        lcd.print("m");
      }
      else {
        lcd.print("P");
        lcd.print(i+1);
        lcd.print("=");
        lcd.print(0.00);
        lcd.print("W");
      }
    }
  }

  // reset sample count and analog samples
  sample_count = 0;
  for (i = 0; i < 6; i++){
    sumVoltage[i] = 0;
    sumCurrent[i] = 0;
  }
}

void setDispMode() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    if (dispState < 3){
      dispState++;
    }
    else dispState = 1;
   dispChannel = 0;
  }
  last_interrupt_time = interrupt_time;
  millis();
}

void setDispChan() {
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    if (dispChannel < 6){
      dispChannel++;
    }
    else dispChannel = 1;
    dispState = 0;
  }
  last_interrupt_time = interrupt_time;
  millis();
}

