/*--------------------------------------------------------------
  Program:      power_measure

  Description:  Calculates and displays instantaneous voltage,
                current, and power on an LCD, refreshing at < 
                1Hz. Voltage calculated based on reading from a 
                voltage divider on analog input 0, current 
                calculated based on reading from current sense 
                amplifier on analog input 1, power calculated 
                based on voltage and current measurements.

  Date:         12 November 2016

  Authors:      Christain Knight, Michael Shea
--------------------------------------------------------------*/

// include LCD library and initialize the interface pins
#include <LiquidCrystal.h>
LiquidCrystal lcd(12,11,5,4,3,2);

#define NUM_SAMPLES 100  // number of analog samples to take per reading

float sumVoltage = 0;   // sum of samples taken for voltage measurement
float sumCurrent = 0;   // sum of samples taken for current measurement

unsigned char sample_count = 0;   // current sample number
float voltage = 0.0;              // calculated voltage
float current = 0.0;              // calculated current

// setup routine runs once upon startup/reset:
void setup()  {
  Serial.begin(9600); // initialize serial communication at 9600 bits per second
  lcd.begin(20,4);    // set up the LCD's number of columns and rows
}

void loop()
{
  // take a number of analog samples and add them up
  while (sample_count < NUM_SAMPLES) {
    sumVoltage += analogRead(A0);
    sumCurrent += analogRead(A1);
    sample_count++;
    delay(10);
  }

  // calculate voltage and current from analog samples
  voltage = ((float)sumVoltage / (float)NUM_SAMPLES * 5) / 1023;
  current = ((float)sumCurrent / (float)NUM_SAMPLES * 5) / 1023;

  // print to LCD 
  lcd.clear();
  if ((voltage * 6) > 0) {
    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(voltage * 6);
    lcd.print(" V");
  }
  else {
    lcd.setCursor(0,1);
    lcd.print("V = ");
    lcd.print(0.00);
    lcd.print(" V");
    lcd.setCursor(0,2);
    lcd.print("I = ");
    lcd.print(0.00);
    lcd.print(" A");
    lcd.setCursor(0,3);
    lcd.print("P = ");
    lcd.print(0.00);
    lcd.print(" W");
  }
  if (((current - 2.496) * 5) > 0) {
    lcd.setCursor(0,2);
    lcd.print("I = ");
    lcd.print((current - 2.496) * 5);
    lcd.print(" A");
    lcd.setCursor(0,3);
    lcd.print("P = ");
    lcd.print(((current - 2.496) * 5) * (voltage * 6));
    lcd.print(" W");
  }
  else {
    lcd.setCursor(0,2);
    lcd.print("I = ");
    lcd.print(0.00);
    lcd.print(" A");
    lcd.setCursor(0,3);
    lcd.print("P = ");
    lcd.print(0.00);
    lcd.print(" W");
  }

  // print to Serial Monitor  
//  if ((voltage * 6) > 0) {
//    Serial.println("V = ");
//    Serial.println(voltage * 6);
//    Serial.println(" V");
//  }
//  else {
//    Serial.println("V = ");
//    Serial.println(0.00);
//    Serial.println(" V");
//    Serial.println("I = ");
//    Serial.println(0.00);
//    Serial.println(" A");
//    Serial.println("P = ");
//    Serial.println(0.00);
//    Serial.println(" W");
//  }
//  if (((current - 2.496) * 5) > 0) {
//    Serial.println("I = ");
//    Serial.println((current - 2.496) * 5);
//    Serial.println(" A");
//    Serial.println("P = ");
//    Serial.println(((current - 2.496) * 5) * (voltage * 6));
//    Serial.println(" W");
//  }
//  else {
//    Serial.println("I = ");
//    Serial.println(0.00);
//    Serial.println(" A");
//    Serial.println("P = ");
//    Serial.println(0.00);
//    Serial.println(" W");
//  }

  // reset sample count and analog samples
  sample_count = 0;
  sumVoltage = 0;
  sumCurrent = 0;
}
