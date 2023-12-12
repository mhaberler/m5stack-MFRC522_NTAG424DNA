// Shows the content of a Mifare Classic tag formatted as an NDEF tag
// It serves as an example of how to parse the records one at a time
// This example requires #define NDEF_USE_SERIAL to be uncommented in Ndef.h
#ifdef M5UNIFIED
#include <M5Unified.h>
#else
#include <Arduino.h>
#endif

#include <MFRC522Debug.h>
#include <MFRC522DriverI2C.h>
#include <MFRC522v2.h>
#include <SPI.h>

#include "NfcAdapter.h"

const uint8_t customAddress = 0x28;

TwoWire &customI2C = Wire;

MFRC522DriverI2C driver{customAddress, customI2C};  // Create I2C driver.

MFRC522 mfrc522{driver};  // Create MFRC522 instance.

NfcAdapter nfc = NfcAdapter(&mfrc522);

void setup(void) {
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
  Serial.println(
      "Extended NDEF Reader\nPlace an unformatted Mifare Classic "
      "tag on the reader to show contents.");
  mfrc522.PCD_Init();  // Init MFRC522
  MFRC522Debug::PCD_DumpVersionToSerial(
      mfrc522, Serial);  // Show version of PCD - MFRC522 Card Reader.
  nfc.begin();
}

void loop(void) {
  mfrc522.PCD_StopCrypto1();

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    switch (mfrc522.PICC_GetType(mfrc522.uid.sak)) {
      case MFRC522Constants::PICC_TYPE_MIFARE_1K:
      case MFRC522Constants::PICC_TYPE_MIFARE_UL:

      {
        Serial.println("Reading NFC tag");
        NfcTag tag = nfc.read();
        Serial.println(tag.getTagType());
        Serial.print("UID: ");
        Serial.println(tag.getUidString());

        if (tag.hasNdefMessage())  // every tag won't have a message
        {
          NdefMessage message = tag.getNdefMessage();
          Serial.print("\nThis NFC Tag contains an NDEF Message with ");
          Serial.print(message.getRecordCount());
          Serial.print(" NDEF Record");
          if (message.getRecordCount() != 1) {
            Serial.print("s");
          }
          Serial.println(".");

          // cycle through the records, printing some info from each
          int recordCount = message.getRecordCount();
          for (int i = 0; i < recordCount; i++) {
            Serial.print("\nNDEF Record ");
            Serial.println(i + 1);
            NdefRecord record = message.getRecord(i);
            // NdefRecord record = message[i]; // alternate syntax

            Serial.print("  TNF: ");
            Serial.println(record.getTnf());
            Serial.print("  Type: ");
            PrintHexChar(record.getType(),
                         record.getTypeLength());  // will be "" for TNF_EMPTY

            // The TNF and Type should be used to determine how your
            // application processes the payload There's no generic
            // processing for the payload, it's returned as a byte[]
            int payloadLength = record.getPayloadLength();
            const byte *payload = record.getPayload();

            // Print the Hex and Printable Characters
            Serial.print("  Payload (HEX): ");
            PrintHexChar(payload, payloadLength);

            // Force the data into a String (might work depending on the
            // content) Real code should use smarter processing
            String payloadAsString = "";
            for (int c = 0; c < payloadLength; c++) {
              payloadAsString += (char)payload[c];
            }
            Serial.print("  Payload (as String): ");
            Serial.println(payloadAsString);

            // id is probably blank and will return ""
            if (record.getIdLength() > 0) {
              Serial.print("  ID: ");
              PrintHexChar(record.getId(), record.getIdLength());
            }
          }
        }
        delay(2000);
      } break;
      case MFRC522Constants::PICC_TYPE_ISO_14443_4:
        Serial.println("PICC_TYPE_ISO_14443_4");
        MFRC522Debug::PICC_DumpToSerial(mfrc522, Serial, &mfrc522.uid);
        break;

      default:
        break;
    }
  }
}