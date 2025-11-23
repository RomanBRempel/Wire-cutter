#include <Arduino.h>
#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <EEPROM.h>

// Function prototypes
int getPressedButton();
void do_initialize();
void do_state_stop();
void do_run();
void do_set();
void do_partial_cut();
void do_strip_start();
void do_strip_end();
void saveSettings();
void loadSettings();

LiquidCrystal lcd(8, 9, 4, 5, 6, 7 );
Servo myservo;

#define STEPPER1_ENA_PIN 22
#define STEPPER1_DIR_PIN 24
#define STEPPER1_STEP_PIN 26

#define STEPPER2_ENA_PIN 23
#define STEPPER2_DIR_PIN 25
#define STEPPER2_STEP_PIN 27

#define STEPPER3_ENA_PIN 28
#define STEPPER3_DIR_PIN 30
#define STEPPER3_STEP_PIN 32

AccelStepper stepper1(AccelStepper::DRIVER, STEPPER1_STEP_PIN, STEPPER1_DIR_PIN);
AccelStepper stepper2(AccelStepper::DRIVER, STEPPER2_STEP_PIN, STEPPER2_DIR_PIN);
AccelStepper stepper3(AccelStepper::DRIVER, STEPPER3_STEP_PIN, STEPPER3_DIR_PIN);

int prevButtonValue;
word currentMillis;
unsigned long previousMillis = 0;
unsigned long previousMillisTextBlink = 0;
unsigned long previousMillisAction = 0;

int overallstate;
unsigned long previousstate = 0;
const int state_stop   = 0;
const int state_run = 1;
const int state_set = 2;
const int state_initialize   = 100;

int action_faze;
const int action_faze_1 = 1;
const int action_faze_2 = 2;
const int action_faze_3 = 3;
const int action_faze_4 = 4;
const int action_faze_5 = 5;
const int action_faze_6 = 6;
const int action_faze_7 = 7;
const int action_faze_8 = 8;
const int action_faze_9 = 9;
const int action_faze_10 = 10;

int CycleCountTarget = 1;
int CycleCount = 1;
int TargetLenght = 200;
int TargetPuls;

// Strip wire settings
int stripMode = 0;  // 0=Off, 1=Start, 2=End, 3=Both
int stripLength = 10; // Length of wire strip in mm
int settingsPage = 0; // 0=Main, 1=Strip settings

boolean showtext = false;

int button;
const int BUTTON_NONE   = 0;
const int BUTTON_RIGHT  = 1;
const int BUTTON_UP     = 2;
const int BUTTON_DOWN   = 3;
const int BUTTON_LEFT   = 4;
const int BUTTON_SELECT = 5;

boolean ledstate = true;
int cutterposition;
int servoposition;
int cutterposition0pen=-350;

int getPressedButton() {
  int buttonValue;
  buttonValue = analogRead(0);
  if (prevButtonValue == buttonValue)
  {
    if (currentMillis - previousMillis >= 100) {
      previousMillis = currentMillis;
      if (buttonValue < 50) {
        return BUTTON_RIGHT;
      }
      else if (buttonValue < 100) {
        return BUTTON_UP;
      }
      else if (buttonValue < 300) {
        return BUTTON_DOWN;
      }
      else if (buttonValue < 500) {
        return BUTTON_LEFT;
      }
      else if (buttonValue < 700) {
        return BUTTON_SELECT;
      }
      return BUTTON_NONE;
    }
    else {
      return BUTTON_NONE;
    }
  }
  else  {
    if (buttonValue < 50) {
      return BUTTON_RIGHT;
    }
    else if (buttonValue < 100) {
      return BUTTON_UP;
    }
    else if (buttonValue < 300) {
      return BUTTON_DOWN;
    }
    else if (buttonValue < 500) {
      return BUTTON_LEFT;
    }
    else if (buttonValue < 700) {
      return BUTTON_SELECT;
    }
    return BUTTON_NONE;
  }
}

void setup() {
  myservo.attach(51);
  servoposition = 65;
  myservo.write(servoposition);

  stepper1.setMaxSpeed(25000.0);
  stepper1.setAcceleration(95000.0);
  //stepper1.setEnablePin(STEPPER1_ENA_PIN);

  stepper2.setMaxSpeed(5000.0);
  stepper2.setAcceleration(10000.0);

  stepper3.setMaxSpeed(5000.0);
  stepper3.setAcceleration(10000.0);

  lcd.begin(16, 2);
  lcd.print("WiCu_0.2");
  
  // Load settings from EEPROM
  loadSettings();
  
  overallstate = state_initialize;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(40, INPUT);
}

