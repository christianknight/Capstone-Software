#include <SPI.h>    // SPI library
#include <Wire.h>   // i2c library
#include <LiquidCrystal_I2C.h>  // LCD library

// Digital I/O Pin Mapping
// -----------------------
// SPI Pins
#define SPI_CLOCK 13  // Cannot be changed
#define SPI_DATA  11  // Cannot be changed
// Shift register pins
#define SHIFT_ENABLE  5 
#define SHIFT_LATCH   4
#define SHIFT_READ    3   // Interrupt pin
// 4:1 Analog MUX pins
#define MUX_S1 12
#define MUX_S0 8
// Load selector pins
//#define LS_BAT_ENABLE 7
//#define LS_AUX_ENABLE 8
// PWM pins
// Using pins 9 and 10 for PWM since this does not affect the delay() and millis() functions
#define PWM_AC 10 // PWM pin for alternator controller
#define PWM_CC 9  // PWM pin for charge controller
// Settings button
#define SETTINGS_BUTTON 2;

// Shift register mapping
// ----------------------
// Note: Commented numbers refer to pin designation on "Interface_Mapping.jpeg"
// Byte 5
// ------
#define CC_OFF_BUTTON       23 // 48
#define CC_BAT_BUTTON       22 // 47
#define CC_HIGH_BUTTON      21 // 46
#define CC_LOW_BUTTON       20 // 45
#define CC_FINE_BUTTON      19 // 44
#define CC_CRSE_BUTTON      18 // 43
#define SS_OFF_BUTTON       17 // 42
// Bit #16 not used
// Byte 4
// ------
#define SS_AUTO_BUTTON      15 // 40
#define SS_MANU_BUTTON      14 // 39
#define SS_AC_BUTTON        13 // 38
#define SS_BIKE_BUTTON      12 // 37
#define SS_AUX_BUTTON       11 // 36
#define CC_UP_BUTTON        10 // 35
#define AC_EN_BUTTON        9 // 34
// Bit #8 not used
// Byte 3
// ------
#define CC_DOWN_BUTTON      7 // 32
#define LS_OFF_BUTTON       6 // 31
#define LS_BAT_BUTTON       5 // 30
#define LS_AUX_BUTTON       4 // 29
// Bit #3 not used
// Bit #2 not used
// Bit #1 not used
// Bit #0 not used
// Byte 2
// ------
#define CC_OFF_LED_RED      23 // 24
#define CC_BAT_LED_GREEN    22 // 23
#define CC_HIGH_LED_GREEN   21 // 22
#define CC_LOW_LED_GREEN    20 // 21
#define CC_FINE_LED_GREEN   19 // 20
#define CC_CRSE_LED_GREEN   18 // 19
#define SS_OFF_LED_RED      17 // 18
#define LS_AUX_ENABLE       16 // 17
// Byte 1
// ------
#define SS_AUTO_LED_GREEN   15 // 16
#define SS_MANU_LED_GREEN   14 // 15
#define SS_AC_LED_GREEN     13 // 14
#define SS_AC_LED_YELLOW    12 // 13
#define SS_BIKE_LED_GREEN   11 // 12
#define SS_BIKE_LED_YELLOW  10 // 11
#define SS_AUX_LED_GREEN    9 // 10
#define LS_BAT_ENABLE       8 // 9
// Byte 0
// ------
#define SS_AUX_LED_YELLOW   7 // 8
#define AC_EN_LED_GREEN     6 // 7
#define SS_AC_ENABLE        5 // 6
#define LS_OFF_LED_RED      4 // 5
#define LS_BAT_LED_GREEN    3 // 4
#define LS_AUX_LED_GREEN    2 // 3
#define SS_AUX_ENABLE       1 // 2
#define SS_BIKE_ENABLE      0 // 1
  
// Operating mode and state holder constants
// -----------------------------------------
// Source selector
#define SS_OFF  "off "
#define SS_AUTO "auto"
#define SS_MANU "manu"
#define SS_NONE "none"
#define SS_AC   "ac  "
#define SS_BIKE "bike"
#define SS_AUX  "aux "
// Charge controller  
#define CC_OFF  "off "
#define CC_BAT  "bat "
#define CC_HIGH "high"
#define CC_LOW  "low "
#define CC_FINE "fine"
#define CC_CRSE "crse"
// Load selector
#define LS_OFF  "off "
#define LS_BAT  "bat "
#define LS_AUX  "aux "
// Alternator controller
#define AC_OFF  "off "
#define AC_ON   "on  "
// Battery states
#define BAT_NOT_CHARGING "  N/A  "
#define BAT_CHARGING_CC  "Const I"
#define BAT_CHARGING_CV  "Const V"
#define BAT_FLOAT        "Standby"

// Miscellaneous
#define YES 1
#define NO 0

// ==============================================================================================

// Operating mode and stator holder variables
// ==========================================
// Source Selector
// ---------------
String ss_mode = SS_OFF;     // Current source selector mode
String ss_source = SS_NONE;  // Current source selector active source
// Source availability, based on the voltage value
bool ss_ac_available = NO;
bool ss_bike_available = NO;
bool ss_aux_available = NO;
// Charge Controller
// -----------------
String cc_mode = CC_OFF;     // Current charge controller mode
String ls_mode = LS_OFF;     // Current load selector mode
String ac_mode = AC_OFF;     // Current charge controller mode
// Miscellaneous
// -------------
String bat_mode = "not ";


// Button press variables
// ----------------------
// 0 for not pressed, 1 for pressed
// Source selector
bool ss_off_button = 0;
bool ss_auto_button = 0;
bool ss_manu_button = 0;
bool ss_ac_button = 0;
bool ss_bike_button = 0;
bool ss_aux_button = 0;
// Charge controller
bool cc_off_button = 0;
bool cc_bat_button = 0;
bool cc_high_button = 0;
bool cc_low_button = 0;
bool cc_fine_button = 0;
bool cc_crse_button = 0;
bool cc_up_button = 0;
bool cc_down_button = 0;
// Load selector
bool ls_off_button = 0;
bool ls_bat_button = 0;
bool ls_aux_button = 0;
// Alternator controller
bool ac_en_button = 0;

/*
float val_1 = 0;
float val_2 = 0;
float val_3 = 0;
float val_4 = 0;
*/

// Analog measurement variables
// -----------------------------
const unsigned int num_samples = 10; // Number of samples to take for ADC measurements
const unsigned int measurement_delay_ms = 10; //20;
// Source selector input voltages
float ADC_ss_ac = 0;
float ADC_ss_bike = 0;
float ADC_ss_aux = 0;
// Miscellaneous measurements
float ADC_cc_pot = 0; // Duty cycle potentiometer for charge controller
float ADC_cc_volt = 0; // Charge controller output voltage
float ADC_ac_curr = 0; // Alternator controller field winding current

// Power monitoring easurement variables
// -------------------------------------
// Voltage measurements
float P1_aux_volt = 0;
float P2_bus_volt = 0;
float P3_sys_volt = 0;
float P4_ac_volt = 0;
float P5_dc_volt = 0;
float P6_in_volt = 0;
float P7_bat_volt = 0;
// Current measurements
float P1_aux_curr = 0;
float P2_bus_curr = 0;
float P3_sys_curr = 0;
float P4_ac_curr = 0;
float P5_dc_curr = 0;
float P6_in_curr = 0;
float P7_bat_curr = 0;
// Power measurements
float P1_aux_pwr = 0;
float P2_bus_pwr = 0;
float P3_sys_pwr = 0;
float P4_ac_pwr = 0;
float P5_dc_pwr = 0;
float P6_in_pwr = 0;
float P7_bat_pwr = 0;

// 0x50 <-- EEPROM address
// LCD initialization variables
LiquidCrystal_I2C lcd_1(0x27, 20, 4);  // 20 chars and 4 line display @ address 0x27
LiquidCrystal_I2C lcd_2(0x3F, 20, 4);  // 20 chars and 4 line display @ address 0x3E
LiquidCrystal_I2C lcd_3(0x3E, 20, 4);  // 20 chars and 4 line display @ address 0x3F

// LCD position variables
uint8_t LCD_row_duty_cycle = 3;
uint8_t LCD_row_efficiency = 4;
uint8_t LCD_row_vout = 4;
uint8_t LCD_row_alt_curr = 4;

// Charge controller variables
const uint8_t cc_duty_cycle_max = 255*0.75; // Max duty cycle value of 75%, rounded down for byte data type
const uint8_t cc_duty_cycle_crse_step = 5; // Duty cycle increment/decrement value for course mode
signed short cc_duty_cycle = 0; // Signed so that if decremented below zero, easy to set equal to zero
float cc_duty_cycle_percent = 0; // Duty cycle expressed as a percent
unsigned int cc_duty_cycle_count = 1; // Duty cycle decrement value
const float cc_volt_cycle_upper = 14.8; // Charging voltage upper bound
const float cc_volt_cycle_lower = 14.6; // Charging voltage lower bound
const float cc_volt_float_upper = 13.8; // Standby voltage upper bound
const float cc_volt_float_lower = 13.6; // Standby voltage upper bound
const float cc_curr_cycle_max = 2; // Max charging current, units of amps
const float cc_curr_cycle_min = 0.1; // Min charging current before considered fully charged, units of amps
const float cc_curr_in_max_other = 3; // Maximum input current for other sources (BIKE and AUX)
const float cc_curr_in_max_ac = 1; // Maximum input current for AC source
const float cc_curr_out_max = 3; // Maximum charge controller current
const float cc_volt_high = 14; // High mode voltage, units of volts
const float cc_volt_low = 13.5; // Low mode voltage, units of volts

