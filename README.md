# Wire Cutter Project

This project controls an automated wire cutting machine using an Arduino Mega. It features a user interface via an LCD and buttons to set wire length and quantity, and uses stepper motors for feeding and a servo for the cutting mechanism.

## Features

*   **Automated Cutting**: Automatically feeds and cuts wire to specified lengths.
*   **User Interface**: 16x2 LCD display with button controls (Right, Up, Down, Left, Select).
*   **Configurable Settings**:
    *   **Target Length**: Adjustable wire length (in steps/units).
    *   **Cycle Count**: Adjustable number of wires to cut.
*   **Motor Control**:
    *   3 Stepper Motors (Feed, Transport, Aux).
    *   1 Servo Motor (Cutter actuation).
*   **Safety/Status**: LED indicators and sensor feedback (Cutter position).

## Hardware Requirements

*   **Microcontroller**: Arduino Mega (or compatible).
*   **Display**: 16x2 LCD (LiquidCrystal compatible).
*   **Motors**:
    *   3x Stepper Motors with Drivers (e.g., A4988, DRV8825).
    *   1x Servo Motor.
*   **Input**: Analog Keypad (5 buttons on Analog Pin 0).
*   **Sensors**: Digital input for cutter position (Pin 40).

## Pin Configuration

| Component | Pin(s) | Notes |
| :--- | :--- | :--- |
| **LCD** | 8, 9, 4, 5, 6, 7 | RS, EN, D4, D5, D6, D7 |
| **Servo** | 51 | Cutter mechanism |
| **Stepper 1** | Step: 26, Dir: 24 | Enable: 22 (Defined but commented out) |
| **Stepper 2** | Step: 27, Dir: 25 | Enable: 23 |
| **Stepper 3** | Step: 32, Dir: 30 | Enable: 28 |
| **Buttons** | A0 | Analog ladder (Right, Up, Down, Left, Select) |
| **Cutter Sensor**| 40 | Digital Input |
| **Built-in LED** | 13 | Status |

## Dependencies

This project requires the following Arduino libraries:
*   `AccelStepper`
*   `LiquidCrystal`
*   `Servo`

## Usage

1.  **Power On**: The system initializes and checks the cutter position.
2.  **Menu**:
    *   **STOP State**: Shows "START" and current settings.
    *   **SET State**: Allows adjusting Length (Left/Right) and Count (Up/Down).
3.  **Operation**:
    *   Press **SELECT** to start the cutting cycle.
    *   The machine will feed the wire and cut it the specified number of times.
    *   Pressing buttons during operation may pause or reset depending on the state (see code for details).

## Setup

1.  Open `src/sketch.ino` in VS Code with the Arduino extension or Arduino IDE.
2.  Install the required libraries (`AccelStepper`, `LiquidCrystal`, `Servo`).
3.  Select board `Arduino Mega or Mega 2560`.
4.  Upload the sketch.
