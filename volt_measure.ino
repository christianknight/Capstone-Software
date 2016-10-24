/*--------------------------------------------------------------
  Program:      volt_measure

  Description:  Reads value on analog input 2 and calculates
                the voltage based on the voltage divider.

  Date:         24 October 2016
  
  Christain Knight, Michael Shea
--------------------------------------------------------------*/

// number of analog samples to take per reading
#define NUM_SAMPLES 50

int sum = 0;                    // sum of samples taken
unsigned char sample_count = 0; // current sample number
float voltage = 0.0;            // calculated voltage

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    // take a number of analog samples and add them up
    while (sample_count < NUM_SAMPLES) {
        sum += analogRead(A2);
        sample_count++;
        delay(10);
    }
    // calculate the voltage
    // use 5.0 for a 5.0V ADC reference voltage
    // 5.015V is the calibrated reference voltage
    voltage = ((float)sum / (float)NUM_SAMPLES * 5.015) / 1024.0;
    // send voltage for display on Serial Monitor
    // voltage multiplied by calibration factor
    Serial.print(voltage * 8.47);
    Serial.println (" V");
    sample_count = 0;
    sum = 0;
}
