#include <LiquidCrystal.h>
#include <NewPing.h>

#define DETECTION_DISTANCE 8.0
#define WATER_CHANGE_THRESHOLD 0.3
#define MEASURE_INTERVAL 3000
#define DEBOUNCE_DELAY 50
#define PUMP_DELAY_200ML 16000
#define PUMP_DELAY_500ML 40000
#define BUZZER_DURATION 1000
#define BUZZER_INTERVAL 1500

#define relayPin 2
#define buttonPin 7
#define buzzerPin A0
#define trigPin1 3
#define echoPin1 4
#define trigPin2 5
#define echoPin2 6

NewPing sonar1(trigPin1, echoPin1, 20);
NewPing sonar2(trigPin2, echoPin2, 20);

LiquidCrystal lcd(12, 13, 8, 9, 10, 11);

int mode = 0;
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
bool systemOn = true;
bool containerDetected = false;
float lastWaterDistance = -1;
unsigned long lastMeasureTime = 0;
bool pumpState = false;
bool systemHalted = false;
bool isInitialized = false;
bool pumping = false;
unsigned long pumpStartTime = 0;
bool buzzing = false;
unsigned long buzzerStartTime = 0;
bool overflowDetected = false;
unsigned long lastBuzzerTime = 0;
bool bluetoothState = true;

float measureDistance(NewPing &sonar) {
  unsigned int duration = sonar.ping();
  float distance = (duration / 2) * 0.0343;
  if (distance == 0 || distance > 20) return -1;
  return distance;
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print((systemOn && bluetoothState) ? "ON" : "OFF");
  lcd.setCursor(0, 1);
  if (mode == 0) lcd.print("Mode: Auto");
  else if (mode == 1) lcd.print("Mode: 200ml");
  else if (mode == 2) lcd.print("Mode: 500ml");
  else lcd.print("Mode: Off");
}

void updateSerial(float distContainer, float distWater) {
  if (!bluetoothState) {
    Serial.println("System stopped: Bluetooth OFF");
  } else if (mode == 3) {
    Serial.println("System Off");
  } else if (systemHalted || overflowDetected) {
    Serial.println("System Halted");
  } else if (containerDetected) {
    Serial.print("DistC: ");
    if (distContainer >= 0) {
      char buffer[6];
      dtostrf(distContainer, 5, 1, buffer);
      Serial.print(buffer);
    } else {
      Serial.print("Err");
    }
    Serial.print("cm, DistW: ");
    if (distWater >= 0) {
      char buffer[6];
      dtostrf(distWater, 5, 1, buffer);
      Serial.print(buffer);
    } else {
      Serial.print("Err");
    }
    Serial.println("cm");
  } else {
    Serial.println("No Container");
  }
}

void controlPump(bool state) {
  digitalWrite(relayPin, state ? HIGH : LOW);
  pumpState = state;
}

bool detectContainer() {
  float distance = measureDistance(sonar1);
  return distance > 0 && distance < DETECTION_DISTANCE;
}

void controlBuzzer(bool state) {
  digitalWrite(buzzerPin, state ? HIGH : LOW);
  buzzing = state;
  if (state) buzzerStartTime = millis();
}

void checkBluetoothCommand() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == '0') {
      if (bluetoothState) {
        bluetoothState = false;
        controlPump(false);
        controlBuzzer(false);
        containerDetected = false;
        pumping = false;
        systemHalted = false;
        overflowDetected = false;
        lastWaterDistance = -1;
        Serial.println("Bluetooth OFF: System stopped");
        updateLCD();
      }
    } else if (command == '1') {
      if (!bluetoothState) {
        bluetoothState = true;
        mode = 0;
        systemOn = true;
        Serial.println("Bluetooth ON: System started, mode set to Auto");
        updateLCD();
      }
    }
  }
}

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  digitalWrite(buzzerPin, LOW);
  lcd.begin(16, 2);
  Serial.begin(9600);
  updateLCD();
}