// Shift register variables
uint32_t shift_buttons = 0;
uint32_t shift_outputs = 0; 

// Miscellaneous variables
const int loops_per_update = 2; // Number of SS and CC updates before printing values
float efficiency = 75.8; // Charge controller efficiency
float vout = 0; // Charge controller output voltage
const uint8_t ac_duty_cycle = 9; // Alternator current contoller duty cycle [0-255]
bool error = 0; // error flag
float ss_min_volt = 8; // Minimum voltage for source to be considered available, units of volts


// Analog Measurement Constant Variables
  // =====================================
  // =====================================
  const float ADC_scale = 5/1023.0/num_samples;
  
  // Measurement scales
  // ------------------
  // Power Measurement #2
  const float ADC_ac_curr_scale = 0.003164;
  const float ADC_cc_volt_scale = 0.021081;
  const float ADC_cc_pot_scale  = 5/1023.0;
  const float ADC_ss_ac_scale   = 0.066641;
  const float ADC_ss_bike_scale = 0.068046;
  const float ADC_ss_aux_scale  = 0.065485;

  // Slopes
  // ======
  // Voltage Measurements Slopes
  // ---------------------------
  // Power Measurement #1
  const float P1_aux_volt_slope_1  = 0.032909;
  const float P1_aux_volt_slope_2  = 0.033947;
  const float P1_aux_volt_slope_3  = 0.033559;
  const float P1_aux_volt_slope_4  = 0.033707;
  // Power Measurement #2
  const float P2_bus_volt_slope_1  = 0.034060;
  const float P2_bus_volt_slope_2  = 0.033837;
  const float P2_bus_volt_slope_3  = 0.033784;
  const float P2_bus_volt_slope_4  = 0.033784;
 // Power Measurement #5
  const float P5_dc_volt_slope_1   = 0.034014;
  const float P5_dc_volt_slope_2   = 0.033976;
  const float P5_dc_volt_slope_3   = 0.033784;
  const float P5_dc_volt_slope_4   = 0.033784;
  // Power Measurement #6
  const float P6_in_volt_slope_1   = 0.053706;
  const float P6_in_volt_slope_2   = 0.053312;
  const float P6_in_volt_slope_3   = 0.053332;
  const float P6_in_volt_slope_4   = 0.053191;
    
  // Current Measurement Slopes
  // --------------------------
  // Power Measurement #1
  const float P1_aux_curr_slope_1  = 0.010000;
  const float P1_aux_curr_slope_2  = 0.010030;
  const float P1_aux_curr_slope_3  = 0.010101;
  const float P1_aux_curr_slope_4  = 0.009992;
  const float P1_aux_curr_slope_5  = 0.009957;
  // Power Measurement #2
  const float P2_bus_curr_slope_1  = 0.009930;
  const float P2_bus_curr_slope_2  = 0.010173;
  const float P2_bus_curr_slope_3  = 0.010040;
  const float P2_bus_curr_slope_4  = 0.009925;
  const float P2_bus_curr_slope_5  = 0.009914;
 // Power Measurement #3
  const float P3_sys_curr_slope_1  = 0.010010;
  const float P3_sys_curr_slope_2  = 0.009891;
  const float P3_sys_curr_slope_3  = 0.010089;
  const float P3_sys_curr_slope_4  = 0.009907;
  const float P3_sys_curr_slope_5  = 0.009872;
 // Power Measurement #4
  const float P4_ac_curr_slope_1   = 0.010091;
  const float P4_ac_curr_slope_2   = 0.010101;
  const float P4_ac_curr_slope_3   = 0.010146;
  const float P4_ac_curr_slope_4   = 0.010012;
  const float P4_ac_curr_slope_5   = 0.009947;
 // Power Measurement #5
  const float P5_dc_curr_slope_1   = 0.010132;
  const float P5_dc_curr_slope_2   = 0.010132;
  const float P5_dc_curr_slope_3   = 0.010284;
  const float P5_dc_curr_slope_4   = 0.010093;
  const float P5_dc_curr_slope_5   = 0.010045;
  // Power Measurement #6
  const float P6_in_curr_slope_1   = 0.009901;
  const float P6_in_curr_slope_2   = 0.010152;
  const float P6_in_curr_slope_3   = 0.010020;
  const float P6_in_curr_slope_4   = 0.010000;
  const float P6_in_curr_slope_5   = 0.009903;

  // Intercepts
  // ==========
  // Voltage Measurement Intercepts
  // ------------------------------
  // Power Measurement #1
  const float P1_aux_volt_intercept_1 = 0.115401;
  const float P1_aux_volt_intercept_2 = 0.087508;
  const float P1_aux_volt_intercept_3 = 0.200685;
  const float P1_aux_volt_intercept_4 = 0.135503;
  // Power Measurement #2
  const float P2_bus_volt_intercept_1 = 0.113760;
  const float P2_bus_volt_intercept_2 = 0.119558;
  const float P2_bus_volt_intercept_3 = 0.135135;
  const float P2_bus_volt_intercept_4 = 0.135135;
  // Power Measurement #5
  const float P5_dc_volt_intercept_1  = 0.111905;
  const float P5_dc_volt_intercept_2  = 0.112877;
  const float P5_dc_volt_intercept_3  = 0.168919;
  const float P5_dc_volt_intercept_4  = 0.168919;
  // Power Measurement #6 
  const float P6_in_volt_intercept_1  = 0.124060;
  const float P6_in_volt_intercept_2  = 0.137812;
  const float P6_in_volt_intercept_3  = 0.134130;
  const float P6_in_volt_intercept_4  = 0.212766;
     
  // Current Measurement Intercepts
  // ------------------------------
  // Power Measurement #1
  const float P1_aux_curr_intercept_1 =-0.006300;
  const float P1_aux_curr_intercept_2 =-0.006770;
  const float P1_aux_curr_intercept_3 =-0.008586;
  const float P1_aux_curr_intercept_4 =-0.003098;
  const float P1_aux_curr_intercept_5 = 0.000398; 
  // Power Measurement #2
  const float P2_bus_curr_intercept_1 = 0.025869;
  const float P2_bus_curr_intercept_2 = 0.022838;
  const float P2_bus_curr_intercept_3 = 0.025803;
  const float P2_bus_curr_intercept_4 = 0.031262;
  const float P2_bus_curr_intercept_5 = 0.032271; 
  // Power Measurement #3
  const float P3_sys_curr_intercept_1 = 0.025776;
  const float P3_sys_curr_intercept_2 = 0.027250;
  const float P3_sys_curr_intercept_3 = 0.022800;
  const float P3_sys_curr_intercept_4 = 0.031405;
  const float P3_sys_curr_intercept_5 = 0.034776; 
  // Power Measurement #4
  const float P4_ac_curr_intercept_1  =-0.000858;
  const float P4_ac_curr_intercept_2  =-0.001010;
  const float P4_ac_curr_intercept_3  =-0.002131;
  const float P4_ac_curr_intercept_4  = 0.004505;
  const float P4_ac_curr_intercept_5  = 0.010942;
  // Power Measurement #5
  const float P5_dc_curr_intercept_1  =-0.037741;
  const float P5_dc_curr_intercept_2  =-0.037741;
  const float P5_dc_curr_intercept_3  =-0.042061;
  const float P5_dc_curr_intercept_4  =-0.031994;
  const float P5_dc_curr_intercept_5  =-0.027070;
  // Power Measurement #6 
  const float P6_in_curr_intercept_1  = 0.031188;
  const float P6_in_curr_intercept_2  = 0.028173;
  const float P6_in_curr_intercept_3  = 0.031062;
  const float P6_in_curr_intercept_4  = 0.032000;
  const float P6_in_curr_intercept_5  = 0.041347;


  // Upper Bounds for Ranges
  // =======================
  // Voltage Measurement Bounds
  // --------------------------
  // Power Measurement #1
  const float P1_aux_volt_bound_0 = 4.09;
  const float P1_aux_volt_bound_1 = 26.88;
  const float P1_aux_volt_bound_2 = 292;
  const float P1_aux_volt_bound_3 = 440.99;
  // Power Measurement #2
  const float P2_bus_volt_bound_0 = 4;
  const float P2_bus_volt_bound_1 = 26.02;
  const float P2_bus_volt_bound_2 = 292;
  const float P2_bus_volt_bound_3 = 440;
  // Power Measurement #5
  const float P5_dc_volt_bound_0  = 4.06;
  const float P5_dc_volt_bound_1  = 26.11;
  const float P5_dc_volt_bound_2  = 291;
  const float P5_dc_volt_bound_3  = 439;
  // Power Measurement #6
  const float P6_in_volt_bound_0  = 7;
  const float P6_in_volt_bound_1  = 34.93;
  const float P6_in_volt_bound_2  = 184.99;
  const float P6_in_volt_bound_3  = 560;

  // Current Measurement Bounds
  // --------------------------
  // Power Measurement #1
  const float P1_aux_curr_bound_0 = 5.63;
  const float P1_aux_curr_bound_1 = 15.63;
  const float P1_aux_curr_bound_2 = 25.6;
  const float P1_aux_curr_bound_3 = 50.35;
  const float P1_aux_curr_bound_4 = 100.39;
  // Power Measurement #2
  const float P2_bus_curr_bound_0 = 2.43;
  const float P2_bus_curr_bound_1 = 12.5;
  const float P2_bus_curr_bound_2 = 22.33;
  const float P2_bus_curr_bound_3 = 47.23;
  const float P2_bus_curr_bound_4 = 97.61;
  // Power Measurement #3
  const float P3_sys_curr_bound_0 = 2.42;
  const float P3_sys_curr_bound_1 = 12.41;
  const float P3_sys_curr_bound_2 = 22.52;
  const float P3_sys_curr_bound_3 = 47.3;
  const float P3_sys_curr_bound_4 = 97.77;
  // Power Measurement #4
  const float P4_ac_curr_bound_0  = 5.04;
  const float P4_ac_curr_bound_1  = 14.95;
  const float P4_ac_curr_bound_2  = 24.85;
  const float P4_ac_curr_bound_3  = 49.49;
  const float P4_ac_curr_bound_4  = 99.43;
  // Power Measurement #5
  const float P5_dc_curr_bound_0  = 8.66;
  const float P5_dc_curr_bound_1  = 18.53;
  const float P5_dc_curr_bound_2  = 28.4;
  const float P5_dc_curr_bound_3  = 52.71;
  const float P5_dc_curr_bound_4  = 102.25;
  // Power Measurement #6
  const float P6_in_curr_bound_0  = 1.9;
  const float P6_in_curr_bound_1  = 12;
  const float P6_in_curr_bound_2  = 21.85;
  const float P6_in_curr_bound_3  = 46.8;
  const float P6_in_curr_bound_4  = 96.8;


