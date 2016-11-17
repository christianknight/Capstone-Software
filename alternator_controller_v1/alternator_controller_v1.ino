// Only have 8 bits of precision for duty cycle
// Voltage measurements:
  // Calibrated to 2.63V, accurate up to 
  // Current: 
  // Voltage:
  
// Pin labels
// ----------
// Analog mesurement pin
#define OUTPUT_VOLT_PIN   A0
#define OUTPUT_CURR_PIN   A1
// Push button pins
#define UP_BUTTON         A4
#define ON_OFF_BUTTON     A3
#define DOWN_BUTTON       A2
// LED pin
#define ON_LED            2
// PWM pin
#define PWM_PIN           3


// State holders
// -------------
// Source selector operating mode
enum operating_mode {off, on};
enum operating_mode current_operating_mode = off;

// Global variables
// ----------------
// Duty cycle
unsigned int duty_cycle = 0;
const unsigned int max_duty_cycle = 0.4*255; // 100% duty cycle is equivalent to max_duty_cycle = 255
// Current
float output_curr_val; // Units of amps
float output_curr_ref = 0; // Units of amps
float output_curr_tol = 0.05; // Units of percentage
const float output_curr_max = 2; // Units of amps
// Voltage
float output_volt_val; // Units of volts
const float output_volt_max = 10; // Units of volts


float tolerance = 0.01;

void setup() {
  // Pin initializations
  // -------------------
  // Analog measurement pin
  pinMode(OUTPUT_CURR_PIN, INPUT);
  pinMode(OUTPUT_VOLT_PIN, INPUT);
  // Button pins
  pinMode(UP_BUTTON, INPUT);
  pinMode(ON_OFF_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  // LED pin
  pinMode(ON_LED, OUTPUT);
  digitalWrite(ON_LED, LOW);
  // PWM pin
  pinMode(PWM_PIN, OUTPUT);
  digitalWrite(PWM_PIN, LOW);

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
  int i;
  for (i = 0; i < 10; i++) {
    Update_current_mode();
    Update_duty_cycle();
    delay(50);
  }
  
  /*
  duty_cycle = (analogRead(MANUAL_MODE_POT)/1023.0)*255;
  analogWrite(PWM_PIN, duty_cycle);
  */
  
  
  output_curr_val = (analogRead(OUTPUT_CURR_PIN)/1023.0)*5*0.68; // Calibrated to Iout = 1A; Vadc = 1.48V; Max current of 3A
  output_volt_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*3.07; // Calibrated to Vout = 7.68, Vadc = 2.5V; Max voltage of 15V
  Print_output_curr_val();
  Print_output_curr_ref();
  Print_output_volt_val();
  Print_duty_cycle();
  Serial.print("-----------------------\n");
  

}

// Update current mode
// -------------------
// Polls the push buttons
void Update_current_mode(void) {
  if ((digitalRead(ON_OFF_BUTTON) == HIGH) && (current_operating_mode != off)) {
    current_operating_mode = off;
    duty_cycle = 0;
    output_curr_ref = 0;
    analogWrite(PWM_PIN, 0);
    digitalWrite(ON_LED, LOW);
    while (digitalRead(ON_OFF_BUTTON) == HIGH);
    delay(250);
  }
  else if ((digitalRead(ON_OFF_BUTTON) == HIGH) && (current_operating_mode != on)) {
    current_operating_mode = on;
    duty_cycle = 0;
    output_curr_ref = 0;
    digitalWrite(ON_LED, HIGH);
    while (digitalRead(ON_OFF_BUTTON) == HIGH);
    delay(250);
  }
  else if ((digitalRead(DOWN_BUTTON) == HIGH) && (current_operating_mode == on) && (output_curr_ref > 0)) {
    output_curr_ref -= 0.25;
    if (output_curr_ref < 0) output_curr_ref = 0;
    while (digitalRead(DOWN_BUTTON) == HIGH);
    delay(250);
  }
  else if ((digitalRead(UP_BUTTON) == HIGH) && (current_operating_mode == on) && (output_curr_ref < output_curr_max)) {
    output_curr_ref += 0.25;
    if (output_curr_ref > output_curr_max) output_curr_ref = output_curr_max;
    while (digitalRead(DOWN_BUTTON) == HIGH);
    delay(250);
  }
}

// Update duty cycle
// -----------------
void Update_duty_cycle(void) {
  // Read current output current val
  // -------------------------------
  output_curr_val = (analogRead(OUTPUT_CURR_PIN)/1023.0)*5*0.68; // Calibrated to Iout = 1A; Vadc = 1.48V; Max current of 3A
  
  if (output_curr_ref == 0) {
    duty_cycle = 0;
    analogWrite(PWM_PIN, duty_cycle);
  }

  // If output current below tolerance, increase duty cycle
  else if ((output_curr_val <= (output_curr_ref - output_curr_ref * output_curr_tol)) && (duty_cycle < max_duty_cycle)) {
    duty_cycle++;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output current above tolerance, decrease duty cycle
  else if ((output_curr_val >= (output_curr_ref + output_curr_ref * output_curr_tol)) && (duty_cycle > 0)) {
    duty_cycle--;
    analogWrite(PWM_PIN, duty_cycle);
  }

}

// Print output current
// ------------------------------
void Print_output_curr_val(void) {
  Serial.print("Output current = "); 
  Serial.print(output_curr_val);
  Serial.print("A\n");
}

// Print reference
// ------------------------------
void Print_output_curr_ref(void) {
  Serial.print("Refence current = "); 
  Serial.print(output_curr_ref);
  Serial.print("A\n");
}

// Print output voltage
// ------------------------------
void Print_output_volt_val(void) {
  Serial.print("Output voltage = "); 
  Serial.print(output_volt_val);
  Serial.print("V\n");
}

// Print duty cycle
// ------------------------------
void Print_duty_cycle(void) {
  Serial.print("Duty cycle = ");
  Serial.print(duty_cycle);
  Serial.print("%\n");
}