void loop() {

  cutterposition = digitalRead(40);

  currentMillis = millis();
  button = getPressedButton();

  if (currentMillis - previousMillisTextBlink >= 450) {
    previousMillisTextBlink = currentMillis;
    showtext = !showtext;
  }

  switch (overallstate) {
    case state_initialize:
      do_initialize();
      break;
    case state_stop:
      do_state_stop();
      break;
    case state_run:
      do_run();
      break;
    case state_set:
      do_set();
      break;
  }
}


void do_state_stop() {
  if (showtext) {
    lcd.setCursor(11, 0);
    if (cutterposition == HIGH) {
      lcd.print("START");
    }
    else if (cutterposition == LOW)
    {
      lcd.print("start");
    }
    else
    {
      lcd.print("Start");
    }
    lcd.setCursor(0, 1);
    lcd.print(TargetLenght);
    lcd.setCursor(11, 1);
    lcd.print(CycleCountTarget);
  }
  else
  { lcd.setCursor(11, 0);
    lcd.print("     ");
  }

  switch (button) {
    case BUTTON_RIGHT:
      stepper2.runToNewPosition(stepper2.currentPosition() - 50);
      stepper3.runToNewPosition(stepper3.currentPosition() - 50);
      break;
    case BUTTON_LEFT:
      stepper2.runToNewPosition(stepper2.currentPosition() + 50);
      stepper3.runToNewPosition(stepper3.currentPosition() + 50);
      break;
    case BUTTON_UP:
      // CycleCountTarget = CycleCountTarget + 1;
      break;
    case BUTTON_DOWN:
      lcd.setCursor(11, 0);
      lcd.print("SET");
      overallstate = state_set;
      //CycleCountTarget = CycleCountTarget - 1;
      break;
    case BUTTON_SELECT:
      stepper1.enableOutputs();
      stepper2.enableOutputs();
      stepper3.enableOutputs();
      stepper2.setCurrentPosition(0);
      stepper3.setCurrentPosition(0);
      overallstate = state_run;
      action_faze = 1;
      stepper2.run();
      stepper3.run();
      CycleCount = CycleCountTarget;
      TargetPuls = TargetLenght * (200 / (3.14 * 29));
      do_run();
      break;
  }
}

void do_run() {
  switch (action_faze) {
    case action_faze_1:
      myservo.write(servoposition);
      stepper1.runToNewPosition(cutterposition0pen);
      action_faze = 2;
      break;
    case action_faze_2:
      stepper2.runToNewPosition(50);
      action_faze = 3;
      break;
    case action_faze_3:
      // Strip wire at start if needed
      if (stripMode == 1 || stripMode == 3) {
        do_strip_start();
      } else {
        do_partial_cut();
      }
      action_faze = 4;
      break;
    case action_faze_4:
      //stepper2.runToNewPosition(0);
      myservo.write(servoposition);
      stepper1.runToNewPosition(cutterposition0pen);
      action_faze = 5;
      break;
    case action_faze_5:
      // Calculate target position considering end strip
      int feedTarget = TargetPuls;
      if (stripMode == 2 || stripMode == 3) {
        // If stripping at end, feed less (minus strip length)
        int stripPuls = stripLength * (200 / (3.14 * 29));
        feedTarget = TargetPuls - stripPuls;
      }
      
      stepper2.move(feedTarget);
      stepper3.move(feedTarget);
      if (stepper2.currentPosition() >= feedTarget) {
        action_faze = 6;
      }
      else {
        stepper2.run();
        stepper3.run();
      }
      break;
    case action_faze_6:
      myservo.write(servoposition -10);
      stepper1.setMaxSpeed(25000);
      stepper1.setAcceleration(95000);
      stepper1.runToNewPosition(0);
      if (cutterposition == LOW) {
        stepper1.moveTo(stepper1.currentPosition() + 1);
        stepper1.run();
      }
      else if (cutterposition == HIGH)
      {
        stepper1.stop();
        stepper1.setCurrentPosition(0);
        action_faze = 7;
      }
      break;
    case action_faze_7:
      myservo.write(servoposition -50);
      stepper1.runToNewPosition(cutterposition0pen);
      stepper2.setCurrentPosition(0);
      stepper2.runToNewPosition(-5);
      stepper3.setCurrentPosition(0);
      stepper3.runToNewPosition(-50);
      action_faze = 8;
      break;
    case action_faze_8:
      // Strip wire at end if needed
      if (stripMode == 2 || stripMode == 3) {
        do_strip_end();
      } else {
        do_partial_cut();
      }
      action_faze = 9;
      break;
    case action_faze_9:
      //stepper3.runToNewPosition(0);
      action_faze = 10;
      myservo.write(servoposition +30);
      myservo.write(servoposition );
      break;
    case action_faze_10:
      stepper1.runToNewPosition(cutterposition0pen);
      stepper3.runToNewPosition(450);
      stepper2.runToNewPosition(0);
      action_faze = 0;
      if (CycleCount == 1) {
        overallstate = state_stop;
        CycleCount = CycleCountTarget;
      } 
      else
      { CycleCount = CycleCount - 1;
        lcd.setCursor(8, 1);
        lcd.print("        ");
        lcd.print(CycleCount + "/" + CycleCountTarget);
        stepper2.setCurrentPosition(0);
        stepper3.setCurrentPosition(0);
        action_faze = 1;
        stepper2.run();
        stepper3.run();
      }
      break;
  }

}

