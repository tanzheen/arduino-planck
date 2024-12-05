#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <LiquidCrystal.h>
#include "X9C10X.h"
#include <tuple>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <cmath>

// setting up the digipot X9C
X9C10X pot(10000);

// Initialise the addresses of the 3 Adafruit sensors
Adafruit_INA219 ina219_1(0x40);  // First INA219 at address 0x40
Adafruit_INA219 ina219_2(0x41);

// Set pins for the LCD panel 
const int rs = 12, en = 11, d4 = 10, d5 = 8, d6 = 9, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
float turn_on_v1 = 0;
float turn_on_v2 = 0; 
float mean_turn_on = 0 ; 
std::vector<float> load_voltages;
std::vector<float> currents;
float min_curr = 11;
float max_curr = 45;
float wave = 650;


void display_planck(float h){
  lcd.setCursor(0, 0);
  lcd.print("Planck's constant:");
  lcd.setCursor(0, 1);
  lcd.print(h, 10);
  lcd.clear();
  delay(100);
}


// Function for tracking the voltages and currents for each of the lasers
std::tuple<std::vector<float>, std::vector<float>, std::vector<float>, std::vector<float>> measureTurnOnVolts_multi(Adafruit_INA219 &ina219_1, Adafruit_INA219 &ina219_2, X9C10X &pot) {
    
  // Vectors to store load voltages and current values
  std::vector<float> load_voltages1;
  std::vector<float> currents1;
  std::vector<float> load_voltages2;
  std::vector<float> currents2;

  
  delay(500);
  for (int level = 0; level < 100; level++) {
    pot.incr(); // Slowly increase the level of the digi pot 
    Serial.print("Digi pot level: "); Serial.println(level);
    delay(500);

    // Measure for the first sensor 
    float shuntvoltage1 = ina219_1.getShuntVoltage_mV();
    float busvoltage1 = ina219_1.getBusVoltage_V();
    float current_mA1 = ina219_1.getCurrent_mA();
    float loadvoltage1 = busvoltage1 + (shuntvoltage1 / 1000); // Convert mV to V

    // Store the load voltage and current in vectors
    load_voltages1.push_back(loadvoltage1);
    currents1.push_back(current_mA1);

    delay(100);

    // Measure for the 2nd sensor
    float shuntvoltage2 = ina219_2.getShuntVoltage_mV();
    float busvoltage2 = ina219_2.getBusVoltage_V();
    float current_mA2 = ina219_2.getCurrent_mA();
    float loadvoltage2 = busvoltage2 + (shuntvoltage2 / 1000); // Convert mV to V

    // Store the load voltage and current in vectors
    load_voltages2.push_back(loadvoltage2);
    currents2.push_back(current_mA2);
    delay(100);
    Serial.print("Bus Voltage 1:   "); Serial.print(busvoltage1, 5); Serial.print(" V,   ");     
    Serial.print("Bus Voltage 2:   "); Serial.print(busvoltage2, 5); Serial.println(" V ");

    Serial.print("Shunt Voltage 1: "); Serial.print(shuntvoltage1, 5); Serial.print(" mV,  ");
    Serial.print("Shunt Voltage 2: "); Serial.print(shuntvoltage2, 5); Serial.println(" mV");

    Serial.print("Load Voltage 1:  "); Serial.print(loadvoltage1,5); Serial.print(" V,  ");
    Serial.print("Load Voltage 2:  "); Serial.print(loadvoltage2,5); Serial.println(" V");
     
    Serial.print("Current 1:       "); Serial.print(current_mA1,5); Serial.print(" mA,  ");
    Serial.print("Current 2:       "); Serial.print(current_mA2,5); Serial.println(" mA");
    Serial.println("");

   

    
  }

  // Return the vectors as a tuple
  return std::make_tuple(load_voltages1, currents1, load_voltages2, currents2);
}

float calc_planck(float turn_on_v, float w ) {
  const float E = 1.60217663 * pow(10,-19);
  const float c = 299792458;
  const float lambda = w * pow(10,-9);  // Adjust wavelength here

  float h = E * turn_on_v * lambda / c;
  h = h * pow(10,34);

  Serial.print("Planck's constant (h): "); Serial.println(h,10);
  return h ;
}

//this function is to filter the load voltages and currents for the linear region 
std::tuple<std::vector<float>, std::vector<float>> linear_voltages_by_current( std::vector<float>& load_voltages,  std::vector<float>& currents, float max_curr, float min_curr) {
    std::vector<float> filtered_load_voltages;
    std::vector<float> filtered_currents;
    
    for (size_t i = 0; i < currents.size(); i++) {
        if (currents[i] >= min_curr && currents[i] <= max_curr) {
            // Keep the corresponding load voltage if the current is between 11 and 45
            filtered_load_voltages.push_back(load_voltages[i]);
            filtered_currents.push_back(currents[i]);
        }
    }

    return std::make_tuple(filtered_load_voltages, filtered_currents);
}