// ==============================================================================================

void setup() {
  // Serial Communitication Initialization
  Serial.begin(9600);
  
  // SPI Initializations
  // -------------------
  // Set SPI pin directions
  pinMode(SPI_CLOCK, OUTPUT);
  pinMode(SPI_DATA,  OUTPUT);
  // Set SPI outputs low
  digitalWrite(SPI_CLOCK, LOW);
  digitalWrite(SPI_DATA,  LOW);
  // Settings and communication startup
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  SPI.begin();
  
  // Shift register initializations
  pinMode(SHIFT_ENABLE, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(SHIFT_READ, INPUT);
  digitalWrite(SHIFT_ENABLE, LOW);
  digitalWrite(SHIFT_LATCH, LOW);

  // 4:1 Analog MUX initializations
  analogReference(DEFAULT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S0, OUTPUT);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S0, LOW);
  
  /*
  // ADC pins
  pinMode(FINE_PIN, INPUT);
  pinMode(VOUT_PIN, INPUT);

  // Load selector pins
  pinMode(LS_BAT_ENABLE, OUTPUT);
  pinMode(LS_AUX_ENABLE, OUTPUT);
  digitalWrite(LS_BAT_ENABLE, LOW);
  digitalWrite(LS_AUX_ENABLE, LOW);
  */

  // PWM pins initializations
  pinMode(PWM_AC, OUTPUT);
  pinMode(PWM_CC, OUTPUT);
  analogWrite(PWM_AC, 0);
  analogWrite(PWM_CC, 0);
  TCCR1B = TCCR1B & 0xF9;  // Set the PWM frequency on pin 9 and 10 to 31.250kHz

  // LCD Initializations
  //lcd_1.noBacklight(); // turn off backlight
  lcd_1.init();  // initialize the lcd
  lcd_2.init();  // initialize the lcd
  lcd_3.init();  // initialize the lcd
  lcd_1.clear();
  lcd_2.clear();
  lcd_3.clear();
  lcd_1.backlight(); // turn on backlight
  lcd_2.backlight(); // turn on backlight
  lcd_3.backlight(); // turn on backlight
  
  // Clear previous shift register contents and enable output
  Update_shift_registers();
  digitalWrite(SHIFT_ENABLE, HIGH);

  // Startup LED animation
  //LED_anima
  
  // Intialize LEDs to "off" state
  bitSet(shift_outputs, SS_OFF_LED_RED);
  bitSet(shift_outputs, CC_OFF_LED_RED);
  bitSet(shift_outputs, LS_OFF_LED_RED);
  
  Turn_buttons_on(); // Turn on push buttons
  attachInterrupt(1, ISP_Button_Press, RISING);  // Set up interrupt for push buttons
  
  Serial.print("\n");
}

// ==============================================================================================

// Total loop duration is about 500ms to meet our 1Hz refresh rate spec
void loop() {
  //Serial.print(analogRead(A2)); Serial.print("\n");

  
  // Update measurements
  Get_Analog_Measurements();
  //Get_Analog_Measurements_Test();
  Calculate_Power_Measurements();



  // Display information
  LCD_print_power_measurements();
  LCD_print_cc_info();

  Serial.print(millis()/1000.0);
  Serial.print(" "); 
  if (bat_mode == BAT_NOT_CHARGING)
    Serial.print("of");
  if (bat_mode == BAT_CHARGING_CC)
    Serial.print("cc");
  if (bat_mode == BAT_CHARGING_CV)
    Serial.print("cv");
  if (bat_mode == BAT_FLOAT)
    Serial.print("fl");
  Serial.print(" ");
  Serial.print(cc_duty_cycle_percent);
  Serial.print(" ");
  Serial.print(efficiency);
  Serial.print(" ");
  Serial.print(P7_bat_volt,5);
  Serial.print(" ");
  Serial.print(P7_bat_curr,5);
  Serial.print("\n");
  
  /*
  Serial.print("Bat Status: "); Serial.print(bat_mode); Serial.print("\n");
  Serial.print("Duty Cycle: "); Serial.print(cc_duty_cycle*100.0/255.0); Serial.print("%\n");
  Serial.print("Efficiency: "); Serial.print(P2_bus_pwr/P6_in_pwr*100.0); Serial.print("%\n");
  Serial.print("Bat volt = "); Serial.print(P7_bat_volt,5); Serial.print("\n");
  Serial.print("Bat curr = "); Serial.print(P7_bat_curr,5); Serial.print("\n");
  Serial.print("================\n");
  */

  // Adjust operating status
  Update_SS_Status();
  Update_CC_Status();

}

// ==============================================================================================

// LCD functions
// -------------
void LCD_print_cc_info(void) {
  //lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print(" Charge Controller  ");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Bat Status: ");
  lcd_1.print(bat_mode);
  //Serial.print("Bat Status: "); Serial.print(bat_mode); Serial.print("\n");

  lcd_1.setCursor(0, 2);
  lcd_1.print("Duty Cycle: ");
  cc_duty_cycle_percent = cc_duty_cycle*100.0/255.0; 
  if (cc_duty_cycle_percent < 100) lcd_1.print("0");
  if (cc_duty_cycle_percent < 10) lcd_1.print("0");
  lcd_1.print(cc_duty_cycle_percent, 2);
  lcd_1.print("%");
  //Serial.print("Duty Cycle: "); Serial.print(cc_duty_cycle_percent); Serial.print("%\n");

  lcd_1.setCursor(0, 3);
  lcd_1.print("Efficiency: ");
  if (cc_duty_cycle_percent == 0)
    efficiency = 0;
  else
    efficiency = P2_bus_pwr/P6_in_pwr*100.0;
  if (efficiency > 100) efficiency = 100;
  // fix: if (efficiency == NAN) efficiency = 0;
  if (efficiency < 100) lcd_1.print("0");
  if (efficiency < 10) lcd_1.print("0");
  lcd_1.print(efficiency, 2);
  lcd_1.print("%");
  //Serial.print("Efficiency: "); Serial.print(efficiency); Serial.print("%\n");
  
} // LCD_print_cc_info