void do_initialize() {
  if (showtext) {
    lcd.setCursor(11, 0);
    if (cutterposition == HIGH) {
      lcd.print("INITIALIZE");
    }
    else if (cutterposition == LOW)
    {
      lcd.print("init");
    }
    else
    {
      lcd.print("Init");
    }
  }
  else
  { lcd.setCursor(11, 0);
    lcd.print("     ");
  }

  if (cutterposition == LOW) {

    stepper1.moveTo(stepper1.currentPosition() + 100);
    stepper1.run();
  }
  else if (cutterposition == HIGH)
  {
    stepper1.stop();
    stepper1.setCurrentPosition(0);
    stepper1.runToNewPosition(cutterposition0pen);
    overallstate = state_stop;
  }

  myservo.write(servoposition );
}


void do_set() {

  if (settingsPage == 0) {
    // Main settings page
    lcd.setCursor(0, 0);
    lcd.print("Len:    Cnt:    ");
    
    if (showtext) {
      lcd.setCursor(4, 0);
      lcd.print(TargetLenght);
      lcd.setCursor(12, 0);
      lcd.print(CycleCountTarget);
      lcd.setCursor(0, 1);
      lcd.print("UP=Pg2 SEL=Exit ");
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }

    switch (button) {
      case BUTTON_RIGHT:
        TargetLenght = TargetLenght - 100;
        if (TargetLenght < 200) {
          TargetLenght = 200;
        }
        saveSettings();
        break;
      case BUTTON_LEFT:
        TargetLenght = TargetLenght + 100;
        saveSettings();
        break;
      case BUTTON_UP:
        settingsPage = 1;
        lcd.clear();
        break;
      case BUTTON_DOWN:
        CycleCountTarget = CycleCountTarget - 1;
        if (CycleCountTarget < 1) {
          CycleCountTarget = 1;
        }
        saveSettings();
        break;
      case BUTTON_SELECT:
        settingsPage = 0;
        overallstate = state_stop;
        lcd.clear();
        break;
    }
  }
  else if (settingsPage == 1) {
    // Strip settings page
    lcd.setCursor(0, 0);
    lcd.print("Strip:");
    
    // Display strip mode
    lcd.setCursor(6, 0);
    switch (stripMode) {
      case 0: lcd.print("Off  "); break;
      case 1: lcd.print("Start"); break;
      case 2: lcd.print("End  "); break;
      case 3: lcd.print("Both "); break;
    }
    
    if (showtext) {
      lcd.setCursor(0, 1);
      lcd.print("Len:");
      lcd.print(stripLength);
      lcd.print("mm UP=Pg1");
    }
    else {
      lcd.setCursor(0, 1);
      lcd.print("                ");
    }

    switch (button) {
      case BUTTON_RIGHT:
        stripLength = stripLength - 1;
        if (stripLength < 5) {
          stripLength = 5;
        }
        saveSettings();
        break;
      case BUTTON_LEFT:
        stripLength = stripLength + 1;
        if (stripLength > 50) {
          stripLength = 50;
        }
        saveSettings();
        break;
      case BUTTON_UP:
        settingsPage = 0;
        lcd.clear();
        break;
      case BUTTON_DOWN:
        stripMode = (stripMode + 1) % 4;
        saveSettings();
        break;
      case BUTTON_SELECT:
        settingsPage = 0;
        overallstate = state_stop;
        lcd.clear();
        break;
    }
  }
}

