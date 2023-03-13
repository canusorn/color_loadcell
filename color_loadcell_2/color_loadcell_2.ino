/*
 * การต่อสาย
 *
 * Loadcell hx711
 * DT  -> 18
 * SCK -> 19
 * VCC -> 3.3v
 * GND -> GND
 *
 * TCS34725
 * SDA  -> 21
 * SCL  -> 22
 * 3.3v -> 3.3v
 * GND  -> GND
 *
 *
 * ไลบรารี่เพิ่มเติม
 * 1.Adafruit_TCS34725 โหลดจาก library manager ได้เลย กดติดตั้ง เลือกติดตั้งทั้งหมด
 * 2.HX711 โหลดจาก library manager
 * 3.TridentTD_LineNotify โหลดจาก library manager
 * 4.blynk โหลดจาก library manager
 *
 */

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial // ปริ้นค่าดีบัก blynk ทาง Serial

/* ข้อมูลจาก Blynk */
#define BLYNK_TEMPLATE_ID "TMPLcuspclyY"
#define BLYNK_TEMPLATE_NAME "Loadcell and Color"
#define BLYNK_AUTH_TOKEN "auiQQ6M5S3pyXqBG5henvGKFoQreLOyy"

#include <WiFi.h>                 // ไวไฟ
#include <WiFiClient.h>           // ไวไฟ
#include <BlynkSimpleEsp32.h>     // Blynk
#include <Adafruit_TCS34725.h>    // เซนเซอร์สี
#include <HX711.h>                // โหลดเซลล์
#include <TridentTD_LineNotify.h> // ไลน์

#define SSID "ssid"            // ใส่ชื่อไวไฟ
#define PASSWORD "pass"        // ใส่ชื่อพาสเวิด
#define LINE_TOKEN "linetoken" // ใส่โทเคนไวไฟ

#define RED_NOTIFY 500  // ระดับสีแดงที่ต้องการแจ้งเตือน
#define WEIGHT_NOTIFY 5 // ระดับน้ำหนักที่ต้องการแจ้งเตือน [หน่วยเปอร์เซน]

// Color Sensor Setup สร้าง object ชื่อ tcs
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// Load Cell Setup  สร้าง object  ชื่อ scale
HX711 scale;

// ตั้งค่า pin สำหรับ loadcell
const int DOUT_PIN = 18;
const int CLK_PIN = 19;

// ตัวแปลสำหรับ state
int color_state = 0;    // 0-start 2notify
int loadcell_state = 0; // 0-start 1ready 2notify

int max_load = 0; // เก็บค่าน้ำหนักสูงสุด
int sample = 0;   // เก็บค่าดีเลย์จับน้ำหนัก

void setup()
{
  Serial.begin(115200); // เริ่มทำงาน serial

  // เชื่อมต่อไวไฟ
  Blynk.begin(BLYNK_AUTH_TOKEN, SSID, PASSWORD);
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());

  // กำหนด Line Token
  LINE.setToken(LINE_TOKEN);

  // Color Sensor Setup เริ่มทำงานเซนเซอร์สี
  if (!tcs.begin())
  {
    Serial.println("Color Sensor not found");
  }

  // Load Cell Setup เริ่มทำงาน loadcell
  scale.begin(DOUT_PIN, CLK_PIN);
  scale.set_scale(770); // ตัวคูณสำหรับคาลิเบรทค่าน้ำหนัก
  scale.tare();         // รีเซ็ต0
}

void loop()
{
  Blynk.run(); // Blynk

  uint16_t r, g, b, c;            // ตัวแปรเก็บค่าสี
  tcs.getRawData(&r, &g, &b, &c); // อ่านค่าสีจากเซนเซอร์
  // แสดงค่าสีออก Serial monitor
  Serial.print("Red: ");
  Serial.print(r);
  Serial.print(" Green: ");
  Serial.print(g);
  Serial.print(" Blue: ");
  Serial.print(b);
  Serial.print(" Clear: ");
  Serial.println(c);

  // ส่งค่าสีแดงไป V0
  Blynk.virtualWrite(V0, r);

  // เงื่อนไขจับสีเลือด
  if (color_state == 0) // state0 ยังจับสีแดงไม่ได้
  {
    if (r >= RED_NOTIFY) // ถ้าจับสีแดงได้
    {
      color_state = 1;             // เปลี่ยน state 1
      LINE.notify("ตรวจจับเลือดได้"); // แจ้งเตือนไลน์
    }
  }
  else // state1 จับสีแดงได้แล้ว
  {
    if (r < RED_NOTIFY) // เมื่อสีแดงหายไป
    {
      color_state = 0; // เปลี่ยนกลับ state 0
    }
  }

  // Read load cell data
  float weight = scale.get_units(); // อ่านค่านำ้หนัก
  // แสดงค่านำหนักทาง Serial monitor
  Serial.print("Weight: ");
  Serial.print(weight);
  Serial.println(" g");

  // ส่งค่าน้ำหนักไป V1
  Blynk.virtualWrite(V1, weight);

  // เงื่อนไขวัดน้ำหนัก
  if (loadcell_state == 0) // state0 ยังไม่มีน้ำหนัก
  {
    if (weight > 100) // ตรวจจับน้ำหนักได้แล้ว
    {
      sample++;
      if (sample >= 5)
      {
        sample = 0;         // เครียร์ค่า
        max_load = weight;  // กำหนดค่าน้ำหนักสูงสุด
        loadcell_state = 1; // เปลี่ยน state 1
      }
    }
  }
  else if (loadcell_state == 1) // state1 มีน้ำหนักแต่ยังไม่ต่ำ
  {
    if (weight < max_load * WEIGHT_NOTIFY / 100) // ตรวจจับน้ำหนักต่ำ
    {
      loadcell_state = 0;      // เปลี่ยน state 0
      LINE.notify("น้ำเกลือต่ำ"); // แจ้งเตือนไลน์
      max_load = 0;            // เครียร์ค่า max
    }
  }

  delay(1000); // ดีเลย์ 1 วินาที
}
