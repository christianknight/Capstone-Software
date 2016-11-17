// Only have 8 bits of precision for duty cycle

// Pin labels
// ----------
// Analog mesurement pin
#define OUTPUT_VOLT_PIN     A5
// Push button pins
#define OFF_MODE_BUTTON     A4
#define HIGH_MODE_BUTTON    A3
#define LOW_MODE_BUTTON     A2
#define MANUAL_MODE_BUTTON  A1
// Potentiometer pin
#define MANUAL_MODE_POT     A0
// PWM pin
#define PWM_PIN             3
// LED pins
#define OFF_MODE_LED        4
#define HIGH_MODE_LED       5
#define LOW_MODE_LED        6
#define MANUAL_MODE_LED     7

// State holders
// -------------
// Source selector operating mode
enum operating_mode {off, high, low, manual};
enum operating_mode current_operating_mode = off;

// Global variables
// ----------------
// Output voltage reference value
const float high_mode_output_ref  = 14.0; // Units of volts
const float low_mode_output_ref   = 13.5; // Units of volts
float output_ref = 0;
// Duty cycle
unsigned int duty_cycle = 0;
const unsigned int max_duty_cycle = 0.7*255; // 100% duty cycle is equivalent to max_duty_cycle = 255
// Output voltage value
float output_val;
float tolerance = 0.02;

void setup() {
  // Initializations
  // ---------------
  // Analog measurement pin
  pinMode(OUTPUT_VOLT_PIN, INPUT);
  // Push button pins
  pinMode(OFF_MODE_BUTTON, INPUT);
  pinMode(HIGH_MODE_BUTTON, INPUT);
  pinMode(LOW_MODE_BUTTON, INPUT);
  pinMode(MANUAL_MODE_BUTTON, INPUT);
  // Potentiometer pin
  pinMode(MANUAL_MODE_POT, INPUT);
  // PWM pin
  pinMode(PWM_PIN, OUTPUT);
  // LED pins
  pinMode(OFF_MODE_LED, OUTPUT);
  pinMode(HIGH_MODE_LED, OUTPUT);
  pinMode(LOW_MODE_LED, OUTPUT);
  pinMode(MANUAL_MODE_LED, OUTPUT);
  digitalWrite(OFF_MODE_LED, HIGH);
  digitalWrite(HIGH_MODE_LED, LOW);
  digitalWrite(LOW_MODE_LED, LOW);
  digitalWrite(MANUAL_MODE_LED, LOW);

  // PWM initialization
  // ------------------
  TCCR2B = TCCR2B & 0xF8 | 0x01;  // Set the PWM frequency on pin 3 to 32kHz

  // Serial communication initialization
  // -----------------------------------
  Serial.begin(9600);
}
void loop() {
  // To Do:
  // ------
  // Set up pushbuttons to interrupts
  // Proturb and observe

  // Constant output voltage regulation
  // ----------------------------------
  int j;
  for (j = 0; j < 10; j++){
    Update_current_mode();
    // Update reference voltage if in manual mode
    if (current_operating_mode == manual) {
      output_ref = (analogRead(MANUAL_MODE_POT)/1023.0)*20; // Max output of 20V
    }
    Update_duty_cycle();
    delay(50);
  }
  
  /*
  duty_cycle = (analogRead(MANUAL_MODE_POT)/1023.0)*255;
  analogWrite(PWM_PIN, duty_cycle);
  */
  
  ///*
  Print_output_ref();
  Print_output_val();
  Print_duty_cycle();
  Serial.print("-----------------------\n");
  //*/ 
}

// Update current mode
// -------------------
// Polls the push buttons
void Update_current_mode(void) {
  if ((digitalRead(OFF_MODE_BUTTON) == HIGH) && (current_operating_mode != off)) {
    current_operating_mode = off;
    duty_cycle = 0;
    output_ref = 0;
    Turn_mode_LEDs_off();
    digitalWrite(OFF_MODE_LED, HIGH);
  }
  if ((digitalRead(HIGH_MODE_BUTTON) == HIGH) && (current_operating_mode != high)) {
    current_operating_mode = high;
    output_ref = high_mode_output_ref;
    Turn_mode_LEDs_off();
    digitalWrite(HIGH_MODE_LED, HIGH);
  }
  if ((digitalRead(LOW_MODE_BUTTON) == HIGH) && (current_operating_mode != low)) {
    current_operating_mode = low;
    output_ref = low_mode_output_ref;
    Turn_mode_LEDs_off();
    digitalWrite(LOW_MODE_LED, HIGH);
  }
  if ((digitalRead(MANUAL_MODE_BUTTON) == HIGH) && (current_operating_mode != manual)) {
    current_operating_mode = manual;
    output_ref = 0; // Value updated in proceeding code
    Turn_mode_LEDs_off();
    digitalWrite(MANUAL_MODE_LED, HIGH);
  }
}

