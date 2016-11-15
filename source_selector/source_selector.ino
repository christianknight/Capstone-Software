#define AVAILABLE_THRES 8  // Minimum voltage value in units of volts for a source to be considered available
#define SOURCE_1 1
#define SOURCE_2 2
#define SOURCE_3 3

// Pin labels
// ----------
// Gate pin labels
const int source_1_gate_pin = 49;
const int source_2_gate_pin = 51;
const int source_3_gate_pin = 53;
// Voltage measurement pin labels
const int source_1_volt_pin = A10;
const int source_2_volt_pin = A9;
const int source_3_volt_pin = A8;
// Button pin labels
const int source_1_button         = 9;
const int source_2_button         = 8;
const int source_3_button         = 7;
const int auto_select_mode_button = 6;
const int off_mode_button         = 30;
// LEDs pin labels
const int error_detected_LED    = 10;
const int source_1_available_LED    = 43;
const int source_1_active_LED   = 44;
const int source_2_available_LED    = 41;
const int source_2_active_LED   = 42; 
const int source_3_available_LED    = 39;
const int source_3_active_LED   = 40;
const int auto_select_mode_LED  = 37;
const int off_mode_LED          = 38;

// State holders and priority setting
// ----------------------------------
// Source selector operating mode
enum operating_mode {off, manual_select, auto_select};
enum operating_mode current_operating_mode = off;
// Source priority for auto_select mode
int source_priority[] = {SOURCE_1, SOURCE_2, SOURCE_3}; // List source in order in which they will be chosen
// Source selector active source
enum active_source {none, source_1, source_2, source_3};
enum active_source current_active_source = none;

// Global variables
// ----------------
// Voltage measurements
float source_1_volt_val;
float source_2_volt_val;
float source_3_volt_val;
// Source availability (HIGH = available, LOW = unavailable);
bool source_1_availability = LOW;
bool source_2_availability = LOW;
bool source_3_availability = LOW;

void setup() {
  // Initialize gate pins
  pinMode(source_1_gate_pin, OUTPUT);
  pinMode(source_2_gate_pin, OUTPUT);
  pinMode(source_3_gate_pin, OUTPUT);
  // Make sure gates are low
  digitalWrite(source_1_gate_pin, LOW);
  digitalWrite(source_2_gate_pin, LOW);
  digitalWrite(source_3_gate_pin, LOW);

  // Initialize voltage measurements
  pinMode(source_1_volt_pin, INPUT);
  pinMode(source_2_volt_pin, INPUT);
  pinMode(source_3_volt_pin, INPUT);
  
  // Initiallize buttons
  pinMode(source_1_button,          INPUT);
  pinMode(source_2_button,          INPUT);
  pinMode(source_3_button,          INPUT);
  pinMode(auto_select_mode_button,  INPUT);
  pinMode(off_mode_button,          INPUT);

  // Initiallize LEDs
  pinMode(error_detected_LED,   OUTPUT);
  pinMode(source_1_available_LED,   OUTPUT);
  pinMode(source_1_active_LED,  OUTPUT);
  pinMode(source_2_available_LED,   OUTPUT);
  pinMode(source_2_active_LED,  OUTPUT);
  pinMode(source_3_available_LED,   OUTPUT);
  pinMode(source_3_active_LED,  OUTPUT);
  pinMode(auto_select_mode_LED, OUTPUT);
  pinMode(off_mode_LED,         OUTPUT);
  // Make sure LEDs are off
  digitalWrite(error_detected_LED,    LOW);
  digitalWrite(source_1_available_LED,    LOW);
  digitalWrite(source_1_active_LED,   LOW);
  digitalWrite(source_2_available_LED,    LOW);
  digitalWrite(source_2_active_LED,   LOW);
  digitalWrite(source_3_available_LED,    LOW);
  digitalWrite(source_3_active_LED,   LOW);
  digitalWrite(auto_select_mode_LED,  LOW);
  digitalWrite(off_mode_LED,          HIGH);

  // Serial communication initialization
  Serial.begin(9600);
}

void loop() {
  // To do
  // -----
  // Hoop up sources to interrupt
  // Hook up "off button" to interrupt
 

  Measure_input_voltages();
  Update_source_availability();
  Update_source_mode_and_activity();

}

// Source voltage measurement
// --------------------------
// Voltage dividers have been calibrated
void Measure_input_voltages(void) {
  source_1_volt_val = (analogRead(source_1_volt_pin)/1023.0)*5*10.98;
  source_2_volt_val = (analogRead(source_2_volt_pin)/1023.0)*5*10.96;
  source_3_volt_val = (analogRead(source_3_volt_pin)/1023.0)*5*11.3;
}

// Source voltage serial print
// ---------------------------
void Display_input_voltages(void) {
  Serial.print("-----------------\n");
  Serial.print("Source 1 = ");Serial.print(source_1_volt_val);Serial.print("V\n");
  Serial.print("Source 2 = ");Serial.print(source_2_volt_val);Serial.print("V\n");
  Serial.print("Source 3 = ");Serial.print(source_3_volt_val);Serial.print("V\n");
}

