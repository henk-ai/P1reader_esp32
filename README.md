# ESP32-S3 DSMR P1 reader

Eenvoudig Arduino voorbeeld dat een DSMR 5 slimme meter uitleest via de P1-poort
met behulp van de [matthijskooijman/arduino-dsmr](https://github.com/matthijskooijman/arduino-dsmr)
library.

## Bestanden

* `P1reader_esp32.ino` – Voorbeeld sketch voor de ESP32-S3.

## Gebruik

1. Installeer de **arduino-dsmr** library in de Arduino IDE of via PlatformIO.
2. Open de `P1reader_esp32.ino` sketch.
3. Pas indien nodig de pinnen `P1_RX_PIN`, `P1_TX_PIN` en `P1_RTS_PIN` aan je eigen
   hardware aan.
4. Upload naar de ESP32-S3.
5. Monitor de seriële poort op 115200 baud om de telegram-waarden te zien.
