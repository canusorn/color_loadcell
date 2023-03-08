#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <HX711.h>

// Color Sensor Setup
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Load Cell Setup
HX711 scale;

const int DOUT_PIN = 3;
const int CLK_PIN = 2;

void setup() {
  Serial.begin(115200);
  
  // Color Sensor Setup
  if (!tcs.begin()) {
    Serial.println("Color Sensor not found");
  }
  
  // Load Cell Setup
  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(2280.f); // Set this to the calibration factor of your load cell
  scale.tare();
}

void loop() {
  // Read color sensor data
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  Serial.print("Red: ");
  Serial.print(r);
  Serial.print(" Green: ");
  Serial.print(g);
  Serial.print(" Blue: ");
  Serial.print(b);
  Serial.print(" Clear: ");
  Serial.println(c);
  
  // Read load cell data
  float weight = scale.get_units();
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");
  
  delay(1000);
}
