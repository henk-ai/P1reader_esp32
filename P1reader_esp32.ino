#include <Arduino.h>
#include <dsmr.h>

// Change these pin numbers to match your wiring.
constexpr int P1_RX_PIN = 44;   // UART input from the smart meter
constexpr int P1_TX_PIN = 43;   // Not used by the meter but required by HardwareSerial
constexpr int P1_RTS_PIN = 48;  // Optional: controls the smart meter's data request pin

// The library lets you declare which values from the telegram you want to decode.
using P1Data = dsmr::ParsedData<
  dsmr::Identification,
  dsmr::P1Version,
  dsmr::Timestamp,
  dsmr::ElectricityDeliveredTariff1,
  dsmr::ElectricityDeliveredTariff2,
  dsmr::ElectricityReturnedTariff1,
  dsmr::ElectricityReturnedTariff2,
  dsmr::PowerDelivered,
  dsmr::PowerReturned,
  dsmr::GasDelivered
>;

HardwareSerial P1Serial(1);
P1Data data;

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
  if (dsmr::telegram::read(P1Serial, data)) {
    Serial.println(F("--- New telegram ---"));

    if (data.timestamp_present) {
      Serial.print(F("Timestamp: "));
      Serial.println(data.timestamp);
    }

    if (data.electricity_delivered_tariff1_present) {
      Serial.print(F("Delivered T1: "));
      Serial.println(data.electricity_delivered_tariff1);
    }

    if (data.electricity_delivered_tariff2_present) {
      Serial.print(F("Delivered T2: "));
      Serial.println(data.electricity_delivered_tariff2);
    }

    if (data.electricity_returned_tariff1_present) {
      Serial.print(F("Returned T1: "));
      Serial.println(data.electricity_returned_tariff1);
    }

    if (data.electricity_returned_tariff2_present) {
      Serial.print(F("Returned T2: "));
      Serial.println(data.electricity_returned_tariff2);
    }

    if (data.power_delivered_present) {
      Serial.print(F("Power delivered: "));
      Serial.println(data.power_delivered);
    }

    if (data.power_returned_present) {
      Serial.print(F("Power returned: "));
      Serial.println(data.power_returned);
    }

    if (data.gas_delivered_present) {
      Serial.print(F("Gas delivered: "));
      Serial.println(data.gas_delivered);
    }
  }
}

