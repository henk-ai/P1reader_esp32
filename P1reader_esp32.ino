#include <Arduino.h>

// Change these pin numbers to match your wiring.
constexpr int P1_RX_PIN = 44;   // UART input from the smart meter
constexpr int P1_TX_PIN = 43;   // Not used by the meter but required by HardwareSerial
constexpr int P1_RTS_PIN = 48;  // Optional: controls the smart meter's data request pin

struct P1Data {
  bool identificationPresent = false;
  String identification;

  bool p1VersionPresent = false;
  String p1Version;

  bool timestampPresent = false;
  String timestamp;

  bool electricityDeliveredTariff1Present = false;
  float electricityDeliveredTariff1 = 0.0f;

  bool electricityDeliveredTariff2Present = false;
  float electricityDeliveredTariff2 = 0.0f;

  bool electricityReturnedTariff1Present = false;
  float electricityReturnedTariff1 = 0.0f;

  bool electricityReturnedTariff2Present = false;
  float electricityReturnedTariff2 = 0.0f;

  bool powerDeliveredPresent = false;
  float powerDelivered = 0.0f;

  bool powerReturnedPresent = false;
  float powerReturned = 0.0f;

  bool gasDeliveredPresent = false;
  float gasDelivered = 0.0f;
};

HardwareSerial P1Serial(1);

