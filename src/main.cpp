#include <Arduino.h>
#include <M5Unified.h>
#include <SCServo.h>
#include <Avatar.h>

using namespace m5avatar;
Avatar avatar;

SCSCL sc;
static u16 x_offset = 160; // for Pan
static u16 y_offset = 160; // for Tilt
static u8 x_id = 1;        // Pan
static u8 y_id = 2;        // Tilt

void sw_tone()
{
  M5.Mic.end(); // Turn off the microphone while using the speaker
  M5.Speaker.tone(1000, 100);
  delay(500);
  M5.Speaker.end();
  M5.Mic.begin();
}

u16 convertSCS0009Pos(u16 degree)
{
  return map(degree, 0, 325, 1023, 0);
}

void scsMove(u8 id, u16 degree, u16 offset = 0, u16 mtime = 300, u16 mspeed = 0, uint32_t aDelay = 0)
{
  sc.WritePos(id, convertSCS0009Pos(degree + offset), mtime, mspeed);
  delay(50 + aDelay); //
}

void testServo()
{
  avatar.setSpeechText("テストモード!");
  sw_tone();
  delay(1000);
  avatar.setSpeechText("");

  scsMove(x_id, -30, x_offset);
  avatar.setExpression(m5avatar::Expression::Happy);
  delay(1000);
  scsMove(y_id, -15, y_offset);
  avatar.setExpression(m5avatar::Expression::Angry);
  delay(1000);
  scsMove(x_id, 0, x_offset);
  avatar.setExpression(m5avatar::Expression::Sad);
  delay(1000);
  scsMove(y_id, 0, y_offset);
  avatar.setExpression(m5avatar::Expression::Neutral);
  delay(1000);

  scsMove(x_id, 30, x_offset);
  avatar.setExpression(m5avatar::Expression::Doubt);
  delay(1000);
  scsMove(y_id, -15, y_offset);
  avatar.setExpression(m5avatar::Expression::Sleepy);
  delay(1000);
  scsMove(x_id, 0, x_offset);
  avatar.setExpression(m5avatar::Expression::Happy);
  delay(1000);
  scsMove(y_id, 0, y_offset);
  avatar.setExpression(m5avatar::Expression::Neutral);
  delay(1000);
}

void initServoLoc()
{
  scsMove(x_id, 0, x_offset, 150);
  sw_tone();

  avatar.setExpression(m5avatar::Expression::Happy);
  avatar.setMouthOpenRatio(1.0);
  avatar.setSpeechText("こんにちは");
  scsMove(y_id, -15, y_offset, 150);
  delay(1000);

  scsMove(y_id, 0, y_offset, 150);
  delay(300);

  avatar.setExpression(m5avatar::Expression::Neutral);
  avatar.setMouthOpenRatio(0.0);
  avatar.setSpeechText("");
}

void initServo()
{
#if defined(ARDUINO_M5Stack_Core_ESP32)
  Serial2.begin(1000000, SERIAL_8N1, 16, 17); // Core Basic
#elif defined(ARDUINO_M5STACK_Core2)
  Serial2.begin(1000000, SERIAL_8N1, 13, 14); // Core 2
#elif defined(ARDUINO_M5STACK_CORES3)
  Serial2.begin(1000000, SERIAL_8N1, 18, 17); // Core S3
#endif

  sc.pSerial = &Serial2;

  sc.PWMMode(1, false); // true: PWM, false: Serial

  initServoLoc();
}

void testRotation()
{
  avatar.setSpeechText("PWM回転テスト!");
  sw_tone();
  delay(1000);
  avatar.setSpeechText("");

  sc.PWMMode(x_id, true); // PWMサーボモード
  delay(50);

  sc.WritePWM(x_id, 300);
  avatar.setExpression(m5avatar::Expression::Happy);
  avatar.setEyeOpenRatio(1.0);
  avatar.setMouthOpenRatio(1.0);
  delay(3000);

  sc.PWMMode(x_id, false); // シリアルサーボモード
  delay(50);

  scsMove(x_id, 0, x_offset);
  delay(1000);

  avatar.setExpression(m5avatar::Expression::Neutral);
  avatar.setEyeOpenRatio(0.0);
  avatar.setMouthOpenRatio(0.0);
}

#if defined(ARDUINO_M5STACK_Core2) || defined(ARDUINO_M5STACK_CORES3)
struct box_t
{
  int x;
  int y;
  int w;
  int h;
  int touch_id = -1;

  void setupBox(int x, int y, int w, int h)
  {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
  }
  bool contain(int x, int y)
  {
    return this->x <= x && x < (this->x + this->w) && this->y <= y && y < (this->y + this->h);
  }
};
static box_t box_servo;
static box_t box_stt;
static box_t box_BtnA;
static box_t box_BtnB;
static box_t box_BtnC;

void setPanelBtn()
{
  box_BtnA.setupBox(0, 0, M5.Display.width() / 3 - 10, M5.Display.height());
  box_BtnB.setupBox(M5.Display.width() / 3, 0, M5.Display.width() / 3, M5.Display.height());
  box_BtnC.setupBox(M5.Display.width() * 2 / 3 + 10, 0, M5.Display.width() / 3 - 10, M5.Display.height());
}
#endif

void setup()
{
  M5.begin();

  avatar.init();
  avatar.setSpeechFont(&fonts::efontJA_16);
  initServo();

#if defined(ARDUINO_M5STACK_Core2) || defined(ARDUINO_M5STACK_CORES3)
  setPanelBtn();
#endif
}

void loop()
{
  M5.update();

#if defined(ARDUINO_M5Stack_Core_ESP32)
  if (M5.BtnA.wasClicked())
  {
    testRotation();
  }
  if (M5.BtnB.wasClicked())
  {
    initServoLoc();
  }
  if (M5.BtnC.wasClicked())
  {
    testServo();
  }
#elif defined(ARDUINO_M5STACK_Core2) || defined(ARDUINO_M5STACK_CORES3)
  auto count = M5.Touch.getCount();
  if (count)
  {
    auto t = M5.Touch.getDetail();
    if (t.wasClicked())
    {
      if (box_BtnA.contain(t.x, t.y))
      {
        testRotation();
      }
      if (box_BtnB.contain(t.x, t.y))
      {
        initServoLoc();
      }
      if (box_BtnC.contain(t.x, t.y))
      {
        testServo();
      }
    }
  }
#endif
}
