/*--------------------------------------------------------------
  Program:      power_measure

  Description:  Calculates and displays instantaneous voltage,
                current, and power on an LCD, with a refresh
                rate of 1Hz. Voltage calculated based on reading
                from a voltage divider on analog input 2,
                current calculated based on reading from current
                sense amplifier on analog input 3, power
                calculated based on voltage and current
                measurements.

  Date:         7 November 2016

  Authors:      Christain Knight, Michael Shea
--------------------------------------------------------------*/

// number of analog samples to take per reading
#define NUM_SAMPLES 50

int sum1 = 0;                    // sum of samples taken
int sum2 = 0;                    // sum of samples taken

unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage
float current = 0.0;            // calculated current

// setup routine runs once upon startup/reset:
void setup()  {
  // initialize serial communication at 9600 bits per second
  Serial.begin(9600);
}

void loop()
{
  // take a number of analog samples and add them up
  while (sample_count < NUM_SAMPLES) {
    sum1 += analogRead(A2);
    sum2 += analogRead(A3);
    sample_count++;
    delay(50);
  }

  voltage = ((float)sum1 / (float)NUM_SAMPLES * 4.922) / 1023;
  current = ((float)sum2 / (float)NUM_SAMPLES * 4.922) / 1023;

  if ((voltage * 6) < 0) {
    Serial.println("Vin = ");
    Serial.print(0.00);
    Serial.println(" V");
  }

  if (((current - 2.48) * 5) < 0) {
    Serial.println("Iin = ");
    Serial.print(0.00);
    Serial.println(" A");
    Serial.println("Pin = ");
    Serial.print(0.00);
    Serial.println(" W");
  }

  Serial.println("Vin = ");
  Serial.print(voltage * 6);
  Serial.println(" V");
  Serial.println("Iin = ");
  Serial.print((current - 2.488) * 5);
  Serial.println(" A");
  Serial.println("Pin = ");
  Serial.print(((current - 2.488) * 5) * (voltage * 6));
  Serial.println(" W");

  sample_count = 0;
  sum1 = 0;
  sum2 = 0;
}
