#include <phyphoxBle.h>

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(460800, SERIAL_8N1, RXD2, TXD2);

  PhyphoxBLE::start("Sensor Chain");
}

const byte packageSize = 2;

int packageIndex = 0;
int package[packageSize];
bool aligned = false;
int sensorIndex = 0;
double a = 2.0*9.81 / (double)(1<<15);
signed short sensors[30];

unsigned long t0 = 0;

void loop() {
  while (Serial2.available()) {
    package[packageIndex++] = Serial2.read();
    if (aligned && packageIndex == packageSize) {
      if (
            package[0] == 0xf0 &&
            package[1] == 0x0f
//                package[0] == 0xa0 &&
//                package[1] == 0xb0 &&
//                package[2] == 0xc0 &&
//                package[3] == 0xd0 &&
//                package[4] == 0xe0 &&
//                package[5] == 0xf0
                ) {
          unsigned long t = millis();
          Serial.print("Frame ");
          Serial.print(millis() - t0);
          Serial.print("ms / ");
          t0 = t;
          Serial.print("Timestamp: ");
          Serial.print((sensors[1]<<16) | sensors[0]);
          Serial.print(" >>> ");
          for (int i = 2; i < sensorIndex; i++) {
            Serial.print(a*sensors[i]);
            Serial.print(" | ");
          }
          Serial.println("==> OUT");

          uint8_t* data = reinterpret_cast<uint8_t*>(sensors);
          PhyphoxBLE::write(data, sensorIndex*2);
          
          sensorIndex = 0;
      } else {
/*
          double accX = a * (signed short)(((package[1] & 0xff) << 8) | package[0] & 0xff);
          double accY = a * (signed short)(((package[3] & 0xff) << 8) | package[2] & 0xff);
          double accZ = a * (signed short)(((package[5] & 0xff) << 8) | package[4] & 0xff);

          Serial.print("Sensor ");
          Serial.print(sensorIndex);
          Serial.print(": x = ");
          Serial.print(accX);
          Serial.print("m/s², y = ");
          Serial.print(accY);
          Serial.print("m/s², z = ");
          Serial.print(accZ);
          Serial.print("m/s², dt = ");
          Serial.print(millis()-t0);
          Serial.print("ms [");
          for (int i = 0; i < 6; i++)
            Serial.print(package[i], HEX);
          Serial.println("]");
*/
          //sensors[sensorIndex] = (signed short)(((package[5] & 0xff) << 8) | package[4] & 0xff);
          sensors[sensorIndex] = (signed short)(((package[1] & 0xff) << 8) | package[0] & 0xff);

          sensorIndex++;
          if (sensorIndex > 30) {
            Serial.println("Alignment error...");
            aligned = false;
          }
      }
      packageIndex = 0;
    } else if (!aligned && packageIndex == packageSize) {
      Serial.print("Waiting for header: ");
      for (int i = 0; i < packageSize; i++) {
        Serial.print(package[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
      if (
            package[0] == 0xf0 &&
            package[1] == 0x0f
//                package[0] == 0xa0 &&
//                package[1] == 0xb0 &&
//                package[2] == 0xc0 &&
//                package[3] == 0xd0 &&
//                package[4] == 0xe0 &&
//                package[5] == 0xf0
                ) {
        t0 = millis();
        aligned = true;
        sensorIndex = 0;
        packageIndex = 0;
      } else {
        for (int i = 0; i < packageSize-1; i++)
          package[i] = package[i+1];
        packageIndex--;
      }
    } else if (packageIndex > packageSize) {
      Serial.println("Odd... Should not happen.");
      packageIndex = 0;
    }
  }
}
