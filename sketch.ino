#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pins
#define SENSOR1_PIN 33
#define SENSOR2_PIN 35
#define SENSOR3_PIN 34
#define SENSOR4_PIN 32
#define RELAY_PIN 25
#define BUZZER_PIN 26

// Thresholds
#define SENSOR_DISCONNECT_LOW 0.02
#define SENSOR_DISCONNECT_HIGH 3.29
#define INVALID_LOW 0.05
#define INVALID_HIGH 3.25

// Frozen ADC tuning
#define FROZEN_DELTA 0.003
#define FROZEN_TIME 120000  // 15 sec before frozen fault

// Timing
#define SENSOR_INTERVAL 200
#define LCD_INTERVAL 500
#define SERIAL_INTERVAL 1000
#define BUZZER_FAST 300
#define BUZZER_SLOW 1000
#define RECOVERY_TIME 4000
#define RELAY_VERIFY_TIME 500

enum RuntimeMode {
  MODE_NORMAL,
  MODE_DEGRADED,
  MODE_FAILSAFE,
  MODE_SHUTDOWN
};

RuntimeMode currentMode = MODE_NORMAL;

float sensorVoltage[4];
float previousVoltage[4];

bool sensorHealthy[4] = {true, true, true, true};
bool sensorDisconnected[4] = {false, false, false, false};
bool invalidReading[4] = {false, false, false, false};
bool frozenADC[4] = {false, false, false, false};

unsigned long stableStartTime[4] = {0, 0, 0, 0};

bool relayCommand = true;
bool relayFeedback = true;
bool relayMismatch = false;
bool buzzerState = false;

unsigned long lastSensorTime = 0;
unsigned long lastLCDTime = 0;
unsigned long lastSerialTime = 0;
unsigned long lastBuzzerTime = 0;
unsigned long recoveryStartTime = 0;
unsigned long relayCommandTime = 0;

String lastLine0 = "";
String lastLine1 = "";

struct FaultLog {
  unsigned long timestamp;
  String faultName;
  int module;
};

#define MAX_LOGS 10
FaultLog faultLogs[MAX_LOGS];
int logIndex = 0;
int totalLogs = 0;

float readVoltage(int pin) {
  int adc = analogRead(pin);
  return (adc * 3.3) / 4095.0;
}

String modeToString(RuntimeMode mode) {
  if (mode == MODE_NORMAL) return "NORMAL";
  if (mode == MODE_DEGRADED) return "DEGRADED";
  if (mode == MODE_FAILSAFE) return "FAILSAFE";
  if (mode == MODE_SHUTDOWN) return "SHUTDOWN";
  return "UNKNOWN";
}

void printLCDLine(int row, String text) {
  if (text.length() > 16) text = text.substring(0, 16);
  while (text.length() < 16) text += " ";

  if (row == 0 && text != lastLine0) {
    lcd.setCursor(0, 0);
    lcd.print(text);
    lastLine0 = text;
  }

  if (row == 1 && text != lastLine1) {
    lcd.setCursor(0, 1);
    lcd.print(text);
    lastLine1 = text;
  }
}

void addFaultLog(String name, int module) {
  faultLogs[logIndex].timestamp = millis();
  faultLogs[logIndex].faultName = name;
  faultLogs[logIndex].module = module;

  logIndex++;
  if (logIndex >= MAX_LOGS) logIndex = 0;
  if (totalLogs < MAX_LOGS) totalLogs++;
}

void commandRelay(bool state) {
  if (relayCommand != state) {
    relayCommand = state;
    relayCommandTime = millis();
    digitalWrite(RELAY_PIN, state ? HIGH : LOW);
  }
}

void readRelayFeedback() {
  relayFeedback = digitalRead(RELAY_PIN);
}

void readSensorsTask() {
  sensorVoltage[0] = readVoltage(SENSOR1_PIN);
  sensorVoltage[1] = readVoltage(SENSOR2_PIN);
  sensorVoltage[2] = readVoltage(SENSOR3_PIN);
  sensorVoltage[3] = readVoltage(SENSOR4_PIN);
}

void detectSensorFaultsTask() {
  for (int i = 0; i < 4; i++) {
    bool oldHealthy = sensorHealthy[i];

    sensorDisconnected[i] = false;
    invalidReading[i] = false;
    frozenADC[i] = false;

    if (sensorVoltage[i] <= SENSOR_DISCONNECT_LOW ||
        sensorVoltage[i] >= SENSOR_DISCONNECT_HIGH) {
      sensorDisconnected[i] = true;
    }

    if (sensorVoltage[i] < INVALID_LOW || sensorVoltage[i] > INVALID_HIGH) {
      invalidReading[i] = true;
    }

    float diff = abs(sensorVoltage[i] - previousVoltage[i]);

    if (!sensorDisconnected[i] && !invalidReading[i]) {
      if (diff < FROZEN_DELTA) {
        if (stableStartTime[i] == 0) {
          stableStartTime[i] = millis();
        }

        if (millis() - stableStartTime[i] >= FROZEN_TIME) {
          frozenADC[i] = true;
        }
      } else {
        stableStartTime[i] = 0;
      }
    } else {
      stableStartTime[i] = 0;
    }

    sensorHealthy[i] = !(sensorDisconnected[i] || invalidReading[i] || frozenADC[i]);

    if (oldHealthy && !sensorHealthy[i]) {
      if (sensorDisconnected[i]) addFaultLog("DISCONNECT", i + 1);
      else if (invalidReading[i]) addFaultLog("INVALID", i + 1);
      else if (frozenADC[i]) addFaultLog("FROZEN_ADC", i + 1);
    }

    previousVoltage[i] = sensorVoltage[i];
  }
}