void LCD_print_power_measurements(void) {
  // Voltage, current, and power measurements are displayed with 4 significant digits

  // LCD Display #2
  // --------------
  // --------------
  lcd_2.setCursor(0, 0);
  lcd_2.print("Power Monitoring Sys");
  
  // P6: Input node measurements
  // ---------------------------
  lcd_2.setCursor(0, 1);
  // Print voltage measurement
  if (P6_in_volt < 10) lcd_2.print("0");
  lcd_2.print(P6_in_volt, 2); lcd_2.print("V ");
  // Print current measurement
  if (P6_in_curr > 10) lcd_2.print(P6_in_curr, 2);
  else lcd_2.print(P6_in_curr, 3);
  lcd_2.print("A ");
  // Print power measurement
  if (P6_in_pwr < 100) lcd_2.print("0");
  if (P6_in_pwr < 10) lcd_2.print("0");
  lcd_2.print(P6_in_pwr, 1); lcd_2.print("W");
  
  // P2: Bus node measurements
  // -------------------------
  lcd_2.setCursor(0, 2);
  // Print voltage measurement
  if (P2_bus_volt < 10) lcd_2.print("0");
  lcd_2.print(P2_bus_volt, 2); lcd_2.print("V ");
  // Print current measurement
  if (P2_bus_curr > 10) lcd_2.print(P2_bus_curr, 2);
  else lcd_2.print(P2_bus_curr, 3);
  lcd_2.print("A ");
  // Print power measurement
  if (P2_bus_pwr < 100) lcd_2.print("0");
  if (P2_bus_pwr < 10) lcd_2.print("0");
  lcd_2.print(P2_bus_pwr, 1); lcd_2.print("W");

  // P1: Auxiliary output node measurements
  // --------------------------------------
  lcd_2.setCursor(0, 3);
  // Print voltage measurement
  if (P1_aux_volt < 10) lcd_2.print("0");
  lcd_2.print(P1_aux_volt, 2); lcd_2.print("V ");
  // Print current measurement
  if (P1_aux_curr > 10) lcd_2.print(P1_aux_curr, 2);
  else lcd_2.print(P1_aux_curr, 3);
  lcd_2.print("A ");
  // Print power measurement
  if (P1_aux_pwr < 100) lcd_2.print("0");
  if (P1_aux_pwr < 10) lcd_2.print("0");
  lcd_2.print(P1_aux_pwr, 1); lcd_2.print("W");


  // LCD Display #3
  // --------------
  // --------------
  
  // P5: 12VDC output node measurements
  // ----------------------------------
  lcd_3.setCursor(0, 0);
  // Print voltage measurement
  if (P5_dc_volt < 10) lcd_3.print("0");
  lcd_3.print(P5_dc_volt, 2); lcd_3.print("V ");
  // Print current measurement
  if (P5_dc_curr > 10) lcd_3.print(P5_dc_curr, 2);
  else lcd_3.print(P5_dc_curr, 3);
  lcd_3.print("A ");
  // Print power measurement
  if (P5_dc_pwr < 100) lcd_3.print("0");
  if (P5_dc_pwr < 10) lcd_3.print("0");
  lcd_3.print(P5_dc_pwr, 1); lcd_3.print("W");
  
  // P4: 120VAC node measurements (Actually the 12VDC output going to the inverter)
  // ------------------------------------------------------------------------------
  lcd_3.setCursor(0, 1);
  // Print voltage measurement
  if (P4_ac_volt < 10) lcd_3.print("0");
  lcd_3.print(P4_ac_volt, 2); lcd_3.print("V ");
  // Print current measurement
  if (P4_ac_curr > 10) lcd_3.print(P4_ac_curr, 2);
  else lcd_3.print(P4_ac_curr, 3);
  lcd_3.print("A ");
  // Print power measurement
  if (P4_ac_pwr < 100) lcd_3.print("0");
  if (P4_ac_pwr < 10) lcd_3.print("0");
  lcd_3.print(P4_ac_pwr, 1); lcd_3.print("W");
  
  // P3: Systems power node measurements
  // -----------------------------------
  lcd_3.setCursor(0, 2);
  // Print voltage measurement
  if (P3_sys_volt < 10) lcd_3.print("0");
  lcd_3.print(P3_sys_volt, 2); lcd_3.print("V ");
  // Print current measurement
  if (P3_sys_curr > 10) lcd_3.print(P3_sys_curr, 2);
  else lcd_3.print(P3_sys_curr, 3);
  lcd_3.print("A ");
  // Print power measurement
  if (P3_sys_pwr < 100) lcd_3.print("0");
  if (P3_sys_pwr < 10) lcd_3.print("0");
  lcd_3.print(P3_sys_pwr, 1); lcd_3.print("W");
  
  // P7: Battery output node measurements
  // ------------------------------------
  lcd_3.setCursor(0, 3);
  // Print voltage measurement
  if (P7_bat_volt < 10) lcd_3.print("0");
  lcd_3.print(P7_bat_volt, 2); lcd_3.print("V");
  // Print current measurement
  if (P7_bat_curr >= 0) {
    lcd_3.print("+");
    if (P7_bat_curr > 10) lcd_3.print(P7_bat_curr, 1);
    else lcd_3.print(P7_bat_curr, 3);
  }
  else {
    lcd_3.print("-");
    if (P7_bat_curr < -10) lcd_3.print(-1*P7_bat_curr, 1);
    else lcd_3.print(-1*P7_bat_curr, 3);
  }
  lcd_3.print("A");
  // Print power measurement
  if (P7_bat_pwr >= 0) {
    lcd_3.print("+");
    if (P7_bat_pwr >= 100) {
      lcd_3.print((int)P7_bat_pwr);
      lcd_3.print(".");
    }
    else {
      if (P7_bat_pwr < 100) lcd_3.print("0");
      if (P7_bat_pwr < 10) lcd_3.print("0");
      lcd_3.print(P7_bat_pwr, 1);
    }
  }
  else {
    lcd_3.print("-");
    if (P7_bat_pwr <= -100) {
      lcd_3.print(-1*(int)P7_bat_pwr);
      lcd_3.print(".");
    }
    else {
      if (P7_bat_pwr > -100) lcd_3.print("0");
      if (P7_bat_pwr > -10) lcd_3.print("0");
      lcd_3.print(-1*P7_bat_pwr, 1);
    }
  }
  lcd_3.print("W");
} // LCD_print_power_measurements

void LCD_clear_row(String lcd_number, uint8_t row) {
  if (lcd_number == "lcd_1") {
      lcd_1.setCursor(0, row);
      lcd_1.print("                    ");
  }
  else if (lcd_number == "lcd_2") {
      lcd_2.setCursor(0, row);
      lcd_2.print("                    ");
  }
  else if (lcd_number == "lcd_3") {
      lcd_3.setCursor(0, row);
      lcd_3.print("                    ");
  }
} // LCD_clear_row

// ==============================================================================================

void Update_source_selector_mode(void) {
  if ((ss_off_button == HIGH) && (!(ss_mode == SS_OFF))) {
    ss_mode = SS_OFF;
    Turn_source_selector_mode_LEDs_off();
    bitSet(shift_outputs, SS_OFF_LED_RED);
    Update_shift_registers();
  }
  else if ((ss_auto_button == HIGH) && (!(ss_mode == SS_AUTO))) {
    ss_mode = SS_AUTO;
    Turn_source_selector_mode_LEDs_off();
    bitSet(shift_outputs, SS_AUTO_LED_GREEN);
    Update_shift_registers();
  }
  else if ((ss_manu_button == HIGH) && (!(ss_mode == SS_MANU))) {
    ss_mode = SS_MANU;
    Turn_source_selector_mode_LEDs_off();
    bitSet(shift_outputs, SS_MANU_LED_GREEN);
    Update_shift_registers();
  }
  else if (ss_ac_button == HIGH) {
    if (!(ss_mode == SS_MANU)) {
      ss_mode = SS_MANU;
      Turn_source_selector_mode_LEDs_off();
      bitSet(shift_outputs, SS_MANU_LED_GREEN);
      Update_shift_registers();
    }
    if (ss_source == SS_AC) ss_source = SS_NONE;
    else ss_source = SS_AC;
  }
  else if (ss_bike_button == HIGH) {
    if (!(ss_mode == SS_MANU)) {
      ss_mode = SS_MANU;
      Turn_source_selector_mode_LEDs_off();
      bitSet(shift_outputs, SS_MANU_LED_GREEN);
      Update_shift_registers();
    }
    if (ss_source == SS_BIKE) ss_source = SS_NONE;
    else ss_source = SS_BIKE;
  }
  else if (ss_aux_button == HIGH) {
    if (!(ss_mode == SS_MANU)) {
      ss_mode = SS_MANU;
      Turn_source_selector_mode_LEDs_off();
      bitSet(shift_outputs, SS_MANU_LED_GREEN);
      Update_shift_registers();
    }
    if (ss_source == SS_AUX) ss_source = SS_NONE;
    else ss_source = SS_AUX;
  }
} // Update_source_selector_mode

void Update_charge_controller_mode(void) {
  if ((cc_off_button == HIGH) && (!(cc_mode == CC_OFF))) {
    cc_mode = CC_OFF;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_OFF_LED_RED);
    Update_shift_registers();
  }
  else if ((cc_bat_button == HIGH) && (!(cc_mode == CC_BAT))) {
    cc_mode = CC_BAT;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_BAT_LED_GREEN);
    Update_shift_registers();
  }
 else if ((cc_high_button == HIGH) && (!(cc_mode == CC_HIGH))) {
    cc_mode = CC_HIGH;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_HIGH_LED_GREEN);
    Update_shift_registers();
  }
  else if ((cc_low_button == HIGH) && (!(cc_mode == CC_LOW))) {
    cc_mode = CC_LOW;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_LOW_LED_GREEN);
    Update_shift_registers();
  }
  else if ((cc_fine_button == HIGH) && (!(cc_mode == CC_FINE))) {
    cc_mode = CC_FINE;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_FINE_LED_GREEN);
    Update_shift_registers();
  }
  else if ((cc_crse_button == HIGH) && (!(cc_mode == CC_CRSE))) {
    cc_mode = CC_CRSE;
    Turn_charge_controller_mode_LEDs_off();
    bitSet(shift_outputs, CC_CRSE_LED_GREEN);
    Update_shift_registers();
  }
} // Update_charge_controller_mode

