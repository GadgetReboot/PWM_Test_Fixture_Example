// XY-PWM1 Module Serial UART Control Interface Example
// Used in a functional test system for a DC motor with PWM speed control
// Target hardware:  NodeMCU ESP8266
// Current monitored by INA219 Current Sensor
// Status info displayed on 128x64 I2C OLED
// Motor is exercised with a certain speed, then if the current is
// within an expected range, the test passes, otherwise fails.
//
// Gadget Reboot
//
// Required Libraries
// https://github.com/adafruit/Adafruit_INA219
// https://github.com/adafruit/Adafruit_SSD1306

#include <Wire.h>
#include <Adafruit_INA219.h>      // INA219 current monitor interface
Adafruit_INA219 ina219;

#include <Adafruit_SSD1306.h>     // 128x64 OLED
#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

#define button  D5                 // test-start button input
#define passLED D6                 // LED for test pass
#define failLED D7                 // LED for test fail
#define lowMotorSpeedCurrent  60   // motor low  speed current target = 60mA
#define highMotorSpeedCurrent 100  // motor high speed current target = 100mA
#define lowMotorSpeedDuty  "D040"  // motor low  speed is 40% duty cycle
#define highMotorSpeedDuty "D065"  // motor high speed is 65% duty cycle
#define pwmFreq "F800"             // motor PWM frequency is 800Hz
int motorCurrent = 0;              // store measured motor current

void setup() {

  // commands will be sent to PWM generator over UART
  // uses Serial1 Tx pin on NodeMCU
  Serial1.begin(9600);

  // configure button/LEDs
  pinMode(button, INPUT_PULLUP);
  digitalWrite(passLED, LOW);
  digitalWrite(failLED, LOW);
  pinMode(passLED, OUTPUT);
  pinMode(failLED, OUTPUT);

  // initialize ina219 with default measurement range of max 2A
  ina219.begin();

  // initialize OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.display();

  // configure PWM generator
  Serial1.print("OFF");               // turn off PWM output
  delay(200);
  Serial1.print("MODE0");             // set to normal mode (coarse control)
  delay(200);
  Serial1.print(pwmFreq);             // set PWM frequency
  delay(200);
  Serial1.print(lowMotorSpeedDuty);   // set duty cycle for low motor speed
  delay(200);

}

void loop() {
  // each time the loop starts, the system is
  // checking for a button press to start a test
  // most of the loop is skipped until there is a button press

  // initialize pass/fail LEDs and test status variable
  boolean testResult = true;
  digitalWrite(passLED, LOW);
  digitalWrite(failLED, LOW);

  // prompt operator to begin test
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Press Button\nTo Start Test");
  display.display();

  // begin functional test if button is pressed
  // otherwise skip to end of loop 
  if (digitalRead(button) == LOW) {

    // run motor at low speed
    Serial1.print(lowMotorSpeedDuty);   // set duty cycle to low motor speed
    delay(200);
    Serial1.print("ON");                // turn on PWM output
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Low Speed Test");
    display.print("Target: >");
    display.print(lowMotorSpeedCurrent);
    display.println(" mA");
    display.print("Result:  ");
    display.display();
    delay(2000);                        // let motor run a few seconds before measuring

    // read motor current from ina219
    motorCurrent = 0;
    for (int i = 0;  i < 20; i++) {
      motorCurrent = motorCurrent + ina219.getCurrent_mA();
    }
    motorCurrent = motorCurrent / 20;
    display.print(motorCurrent);
    display.println(" mA");
    display.display();

    if (motorCurrent < lowMotorSpeedCurrent) {
      testResult = false;
      // update display
      display.print("Test FAIL!");
      display.display();
      digitalWrite(failLED, HIGH);
    }

    if (testResult == true) {

      delay(2000);  // continue showing previous test pass result on-screen before continuing

      // run motor at high speed
      Serial1.print(highMotorSpeedDuty);   // set duty cycle to high motor speed
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("High Speed Test");
      display.print("Target: >");
      display.print(highMotorSpeedCurrent);
      display.println(" mA");
      display.print("Result:  ");
      display.display();
      delay(2000);                         // let motor run a few seconds before measuring

      // read motor current from ina219
      motorCurrent = 0;
      for (int i = 0; i < 20; i++) {
        motorCurrent = motorCurrent + ina219.getCurrent_mA();
      }
      motorCurrent = motorCurrent / 20;
      display.print(motorCurrent);
      display.println(" mA");
      display.display();

      if (motorCurrent < highMotorSpeedCurrent) {
        testResult = false;
        // update display
        display.print("Test FAIL!");
        display.display();
        digitalWrite(failLED, HIGH);
      }
      else {
        // update display
        display.print("Test PASS!");
        display.display();
        digitalWrite(passLED, HIGH);
      }
    }
    // test complete, turn off motor
    Serial1.print("OFF");
    delay(2000);  // allow time for operator to view pass/fail message
    
  } // end button press check
}  // end main loop
