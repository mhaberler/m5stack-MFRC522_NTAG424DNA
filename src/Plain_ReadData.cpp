/*
 * Typical pin layout used:
 * ------------------------------------------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino   Arduino Arduino
 * NodeMCU Reader/PCD   Uno/101       Mega      Nano v3   Leonardo/Micro   Pro
 * Micro Signal      Pin          Pin           Pin       Pin       Pin Pin Pin
 * ------------------------------------------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9        RESET/ICSP-5 RST
 * D1 / GPIO5 (#define RST_PIN 5) SPI SS      SDA(SS)      10            53 D10
 * 10               10          D2 / GPIO4 (#define SS_PIN 4) SPI MOSI    MOSI
 * 11 / ICSP-4   51        D11       ICSP-4           16          D7 / GPIO13
 * SPI MISO    MISO         12 / ICSP-1   50        D12       ICSP-1 14 D6 /
 * GPIO12 SPI SCK     SCK          13 / ICSP-3   52        D13       ICSP-3 15
 * D5 / GPIO14
 *
 * More pin layouts for other boards can be found here:
 * https://github.com/miguelbalboa/rfid#pin-layout
 */
#ifdef M5UNIFIED
#include <M5Unified.h>
#else
#include <Arduino.h>
#endif

#include <MFRC522Debug.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522v2.h>
#include <SPI.h>

#include <MFRC522_NTAG424DNA.h>

const uint8_t customAddress = 0x28;
TwoWire &customI2C = Wire;

MFRC522DriverI2C driver{customAddress, customI2C};  // Create I2C driver.
MFRC522_NTAG424DNA ntag{driver};

bool deselectAndWakeupA = false;
void printHex(byte *buffer, uint16_t bufferSize);

void setup() {
#ifdef M5UNIFIED
  M5.begin();
#else
  Serial.begin(115200);
#endif

#ifdef STARTUP_DELAY
  delay(STARTUP_DELAY);
#endif
#ifdef I2C0_SDA
  Wire.begin(I2C0_SDA, I2C0_SCL, I2C9_SPEED);
#else
  Wire.begin();
#endif
  ntag.PCD_Init();

  Serial.println(F("POWER ON"));
}

void loop() {
  ntag.PCD_StopCrypto1();

  // Errors other than timeout appear often when slowly approaching a card to
  // the reader. When it happens, the card has to be deselected and woken up.
  if (deselectAndWakeupA) {
    deselectAndWakeupA = false;
    if (!ntag.PICC_TryDeselectAndWakeupA()) return;
  } else if (!ntag.PICC_IsNewCardPresent()) {
    return;
  }

  if (!ntag.PICC_ReadCardSerial()) return;

  MFRC522_NTAG424DNA::DNA_StatusCode dna_statusCode;
  dna_statusCode = ntag.DNA_Plain_ISOSelectFile_Application();
  if (dna_statusCode != MFRC522_NTAG424DNA::DNA_STATUS_OK) {
    Serial.print(F("Plain_ISOSelectFile STATUS NOT OK: "));
    // For a list of status codes corresponding to these numbers, refer to
    // https://github.com/obsttube/mfrc522_ntag424dna
    Serial.println(dna_statusCode);
    if (dna_statusCode != MFRC522_NTAG424DNA::DNA_STATUS_TIMEOUT)
      // Errors other than timeout appear often when slowly approaching a card
      // to the reader. When it happens, the card has to be deselected and woken
      // up.
      deselectAndWakeupA = true;
    return;
  }

  MFRC522_NTAG424DNA::DNA_File file;
  uint16_t lengthToRead;
  file = MFRC522_NTAG424DNA::DNA_FILE_NDEF;
  lengthToRead = 256;
  // PROPRIETARY file by default requires key2 and CommMode FULL to read (see
  // the Full_ReadData example). CC file is only 32 B long. If lengthToRead is
  // longer than that, there WILL be an error!
  /*file = MFRC522_NTAG424DNA::DNA_FILE_CC;
  lengthToRead = 32;*/
  byte *backData = new byte[lengthToRead];
  memset(backData, 0, lengthToRead);
  uint16_t backLen = lengthToRead;
  byte offset = 0;
  dna_statusCode =
      ntag.DNA_Plain_ReadData(file, lengthToRead, offset, backData, &backLen);
  if (dna_statusCode != MFRC522_NTAG424DNA::DNA_STATUS_OK) {
    Serial.print(F("Plain_ReadData STATUS NOT OK: "));
    Serial.println(dna_statusCode);
    if (dna_statusCode != MFRC522_NTAG424DNA::DNA_STATUS_TIMEOUT)
      // Errors other than timeout appear often when slowly approaching a card
      // to the reader. When it happens, the card has to be deselected and woken
      // up.
      deselectAndWakeupA = true;
  } else {
    Serial.print(F("File contents:"));
    printHex(backData, backLen);
    Serial.println();
  }
  delete[] backData;
}

void printHex(byte *buffer, uint16_t bufferSize) {
  for (uint16_t i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