void Update_load_selector_mode(void) {
  if ((ls_off_button == HIGH) && (!(ls_mode == LS_OFF))) {
    ls_mode = LS_OFF;
    Turn_load_selector_mode_LEDs_off();
    bitSet(shift_outputs, LS_OFF_LED_RED);
    bitClear(shift_outputs, LS_BAT_ENABLE);
    bitClear(shift_outputs, LS_AUX_ENABLE);
    Update_shift_registers();
  }
  else if ((ls_bat_button == HIGH) && (!(ls_mode == LS_BAT))) {
    ls_mode = LS_BAT;
    Turn_load_selector_mode_LEDs_off();
    bitSet(shift_outputs, LS_BAT_LED_GREEN);
    bitClear(shift_outputs, LS_AUX_ENABLE);
    Update_shift_registers();
    delayMicroseconds(5000);
    bitSet(shift_outputs, LS_BAT_ENABLE);
    Update_shift_registers();
  }
  else if ((ls_aux_button == HIGH) && (!(ls_mode == LS_AUX))) {
    ls_mode = LS_AUX;
    Turn_load_selector_mode_LEDs_off();
    bitSet(shift_outputs, LS_AUX_LED_GREEN);
    bitClear(shift_outputs, LS_BAT_ENABLE);
    Update_shift_registers();
    delayMicroseconds(5000);
    bitSet(shift_outputs, LS_AUX_ENABLE);
    Update_shift_registers(); 
  }
} // Update_load_selector_mode

void Update_alternator_controller_mode(void) {
  if ((ac_en_button == HIGH) && (!(ac_mode == AC_OFF))) {
    ac_mode = AC_OFF;
    bitClear(shift_outputs, AC_EN_LED_GREEN);
    Update_shift_registers();  
    analogWrite(PWM_AC, 0);  
  }
  else if ((ac_en_button == HIGH) && (!(ac_mode == AC_ON))) {
    ac_mode = AC_ON;
    bitSet(shift_outputs, AC_EN_LED_GREEN);
    Update_shift_registers();
    analogWrite(PWM_AC, ac_duty_cycle);  
  }
} // Update_alternator_controller_mode

void Turn_source_selector_mode_LEDs_off(void) {
  bitClear(shift_outputs, SS_OFF_LED_RED);
  bitClear(shift_outputs, SS_AUTO_LED_GREEN);
  bitClear(shift_outputs, SS_MANU_LED_GREEN);
  //bitClear(shift_outputs, SS_AC_LED_GREEN);
  //bitClear(shift_outputs, SS_BIKE_LED_GREEN);
  //bitClear(shift_outputs, SS_AUX_LED_GREEN);
  Update_shift_registers();
} // Turn_source_selector_mode_LEDs_off

void Turn_charge_controller_mode_LEDs_off(void) {
  bitClear(shift_outputs, CC_OFF_LED_RED);
  bitClear(shift_outputs, CC_BAT_LED_GREEN);
  bitClear(shift_outputs, CC_HIGH_LED_GREEN);
  bitClear(shift_outputs, CC_LOW_LED_GREEN);
  bitClear(shift_outputs, CC_FINE_LED_GREEN);
  bitClear(shift_outputs, CC_CRSE_LED_GREEN);
  Update_shift_registers();
} // Turn_charge_controller_mode_LEDs_off

void Turn_load_selector_mode_LEDs_off(void) {
  bitClear(shift_outputs, LS_OFF_LED_RED);
  bitClear(shift_outputs, LS_BAT_LED_GREEN);
  bitClear(shift_outputs, LS_AUX_LED_GREEN);
  Update_shift_registers();
} // Turn_load_selector_mode_LEDs_off

