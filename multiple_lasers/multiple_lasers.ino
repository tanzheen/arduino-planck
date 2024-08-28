#include <Wire.h>
#include <Adafruit_INA219.h>
#include <SPI.h>
#include <LiquidCrystal.h>
// set pin 10 as the slave select for the digital potentiometer
//  * CS - to digital pin 10  (SS pin)
//  * SDI - to digital pin 11 (MOSI pin)
//  * CLK - to digital pin 13 (SCK pin)
const int slaveSelectPin = 10;

// Initialise the addresses of the 3 adafruit sensors
Adafruit_INA219 ina219_0(0x40);  // First INA219 at address 0x40
Adafruit_INA219 ina219_1(0x41);  // Second INA219 at address 0x41
Adafruit_INA219 ina219_2(0x44);  // Third INA219 at address 0x44

// set pins for the LCD panel 
const int rs = 12, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// put your setup code here, to run once:
void setup() {

// Setup multiple INA219 sensors
  Serial.begin(9600);
  Wire.begin();
  // Initialise the 1st INA219 
    Serial.println("Failed to find INA219 loc0 chip");
  if (! ina219_0.begin()) {
    while (1) { delay(10); }
  }
  Serial.println("INA219 loc0 Found!");
  
  // Initialise the 2nd INA219
  if (! ina219_1.begin()) {
    Serial.println("Failed to find INA219 loc1 chip");
    while (1) { delay(10); }
  }
  Serial.println("INA219 loc1 Found!");

  // Initialise the 3rd INA219
  if (! ina219_2.begin()) {
    Serial.println("Failed to find INA219 loc2 chip");
    while (1) { delay(10); }
  }
  Serial.println("INA219 loc2 Found!");


// Digital potentiometer setup 
  //set the slave select pin as the output
  pinMode(slaveSelectPin, OUTPUT);
  // initialize SPI for digital potentiometer control:
  SPI.begin();
  //Configure all resistor channels of digital potentiometer to max
  for (int channel = 0; channel < 6 ; channel ++ ){
    digitalPotWrite(channel, 255)
  }

// Initilise empty turn on voltages to keep track for the 3 lasers
  float turn_on_voltage0 = 0;
  float turn_on_voltage1 = 0; 
  float turn_on_voltage2 = 0; 

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}


// put your main code here, to run repeatedly:
void loop() {

  //retrieve turn on voltage for each of the laser diodes
  for (int channel = 0; channel < 3; channel++) {
    if (channel ==0){
      turn_on_voltage0 = measureTurnOnVoltage(ina219_0, channel)
      Serial.print("Turn on Voltage 0: ");Serial.print(turn_on_voltage0); Serial.println(" V");
    } else if (channel ==1 ){
      turn_on_voltage1 = measureTurnOnVoltage(ina219_1, channel)
      Serial.print("Turn on Voltage 1: ");Serial.print(turn_on_voltage1); Serial.println(" V");
    } else if (channel ==2 ){
      turn_on_voltage2 = measureTurnOnVoltage(ina219_2, channel)
      Serial.print("Turn on Voltage 2: ");Serial.print(turn_on_voltage2); Serial.println(" V");
    }
  delay(10);

  //calculate planck's constant and display on LCD
  calc_disp_planck_slope(turn_on_voltage0, turn_on_voltage1, turn_on_voltage2)
  //calc_disp_planck_avg(turn_on_voltage0, turn_on_voltage1, turn_on_voltage2)
}

// This function is to set the resistance of the digital potentiometer 
// Set specific channel with a specific level of resistance
void digitalPotWrite(int address, int value) {

  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin, LOW);

  delay(100);

  //  send in the address and value via SPI:

  SPI.transfer(address); //Set channel
  SPI.transfer(value); //Set resistance

  delay(100);

  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin, HIGH);
}

// Function for measuring turn on voltage for each of the lasers
void measureTurnOnVolts(Adafruit_INA219 &ina219, int channel) {
  
   // go through the six channels of the digital pot:
   // number of channels might be less than 6; depends on number of lasers we use
  float turn_on_volts = 0; 
    
  // change the resistance on this channel from max to min:
  for (int level = 0; level < 255; level++) {
    digitalPotWrite(channel, 255 - level);
    delay(10);
    //initialise the following variables; preparing for V and I measurement
    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
    float power_mW = 0;

    //Retrieve voltage and current values
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000); //Convert mV to V

    //printing the values
    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
    Serial.println("");
    delay (10);

    if (current_mA > 0 ){
      turn_on_volts  = loadvoltage;
      return turn_on_volts;
    }

  }
}

void calc_disp_planck_slope(float turn_on_v0, float turn_on_v1, float turn_on_v2){
  // calculating using the slope method
  const int numPoints = 3;  // Number of data points
  float e = 1.60217663e-19;
  float c = 299792458;
  // x values: 1/lambda; NEED TO CHANGE THE WAVELENGTH
  // y values: turn on voltage
  float l0_lambda = 660e-9;
  float l0_rep_lambda = 1.0/l0_lambda; 
  float l1_lambda = 660e-9;
  float l1_rep_lambda = 1.0/l0_lambda; 
  float l2_lambda = 660e-9;
  float l2_rep_lambda = 1.0/l0_lambda; 
  float xValues[numPoints] = {l0_rep_lamda , l1_rep_lambda , l2_rep_lambda}; 
  float yValues[numPoints] = {turn_on_v0 , turn_on_v1, turn_on_v2};

  // Calculate sums needed for the best-fit line
  for (int i = 0; i < numPoints; i++) {
    sumX += xValues[i];
    sumY += yValues[i];
    sumXY += xValues[i] * yValues[i];
    sumX2 += xValues[i] * xValues[i];
  }

  // Calculate slope (m) 
  float m = (numPoints * sumXY - sumX * sumY) / (numPoints * sumX2 - sumX * sumX);
  // Display the results
  
  float h = m * e/c
  Serial.print("Planck's constant (h): ");Serial.println(h);
  // Display the slope on the LCD
  lcd.setCursor(0, 0);  // Set cursor to first line
  lcd.print("Planck's constant:");
  lcd.setCursor(0, 1);  // Set cursor to second line
  lcd.print(h, 10);  // Print slope with 6 decimal places
}


void calc_disp_planck_avg(float turn_on_v0, float turn_on_v1, float turn_on_v2){
  // calculating using the slope method
  const int numPoints = 3;  // Number of data points
  float e = 1.60217663e-19;
  float c = 299792458;
  // x values: 1/lambda; NEED TO CHANGE THE WAVELENGTH
  // y values: turn on voltage
  float l0_lambda = 660e-9;
  float l1_lambda = 660e-9; 
  float l2_lambda = 660e-9;
  float h0 = e * turn_on_v0* l0_lambda/c;
  float h1 = e * turn_on_v1_ l1_lambda/c;
  float h2 = e * turn_on_v2* l2_lambda/c;

  float avg_h = (h0 + h1 + h2)/numPoints; 
  Serial.print("Planck's constant (h): ");Serial.println(avg_h);
  // Display the slope on the LCD
  lcd.setCursor(0, 0);  // Set cursor to first line
  lcd.print("Planck's constant:");
  lcd.setCursor(0, 1);  // Set cursor to second line
  lcd.print(avg_h, 10);  // Print slope with 6 decimal places
}