namespace {

String telegramBuffer;
bool telegramStarted = false;
bool telegramTerminatorSeen = false;

bool readTelegram(Stream &serial, String &telegram) {
  while (serial.available()) {
    char c = static_cast<char>(serial.read());

    if (!telegramStarted) {
      if (c != '/') {
        continue;  // Wait for the telegram header.
      }
      telegramStarted = true;
      telegramBuffer = "";
      telegramTerminatorSeen = false;
    }

    telegramBuffer += c;

    if (c == '!') {
      telegramTerminatorSeen = true;
    } else if (telegramTerminatorSeen && c == '\n') {
      telegram = telegramBuffer;
      telegramBuffer = "";
      telegramStarted = false;
      telegramTerminatorSeen = false;
      return true;
    }

    if (telegramBuffer.length() > 1024) {
      telegramBuffer = "";
      telegramStarted = false;
      telegramTerminatorSeen = false;
    }
  }

  return false;
}

bool lineForObis(const String &telegram, const String &obis, String &line) {
  int start = telegram.indexOf(obis);
  if (start < 0) {
    return false;
  }

  int end = telegram.indexOf('\n', start);
  if (end < 0) {
    end = telegram.length();
  }

  line = telegram.substring(start, end);
  return true;
}

bool extractParenthesizedValue(const String &line, size_t pairIndex, String &value) {
  size_t currentPair = 0;
  int start = -1;

  for (size_t i = 0; i < line.length(); ++i) {
    char c = line.charAt(i);
    if (c == '(') {
      start = static_cast<int>(i) + 1;
    } else if (c == ')' && start >= 0) {
      if (currentPair == pairIndex) {
        value = line.substring(start, static_cast<int>(i));
        return true;
      }
      ++currentPair;
      start = -1;
    }
  }

  return false;
}

float parseNumber(const String &token) {
  String number = token;
  int unitPos = number.indexOf('*');
  if (unitPos >= 0) {
    number = number.substring(0, unitPos);
  }

  number.trim();
  return number.toFloat();
}

bool parseTelegram(const String &telegram, P1Data &data) {
  auto assignString = [](String &target, bool &flag, const String &value) {
    target = value;
    target.trim();
    flag = true;
  };

  auto assignFloat = [](float &target, bool &flag, const String &value) {
    target = parseNumber(value);
    flag = true;
  };

  String line;
  String value;

  if (lineForObis(telegram, "0-0:96.1.1", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignString(data.identification, data.identificationPresent, value);
  }

  if (lineForObis(telegram, "1-3:0.2.8", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignString(data.p1Version, data.p1VersionPresent, value);
  }

  if (lineForObis(telegram, "0-0:1.0.0", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignString(data.timestamp, data.timestampPresent, value);
  }

  if (lineForObis(telegram, "1-0:1.8.1", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.electricityDeliveredTariff1, data.electricityDeliveredTariff1Present, value);
  }

  if (lineForObis(telegram, "1-0:1.8.2", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.electricityDeliveredTariff2, data.electricityDeliveredTariff2Present, value);
  }

  if (lineForObis(telegram, "1-0:2.8.1", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.electricityReturnedTariff1, data.electricityReturnedTariff1Present, value);
  }

  if (lineForObis(telegram, "1-0:2.8.2", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.electricityReturnedTariff2, data.electricityReturnedTariff2Present, value);
  }

  if (lineForObis(telegram, "1-0:1.7.0", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.powerDelivered, data.powerDeliveredPresent, value);
  }

  if (lineForObis(telegram, "1-0:2.7.0", line) &&
      extractParenthesizedValue(line, 0, value)) {
    assignFloat(data.powerReturned, data.powerReturnedPresent, value);
  }

  if (lineForObis(telegram, "0-1:24.2.1", line) &&
      extractParenthesizedValue(line, 1, value)) {
    assignFloat(data.gasDelivered, data.gasDeliveredPresent, value);
  }

  return data.identificationPresent || data.p1VersionPresent || data.timestampPresent ||
         data.electricityDeliveredTariff1Present || data.electricityDeliveredTariff2Present ||
         data.electricityReturnedTariff1Present || data.electricityReturnedTariff2Present ||
         data.powerDeliveredPresent || data.powerReturnedPresent || data.gasDeliveredPresent;
}

}  // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  pinMode(P1_RTS_PIN, OUTPUT);
  digitalWrite(P1_RTS_PIN, HIGH);  // Request data from the P1 port

  // The P1 port of DSMR 5 meters uses 115200 8N1. The final "true" inverts the signal.
  P1Serial.begin(115200, SERIAL_8N1, P1_RX_PIN, P1_TX_PIN, true);

  Serial.println(F("DSMR P1 reader ready"));
}

void loop() {
  String telegram;
  if (!readTelegram(P1Serial, telegram)) {
    return;
  }

  P1Data data;
  if (!parseTelegram(telegram, data)) {
    Serial.println(F("Kon telegram niet parseren."));
    return;
  }

  Serial.println(F("--- New telegram ---"));

  if (data.identificationPresent) {
    Serial.print(F("Meter ID: "));
    Serial.println(data.identification);
  }

  if (data.p1VersionPresent) {
    Serial.print(F("P1 versie: "));
    Serial.println(data.p1Version);
  }

  if (data.timestampPresent) {
    Serial.print(F("Timestamp: "));
    Serial.println(data.timestamp);
  }

  if (data.electricityDeliveredTariff1Present) {
    Serial.print(F("Delivered T1 (kWh): "));
    Serial.println(data.electricityDeliveredTariff1, 3);
  }

  if (data.electricityDeliveredTariff2Present) {
    Serial.print(F("Delivered T2 (kWh): "));
    Serial.println(data.electricityDeliveredTariff2, 3);
  }

  if (data.electricityReturnedTariff1Present) {
    Serial.print(F("Returned T1 (kWh): "));
    Serial.println(data.electricityReturnedTariff1, 3);
  }

  if (data.electricityReturnedTariff2Present) {
    Serial.print(F("Returned T2 (kWh): "));
    Serial.println(data.electricityReturnedTariff2, 3);
  }

  if (data.powerDeliveredPresent) {
    Serial.print(F("Power delivered (kW): "));
    Serial.println(data.powerDelivered, 3);
  }

  if (data.powerReturnedPresent) {
    Serial.print(F("Power returned (kW): "));
    Serial.println(data.powerReturned, 3);
  }

  if (data.gasDeliveredPresent) {
    Serial.print(F("Gas delivered (m3): "));
    Serial.println(data.gasDelivered, 3);
  }
}