void Locate_button_presses(void) {
  int button_press_delay = 0;
  
  // Source selector auto button
  bitSet(shift_buttons, SS_OFF_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ss_off_button = 1;
  }
  else {
    ss_off_button = 0;
  }
  bitClear(shift_buttons, SS_OFF_BUTTON);
  
  // Source selector auto button
  bitSet(shift_buttons, SS_AUTO_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ss_auto_button = 1;
  }
  else {
    ss_auto_button = 0;
  }
  bitClear(shift_buttons, SS_AUTO_BUTTON);
  
  // Source selector manual button
  bitSet(shift_buttons, SS_MANU_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
   if (digitalRead(SHIFT_READ) == HIGH) {
    ss_manu_button = 1;
  }
  else {
    ss_manu_button = 0;
  }
  bitClear(shift_buttons, SS_MANU_BUTTON);
  
  // Source selector ac button
  bitSet(shift_buttons, SS_AC_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ss_ac_button = 1;
  }
  else {
    ss_ac_button = 0;
  }
  bitClear(shift_buttons, SS_AC_BUTTON);
  
  // Source selector bike button
  bitSet(shift_buttons, SS_BIKE_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ss_bike_button = 1;
  }
  else {
    ss_bike_button = 0;
  }
  bitClear(shift_buttons, SS_BIKE_BUTTON); 
  
  // Source selector aux button
  bitSet(shift_buttons, SS_AUX_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ss_aux_button = 1;
  }
  else {
    ss_aux_button = 0;
  }
  bitClear(shift_buttons, SS_AUX_BUTTON);
  
  // Charge controller off button
  bitSet(shift_buttons, CC_OFF_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_off_button = 1;
  }
  else {
    cc_off_button = 0;
  }
  bitClear(shift_buttons, CC_OFF_BUTTON);
  
  // Charge controller battery button
  bitSet(shift_buttons, CC_BAT_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_bat_button = 1;
  }
  else {
    cc_bat_button = 0;
  }  
  bitClear(shift_buttons, CC_BAT_BUTTON);

  // Charge contoller high button
  bitSet(shift_buttons, CC_HIGH_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_high_button = 1;
  }
  else {
    cc_high_button = 0;
  }  
  bitClear(shift_buttons, CC_HIGH_BUTTON);
  
  // Charge controller low button
  bitSet(shift_buttons, CC_LOW_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_low_button = 1;
  }
  else {
    cc_low_button = 0;
  }  
  bitClear(shift_buttons, CC_LOW_BUTTON);

  // Charge controller fine button
  bitSet(shift_buttons, CC_FINE_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_fine_button = 1;
  }
  else {
    cc_fine_button = 0;
  }  
  bitClear(shift_buttons, CC_FINE_BUTTON);
  
  // Charge controller course button
  bitSet(shift_buttons, CC_CRSE_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_crse_button = 1;
  }
  else {
    cc_crse_button = 0;
  }  
  bitClear(shift_buttons, CC_CRSE_BUTTON);

  // Charge controller up button
  bitSet(shift_buttons, CC_UP_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_up_button = 1;
  }
  else {
    cc_up_button = 0;
  }  
  bitClear(shift_buttons, CC_UP_BUTTON);

  // Charge controller down button
  bitSet(shift_buttons, CC_DOWN_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    cc_down_button = 1;
  }
  else {
    cc_down_button = 0;
  }  
  bitClear(shift_buttons, CC_DOWN_BUTTON);
   
  // Load selector off button
  bitSet(shift_buttons, LS_OFF_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
   if (digitalRead(SHIFT_READ) == HIGH) {
    ls_off_button = 1;
  }
  else {
    ls_off_button = 0;
  } 
  bitClear(shift_buttons, LS_OFF_BUTTON);

  // Load selector battery button
  bitSet(shift_buttons, LS_BAT_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ls_bat_button = 1;
  }
  else {
    ls_bat_button = 0;
  }  
  bitClear(shift_buttons, LS_BAT_BUTTON);
  
  // Load selector aux button
  bitSet(shift_buttons, LS_AUX_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ls_aux_button = 1;
  }
  else {
    ls_aux_button = 0;
  }  
  bitClear(shift_buttons, LS_AUX_BUTTON);

  // Alternator controller enable button
  bitSet(shift_buttons, AC_EN_BUTTON);
  Update_shift_registers();
  delayMicroseconds(button_press_delay);
  if (digitalRead(SHIFT_READ) == HIGH) {
    ac_en_button = 1;
  }
  else {
    ac_en_button = 0;
  }  
  bitClear(shift_buttons, AC_EN_BUTTON);
} // Locate_button_presses

void Turn_buttons_off(void) {
  bitClear(shift_buttons, SS_OFF_BUTTON);
  bitClear(shift_buttons, SS_AUTO_BUTTON);
  bitClear(shift_buttons, SS_MANU_BUTTON);
  bitClear(shift_buttons, SS_AC_BUTTON);
  bitClear(shift_buttons, SS_BIKE_BUTTON);
  bitClear(shift_buttons, SS_AUX_BUTTON);
  
  bitClear(shift_buttons, CC_OFF_BUTTON);
  bitClear(shift_buttons, CC_BAT_BUTTON);
  bitClear(shift_buttons, CC_HIGH_BUTTON);
  bitClear(shift_buttons, CC_LOW_BUTTON);
  bitClear(shift_buttons, CC_FINE_BUTTON);
  bitClear(shift_buttons, CC_CRSE_BUTTON);
  bitClear(shift_buttons, CC_UP_BUTTON);
  bitClear(shift_buttons, CC_DOWN_BUTTON);
  
  bitClear(shift_buttons, LS_OFF_BUTTON);
  bitClear(shift_buttons, LS_BAT_BUTTON);
  bitClear(shift_buttons, LS_AUX_BUTTON);

  bitClear(shift_buttons, AC_EN_BUTTON);

  Update_shift_registers();
} // Turn_buttons_off

void Turn_buttons_on(void) {
  bitSet(shift_buttons, SS_OFF_BUTTON);
  bitSet(shift_buttons, SS_AUTO_BUTTON);
  bitSet(shift_buttons, SS_MANU_BUTTON);
  bitSet(shift_buttons, SS_AC_BUTTON);
  bitSet(shift_buttons, SS_BIKE_BUTTON);
  bitSet(shift_buttons, SS_AUX_BUTTON);
  
  bitSet(shift_buttons, CC_OFF_BUTTON);
  bitSet(shift_buttons, CC_BAT_BUTTON);
  bitSet(shift_buttons, CC_HIGH_BUTTON);
  bitSet(shift_buttons, CC_LOW_BUTTON);
  bitSet(shift_buttons, CC_FINE_BUTTON);
  bitSet(shift_buttons, CC_CRSE_BUTTON);
  bitSet(shift_buttons, CC_UP_BUTTON);
  bitSet(shift_buttons, CC_DOWN_BUTTON);
  
  bitSet(shift_buttons, LS_OFF_BUTTON);
  bitSet(shift_buttons, LS_BAT_BUTTON);
  bitSet(shift_buttons, LS_AUX_BUTTON);

  bitSet(shift_buttons, AC_EN_BUTTON);

  Update_shift_registers();
} // Turn_buttons_on

void ISP_Button_Press(void) {
  // Interrupt debouncing variables
  static unsigned long last_interrupt_time = 0;
  unsigned long interrupt_time = millis();
  
  // If interrupts come faster than 200ms, assume it's a bounce and ignore
  if (interrupt_time - last_interrupt_time > 200) {
    
    Turn_buttons_off();
    Locate_button_presses();
    
    // Update modes
    Update_source_selector_mode();
    Update_charge_controller_mode();
    Update_load_selector_mode();
    Update_alternator_controller_mode();
        

    Update_SS_Status();
    Update_CC_Status();

    Clear_button_presses();
    Turn_buttons_on();
    //while(digitalRead(SHIFT_READ)); // Wait while button pressed

  }
  last_interrupt_time = interrupt_time;
} // ISP_Button_Press

void Update_shift_registers(void) {
  // Pull latch low
  digitalWrite(SHIFT_LATCH, LOW);
  delayMicroseconds(5000);

 
  // Transfer data  
  SPI.transfer(shift_buttons>>16 & 0xFF);
  SPI.transfer(shift_buttons>>8  & 0xFF);
  SPI.transfer(shift_buttons     & 0xFF);
  SPI.transfer(shift_outputs>>16 & 0xFF);
  SPI.transfer(shift_outputs>>8  & 0xFF);
  SPI.transfer(shift_outputs     & 0xFF);
 

  /*
  // Transfer data - Used if not usual designated SPI pins, be sure to remove unnecessary SPI code
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_buttons>>16 & 0xFF);
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_buttons>>8 & 0xFF);
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_buttons & 0xFF);
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_outputs>>16 & 0xFF);
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_outputs>>8 & 0xFF);
  shiftOut(SPI_DATA, SPI_CLOCK, MSBFIRST, shift_outputs & 0xFF);
  */
  
  // Pull latch high
  digitalWrite(SHIFT_LATCH, HIGH);
  delayMicroseconds(5000); 
} // Update_shift_registers

void Clear_button_presses(void) {
  // Source selector
  ss_off_button = 0;
  ss_auto_button = 0;
  ss_manu_button = 0;
  ss_ac_button = 0;
  ss_bike_button = 0;
  ss_aux_button = 0;
  
  // Charge controller
  cc_off_button = 0;
  cc_bat_button = 0;
  cc_high_button = 0;
  cc_low_button = 0;
  cc_fine_button = 0;
  cc_crse_button = 0;
  cc_up_button = 0;
  cc_down_button = 0;
  
  // Load selector
  ls_off_button = 0;
  ls_bat_button = 0;
  ls_aux_button = 0;
  
  // Alternator controller
  ac_en_button = 0;
} // Clear_button_presses

// ==============================================================================================

void Get_Analog_Measurements(void) {
  // Variables
  unsigned int i;
  float ADC_sum;

  // S0 = 0, S1 = 0
  // --------------
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  delay(measurement_delay_ms);
    
  // Get channel 1 measurement
  ADC_sum = 0;
  analogRead(A0); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A0);
  }
  P1_aux_volt = ADC_sum/num_samples;
  if (P1_aux_volt < P1_aux_volt_bound_0) P1_aux_volt = 0;
  else if (P1_aux_volt < P1_aux_volt_bound_1) P1_aux_volt = P1_aux_volt_slope_1*P1_aux_volt + P1_aux_volt_intercept_1;
  else if (P1_aux_volt < P1_aux_volt_bound_2) P1_aux_volt = P1_aux_volt_slope_2*P1_aux_volt + P1_aux_volt_intercept_2;
  else if (P1_aux_volt < P1_aux_volt_bound_3) P1_aux_volt = P1_aux_volt_slope_3*P1_aux_volt + P1_aux_volt_intercept_3;
  else P1_aux_volt = P1_aux_volt_slope_4*P1_aux_volt + P1_aux_volt_intercept_4;
  //Serial.print("P1_aux_volt = "); Serial.print(P1_aux_volt,5); Serial.print("\n");
  
  // Get channel 2 measurement
  ADC_sum = 0;
  analogRead(A1); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A1);
  }
  P2_bus_curr = ADC_sum/num_samples;
  if (P2_bus_curr < P2_bus_curr_bound_0) P2_bus_curr = 0;
  else if (P2_bus_curr < P2_bus_curr_bound_1) P2_bus_curr = P2_bus_curr_slope_1*P2_bus_curr + P2_bus_curr_intercept_1;
  else if (P2_bus_curr < P2_bus_curr_bound_2) P2_bus_curr = P2_bus_curr_slope_2*P2_bus_curr + P2_bus_curr_intercept_2;
  else if (P2_bus_curr < P2_bus_curr_bound_3) P2_bus_curr = P2_bus_curr_slope_3*P2_bus_curr + P2_bus_curr_intercept_3;
  else if (P2_bus_curr < P2_bus_curr_bound_4) P2_bus_curr = P2_bus_curr_slope_4*P2_bus_curr + P2_bus_curr_intercept_4;
  else P2_bus_curr = P2_bus_curr_slope_5*P2_bus_curr + P2_bus_curr_intercept_5;
  //Serial.print("P2_bus_curr = "); Serial.print(P2_bus_curr,5); Serial.print("\n");
  
  // Get channel 3 measurement
  ADC_sum = 0;
  analogRead(A2); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A2);
  }
  P1_aux_curr = ADC_sum/num_samples-2;
  if (P1_aux_curr < P1_aux_curr_bound_0) P1_aux_curr = 0;
  else if (P1_aux_curr < P1_aux_curr_bound_1) P1_aux_curr = P1_aux_curr_slope_1*P1_aux_curr + P1_aux_curr_intercept_1;
  else if (P1_aux_curr < P1_aux_curr_bound_2) P1_aux_curr = P1_aux_curr_slope_2*P1_aux_curr + P1_aux_curr_intercept_2;
  else if (P1_aux_curr < P1_aux_curr_bound_3) P1_aux_curr = P1_aux_curr_slope_3*P1_aux_curr + P1_aux_curr_intercept_3;
  else if (P1_aux_curr < P1_aux_curr_bound_4) P1_aux_curr = P1_aux_curr_slope_4*P1_aux_curr + P1_aux_curr_intercept_4;
  else P1_aux_curr = P1_aux_curr_slope_5*P1_aux_curr + P1_aux_curr_intercept_5;
  //Serial.print("P1_aux_curr = "); Serial.print(P1_aux_curr,5); Serial.print("\n");

  // Get channel 4 measurement
  ADC_sum = 0;
  analogRead(A3); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A3);
  }
  ADC_ac_curr = ADC_ac_curr_scale*(ADC_sum/num_samples);
  if (ADC_ac_curr < 0) ADC_ac_curr = 0;
  //Serial.print("ADC_ac_curr = "); Serial.print(ADC_ac_curr,5); Serial.print("\n");

  //Serial.print("----------------\n");

  
  // S0 = 0, S1 = 1
  // --------------
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, HIGH);
  delay(measurement_delay_ms);
    
  // Get channel 1 measurement
  ADC_sum = 0;
  analogRead(A0); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A0);
  }
  P2_bus_volt = ADC_sum/num_samples;
  if (P2_bus_volt < P2_bus_volt_bound_0) P2_bus_volt = 0;
  else if (P2_bus_volt < P2_bus_volt_bound_1) P2_bus_volt = P2_bus_volt_slope_1*P2_bus_volt + P2_bus_volt_intercept_1;
  else if (P2_bus_volt < P2_bus_volt_bound_2) P2_bus_volt = P2_bus_volt_slope_2*P2_bus_volt + P2_bus_volt_intercept_2;
  else if (P2_bus_volt < P2_bus_volt_bound_3) P2_bus_volt = P2_bus_volt_slope_3*P2_bus_volt + P2_bus_volt_intercept_3;
  else P2_bus_volt = P2_bus_volt_slope_4*P2_bus_volt + P2_bus_volt_intercept_4;
  //Serial.print("P2_bus_volt = "); Serial.print(P2_bus_volt,5); Serial.print("\n");
  
  // Get channel 2 measurement
  ADC_sum = 0;
  analogRead(A1); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A1);
  }
  P3_sys_curr = ADC_sum/num_samples;
  if (P3_sys_curr < P3_sys_curr_bound_0) P3_sys_curr = 0;
  else if (P3_sys_curr < P3_sys_curr_bound_1) P3_sys_curr = P3_sys_curr_slope_1*P3_sys_curr + P3_sys_curr_intercept_1;
  else if (P3_sys_curr < P3_sys_curr_bound_2) P3_sys_curr = P3_sys_curr_slope_2*P3_sys_curr + P3_sys_curr_intercept_2;
  else if (P3_sys_curr < P3_sys_curr_bound_3) P3_sys_curr = P3_sys_curr_slope_3*P3_sys_curr + P3_sys_curr_intercept_3;
  else if (P3_sys_curr < P3_sys_curr_bound_4) P3_sys_curr = P3_sys_curr_slope_4*P3_sys_curr + P3_sys_curr_intercept_4;
  else P3_sys_curr = P3_sys_curr_slope_5*P3_sys_curr + P3_sys_curr_intercept_5;
  //Serial.print("P3_sys_curr = "); Serial.print(P3_sys_curr,5); Serial.print("\n");
  
  // Get channel 3 measurement
  ADC_sum = 0;
  analogRead(A2); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A2);
  }
  ADC_ss_ac = ADC_ss_ac_scale*(ADC_sum/num_samples);
  if (ADC_ss_ac < 0) ADC_ss_ac = 0;
  //Serial.print("ADC_ss_ac = "); Serial.print(ADC_ss_ac,5); Serial.print("\n");
  
  // Get channel 4 measurement
  ADC_sum = 0;
  analogRead(A3); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A3);
  }
  ADC_cc_volt = ADC_cc_volt_scale*(ADC_sum/num_samples);
  if (ADC_cc_volt < 0) ADC_cc_volt = 0;
  //Serial.print("ADC_cc_volt = "); Serial.print(ADC_cc_volt,5); Serial.print("\n");

  //Serial.print("----------------\n");


  // S0 = 1, S1 = 0
  // --------------
  digitalWrite(MUX_S0, HIGH);
  digitalWrite(MUX_S1, LOW);
  delay(measurement_delay_ms);
    
  // Get channel 1 measurement
  ADC_sum = 0;
  analogRead(A0); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A0);
  }
  P5_dc_volt = ADC_sum/num_samples;
  if (P5_dc_volt < P5_dc_volt_bound_0) P5_dc_volt = 0;
  else if (P5_dc_volt < P5_dc_volt_bound_1) P5_dc_volt = P5_dc_volt_slope_1*P5_dc_volt + P5_dc_volt_intercept_1;
  else if (P5_dc_volt < P5_dc_volt_bound_2) P5_dc_volt = P5_dc_volt_slope_2*P5_dc_volt + P5_dc_volt_intercept_2;
  else if (P5_dc_volt < P5_dc_volt_bound_3) P5_dc_volt = P5_dc_volt_slope_3*P5_dc_volt + P5_dc_volt_intercept_3;
  else P5_dc_volt = P5_dc_volt_slope_4*P5_dc_volt + P5_dc_volt_intercept_4;
  //Serial.print("P5_dc_volt = "); Serial.print(P5_dc_volt,5); Serial.print("\n");
  
  // Get channel 2 measurement
  ADC_sum = 0;
  analogRead(A1); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A1);
  }
  P4_ac_curr = ADC_sum/num_samples;
  if (P4_ac_curr < P4_ac_curr_bound_0) P4_ac_curr = 0;
  else if (P4_ac_curr < P4_ac_curr_bound_1) P4_ac_curr = P4_ac_curr_slope_1*P4_ac_curr + P4_ac_curr_intercept_1;
  else if (P4_ac_curr < P4_ac_curr_bound_2) P4_ac_curr = P4_ac_curr_slope_2*P4_ac_curr + P4_ac_curr_intercept_2;
  else if (P4_ac_curr < P4_ac_curr_bound_3) P4_ac_curr = P4_ac_curr_slope_3*P4_ac_curr + P4_ac_curr_intercept_3;
  else if (P4_ac_curr < P4_ac_curr_bound_4) P4_ac_curr = P4_ac_curr_slope_4*P4_ac_curr + P4_ac_curr_intercept_4;
  else P4_ac_curr = P4_ac_curr_slope_5*P4_ac_curr + P4_ac_curr_intercept_5;
  //Serial.print("P4_ac_curr = "); Serial.print(P4_ac_curr,5); Serial.print("\n");
  
  // Get channel 3 measurement
  ADC_sum = 0;
  analogRead(A2); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A2);
  }
  ADC_ss_bike = ADC_ss_bike_scale*(ADC_sum/num_samples);
  if (ADC_ss_bike < 0) ADC_ss_bike = 0;
  //Serial.print("ADC_ss_bike = "); Serial.print(ADC_ss_bike,5); Serial.print("\n");
  
  // Get channel 4 measurement
  ADC_sum = 0;
  analogRead(A3); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A3);
  }
  P6_in_curr = ADC_sum/num_samples;
  if (P6_in_curr < P6_in_curr_bound_0) P6_in_curr = 0;
  else if (P6_in_curr < P6_in_curr_bound_1) P6_in_curr = P6_in_curr_slope_1*P6_in_curr + P6_in_curr_intercept_1;
  else if (P6_in_curr < P6_in_curr_bound_2) P6_in_curr = P6_in_curr_slope_2*P6_in_curr + P6_in_curr_intercept_2;
  else if (P6_in_curr < P6_in_curr_bound_3) P6_in_curr = P6_in_curr_slope_3*P6_in_curr + P6_in_curr_intercept_3;
  else if (P6_in_curr < P6_in_curr_bound_4) P6_in_curr = P6_in_curr_slope_4*P6_in_curr + P6_in_curr_intercept_4;
  else P6_in_curr = P6_in_curr_slope_5*P6_in_curr + P6_in_curr_intercept_5;
  //Serial.print("P6_in_curr = "); Serial.print(P6_in_curr,5); Serial.print("\n");

  //Serial.print("----------------\n");


  // S0 = 1, S1 = 1
  // --------------
  digitalWrite(MUX_S0, HIGH);
  digitalWrite(MUX_S1, HIGH);
  delay(measurement_delay_ms);
    
  // Get channel 1 measurement
  ADC_sum = 0;
  analogRead(A0); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A0);
  }
  P6_in_volt = ADC_sum/num_samples;
  if (P6_in_volt < P6_in_volt_bound_0) P6_in_volt = 0;
  else if (P6_in_volt < P6_in_volt_bound_1) P6_in_volt = P6_in_volt_slope_1*P6_in_volt + P6_in_volt_intercept_1;
  else if (P6_in_volt < P6_in_volt_bound_2) P6_in_volt = P6_in_volt_slope_2*P6_in_volt + P6_in_volt_intercept_2;
  else if (P6_in_volt < P6_in_volt_bound_3) P6_in_volt = P6_in_volt_slope_3*P6_in_volt + P6_in_volt_intercept_3;
  else P6_in_volt = P6_in_volt_slope_4*P6_in_volt + P6_in_volt_intercept_4;
  //Serial.print("P6_in_volt = "); Serial.print(P6_in_volt,5); Serial.print("\n");
  
  // Get channel 2 measurement
  ADC_sum = 0;
  analogRead(A1); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A1);
  }
  P5_dc_curr = ADC_sum/num_samples;
  if (P5_dc_curr < P5_dc_curr_bound_0) P5_dc_curr = 0;
  else if (P5_dc_curr < P5_dc_curr_bound_1) P5_dc_curr = P5_dc_curr_slope_1*P5_dc_curr + P5_dc_curr_intercept_1;
  else if (P5_dc_curr < P5_dc_curr_bound_2) P5_dc_curr = P5_dc_curr_slope_2*P5_dc_curr + P5_dc_curr_intercept_2;
  else if (P5_dc_curr < P5_dc_curr_bound_3) P5_dc_curr = P5_dc_curr_slope_3*P5_dc_curr + P5_dc_curr_intercept_3;
  else if (P5_dc_curr < P5_dc_curr_bound_4) P5_dc_curr = P5_dc_curr_slope_4*P5_dc_curr + P5_dc_curr_intercept_4;
  else P5_dc_curr = P5_dc_curr_slope_5*P5_dc_curr + P5_dc_curr_intercept_5;
  //Serial.print("P5_dc_curr = "); Serial.print(P5_dc_curr,5); Serial.print("\n");
  
  // Get channel 3 measurement
  ADC_sum = 0;
  analogRead(A2); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A2);
  }
  ADC_ss_aux = ADC_ss_aux_scale*(ADC_sum/num_samples);
  if (ADC_ss_aux < 0) ADC_ss_aux = 0;
  //Serial.print("ADC_ss_aux = "); Serial.print(ADC_ss_aux,5); Serial.print("\n");
  
  // Get channel 4 measurement
  ADC_sum = 0;
  analogRead(A3); // Throw away first sample
  for (i = 0; i < num_samples; i++) {
    ADC_sum += analogRead(A3);
  }
  //ADC_cc_pot = ADC_cc_pot_scale*(ADC_sum/num_samples);
  ADC_cc_pot = ADC_sum/num_samples;
  if (ADC_cc_pot < 0) ADC_cc_pot = 0;
  //Serial.print("ADC_cc_pot = "); Serial.print(ADC_cc_pot,5); Serial.print("\n");

  //Serial.print("================\n");
 
} // Get_Analog_Measurements