// Turn mode LEDs off
// ------------------
void Turn_mode_LEDs_off(void) {
  digitalWrite(OFF_MODE_LED, LOW);
  digitalWrite(HIGH_MODE_LED, LOW);
  digitalWrite(LOW_MODE_LED, LOW);
  digitalWrite(MANUAL_MODE_LED, LOW);
}

// Print output reference voltage
// ------------------------------
void Print_output_ref(void) {
  Serial.print("Reference voltage = ");
  Serial.print(output_ref);
  Serial.print("V\n");
  delay(500);
}

// Update duty cycle
// -----------------
void Update_duty_cycle(void) {
  // Read current output voltage val
  // -------------------------------
  // Different calibration for each mode to make results more accurate
  /*
  if (current_operating_mode == manual) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.95;
  }
  else if (current_operating_mode == high) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
  }
  else if (current_operating_mode == low) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
  }
  */
  int i;
  int num_samples = 10;
  float current_error;
  float lower_error;
  float upper_error;
  float last_error;
  
  // (1) Sample and record error for current dc
  // ------------------------------------------
  output_val = 0;
  for (i = 0; i < num_samples; i++) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
    delay(10);
  }
  output_val /= num_samples;
  current_error = abs(output_ref - output_val);
  
  // If in off mode, set duty cycle equal to zero
  if (output_ref == 0) {
    duty_cycle = 0;
    analogWrite(PWM_PIN, duty_cycle);
  }
  
  // If error exceeds +/- 2% of output_ref, or if the there is an increase in error for the current duty cycle
  // ---------------------------------------------------------------------------------------------------------
  else if ((output_val >= (output_ref + output_ref*tolerance)) || (output_val <= (output_ref - output_ref*tolerance)) || (current_error > 1.5*last_error)) {
    if (duty_cycle > 0) {
      // (2) Sample and record error for lower dc
      // ----------------------------------------
      analogWrite(PWM_PIN, duty_cycle-1);
      delay(20);
      output_val = 0;
      for (i = 0; i < num_samples; i++) {
        output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
        delay(10);
      }
      output_val /= num_samples;
      lower_error = abs(output_ref - output_val);
      // If error decreases or stays the same
      if (lower_error <= current_error) {
        duty_cycle--;
      }
      // If error increases
      else if (duty_cycle < max_duty_cycle) {
        // (3) Sample and record error for upper dc
        // ----------------------------------------
        // Check to see if error inceases for current dc?
        analogWrite(PWM_PIN, duty_cycle+1);
        delay(20);
        output_val = 0;
        for (i = 0; i < num_samples; i++) {
          output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
          delay(10);
        }
        output_val /= num_samples;
        upper_error = abs(output_ref - output_val);
        // If error decreases
        if (upper_error <= current_error) {
          duty_cycle++;
        }
      }
    }
    else if (duty_cycle < max_duty_cycle) {
      // (3) Sample and record error for upper dc
      // ----------------------------------------
      // Check to see if error inceases for current dc?
      analogWrite(PWM_PIN, duty_cycle+1);
      delay(20);
      output_val = 0;
      for (i = 0; i < num_samples; i++) {
        output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
       delay(10);
      }
      output_val /= num_samples;
      upper_error = abs(output_ref - output_val);
      // If error decreases
      if (upper_error <= current_error) {
        duty_cycle++;
      }
    }
    
    // (1) Sample and record error for current dc
    // ------------------------------------------
    analogWrite(PWM_PIN, duty_cycle);
    delay(20);
    output_val = 0;
    for (i = 0; i < num_samples; i++) {
      output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
      delay(10);
    }
    output_val /= num_samples;
    last_error = abs(output_ref - output_val); 
  }    
}

/*
// Update duty cycle
// -----------------
void Update_duty_cycle(void) {
  // Read current output voltage val
  // -------------------------------
  // Different calibration for each mode to make results more accurate
  if (current_operating_mode == manual) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.95;
  }
  else if (current_operating_mode == high) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
  }
  else if (current_operating_mode == low) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.35;
  }

  // If in off mode, set duty cycle equal to zero
  if (output_ref == 0) {
    duty_cycle = 0;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage below tolerance, increase duty cycle
  else if ((output_val <= (output_ref - output_ref * tolerance)) && (duty_cycle < max_duty_cycle)) {
    duty_cycle++;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage above tolerance, decrease duty cycle
  else if ((output_val >= (output_ref + output_ref * tolerance)) && (duty_cycle > 0)) {
    duty_cycle--;
    analogWrite(PWM_PIN, duty_cycle);
  }
}
*/

// Print output voltage
// ------------------------------
void Print_output_val(void) {
  Serial.print("Output voltage = "); 
  Serial.print(output_val);
  Serial.print("V\n");
  //delay(500);
}

// Print duty cycle
// ------------------------------
void Print_duty_cycle(void) {
  Serial.print("Duty cycle = ");
  Serial.print(duty_cycle);
  Serial.print("%\n");
  //delay(500);
}