void detectRelayMismatchTask() {
  readRelayFeedback();

  if (millis() - relayCommandTime >= RELAY_VERIFY_TIME) {
    bool oldMismatch = relayMismatch;
    relayMismatch = (relayFeedback != relayCommand);

    if (!oldMismatch && relayMismatch) {
      addFaultLog("RELAY_MISMATCH", 0);
    }
  }
}

int getHealthyCount() {
  int count = 0;
  for (int i = 0; i < 4; i++) {
    if (sensorHealthy[i]) count++;
  }
  return count;
}

void updateRuntimeModeTask() {
  int healthyCount = getHealthyCount();

  if (relayMismatch) {
    currentMode = MODE_FAILSAFE;
    commandRelay(false);
    recoveryStartTime = 0;
    return;
  }

  if (healthyCount == 4) {
    if (currentMode == MODE_NORMAL) {
      commandRelay(true);
    } else {
      if (recoveryStartTime == 0) {
        recoveryStartTime = millis();
      }

      currentMode = MODE_DEGRADED;
      commandRelay(false);

      if (millis() - recoveryStartTime >= RECOVERY_TIME) {
        currentMode = MODE_NORMAL;
        commandRelay(true);
        recoveryStartTime = 0;
      }
    }
  }
  else if (healthyCount >= 2) {
    currentMode = MODE_DEGRADED;
    commandRelay(true);
    recoveryStartTime = 0;
  }
  else if (healthyCount == 1) {
    currentMode = MODE_FAILSAFE;
    commandRelay(false);
    recoveryStartTime = 0;
  }
  else {
    currentMode = MODE_SHUTDOWN;
    commandRelay(false);
    recoveryStartTime = 0;
  }
}

void updateBuzzerTask() {
  if (currentMode == MODE_NORMAL) {
    digitalWrite(BUZZER_PIN, LOW);
    buzzerState = false;
  }
  else if (currentMode == MODE_DEGRADED) {
    if (millis() - lastBuzzerTime >= BUZZER_SLOW) {
      lastBuzzerTime = millis();
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
    }
  }
  else {
    if (millis() - lastBuzzerTime >= BUZZER_FAST) {
      lastBuzzerTime = millis();
      buzzerState = !buzzerState;
      digitalWrite(BUZZER_PIN, buzzerState);
    }
  }
}

void updateLCDTask() {
  printLCDLine(0, "MODE:" + modeToString(currentMode));

  String line2 = "R:";
  line2 += relayCommand ? "ON " : "OFF";
  line2 += " S:";
  line2 += getHealthyCount();
  line2 += "/4";

  printLCDLine(1, line2);
}

void printFaultLogs() {
  Serial.println("Fault Logs:");

  if (totalLogs == 0) {
    Serial.println("No logs");
    return;
  }

  for (int i = 0; i < totalLogs; i++) {
    Serial.print("#");
    Serial.print(i + 1);
    Serial.print(" Time:");
    Serial.print(faultLogs[i].timestamp / 1000);
    Serial.print("s ");

    Serial.print("Fault:");
    Serial.print(faultLogs[i].faultName);

    Serial.print(" Module:");
    if (faultLogs[i].module == 0) Serial.println("Relay");
    else Serial.println(faultLogs[i].module);
  }
}

void printSerialTask() {
  Serial.println("====================================");
  Serial.println("FAULT TOLERANT RUNTIME SYSTEM");
  Serial.println("====================================");

  Serial.print("Runtime Mode   : ");
  Serial.println(modeToString(currentMode));

  Serial.print("Relay Command  : ");
  Serial.println(relayCommand ? "ON" : "OFF");

  Serial.print("Relay Feedback : ");
  Serial.println(relayFeedback ? "ON" : "OFF");

  Serial.print("Relay Mismatch : ");
  Serial.println(relayMismatch ? "YES" : "NO");

  Serial.print("Buzzer State   : ");
  Serial.println(buzzerState ? "ON" : "OFF");

  Serial.print("Healthy Sensors: ");
  Serial.print(getHealthyCount());
  Serial.println("/4");

  Serial.println("------------------------------------");

  for (int i = 0; i < 4; i++) {
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(" : ");
    Serial.print(sensorVoltage[i], 2);
    Serial.print(" V | ");

    Serial.print(sensorHealthy[i] ? "HEALTHY" : "FAULTY");

    if (sensorDisconnected[i]) Serial.print(" | DISCONNECT");
    if (invalidReading[i]) Serial.print(" | INVALID");
    if (frozenADC[i]) Serial.print(" | FROZEN_ADC");

    Serial.println();
  }

  Serial.println("------------------------------------");
  printFaultLogs();
  Serial.println();
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.init();
  lcd.backlight();

  printLCDLine(0, "Fault-Tolerant");
  printLCDLine(1, "Runtime System");

  readSensorsTask();

  for (int i = 0; i < 4; i++) {
    previousVoltage[i] = sensorVoltage[i];
    stableStartTime[i] = 0;
  }

  commandRelay(true);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = now;

    readSensorsTask();
    detectSensorFaultsTask();
    detectRelayMismatchTask();
    updateRuntimeModeTask();
  }

  if (now - lastLCDTime >= LCD_INTERVAL) {
    lastLCDTime = now;
    updateLCDTask();
  }

  if (now - lastSerialTime >= SERIAL_INTERVAL) {
    lastSerialTime = now;
    printSerialTask();
  }

  updateBuzzerTask();
}