void loop() {
  checkBluetoothCommand();

  if (!bluetoothState) {
    controlPump(false);
    controlBuzzer(false);
    containerDetected = false;
    pumping = false;
    systemHalted = false;
    overflowDetected = false;
    lastWaterDistance = -1;
    updateSerial(-1, -1);
    updateLCD();
    return;
  }

  float distContainer = -1;
  float distWater = -1;

  if (mode != 3) {
    distContainer = measureDistance(sonar1);
    if (!isInitialized || (containerDetected && !systemHalted && !overflowDetected)) {
      distWater = measureDistance(sonar2);
    }
  }

  updateSerial(distContainer, distWater);
  updateLCD();

  if (!isInitialized) {
    if (distWater >= 0) {
      lastWaterDistance = distWater;
      isInitialized = true;
      Serial.println("System initialized with water level");
    }
    controlPump(false);
    controlBuzzer(false);
    containerDetected = false;
    return;
  }

  if (mode == 3) {
    controlPump(false);
    controlBuzzer(false);
    containerDetected = false;
    pumping = false;
    systemHalted = false;
    overflowDetected = false;
    lastWaterDistance = -1;
  }

  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
    Serial.print("Button state changed to: ");
    Serial.println(reading);
  }
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != buttonState) {
      buttonState = reading;
      if (buttonState == LOW && !pumpState) {
        mode = (mode + 1) % 4;
        systemOn = (mode != 3);
        pumping = false;
        overflowDetected = false;
        systemHalted = false;
        Serial.print("Mode changed to: ");
        if (mode == 0) Serial.println("Auto");
        else if (mode == 1) Serial.println("200ml");
        else if (mode == 2) Serial.println("500ml");
        else Serial.println("Off");
        updateLCD();
      } else if (buttonState == LOW && pumpState) {
        Serial.println("Cannot change mode: Pump is running");
      }
    }
  }
  lastButtonState = reading;

  if (systemHalted || overflowDetected) {
    bool newContainerDetected = (distContainer > 0 && distContainer < DETECTION_DISTANCE);
    if (!newContainerDetected && containerDetected) {
      systemHalted = false;
      overflowDetected = false;
      lastWaterDistance = -1;
      pumping = false;
      controlBuzzer(false);
      Serial.println("Container removed, system reset, mode unchanged");
    }
    controlPump(false);
    if (containerDetected && (millis() - lastBuzzerTime >= BUZZER_INTERVAL)) {
      controlBuzzer(true);
      lastBuzzerTime = millis();
    }
    if (buzzing && millis() - buzzerStartTime >= BUZZER_DURATION) {
      controlBuzzer(false);
    }
    return;
  }

  containerDetected = (distContainer > 0 && distContainer < DETECTION_DISTANCE);

  if (containerDetected && !systemHalted && !overflowDetected) {
    unsigned long currentTime = millis();
    if (currentTime - lastMeasureTime >= MEASURE_INTERVAL) {
      if (lastWaterDistance >= 0 && distWater >= 0) {
        float diff = abs(distWater - lastWaterDistance);
        if (diff < WATER_CHANGE_THRESHOLD) {
          systemHalted = true;
          overflowDetected = true;
          controlBuzzer(true);
          controlPump(false);
          pumping = false;
          lastBuzzerTime = millis();
          Serial.println("Water level unchanged (<3mm) for 3s, pump stopped, buzzer ON");
          updateLCD();
        }
      }
      lastWaterDistance = distWater;
      lastMeasureTime = currentTime;
    }

    if (systemOn && !pumpState && !buzzing) {
      if (mode == 0) {
        controlPump(true);
        Serial.println("Auto mode: Pump started");
      } else if (mode == 1 && !pumping) {
        controlPump(true);
        pumpStartTime = millis();
        pumping = true;
        Serial.println("Starting 200ml dispensing");
      } else if (mode == 2 && !pumping) {
        controlPump(true);
        pumpStartTime = millis();
        pumping = true;
        Serial.println("Starting 500ml dispensing");
      }
    }

    if (pumping) {
      if (mode == 1 && millis() - pumpStartTime >= PUMP_DELAY_200ML) {
        controlPump(false);
        controlBuzzer(true);
        lastBuzzerTime = millis();
        pumping = false;
        systemHalted = true;
        Serial.println("200ml dispensed, pump stopped, buzzer ON");
        updateLCD();
      } else if (mode == 2 && millis() - pumpStartTime >= PUMP_DELAY_500ML) {
        controlPump(false);
        controlBuzzer(true);
        lastBuzzerTime = millis();
        pumping = false;
        systemHalted = true;
        Serial.println("500ml dispensed, pump stopped, buzzer ON");
        updateLCD();
      }
    }
  } else if (!containerDetected && mode != 3) {
    controlPump(false);
    controlBuzzer(false);
    lastWaterDistance = -1;
    pumping = false;
    Serial.println("No container detected, pump stopped");
  }

  if ((systemHalted || overflowDetected) && containerDetected && (millis() - lastBuzzerTime >= BUZZER_INTERVAL)) {
    controlBuzzer(true);
    lastBuzzerTime = millis();
  }
  if (buzzing && millis() - buzzerStartTime >= BUZZER_DURATION) {
    controlBuzzer(false);
  }
}