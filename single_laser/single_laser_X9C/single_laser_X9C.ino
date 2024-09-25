#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include "X9C10X.h"

// setting up the digipot X9C
X9C10X pot(10000);



// Initialise the addresses of the 3 Adafruit sensors
Adafruit_INA219 ina219(0x40);  // First INA219 at address 0x40

// Set pins for the LCD panel 
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Put your setup code here, to run once:
void setup() {
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("hello, world!");

  // Setup INA219 sensors
  Serial.begin(115200);
  Wire.begin();
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  Serial.println("INA219 Found!");

  Serial.println();
  Serial.print("X9C10X_LIB_VERSION: ");
  Serial.println(X9C10X_LIB_VERSION);

  pot.begin(8, 9, 10);  // pulse, direction, select
  pot.setPosition(99); //set to 0/99 dependings on whether we are slowly increasing or decreasing

  // Initialize empty turn on voltages to keep track for the 3 lasers
  float turn_on_v = 0;
}

void loop() {
  const int channel = 0;  // W1 channel
  if (turn_on_v==0){
  float turn_on_v = measureTurnOnVolts_single(ina219, pot);
  }
  //Serial.print("Turn on Voltage: "); Serial.print(turn_on_v); Serial.println(" V");
    
  delay(10);
  //clear LCD display first
  if (turn_on_v>0){
    float h = calc_planck(turn_on_v);
    display_planck(h)
  }
  
}

void display_planck(float h){
  lcd.setCursor(0, 0);
  lcd.print("Planck's constant:");
  lcd.setCursor(0, 1);
  lcd.print(h, 10);
  lcd.clear();
  delay(100);
}

// Function for measuring turn on voltage for each of the lasers
float measureTurnOnVolts_single(Adafruit_INA219 &ina219, X9C10X &pot) {
  float turn_on_volts = 0; 
    
  // Change the resistance on this channel from max to min:
  for (int level = 0; level < 100; level++) {
    pot.setPosition(99 - level); // slowly decrease the level
    delay(10);

    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float loadvoltage = busvoltage + (shuntvoltage / 1000); // Convert mV to V

    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.println("");
    delay(10);

    if (current_mA > 10) { //setting the threshold at 10mA due to leak 
      turn_on_volts = loadvoltage;
      return turn_on_volts;
    }
  }
  return 0; //return nothing because it has not crossed the threshold yet
}

float calc_planck(float turn_on_v) {
  const float E = 1.60217663 * pow(10,-19);
  const float c = 299792458;
  const float lambda = 822 * pow(10,-9);  // Adjust wavelength here

  float h = E * turn_on_v * lambda / c;
  h = h * pow(10,34);

  Serial.print("Planck's constant (h): "); Serial.println(h);
  return h 
}