example for https://github.com/Obsttube/MFRC522_NTAG424DNA using an esp32

I massaged the code to use the better maintained [MFRC522v2 library](https://github.com/OSSLibraries/Arduino_MFRC522v2) - seems to work fine, in my case via I2C

in the process I added the [MFRC522Extended subclass](https://github.com/mhaberler/Arduino_MFRC522v2/commit/a643fc1dc94cd7a8337d1007c05568e7354bcaab) back in:
