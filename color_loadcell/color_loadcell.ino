#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <HX711.h>
#include <TridentTD_LineNotify.h>

#define SSID        "G6PD_2.4G"
#define PASSWORD    "570610193"
#define LINE_TOKEN  "7m68381D2LS8ByeflY4rVEf9pPEXMXllsuFRNGBTFfG"

#define RED_NOTIFY 500    // ระดับสีแดงที่ต้องการแจ้งเตือน
#define WEIGHT_NOTIFY 200  // ระดับน้ำหนักที่ต้องการแจ้งเตือน

// Color Sensor Setup
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Load Cell Setup
HX711 scale;

const int DOUT_PIN = 18;
const int CLK_PIN = 19;

int color_state = 0;  // 0-start 2notify
int loadcell_state = 0; // 0-start 1ready 2notify

void setup() {
  Serial.begin(115200);

  pinMode(26, OUTPUT);
  pinMode(23, OUTPUT);
  digitalWrite(26, LOW);
  digitalWrite(23, HIGH);

  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting to %s\n",  SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);

  // Color Sensor Setup
  if (!tcs.begin()) {
    Serial.println("Color Sensor not found");
  }

  // Load Cell Setup
  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(228); // Set this to the calibration factor of your load cell
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

  // เงื่อนไขจับสีเลือด
  if (color_state == 0) {
    if (r >= RED_NOTIFY) {
      color_state = 2;
      LINE.notify("ตรวจจับเลือดได้");
    }
  } else {
    if (r < RED_NOTIFY) {
      color_state = 0;
    }
  }

  // Read load cell data
  float weight = scale.get_units();
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");

  //เงื่อนไขวัดน้ำหนัก
  if (loadcell_state == 0) {
    if (weight > WEIGHT_NOTIFY) {
      loadcell_state = 1;
    }
  } else if (loadcell_state == 1) {
    if (weight < WEIGHT_NOTIFY) {
      loadcell_state = 2;
      LINE.notify("น้ำเกลือต่ำ");
    }
  } else {
    if (weight > WEIGHT_NOTIFY) {
      loadcell_state = 1;
    }
  }


  delay(1000);
}