// Update source availability
// --------------------------
// If voltage above AVAILABLE_THRES set as available, if not set as unavailable
void Update_source_availability(void) {
  if(source_1_volt_val > AVAILABLE_THRES) {
    source_1_availability = HIGH;
    digitalWrite(source_1_available_LED, HIGH);
  }
  else {
    source_1_availability = LOW;
    digitalWrite(source_1_available_LED, LOW);    
  }
  if(source_2_volt_val > AVAILABLE_THRES) {
    source_2_availability = HIGH;
    digitalWrite(source_2_available_LED, HIGH);
  }
  else {
    source_2_availability = LOW;
    digitalWrite(source_2_available_LED, LOW);    
  } 
  if(source_3_volt_val > AVAILABLE_THRES) {
    source_3_availability = HIGH;
    digitalWrite(source_3_available_LED, HIGH);
  }
  else {
    source_3_availability = LOW;
    digitalWrite(source_3_available_LED, LOW);    
  }
}

// Update source mode and current active source
// --------------------------------------------
// Poll all push push buttons and apply corresponding changes to operating mode and active source
// If operating in auto mode, apply corresponding changes to active source
void Update_source_mode_and_activity(void) {
  
  // Poll push buttons
  // -----------------
  // If source 1 button pressed
  if((digitalRead(source_1_button) == 1) && ((current_operating_mode != manual_select) || (current_active_source != source_1))) {
    Turn_mode_LEDs_off();
    current_operating_mode = manual_select;
    Activate_source_1();
  }
  // If source 2 button pressed
  if((digitalRead(source_2_button) == 1) && ((current_operating_mode != manual_select) || (current_active_source != source_2))) {
    Turn_mode_LEDs_off();
    current_operating_mode = manual_select;
    Activate_source_2();
  }
  // If source 3 button pressed
  if((digitalRead(source_3_button) == 1) && ((current_operating_mode != manual_select) || (current_active_source != source_3))) {
    Turn_mode_LEDs_off();
    current_operating_mode = manual_select;
    Activate_source_3();
  }
  // If auto select mode button pressed
  if((digitalRead(auto_select_mode_button) == 1) && (current_operating_mode != auto_select)) {
    current_operating_mode = auto_select;
    // Note that the current active source is updated in the code below
    Turn_mode_LEDs_off();
    digitalWrite(auto_select_mode_LED, HIGH);
  }
  // If off mode button pressed
  if((digitalRead(off_mode_button) == 1) && (current_operating_mode != off)) {
    current_operating_mode = off;
    current_active_source = none;
    Turn_sources_off();
    Turn_mode_and_source_LEDs_off();
    digitalWrite(off_mode_LED, HIGH);
  
  }

  // Update auto select mode source choice
  // -------------------------------------
  // If current operating mode is auto select
  if(current_operating_mode == auto_select) {
    // Find and select first seen active source based on priority
    int i;
    for (i = 0; i < 3; i++){
      if((source_priority[i] == SOURCE_1) && (source_1_availability == HIGH)) {
        Activate_source_1();
        break;
      }
      if((source_priority[i] == SOURCE_2) && (source_2_availability == HIGH)) {
        Activate_source_2();
        break;
      }
      if((source_priority[i] == SOURCE_3) && (source_3_availability == HIGH)) {
        Activate_source_3();
        break;
      }
    } 
  }
}

// Activate source 1
// -----------------
// Update state holder, deactivate other sources, and update LED
void Activate_source_1(void) {
    current_active_source = source_1;
    Turn_sources_off();
    delay(250); // Prevent backfeeding from different active source which causes oscillation in auto select mode
    digitalWrite(source_1_gate_pin, HIGH);
    Turn_source_LEDs_off();
    digitalWrite(source_1_active_LED, HIGH);
}

// Activate source 2
// -----------------
// Update state holder, deactivate other sources, and update LED
void Activate_source_2(void) {
    current_active_source = source_2;
    Turn_sources_off();
    delay(250); // Prevent backfeeding from different active source which causes oscillation in auto select mode
    digitalWrite(source_2_gate_pin, HIGH);
    Turn_source_LEDs_off();
    digitalWrite(source_2_active_LED, HIGH);
}

// Activate source 3
// -----------------
// Update state holder, deactivate other sources, and update LED
void Activate_source_3(void) {
    current_active_source = source_3;
    Turn_sources_off();
    delay(250); // Prevent backfeeding from different active source which causes oscillation in auto select mode
    digitalWrite(source_3_gate_pin, HIGH);
    Turn_source_LEDs_off();
    digitalWrite(source_3_active_LED, HIGH);
}

// Turn source LEDs off
// --------------------
// Turn the following LEDs off: source 1, source 2, source 3
// Used for changes in active source
void Turn_source_LEDs_off(void) {
  digitalWrite(source_1_active_LED,   LOW);
  digitalWrite(source_2_active_LED,   LOW);
  digitalWrite(source_3_active_LED,   LOW);
}

// Turn mode LEDs off
// ------------------
// Turn the following LEDs off: auto select mode, off mode
// Used for changes in operating mode
void Turn_mode_LEDs_off(void) {
  digitalWrite(auto_select_mode_LED,  LOW);
  digitalWrite(off_mode_LED,          LOW);
}

// Turn source and mode LEDs off
// -----------------------------
// Turn the following LEDs off: source 1, source 2, source 3, auto select mode, off mode
// Used for changes in operating mode or changes in active source 
void Turn_mode_and_source_LEDs_off(void) {
  digitalWrite(source_1_active_LED,   LOW);
  digitalWrite(source_2_active_LED,   LOW);
  digitalWrite(source_3_active_LED,   LOW);
  digitalWrite(auto_select_mode_LED,  LOW);
  digitalWrite(off_mode_LED,          LOW);
}

// Turn source off
// ---------------
void Turn_sources_off(void) {
  digitalWrite(source_1_gate_pin, LOW);
  digitalWrite(source_2_gate_pin, LOW);
  digitalWrite(source_3_gate_pin, LOW);
}
