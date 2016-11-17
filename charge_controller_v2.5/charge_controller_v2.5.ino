// Only have 8 bits of precision for duty cycle

// Pin labels
// ----------
// Potentiometer pin
#define MANUAL_MODE_POT     A0
// Output current measurement
#define OUTPUT_CURR_PIN     A1
// Output voltage measurement
#define OUTPUT_VOLT_PIN     A2
// PWM pin
#define PWM_PIN             3
// LED pins
#define OFF_MODE_LED        4
#define HIGH_MODE_LED       6
#define LOW_MODE_LED        8
#define MANUAL_MODE_LED     10
// Push button pins
#define OFF_MODE_BUTTON     5
#define HIGH_MODE_BUTTON    7
#define LOW_MODE_BUTTON     9
#define MANUAL_MODE_BUTTON  11

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
// Current
float output_curr_val;
const float output_curr_max = 1; // Units of amps
// Duty cycle
unsigned int duty_cycle = 0;
const unsigned int max_duty_cycle = 0.7*255; // 100% duty cycle is equivalent to max_duty_cycle = 255
// Output voltage value
float output_val;
float tolerance = 0.01;

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
  //while(1); 
  // Constant output voltage regulation
  // ----------------------------------
  //int i;
  //for (i = 0; i < 10; i++){
    Update_current_mode();
    // Update reference voltage if in manual mode
    if (current_operating_mode == manual) {
      output_ref = (analogRead(MANUAL_MODE_POT)/1023.0)*25; // Max output of 20V
    }
    Update_duty_cycle();
    //delay(50);
  //}
  //output_curr_val= (analogRead(OUTPUT_CURR_PIN)/1023.0)*10.5;
  //Print_output_curr_val();
  /*
  duty_cycle = (analogRead(MANUAL_MODE_POT)/1023.0)*255;
  analogWrite(PWM_PIN, duty_cycle);
  */
  
  delay(500);
  Print_output_curr_val();
  Print_output_ref();
  Print_output_val();
  Print_duty_cycle();
  Serial.print("-----------------------\n"); 
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
  //delay(500);
}

// Print output current value
// --------------------------
void Print_output_curr_val(void) {
  Serial.print("Output current = ");
  Serial.print(output_curr_val);
  Serial.print("A\n");
  //delay(500);
}

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
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.38;
  }
  else if (current_operating_mode == low) {
    output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.38;
  }

  output_curr_val= (analogRead(OUTPUT_CURR_PIN)/1023.0)*10.5;
  
  // If in off mode, set duty cycle equal to zero
  if (output_ref == 0) {
    duty_cycle = 0;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage below tolerance, increase duty cycle
  else if ((output_val <= (output_ref - output_ref * tolerance)) && (output_curr_val < output_curr_max) && (duty_cycle < max_duty_cycle)) {
  //else if ((output_val <= (output_ref - output_ref * tolerance)) && (duty_cycle < max_duty_cycle)) {
    duty_cycle++;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage above tolerance, decrease duty cycle
  else if (((output_val >= (output_ref + output_ref * tolerance)) || (output_curr_val > output_curr_max)) && (duty_cycle > 0)) {
  //else if ((output_val >= (output_ref + output_ref * tolerance)) && (duty_cycle > 0)) {
    duty_cycle--;
    analogWrite(PWM_PIN, duty_cycle);
  }
}

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
  Serial.print(((float)duty_cycle)/255.0);
  Serial.print("%\n");
  //delay(500);
}