void Calculate_Power_Measurements(void) {
  // Copy voltage values for shared bus node
  P3_sys_volt = P2_bus_volt;
  P4_ac_volt = P2_bus_volt;
  P5_dc_volt = P2_bus_volt;
  P7_bat_volt = P2_bus_volt;

  // If currents are below 150mA, set measurments to 0A
  /*
  if (P1_aux_curr < 0.15)
    P1_aux_curr = 0;
  if (P2_bus_curr < 0.15)
    P2_bus_curr = 0;
  if (P3_sys_curr < 0.15)
    P3_sys_curr = 0;
  if (P4_ac_curr < 0.15)
    P4_ac_curr = 0;
  if (P5_dc_curr < 0.15)
    P5_dc_curr = 0;
  if (P6_in_curr < 0.15)
    P6_in_curr = 0;
  */
  
  // Calculate battery net current based on KCL
  P7_bat_curr = P2_bus_curr - P3_sys_curr - P4_ac_curr - P5_dc_curr;

  // Calculate power measurements from voltage and current measurements
  P1_aux_pwr = P1_aux_volt * P1_aux_curr;
  P2_bus_pwr = P2_bus_volt * P2_bus_curr;
  P3_sys_pwr = P3_sys_volt * P3_sys_curr;
  P4_ac_pwr  = P4_ac_volt  * P4_ac_curr;
  P5_dc_pwr  = P5_dc_volt  * P5_dc_curr;
  P6_in_pwr  = P6_in_volt  * P6_in_curr;
  P7_bat_pwr = P7_bat_volt * P7_bat_curr;
} // Calculate_Power_Measurements