// Function to compute the linear regression for the filtered data
std::pair<float, float> linear_regression(const std::vector<float>& voltages, const std::vector<float>& currents) {
    // Ensure there are enough points to calculate regression
    if (voltages.size() != currents.size() || voltages.size() < 2) {
        Serial.println("Error: Insufficient data points for linear regression or imcompatible sizes of array");
        return std::make_pair(0.0f, 0.0f);  // Return default values if insufficient data
    }

    // Variables to store the sums
    float sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0;
    size_t n = voltages.size();

    // Compute the sums needed for the least squares method
    for (size_t i = 0; i < n; i++) {
        sum_x += voltages[i];
        sum_y += currents[i];
        sum_xy += currents[i] * voltages[i];
        sum_x2 += voltages[i] * voltages[i];
    }

    // Calculate the slope (m) and intercept (b) of the linear equation y = mx + b
    float slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x2 - sum_x * sum_x);
    float intercept = (sum_y - slope * sum_x) / n;

    // Return the slope and intercept as a pair
    return std::make_pair(slope, intercept);
}


// Function to calculate the x-intercept given slope and intercept, giving the turn-on/lasing voltage 
float calc_turn_on_v(float slope, float intercept) {
    if (slope < 1e-6) { // To avoid division by zero
        Serial.println("Error! Slope is Horizontal or negative");
        return 0 ;
    }
    // Calculate x-intercept: x = -intercept / slope
    return -intercept / slope;
}

// Put your setup code here, to run once:
void setup() {
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.print("hello, world!");

  // Setup INA219 sensors
  Serial.begin(9600);
  Wire.begin();
  // Serial.println("Scanning for I2C devices...");
  // for (byte address = 1; address < 127; address++) {
  //   Wire.beginTransmission(address);
  //   if (Wire.endTransmission() == 0) {
  //     Serial.print("Found I2C device at address 0x");
  //     Serial.println(address, HEX);
  //   }
  // }

  if (!ina219_1.begin()) {
    Serial.println("Failed to find INA219_1 chip");
    while (1) { delay(10); }
  }
  ina219_1.setCalibration_16V_400mA(); 
  Serial.println("INA219_1 Found!");
  if (!ina219_2.begin()) {
    Serial.println("Failed to find INA219_2 chip");
    while (1) { delay(10); }
  }
  ina219_2.setCalibration_16V_400mA(); 
  Serial.println("INA219_2 Found!");

  Serial.println();
  Serial.print("X9C10X_LIB_VERSION: ");
  Serial.println(X9C10X_LIB_VERSION);

  pot.begin(4, 3, 5);  // pulse (INC), direction (UD), select(CS)
  pot.setPosition(0); //set to 0/99 dependings on whether we are slowly increasing or decreasing
  delay(1000);

  // Initialize empty turn on voltages to keep track for the 3 lasers
  
}

void loop() {
  const int channel = 0;  // W1 channel
  float slope1= 0 ;
  float intercept1=0 ; 
  float slope2= 0 ;
  float intercept2=0 ; 
  int found =0 ;
  if (mean_turn_on==0){
    std::vector<float> load_voltages1, currents1, load_voltages2, currents2;
    Serial.println("test");
    std::tie(load_voltages1, currents1, load_voltages2, currents2) = measureTurnOnVolts_multi(ina219_1, ina219_2, pot);
    std::vector<float> filtered_voltages1; 
    std::vector<float> filtered_currents1;
    std::vector<float> filtered_voltages2; 
    std::vector<float> filtered_currents2;
    std::tie(filtered_voltages1, filtered_currents1) = linear_voltages_by_current(load_voltages1,  currents1,  max_curr, min_curr);
    std::tie(slope1, intercept1) = linear_regression(filtered_voltages1, filtered_currents1);
    turn_on_v1 =  calc_turn_on_v(slope1,  intercept1);

    std::tie(filtered_voltages2, filtered_currents2) = linear_voltages_by_current(load_voltages2,  currents2,  max_curr, min_curr);
    std::tie(slope2, intercept2) = linear_regression(filtered_voltages2, filtered_currents2);
    turn_on_v2 =  calc_turn_on_v(slope2,  intercept2);
    mean_turn_on = (turn_on_v1 + turn_on_v2)/2 ;
  }
  Serial.print("Turn on Voltage 1: "); Serial.print(turn_on_v1); Serial.println(" V");
  Serial.print("Turn on Voltage 2: "); Serial.print(turn_on_v2); Serial.println(" V");
  Serial.print("Mean Turn on Voltage: "); Serial.print(mean_turn_on); Serial.println(" V");
    
  delay(10);
  //clear LCD display first
  if (mean_turn_on>0 && found ==0){
    float h1 = calc_planck(turn_on_v1, wave);
    float h2 = calc_planck(turn_on_v2, wave);
    float mean_h = calc_planck(mean_turn_on, wave);
    display_planck(mean_h);
    found =1 ;
  }
}
