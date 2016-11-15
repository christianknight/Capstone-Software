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
const int max_duty_cycle = 0.35*255;
// Output voltage value
float output_val;

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

  // Serial communication initialization
  Serial.begin(9600);
}
void loop() {
  // To Do:
  // ------
  // Set up pushbuttons to interrupts
  // Proturb and observe

  Update_current_mode();
  // Update reference voltage if in manual mode
  if (current_operating_mode == manual) {
    output_ref = (analogRead(MANUAL_MODE_POT)/1023.0)*20; // Max output of 20V
  }
  Update_duty_cycle();
  Print_output_ref();
  Print_output_val();
  Print_duty_cycle();
  

  
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
    output_ref = 0; // Value in update in proceeding code
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
  output_val = (analogRead(OUTPUT_VOLT_PIN)/1023.0)*5*4.33;

  // If in off mode, set duty cycle equal to zero
  if (output_ref == 0) {
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage within tolerance, then don't change duty cycle
  //else if ((output_val > (output_ref - output_ref * 0.05)) && (output_val < (output_ref + output_ref * 0.05))
    //return(1);
  // If output voltage below tolerance, increase duty cycle
  else if ((output_val <= (output_ref - output_ref * 0.05)) && (duty_cycle < max_duty_cycle)) {
    duty_cycle++;
    analogWrite(PWM_PIN, duty_cycle);
  }
  // If output voltage above tolerance, decrease duty cycle
  else if ((output_val >= (output_ref + output_ref * 0.05)) && (duty_cycle > 0)) {
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
  Serial.print(duty_cycle);
  Serial.print("%\n");
  //delay(500);
}

/*

float pot_val;
float out_val;
float duty_cycle = 0;
const int DC_max = 75;
float ref_voltage = 15.0;

void setup() {
  // Analog measurements
  //pinMode(POT_pin, INPUT);
  //pinMode(OUT_pin, INPUT);
  
  // PWM stuff
  //pinMode(PWM_pin, OUTPUT);
  //Set_PWM_Frequency__MEGA(PWM_pin, 1);
  //TCCR0B = TCCR0B & 0xF8 | 0x01; // Pin 5; 62.5kHz
  //Serial.begin(9600);
}
void loop() {

  pot_val = (analogRead(POT_pin)/1023.0)*5;
  out_val = (analogRead(OUT_pin)/1023.0)*5*4.39;

  
  if (out_val > (ref_voltage+0.1))
    duty_cycle--;
  else if ((out_val < (ref_voltage-0.1)) && (duty_cycle < DC_max))
    duty_cycle++;
  
  // Make sure DC is within range
  if (duty_cycle > DC_max) {
    analogWrite(PWM_pin, 0);
    Serial.print("Error!\n");
    while(1);
  }
 
  //duty_cycle = pot_val/5.0;
  //analogWrite(PWM_pin, duty_cycle*256);
  analogWrite(PWM_pin, (duty_cycle/100)*256);
  Serial.print("POT: ");  Serial.print(pot_val);
  Serial.print("  OUT: ");  Serial.print(out_val);
  Serial.print("   DC: ");   Serial.print(duty_cycle); 
  Serial.print("\n");
  delay(10 000);
}
*/

/* Set PWM Frequency on Arduino MEGA 2560 */
// TCCR: Timer Counter Control Register
// This function will affect the Arduino timers
/*
void Set_PWM_Frequency__MEGA(int pin, int prescaler) {
  byte mode;
  switch(prescaler) {
    case 1:     mode = 0x01; break;
    case 8:     mode = 0x02; break;
    case 32:    mode = 0x03; break;
    case 64:    mode = 0x04; break;
    case 128:   mode = 0x05; break;
    case 256:   mode = 0x06; break;
    case 1024:  mode = 0x07; break;
    default: return;
  }
  switch(pin) {
    case 2:     TCCR3B = TCCR3B & 0xF8 | mode; break; // 32kHz
    case 3:     TCCR3B = TCCR3B & 0xF8 | mode; break; // 32kHz
    case 4:     TCCR0B = TCCR0B & 0xF8 | mode; break; // 64kHz
    case 5:     TCCR3B = TCCR3B & 0xF8 | mode; break; // 32kHz
    case 6:     TCCR4B = TCCR4B & 0xF8 | mode; break; // 32kHz
    case 7:     TCCR4B = TCCR4B & 0xF8 | mode; break; // 32kHz
    case 8:     TCCR4B = TCCR4B & 0xF8 | mode; break; // 32kHz
    case 9:     TCCR2B = TCCR2B & 0xF8 | mode; break; // 32kHz
    case 10:    TCCR2B = TCCR2B & 0xF8 | mode; break; // 32kHz
    case 11:    TCCR1B = TCCR1B & 0xF8 | mode; break; // 32kHz
    case 12:    TCCR1B = TCCR1B & 0xF8 | mode; break; // 32kHz
    case 13:    TCCR0B = TCCR0B & 0xF8 | mode; break; // 64kHz
    default: return;
  } 
}
*/
