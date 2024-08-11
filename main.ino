#include <Wire.h>
#include <LiquidCrystal.h>
#include "MAX30100_PulseOximeter.h"

// LCD pin configuration
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// LED current configuration
#define IR_LED_CURRENT MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT MAX30100_LED_CURR_27_1MA

// Sensor instance
MAX30100 sensor;

// PulseOximeter instance
PulseOximeter pox;

// Variables for storing sensor data
float eqnValues[20];
int eqnIndex = 0;

// Callback function for beat detection
void onBeatDetected() {
    Serial.println("Beat!");
}

// Initialization function
void setup() {
    Serial.begin(115200);
    Serial.print("Initializing pulse oximeter..");

    if (!pox.begin()) {
        Serial.println("FAILED");
        for(;;);
    } else {
        Serial.println("SUCCESS");
    }

    sensor.setLedsCurrent(IR_LED_CURRENT, RED_LED_CURRENT);
    lcd.begin(16,2);
    lcd.print("Glu  spO2  Hrate");
}

// Function to calculate glucose level
float calculateGlucose(float irVoltage) {
    float irvolt = -(irVoltage-1)/0.7 + 3.3;
    float gluc = 12.4526*(irvolt)*(irvolt)+ 25.6629*(irvolt) + 52.1554;
    return gluc;
}

// Function to calculate average glucose level
void calculateAverageGlucose() {
    if(eqnIndex == 20) {
        float sum = 0;
        for(int i = 0; i < 20; i++) {
            sum += eqnValues[i];
        }
        float average = sum / 20;
        Serial.print("Average: ");
        Serial.print(average);
        Serial.println(" mg/L");

        eqnIndex = 0; // Reset the index for the next 20 values
    }
}

// Main loop function
void loop() {
    sensor.update();
    pox.update();

    uint16_t ir, red;
    float irVoltage, redVoltage;

    while (sensor.getRawValues(&ir, &red)) {
        irVoltage = (float)ir / 65536 * 3.3;
        redVoltage = (float)red / 65536 * 3.3;
        float gluc = calculateGlucose(irVoltage);
        if(pox.getHeartRate()==0){
           lcd.setCursor(0, 1);
                lcd.print( " Please wait... ");
        }
        if(!(pox.getSpO2()==0 || gluc>160 ) ){
            if( pox.getHeartRate()>=75 && pox.getHeartRate()<=95  && pox.getSpO2()>93){
                lcd.setCursor(0, 1);
                lcd.print(String(irVoltage) + "  " + String(pox.getSpO2()) + "    "+ String(int(pox.getHeartRate())) +"     ");
            }
        }

        if(gluc<1000){
            eqnValues[eqnIndex] = gluc;
            eqnIndex++;
        }

        calculateAverageGlucose();
    }
}