void do_partial_cut() {
  stepper1.runToNewPosition(-150);
      stepper1.setMaxSpeed(500);
      stepper1.setAcceleration(1000);
      
      stepper1.runToNewPosition(-69);
      
      stepper1.setMaxSpeed(15000);
      stepper1.setAcceleration(95000);
      
}

// EEPROM addresses
#define EEPROM_ADDR_TARGET_LENGTH 0
#define EEPROM_ADDR_CYCLE_COUNT 2
#define EEPROM_ADDR_STRIP_MODE 4
#define EEPROM_ADDR_STRIP_LENGTH 5
#define EEPROM_MAGIC 0xAB  // Magic byte to check if EEPROM is initialized
#define EEPROM_ADDR_MAGIC 7

void saveSettings() {
  EEPROM.put(EEPROM_ADDR_TARGET_LENGTH, TargetLenght);
  EEPROM.put(EEPROM_ADDR_CYCLE_COUNT, CycleCountTarget);
  EEPROM.write(EEPROM_ADDR_STRIP_MODE, stripMode);
  EEPROM.write(EEPROM_ADDR_STRIP_LENGTH, stripLength);
  EEPROM.write(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
}

void loadSettings() {
  // Check if EEPROM has been initialized
  if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC) {
    EEPROM.get(EEPROM_ADDR_TARGET_LENGTH, TargetLenght);
    EEPROM.get(EEPROM_ADDR_CYCLE_COUNT, CycleCountTarget);
    stripMode = EEPROM.read(EEPROM_ADDR_STRIP_MODE);
    stripLength = EEPROM.read(EEPROM_ADDR_STRIP_LENGTH);
    
    // Validate loaded values
    if (TargetLenght < 200) TargetLenght = 200;
    if (CycleCountTarget < 1) CycleCountTarget = 1;
    if (stripMode < 0 || stripMode > 3) stripMode = 0;
    if (stripLength < 5 || stripLength > 50) stripLength = 10;
  }
}

void do_strip_start() {
  // Calculate pulses for strip length
  int stripPuls = stripLength * (200 / (3.14 * 29));
  
  // 1. Close cutter (partial cut position)
  do_partial_cut();
  
  // 2. Move stepper2 back slightly to strip insulation
  stepper2.runToNewPosition(stepper2.currentPosition() - 5);
  
  // 3. Open cutter
  stepper1.runToNewPosition(cutterposition0pen);
  
  // 4. Return to original position
  stepper2.runToNewPosition(50);
  
  // 5. Feed wire for strip length
  stepper2.runToNewPosition(stepper2.currentPosition() + stripPuls);
  stepper3.runToNewPosition(stepper3.currentPosition() + stripPuls);
}

void do_strip_end() {
  // Calculate pulses for strip length
  int stripPuls = stripLength * (200 / (3.14 * 29));
  
  // 1. Close cutter (partial cut position)
  do_partial_cut();
  
  // 2. Small movement forward with closed cutter to strip insulation
  stepper2.runToNewPosition(stepper2.currentPosition() + 5);
  stepper3.runToNewPosition(stepper3.currentPosition() + 5);
  
  // 3. Open cutter
  stepper1.runToNewPosition(cutterposition0pen);
  
  // 4. Move back to position before stripping
  stepper2.runToNewPosition(stepper2.currentPosition() - 5);
  stepper3.runToNewPosition(stepper3.currentPosition() - 5);
  
  // 5. Feed remaining distance to target length
  stepper2.runToNewPosition(stepper2.currentPosition() + stripPuls);
  stepper3.runToNewPosition(stepper3.currentPosition() + stripPuls);
}
