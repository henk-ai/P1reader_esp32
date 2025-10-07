# ESP32-S3 DSMR P1 reader

Eenvoudig Arduino voorbeeld dat een DSMR 5 slimme meter uitleest via de P1-poort.
De sketch verzamelt een compleet telegram, zoekt daarin naar de relevante OBIS-codes
en toont de gevonden waarden op de seriële monitor. Er is geen externe DSMR-library
meer nodig: alle parsing gebeurt in de sketch zelf.

## Bestanden

* `P1reader_esp32.ino` – Voorbeeld sketch voor de ESP32-S3.

## Gebruik

1. Open de `P1reader_esp32.ino` sketch in de Arduino IDE of PlatformIO.
2. Pas indien nodig de pinnen `P1_RX_PIN`, `P1_TX_PIN` en `P1_RTS_PIN` aan je eigen
   hardware aan.
3. Upload naar de ESP32-S3.
4. Monitor de seriële poort op 115200 baud om de telegram-waarden te zien.
