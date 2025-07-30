# IoT102 Project - Automatic Water Dispensing System

## Project Description
The IoT102 project is an automatic water dispensing system using Arduino, integrated with ultrasonic sensors, an LCD display, and a Bluetooth module for control and monitoring. The system supports three operating modes:
- **Auto**: Automatically dispenses water when a cup is detected and stops when the cup is full.
- **200ml**: Dispenses exactly 200ml of water.
- **500ml**: Dispenses exactly 500ml of water.
- **Off**: Turns off the system.

The system uses ultrasonic sensors to detect the cup and water level, ensuring safety by stopping the pump when overflow is detected or no cup is present. A buzzer is used to alert when dispensing is complete or an error occurs.

## Hardware Configuration
- **Arduino Board**: Arduino Uno or compatible.
- **Ultrasonic Sensors**: 2 HC-SR04 modules (for measuring cup distance and water level).
- **LCD Display**: 16x2 LCD with I2C interface or direct connection (using pins 12, 13, 8, 9, 10, 11).
- **Relay**: Controls the water pump (connected to pin 2).
- **Push Button**: Switches modes (connected to pin 7, using INPUT_PULLUP).
- **Buzzer**: Provides audible alerts (connected to pin A0).
- **Bluetooth Module**: Communicates via Serial (9600 baud) for system control.

## Required Libraries
- **LiquidCrystal**: Controls the LCD display.
- **NewPing**: Handles signals from ultrasonic sensors.

Install libraries via Arduino IDE:
```bash
Sketch > Include Library > Manage Libraries > Search and install "LiquidCrystal" and "NewPing".
```

## Installation
1. **Hardware Connection**:
   - Connect ultrasonic sensor 1 (cup detection): Trig (pin 3), Echo (pin 4).
   - Connect ultrasonic sensor 2 (water level): Trig (pin 5), Echo (pin 6).
   - Connect LCD display: RS (12), EN (13), D4-D7 (8, 9, 10, 11).
   - Connect relay to pin 2, push button to pin 7, buzzer to pin A0.
   - Connect Bluetooth module to Arduino's TX/RX pins.

2. **Download Source Code**:
   - Obtain the source code from the Google Docs document: [Link to document](https://docs.google.com/document/d/1yuAZ-1QF3fxi8i7Ysq0dc15ScVh57hwB1XN5SEfKuJ4/edit?tab=t.0).
   - Open the `.ino` file in Arduino IDE.

3. **Upload Program**:
   - Connect Arduino to the computer via USB cable.
   - In Arduino IDE, select the correct board and port, then click Upload.

## Usage
1. **System Startup**:
   - Upon powering on, the system starts in Auto mode and displays the status on the LCD.
   - Connect via Bluetooth (baud rate 9600) for control.

2. **Mode Switching**:
   - Press the push button to cycle through modes: Auto → 200ml → 500ml → Off.
   - The LCD displays the current mode and system status (ON/OFF).

3. **Bluetooth Control**:
   - Send command `1` via Serial to turn on the system.
   - Send command `0` to turn off the system.

4. **Operation**:
   - Place a cup under the spout; the system detects it and dispenses water based on the selected mode.
   - The buzzer sounds when dispensing is complete (200ml/500ml) or an error occurs (overflow, no cup).
   - The system stops the pump if the water level remains unchanged for 3 seconds (threshold 0.3cm).

## Configuration Parameters
- **DETECTION_DISTANCE**: 8.0 cm (cup detection distance).
- **WATER_CHANGE_THRESHOLD**: 0.3 cm (water level change threshold for overflow detection).
- **MEASURE_INTERVAL**: 3000 ms (measurement interval).
- **PUMP_DELAY_200ML**: 16000 ms (pump duration for 200ml).
- **PUMP_DELAY_500ML**: 40000 ms (pump duration for 500ml).
- **BUZZER_DURATION**: 1000 ms (buzzer sound duration).
- **BUZZER_INTERVAL**: 1500 ms (interval between buzzer sounds).

## Notes
- Ensure ultrasonic sensors are correctly positioned for accurate measurements.
- Verify Bluetooth connection before using remote control commands.
- If the system reports an error or stops, check the cup/water level distance and ensure the cup is properly placed.

## References
- Detailed documentation: [Google Docs](https://docs.google.com/document/d/1yuAZ-1QF3fxi8i7Ysq0dc15ScVh57hwB1XN5SEfKuJ4/edit?tab=t.0).
- NewPing Library: [NewPing Documentation](https://bitbucket.org/teckel12/arduino-new-ping/wiki/Home).
- LiquidCrystal Library: [Arduino LiquidCrystal](https://www.arduino.cc/en/Reference/LiquidCrystal).

## Contribution
If you wish to contribute or improve the source code, please submit a pull request or contact via email: [your-email@example.com].

## License
The project is released under the MIT License. See details in the `LICENSE` file (if available).