void Update_SS_Status(void)
{
  // Check availability of input sources and update LEDs
  // ---------------------------------------------------
  if (ADC_ss_ac > ss_min_volt) {
    ss_ac_available = YES;
    bitSet(shift_outputs, SS_AC_LED_YELLOW);
  }
  else {
    ss_ac_available = NO;
    bitClear(shift_outputs, SS_AC_LED_YELLOW);
  }
  
  if (ADC_ss_bike > ss_min_volt) {
    ss_bike_available = YES;
    bitSet(shift_outputs, SS_BIKE_LED_YELLOW);
  }
  else {
    ss_bike_available = NO;
    bitClear(shift_outputs, SS_BIKE_LED_YELLOW);
  }
  
  if (ADC_ss_aux > ss_min_volt) {
    ss_aux_available = YES;
    bitSet(shift_outputs, SS_AUX_LED_YELLOW);
  }
  else {
    ss_aux_available = NO;
    bitClear(shift_outputs, SS_AUX_LED_YELLOW);
  }

  // Update active LEDS and Enables lines based on mode and availability
  // -------------------------------------------------------------------
  // OFF Mode
  if (ss_mode == SS_OFF) {
      ss_source = SS_NONE;
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
  }
  // AUTOMATIC Mode
  if (ss_mode == SS_AUTO) {
    // BIKE > AUX > AC
    if (ss_bike_available == YES) {
      ss_source = SS_BIKE;
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitSet(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitSet(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
    }
    else if (ss_aux_available == YES) {
      ss_source = SS_AUX;
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitSet(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitSet(shift_outputs, SS_AUX_ENABLE);     
    }
    else if (ss_ac_available == YES) {
      ss_source = SS_AC;
      // LEDs
      bitSet(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitSet(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);      
    }
    else {
      ss_source = SS_NONE;
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
    }
  }
  // MANUAL Mode
  if (ss_mode == SS_MANU) {
    if (ss_source == SS_NONE) {
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
    }
    else if (ss_source == SS_AC) {
      // LEDs
      bitSet(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitSet(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
    }      
    else if (ss_source == SS_BIKE) {
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitSet(shift_outputs, SS_BIKE_LED_GREEN);
      bitClear(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitSet(shift_outputs, SS_BIKE_ENABLE);
      bitClear(shift_outputs, SS_AUX_ENABLE);
    }
    else if (ss_source == SS_AUX) {
      // LEDs
      bitClear(shift_outputs, SS_AC_LED_GREEN);
      bitClear(shift_outputs, SS_BIKE_LED_GREEN);
      bitSet(shift_outputs, SS_AUX_LED_GREEN);
      // Enable lines
      bitClear(shift_outputs, SS_AC_ENABLE);
      bitClear(shift_outputs, SS_BIKE_ENABLE);
      bitSet(shift_outputs, SS_AUX_ENABLE);
    }
  }
  // Apply changes
  Update_shift_registers();
} // Update_SS_Status();

void Update_CC_Status(void) {
  float cc_curr_in_max;
  float cc_volt_out;

  // Monitor bus or auxiliary lines depending on load selector position
   if (ls_mode == LS_BAT)
    cc_volt_out = P2_bus_volt;
  else if (ls_mode == LS_AUX)
    cc_volt_out = P1_aux_volt;

  // Adjust max input current based on source selected
  if (ss_source == SS_AC)
    cc_curr_in_max = cc_curr_in_max_ac;
  else
    cc_curr_in_max = cc_curr_in_max_other;

  // OFF Mode
  if (cc_mode == CC_OFF) {
    bat_mode = BAT_NOT_CHARGING;
    cc_duty_cycle = 0;
  }
  // BAT Mode
  else if (cc_mode == CC_BAT) {
    //bat_mode = BAT_FLOAT;
    if (bat_mode == BAT_NOT_CHARGING) bat_mode = BAT_CHARGING_CC;
    if (bat_mode ==  BAT_CHARGING_CC) {
      // Change mode if max charging voltage reached
      if (cc_volt_out > cc_volt_cycle_upper) {
        bat_mode = BAT_CHARGING_CV;
        cc_duty_cycle--;
      }
      // Otherwise regulate voltage and current
      else if (P6_in_curr > cc_curr_in_max) cc_duty_cycle--;
      else if (P7_bat_curr > cc_curr_cycle_max) cc_duty_cycle--;
      else if (P7_bat_curr < cc_curr_cycle_max - 0.25) cc_duty_cycle++;
    }
    else if (bat_mode ==  BAT_CHARGING_CV) {
      // Change mode if current falls below dropout value
      if (P7_bat_curr < cc_curr_cycle_min) {
        bat_mode = BAT_FLOAT;
        cc_duty_cycle--;
      }
      // Otherwise regulate voltage and current
      else if (P6_in_curr > cc_curr_in_max) cc_duty_cycle--;
      else if (cc_volt_out > cc_volt_cycle_upper) cc_duty_cycle--;
      else if (cc_volt_out < cc_volt_cycle_lower) cc_duty_cycle++;
    }
    else if (bat_mode ==  BAT_FLOAT) {
      // Regulate voltage and current
      if (P6_in_curr > cc_curr_in_max) cc_duty_cycle--;
      else if (cc_volt_out > cc_volt_float_upper) cc_duty_cycle--;
      else if (cc_volt_out < cc_volt_float_lower) cc_duty_cycle++;
    }
  }
  // OTHER Modes
  else {
    // HIGH Voltage Regulation Mode
    if (cc_mode == CC_HIGH) {
      bat_mode = BAT_NOT_CHARGING;
      if (P2_bus_curr > cc_curr_out_max) cc_duty_cycle--;
      else if (P6_in_curr > cc_curr_in_max) cc_duty_cycle--;
      else if (cc_volt_out > (cc_volt_high+0.1)) cc_duty_cycle--;
      else if (cc_volt_out < (cc_volt_high-0.1)) cc_duty_cycle++;
    }
    // LOW Voltage Regulation Mode
    if (cc_mode == CC_LOW) {
      bat_mode = BAT_NOT_CHARGING;  
      if (P2_bus_curr > cc_curr_out_max) cc_duty_cycle--;
      else if (P6_in_curr > cc_curr_in_max) cc_duty_cycle--;
      else if (cc_volt_out > (cc_volt_low+0.1)) cc_duty_cycle--;
      else if (cc_volt_out < (cc_volt_low-0.1)) cc_duty_cycle++;
    }
    // FINE Adjustment Mode
    if (cc_mode == CC_FINE) {
      bat_mode = BAT_NOT_CHARGING;
      cc_duty_cycle = (int)ADC_cc_pot >> 2; // 10-bit to 8-bit conversion
    }
    // COURSE Adjustment Mode
    if (cc_mode == CC_CRSE) {
      bat_mode = BAT_NOT_CHARGING;
      if (cc_down_button == HIGH) {
        cc_duty_cycle -= cc_duty_cycle_crse_step;
      }
      else if (cc_up_button == HIGH) {
        cc_duty_cycle += cc_duty_cycle_crse_step;
      }
    }
    // Clamp duty cycle from 0 to max value
    if (cc_duty_cycle < 0) cc_duty_cycle = 0;
    if (cc_duty_cycle > cc_duty_cycle_max) cc_duty_cycle = cc_duty_cycle_max;

  }
  analogWrite(PWM_CC, cc_duty_cycle);
} // Update_CC_Status
