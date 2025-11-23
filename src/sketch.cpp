#include <Arduino.h>
#include <AccelStepper.h>
#include <LiquidCrystal.h>
#include <Servo.h>

// Function prototypes
int getPressedButton();
void do_initialize();
void do_state_stop();
void do_run();
void do_set();
void do_partial_cut();

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
  lcd.print("WiCu_0.1");
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
      do_partial_cut();
      action_faze = 4;
      break;
    case action_faze_4:
      //stepper2.runToNewPosition(0);
      myservo.write(servoposition);
      stepper1.runToNewPosition(cutterposition0pen);
      action_faze = 5;
      break;
    case action_faze_5:
      stepper2.move(TargetPuls);
      stepper3.move(TargetPuls);
      if (stepper2.currentPosition() >= TargetPuls) {
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
      do_partial_cut();
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

  if (showtext) {
    lcd.setCursor(0, 1);
    lcd.print(TargetLenght);
    lcd.setCursor(11, 1);
    lcd.print(CycleCountTarget);
  }
  else
  { lcd.setCursor(0, 1);
    lcd.print("                ");
  }

  switch (button) {
    case BUTTON_RIGHT:
      TargetLenght = TargetLenght - 100;
      if (TargetLenght < 200 ) {
        TargetLenght = 200;
      }
      break;
    case BUTTON_LEFT:
      TargetLenght = TargetLenght + 100;
      break;
    case BUTTON_UP:
      CycleCountTarget = CycleCountTarget + 1;
      break;
    case BUTTON_DOWN:
      CycleCountTarget = CycleCountTarget - 1;
      if (CycleCountTarget < 1) {
        CycleCountTarget = 1;
      }
      break;
    case BUTTON_SELECT:
      overallstate = state_stop;
      break;
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
