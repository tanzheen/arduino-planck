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
Adafruit_INA219 ina219(0x40);  // First INA219 at address 0x40


// set pins for the LCD panel 
const int rs = 12, en = 9, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// put your setup code here, to run once:
void setup() {

// Setup multiple INA219 sensors
  Serial.begin(9600);
  Wire.begin();
  // Initialise the 1st INA219 
    Serial.println("Failed to find INA219 chip");
  if (! ina219.begin()) {
    while (1) { delay(10); }
  }
  Serial.println("INA219 Found!");
 

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
  float turn_on_v = 0 

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}


// put your main code here, to run repeatedly:
void loop() {

  const channel = 0 //W1 channel 
  turn_on_v = measureTurnOnVoltage(ina219, channel)
  Serial.print("Turn on Voltage: ");Serial.print(turn_on_v); Serial.println(" V");
    
  delay(10);


  calc_disp_planck(turn_on_v)
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



void calc_disp_planck(float turn_on_v){
  // calculating using the slope method
  const int numPoints = 3;  // Number of data points
  float e = 1.60217663e-19;
  float c = 299792458;
  // x values: 1/lambda; NEED TO CHANGE THE WAVELENGTH
  // y values: turn on voltage
  float lambda = 660e-9;

  float h = e * turn_on_v * lambda/c;

  Serial.print("Planck's constant (h): ");Serial.println(h);
  // Display the slope on the LCD
  lcd.setCursor(0, 0);  // Set cursor to first line
  lcd.print("Planck's constant:");
  lcd.setCursor(0, 1);  // Set cursor to second line
  lcd.print(h, 10);  // Print slope with 6 decimal places
}
