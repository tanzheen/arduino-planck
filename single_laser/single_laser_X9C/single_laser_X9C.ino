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
Adafruit_INA219 ina219(0x40);  // use position 2 :  INA219 at address 0x41

// Set pins for the LCD panel 
const int rs = 8, en = 9, d4 = 10, d5 =11, d6 = 12, d7 = 13;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

std::vector<float> load_voltages;
std::vector<float> currents;
float min_curr = 5;
float max_curr = 45;
float wave = 663;
int patience = 3;

void display_planck(float h){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Planck's: ");
  lcd.setCursor(0, 1);
  lcd.print(h, 10);
  delay(100);
}

// Function for tracking the voltages and currents for each of the lasers
std::tuple<std::vector<float>, std::vector<float>> measureTurnOnVolts_single(Adafruit_INA219 &ina219, X9C10X &pot) {
  int patience_count = 0 ;
  // Vectors to store load voltages and current values
  std::vector<float> load_voltages;
  std::vector<float> currents;
  
  delay(500);
  for (int level = 0; level < 100; level++) {
    pot.incr(); // Slowly increase the level of the digi pot 
    Serial.print("Digi pot level: "); Serial.println(level);
    delay(500);

    float shuntvoltage = ina219.getShuntVoltage_mV();
    float busvoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float loadvoltage = busvoltage + (shuntvoltage / 1000); // Convert mV to V
    if (currents.empty() || current_mA > currents.back()) {
      // Store the load voltage and current in vectors
      load_voltages.push_back(busvoltage);
      currents.push_back(current_mA);
      patience_count = 0 ; 
    } else {
      patience_count ++;
    }
    

    Serial.print("Bus Voltage:   "); Serial.print(busvoltage, 5); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage, 5); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage,5); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA,5); Serial.println(" mA");
    Serial.println("");
    delay(300);
    if (patience_count >= patience){
    break;
  }
  }
  

  // Return the vectors as a tuple
  return std::make_tuple(load_voltages, currents);
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
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  ina219.setCalibration_16V_400mA(); 
  Serial.println("INA219 Found!");

  Serial.println();
  Serial.print("X9C10X_LIB_VERSION: ");
  Serial.println(X9C10X_LIB_VERSION);

  pot.begin(3, 2, 4);  // pulse (INC), direction (UD), select(CS)
  

  // Initialize empty turn on voltages to keep track for the 3 lasers
  
}

void loop() {
  const int channel = 0;  // W1 channel
  float turn_on_v = 0;
  float slope= 0 ;
  float intercept=0 ; 
  int found =0 ;
  pot.setPosition(0); //set to 0/99 dependings on whether we are slowly increasing or decreasing
  delay(1000);
  if (turn_on_v==0){
    std::vector<float> load_voltages, currents;
    std::tie(load_voltages, currents) = measureTurnOnVolts_single(ina219, pot);
    std::vector<float> filtered_voltages; 
    std::vector<float> filtered_currents;
    std::tie(filtered_voltages, filtered_currents) = linear_voltages_by_current(load_voltages,  currents,  max_curr, min_curr);
    std::tie(slope, intercept) = linear_regression( filtered_voltages,   filtered_currents);
    turn_on_v =  calc_turn_on_v(slope,  intercept);
  }
  Serial.print("Turn on Voltage: "); Serial.print(turn_on_v); Serial.println(" V");
    
  delay(10);
  //clear LCD display first
  if (turn_on_v>0 && found ==0){
    float h = calc_planck(turn_on_v, wave);
    display_planck(h);
    found =1 ;
  }
  
}
