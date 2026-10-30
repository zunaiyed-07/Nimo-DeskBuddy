#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"
#include <math.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// ==================================================
// YOUR CREDENTIALS - EDIT THESE
// ==================================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
const char* OPENWEATHER_API_KEY = "YOUR_OPENWEATHER_API_KEY";
const char* CITY = "Sirajganj";
const char* COUNTRY_CODE = "BD";
const char* TIMEZONE = "IST-6:00";

// ==================================================
// HARDWARE CONFIGURATION
// ==================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 41
#define SCL_PIN 42
#define BUTTON_PIN 7

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
bool buttonArmed = false;
unsigned long buttonReadyAt = 0;
unsigned long buttonReleaseStart = 0;

// ==================================================
// WEATHER ICONS
// ==================================================
const unsigned char bmp_clear[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,0x00,0x01,0x80,0x00,0x00,0x01,0x80,0x00,
  0x00,0x00,0x00,0x00,0x01,0x03,0xc0,0x80,0x00,0x0f,0xf0,0x00,0x00,0x3f,0xfc,0x00,
  0x00,0x7f,0xfe,0x00,0x00,0xff,0xff,0x00,0x06,0xff,0xff,0x60,0x06,0xff,0xff,0x60,
  0x06,0xff,0xff,0x60,0x00,0xff,0xff,0x00,0x3e,0xff,0xff,0x7c,0x3e,0xff,0xff,0x7c,
  0x3e,0xff,0xff,0x7c,0x00,0xff,0xff,0x00,0x06,0xff,0xff,0x60,0x06,0xff,0xff,0x60,
  0x06,0xff,0xff,0x60,0x00,0xff,0xff,0x00,0x00,0x7f,0xfe,0x00,0x00,0x3f,0xfc,0x00,
  0x01,0x0f,0xf0,0x80,0x00,0x03,0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x00,
  0x00,0x01,0x80,0x00,0x00,0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_clouds[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xe0,0x00,
  0x00,0x0f,0xf8,0x00,0x00,0x1f,0xfc,0x00,0x00,0x3f,0xfe,0x00,0x00,0x3f,0xff,0x00,
  0x00,0x7f,0xff,0x80,0x00,0xff,0xff,0xc0,0x00,0xff,0xff,0xe0,0x01,0xff,0xff,0xf0,
  0x03,0xff,0xff,0xf8,0x07,0xff,0xff,0xfc,0x07,0xff,0xff,0xfc,0x0f,0xff,0xff,0xfe,
  0x0f,0xff,0xff,0xfe,0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,
  0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,0x0f,0xff,0xff,0xfe,
  0x07,0xff,0xff,0xfc,0x03,0xff,0xff,0xf8,0x00,0xff,0xff,0xe0,0x00,0x3f,0xff,0x80,
  0x00,0x0f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_rain[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x03,0xe0,0x00,0x00,0x0f,0xf8,0x00,0x00,0x1f,0xfc,0x00,
  0x00,0x3f,0xfe,0x00,0x00,0x7f,0xff,0x80,0x00,0xff,0xff,0xc0,0x01,0xff,0xff,0xf0,
  0x03,0xff,0xff,0xf8,0x07,0xff,0xff,0xfc,0x0f,0xff,0xff,0xfe,0x1f,0xff,0xff,0xff,
  0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,0x1f,0xff,0xff,0xff,0x0f,0xff,0xff,0xfe,
  0x07,0xff,0xff,0xfc,0x03,0xff,0xff,0xf8,0x00,0xff,0xff,0xe0,0x00,0x3f,0xff,0x80,
  0x00,0x0f,0xfe,0x00,0x00,0x00,0x00,0x00,0x00,0x60,0x0c,0x00,0x00,0x60,0x0c,0x00,
  0x00,0xe0,0x1c,0x00,0x00,0xc0,0x18,0x00,0x03,0x80,0x70,0x00,0x03,0x80,0x70,0x00,
  0x03,0x00,0x60,0x00,0x02,0x00,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char mini_sun[] PROGMEM = {
  0x00,0x00,0x01,0x80,0x00,0x00,0x10,0x08,0x04,0x20,0x03,0xc0,0x27,0xe4,0x07,0xe0,
  0x07,0xe0,0x27,0xe4,0x03,0xc0,0x04,0x20,0x10,0x08,0x00,0x00,0x01,0x80,0x00,0x00
};
const unsigned char mini_cloud[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x01,0xc0,0x07,0xe0,0x0f,0xf0,0x1f,0xf8,0x1f,0xf8,0x3f,0xfc,
  0x3f,0xfc,0x7f,0xfe,0x3f,0xfe,0x1f,0xfc,0x0f,0xf0,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char mini_rain[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x01,0xc0,0x07,0xe0,0x0f,0xf0,0x1f,0xf8,0x1f,0xf8,0x3f,0xfc,
  0x3f,0xfc,0x7f,0xfe,0x3f,0xfe,0x1f,0xfc,0x00,0x00,0x44,0x44,0x22,0x22,0x11,0x11
};
const unsigned char bmp_tiny_drop[] PROGMEM = { 0x10,0x38,0x7c,0xfe,0xfe,0x7c,0x38,0x00 };
const unsigned char bmp_dizzy_stars[] PROGMEM = {
  0x08,0x20,0x14,0x50,0x22,0x88,0x41,0x04,0x82,0x02,0x41,0x04,0x22,0x88,0x14,0x50,
  0x08,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_angry_mark[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x08,0x00,0x1c,0x00,0x3e,0x00,0x7f,0x00,0x3e,0x00,0x1c,0x00,
  0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// ==================================================
// EMOTION PARTICLES
// ==================================================
const unsigned char bmp_heart[] PROGMEM = {
  0x00,0x00,0x0c,0x60,0x1e,0xf0,0x3f,0xf8,0x7f,0xfc,0x7f,0xfc,0x7f,0xfc,0x3f,0xf8,
  0x1f,0xf0,0x0f,0xe0,0x07,0xc0,0x03,0x80,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_zzz[] PROGMEM = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3c,0x00,0x0c,0x00,0x18,0x00,0x30,0x00,0x7e,
  0x00,0x00,0x3c,0x00,0x0c,0x00,0x18,0x00,0x30,0x00,0x7c,0x00,0x00,0x00,0x00,0x00
};
const unsigned char bmp_small_heart[] PROGMEM = {
  0x00, 0x66, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00
};

// ==================================================
// GLOBALS
// ==================================================
unsigned long lastTapTime = 0;
bool lastPinState = false;
bool rawPinState = false;
unsigned long lastPinChange = 0;
unsigned long pressStartTime = 0;
bool isLongPressHandled = false;
const unsigned long LONG_PRESS_TIME = 1000;
const unsigned long DOUBLE_TAP_DELAY = 400;
const unsigned long BUTTON_DEBOUNCE_TIME = 40;

enum DeskMode {
  MODE_CLOCK_FACES,
  MODE_WEATHER_FACE,
  MODE_ANIMATION,
  MODE_FACE_EXPRESSIONS,
  MODE_GAMES,
  MODE_AUTO
};

DeskMode currentMode = MODE_AUTO;
DeskMode modeBeforeMenu = MODE_ANIMATION;
bool menuOpen = false;
int menuDepth = 0;
int selectedMenuOption = 0;
int clockStyle = 0;
int weatherStyle = 0;
int animationVariant = 0;
int selectedAnimation = 0;
int selectedExpression = 1;
bool pendingTap = false;
unsigned long petMoodEnd = 0;
int petPreviousMood = 1;
unsigned long autoModeStarted = 0;
int animationSceneIndex = 0;

#define SCENE_PARTICLE_COUNT 14
struct SceneParticle {
  float x;
  float y;
  float speed;
  int size;
};
SceneParticle sceneParticles[SCENE_PARTICLE_COUNT];

float dinoY = 48;
float dinoVelocityY = 0;
float obstacleX = 128;
unsigned long lastGameUpdate = 0;
unsigned long gameScore = 0;
bool gameOver = false;

#define MOOD_NORMAL     0
#define MOOD_HAPPY      1
#define MOOD_SURPRISED  2
#define MOOD_SLEEPY     3
#define MOOD_ANGRY      4
#define MOOD_SAD        5
#define MOOD_EXCITED    6
#define MOOD_LOVE       7
#define MOOD_SUSPICIOUS 8
#define MOOD_DIZZY      9
#define MOOD_WINK       10
#define MOOD_CRYING     11
#define MOOD_LAUGHING   12
#define MOOD_CONFUSED   13
#define MOOD_BORED      14
#define MOOD_COOL       15
#define MOOD_SHY        16
#define MOOD_STARSTRUCK 17
#define MOOD_SCARED     18
#define MOOD_GRUMPY     19
#define MOOD_EMBARRASSED 20
#define MOOD_SNEEZING   21

int currentMood = MOOD_HAPPY;
int weatherMood = MOOD_HAPPY;
const int FACE_EXPRESSION_COUNT = 22;

int expressionMoodForIndex(int index) {
  static const int moods[FACE_EXPRESSION_COUNT] = {
    MOOD_HAPPY, MOOD_LOVE, MOOD_SURPRISED, MOOD_SLEEPY, MOOD_ANGRY,
    MOOD_SAD, MOOD_EXCITED, MOOD_SUSPICIOUS, MOOD_DIZZY, MOOD_NORMAL,
    MOOD_WINK, MOOD_CRYING, MOOD_LAUGHING, MOOD_CONFUSED, MOOD_BORED,
    MOOD_COOL, MOOD_SHY, MOOD_STARSTRUCK, MOOD_SCARED, MOOD_GRUMPY,
    MOOD_EMBARRASSED, MOOD_SNEEZING
  };
  return moods[constrain(index, 0, FACE_EXPRESSION_COUNT - 1)];
}

void setFaceExpression(int index) {
  selectedExpression = constrain(index, 0, FACE_EXPRESSION_COUNT - 1);
  currentMood = expressionMoodForIndex(selectedExpression);
  animationVariant = selectedExpression % 6;
  petMoodEnd = 0;
}

float temperature = 0.0;
float feelsLike = 0.0;
int humidity = 0;
String weatherMain = "Clear";
String weatherDesc = "Sunny";
unsigned long lastWeatherUpdate = 0;

struct ForecastDay {
  String dayName;
  int temp;
  String iconType;
};
ForecastDay fcast[3];
Preferences settings;

// ==================================================
// EYE PHYSICS ENGINE
// ==================================================
struct Eye {
  float x, y, w, h;
  float targetX, targetY, targetW, targetH;
  float pupilX, pupilY, targetPupilX, targetPupilY;
  float velX, velY, velW, velH, pVelX, pVelY;
  float k = 0.15, d = 0.65, pk = 0.2, pd = 0.6;
  bool blinking;
  unsigned long lastBlink, nextBlinkTime;

  void init(float _x, float _y, float _w, float _h) {
    x = targetX = _x; y = targetY = _y;
    w = targetW = _w; h = targetH = _h;
    pupilX = targetPupilX = 0; pupilY = targetPupilY = 0;
    nextBlinkTime = millis() + random(1000, 4000);
  }
  void update() {
    float ax = (targetX - x) * k; float ay = (targetY - y) * k;
    float aw = (targetW - w) * k; float ah = (targetH - h) * k;
    velX = (velX + ax) * d; velY = (velY + ay) * d;
    velW = (velW + aw) * d; velH = (velH + ah) * d;
    x += velX; y += velY; w += velW; h += velH;
    float pax = (targetPupilX - pupilX) * pk;
    float pay = (targetPupilY - pupilY) * pk;
    pVelX = (pVelX + pax) * pd; pVelY = (pVelY + pay) * pd;
    pupilX += pVelX; pupilY += pVelY;
  }
};

Eye leftEye, rightEye;
unsigned long lastSaccade = 0;
unsigned long saccadeInterval = 3000;
float breathVal = 0;

// ==================================================
// WEATHER FUNCTIONS
// ==================================================
const unsigned char* getBigIcon(String w) {
  if (w == "Clear")                      return bmp_clear;
  if (w == "Clouds")                     return bmp_clouds;
  if (w == "Rain" || w == "Drizzle")     return bmp_rain;
  return bmp_clouds;
}

const unsigned char* getMiniIcon(String w) {
  if (w == "Clear")                                        return mini_sun;
  if (w == "Rain" || w == "Drizzle" || w == "Thunderstorm") return mini_rain;
  return mini_cloud;
}

void updateWeatherMood() {
  if (weatherMain == "Clear")                           weatherMood = MOOD_HAPPY;
  else if (weatherMain == "Rain" || weatherMain == "Drizzle") weatherMood = MOOD_SAD;
  else if (weatherMain == "Thunderstorm")               weatherMood = MOOD_SURPRISED;
  else if (weatherMain == "Clouds")                     weatherMood = MOOD_NORMAL;
  else if (temperature > 35)                           weatherMood = MOOD_ANGRY;
  else if (temperature < 10)                           weatherMood = MOOD_SLEEPY;
  else                                                  weatherMood = MOOD_NORMAL;

  if (currentMode == MODE_AUTO) currentMood = weatherMood;
}

void getWeatherAndForecast() {
  if (WiFi.status() != WL_CONNECTED) return;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q="
    + String(CITY) + "," + String(COUNTRY_CODE)
    + "&appid=" + String(OPENWEATHER_API_KEY) + "&units=metric";
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JSONVar myObject = JSON.parse(payload);
    if (JSON.typeof(myObject) != "undefined") {
      temperature = double(myObject["main"]["temp"]);
      feelsLike   = double(myObject["main"]["feels_like"]);
      humidity    = int(myObject["main"]["humidity"]);
      weatherMain = (const char*)myObject["weather"][0]["main"];
      weatherDesc = (const char*)myObject["weather"][0]["description"];
      if (weatherDesc.length() > 0) weatherDesc[0] = toupper(weatherDesc[0]);
      updateWeatherMood();
    }
  }
  http.end();

  url = "http://api.openweathermap.org/data/2.5/forecast?q="
    + String(CITY) + "," + String(COUNTRY_CODE)
    + "&appid=" + String(OPENWEATHER_API_KEY) + "&units=metric";
  http.begin(url);
  httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();
    JSONVar fo = JSON.parse(payload);
    if (JSON.typeof(fo) != "undefined") {
      struct tm t;
      getLocalTime(&t);
      int today = t.tm_wday;
      const char* days[] = { "SUN","MON","TUE","WED","THU","FRI","SAT" };
      int indices[3] = { 7, 15, 23 };
      for (int i = 0; i < 3; i++) {
        int idx = indices[i];
        fcast[i].temp     = (int)double(fo["list"][idx]["main"]["temp"]);
        fcast[i].iconType = (const char*)fo["list"][idx]["weather"][0]["main"];
        int nextDayIndex  = (today + i + 1) % 7;
        fcast[i].dayName  = days[nextDayIndex];
      }
    }
  }
  http.end();
}

// ==================================================
// BUTTON HANDLER
// ==================================================
void openMenu() {
  if (!menuOpen) modeBeforeMenu = currentMode;
  menuOpen = true;
  menuDepth = 0;
  selectedMenuOption = 0;
  pendingTap = false;
}

int getMenuOptionCount() {
  if (menuDepth == 0) return 6;
  if (currentMode == MODE_CLOCK_FACES) return 3;
  if (currentMode == MODE_WEATHER_FACE) return 3;
  if (currentMode == MODE_ANIMATION) return 11;
  if (currentMode == MODE_FACE_EXPRESSIONS) return FACE_EXPRESSION_COUNT;
  return 1;
}

void enterSelectedMenuOption() {
  if (menuDepth == 0) {
    currentMode = (DeskMode)selectedMenuOption;
    if (currentMode == MODE_CLOCK_FACES ||
        currentMode == MODE_WEATHER_FACE ||
        currentMode == MODE_ANIMATION ||
        currentMode == MODE_FACE_EXPRESSIONS) {
      menuDepth = 1;
      selectedMenuOption = currentMode == MODE_CLOCK_FACES ? clockStyle :
                           currentMode == MODE_WEATHER_FACE ? weatherStyle :
                           currentMode == MODE_FACE_EXPRESSIONS ? selectedExpression :
                           selectedAnimation;
      return;
    }

    menuOpen = false;
    if (currentMode == MODE_AUTO) autoModeStarted = millis();
    if (currentMode == MODE_GAMES) {
      dinoY = 48;
      dinoVelocityY = 0;
      obstacleX = 128;
      gameScore = 0;
      gameOver = false;
      lastGameUpdate = millis();
    }
    return;
  }

  if (currentMode == MODE_CLOCK_FACES) {
    clockStyle = selectedMenuOption;
    settings.putInt("clock", clockStyle);
  } else if (currentMode == MODE_WEATHER_FACE) {
    weatherStyle = selectedMenuOption;
    settings.putInt("weather", weatherStyle);
  } else if (currentMode == MODE_ANIMATION) {
    selectedAnimation = selectedMenuOption;
    settings.putInt("animation", selectedAnimation);
    animationSceneIndex = selectedAnimation;
    resetSceneParticles();
  } else if (currentMode == MODE_FACE_EXPRESSIONS) {
    setFaceExpression(selectedMenuOption);
    settings.putInt("expression", selectedExpression);
  }
  menuOpen = false;
  menuDepth = 0;
}

void jumpDino() {
  if (!gameOver && dinoY >= 47) dinoVelocityY = -7.0f;
}

void handleSingleTap() {
  if (menuOpen) {
    selectedMenuOption = (selectedMenuOption + 1) % getMenuOptionCount();
    return;
  }

  switch (currentMode) {
    case MODE_CLOCK_FACES:
      clockStyle = (clockStyle + 1) % 3;
      settings.putInt("clock", clockStyle);
      break;
    case MODE_WEATHER_FACE:
      weatherStyle = (weatherStyle + 1) % 3;
      settings.putInt("weather", weatherStyle);
      break;
    case MODE_ANIMATION:
      animationSceneIndex = (animationSceneIndex + 1) % 11;
      selectedAnimation = animationSceneIndex;
      settings.putInt("animation", selectedAnimation);
      resetSceneParticles();
      break;
    case MODE_FACE_EXPRESSIONS:
      setFaceExpression((selectedExpression + 1) % FACE_EXPRESSION_COUNT);
      settings.putInt("expression", selectedExpression);
      break;
    case MODE_GAMES:
      if (gameOver) {
        dinoY = 48;
        dinoVelocityY = 0;
        obstacleX = 128;
        gameScore = 0;
        gameOver = false;
        lastGameUpdate = millis();
      } else jumpDino();
      break;
    case MODE_AUTO:
      break;
  }
}

void handleDoubleTap() {
  if (menuOpen) {
    enterSelectedMenuOption();
    return;
  }

  if (currentMode == MODE_FACE_EXPRESSIONS) {
    petPreviousMood = currentMood;
    currentMood = MOOD_LOVE;
    petMoodEnd = millis() + 1400;
    return;
  }

  openMenu();
}

void handleButton() {
  unsigned long now = millis();
  if (now < buttonReadyAt) return;

  bool reading = digitalRead(BUTTON_PIN) == LOW;
  if (!buttonArmed) {
    if (!reading) {
      if (buttonReleaseStart == 0) buttonReleaseStart = now;
      if (now - buttonReleaseStart >= BUTTON_DEBOUNCE_TIME) {
        buttonArmed = true;
        rawPinState = false;
        lastPinState = false;
        pendingTap = false;
        lastPinChange = now;
      }
    } else {
      buttonReleaseStart = 0;
    }
    return;
  }

  if (reading != rawPinState) {
    rawPinState = reading;
    lastPinChange = now;
  }
  if (now - lastPinChange < BUTTON_DEBOUNCE_TIME) return;

  bool currentPinState = rawPinState;
  if (currentPinState != lastPinState) {
    Serial.printf("Button %s\n", currentPinState ? "pressed" : "released");
  }

  if (currentPinState && !lastPinState) {
    pressStartTime = now;
    isLongPressHandled = false;
  } else if (currentPinState && lastPinState) {
    if ((now - pressStartTime > LONG_PRESS_TIME) && !isLongPressHandled) {
      if (menuOpen) {
        menuOpen = false;
        currentMode = modeBeforeMenu;
      } else {
        openMenu();
        Serial.println("Menu opened: tap to move, double-tap to select");
      }
      pendingTap = false;
      isLongPressHandled = true;
    }
  } else if (!currentPinState && lastPinState) {
    if ((now - pressStartTime < LONG_PRESS_TIME) && !isLongPressHandled) {
      if (pendingTap && now - lastTapTime <= DOUBLE_TAP_DELAY) {
        pendingTap = false;
        handleDoubleTap();
      } else {
        pendingTap = true;
        lastTapTime = now;
        if (currentMode == MODE_GAMES && !gameOver) jumpDino();
      }
    }
  }
  lastPinState = currentPinState;

  if (pendingTap && now - lastTapTime > DOUBLE_TAP_DELAY) {
    pendingTap = false;
    if (currentMode != MODE_GAMES || gameOver) handleSingleTap();
  }
}

// ==================================================
// DRAW FUNCTIONS
// ==================================================
void drawDizzyEye(bool isLeft) {
  Eye& e = isLeft ? leftEye : rightEye;
  int ix = (int)e.x, iy = (int)e.y, iw = (int)e.w, ih = (int)e.h;
  int cx = ix + iw / 2, cy = iy + ih / 2;

  display.fillCircle(cx, cy, iw / 2, SSD1306_WHITE);

  unsigned long now = millis();
  float angle = now * 0.025;
  int radius = iw / 3;
  int pupilX = cx + cos(angle) * radius;
  int pupilY = cy + sin(angle) * radius;
  display.fillCircle(pupilX, pupilY, 4, SSD1306_BLACK);
  display.fillCircle(pupilX + 2, pupilY - 2, 1, SSD1306_WHITE);

  int pupilX2 = cx - cos(angle) * radius;
  int pupilY2 = cy - sin(angle) * radius;
  display.fillCircle(pupilX2, pupilY2, 3, SSD1306_BLACK);

  int starOffset = (now / 80) % 4;
  display.drawBitmap(6 - starOffset, 0, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
  display.drawBitmap(106 + starOffset, 0, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
}

void drawAngryEye(bool isLeft) {
  Eye& e = isLeft ? leftEye : rightEye;
  int ix = (int)e.x, iy = (int)e.y, iw = (int)e.w, ih = (int)e.h;

  display.fillRoundRect(ix, iy, iw, ih, 8, SSD1306_WHITE);
  int cx = ix + iw / 2, cy = iy + ih / 2;
  int pupilSize = iw / 2.5;
  int px = cx - 2, py = cy;
  display.fillRoundRect(px - pupilSize/2, py - pupilSize/2, pupilSize, pupilSize, pupilSize/2, SSD1306_BLACK);

  if (isLeft) {
    for (int i = 0; i < 8; i++) display.drawLine(ix-2, iy-2+i, ix+iw+2, iy+6+i, SSD1306_BLACK);
    display.drawBitmap(ix - 12, iy - 6, bmp_angry_mark, 16, 16, SSD1306_WHITE);
  } else {
    for (int i = 0; i < 8; i++) display.drawLine(ix-2, iy+6+i, ix+iw+2, iy-2+i, SSD1306_BLACK);
    display.drawBitmap(ix + iw - 4, iy - 6, bmp_angry_mark, 16, 16, SSD1306_WHITE);
  }
  for (int i = 0; i < 3; i++) display.drawLine(ix+2+i*4, iy+ih-2, ix+6+i*4, iy+ih-6, SSD1306_WHITE);
}

void drawNormalEye(bool isLeft) {
  Eye& e = isLeft ? leftEye : rightEye;
  int ix = (int)e.x, iy = (int)e.y, iw = (int)e.w, ih = (int)e.h;
  int r = (iw < 20) ? 3 : 8;
  int cx = ix + iw / 2, cy = iy + ih / 2;

  if ((currentMood == MOOD_WINK && isLeft) ||
      currentMood == MOOD_LAUGHING ||
      currentMood == MOOD_SNEEZING) {
    int direction = currentMood == MOOD_SNEEZING ? -1 : 1;
    int previousX = ix + 3;
    int previousY = cy + direction * 3;
    for (int step = 1; step <= 6; step++) {
      int nextX = ix + 3 + step * (iw - 6) / 6;
      int nextY = cy + direction * (3 - abs(3 - step));
      display.drawLine(previousX, previousY, nextX, nextY, SSD1306_WHITE);
      previousX = nextX;
      previousY = nextY;
    }
    return;
  }

  if (animationVariant == 1) {
    display.fillCircle(cx, cy, min(iw, ih) / 2, SSD1306_WHITE);
  } else if (animationVariant == 2) {
    display.fillRect(ix, iy, iw, ih, SSD1306_WHITE);
  } else {
    int eyeRadius = animationVariant == 3 ? ih / 2 : r;
    display.fillRoundRect(ix, iy, iw, ih, eyeRadius, SSD1306_WHITE);
  }

  if (animationVariant == 5 && isLeft && (millis() % 6000) < 180) {
    display.drawLine(ix + 2, cy, ix + iw - 2, cy, SSD1306_BLACK);
    return;
  }

  int pw = animationVariant == 4 ? iw / 3 : iw / 2.2;
  int ph = animationVariant == 4 ? ih / 3 : ih / 2.2;
  if (currentMood == MOOD_SCARED) {
    pw = max(5, iw / 4);
    ph = max(5, ih / 4);
  }
  if (animationVariant == 5) ph = max(4, ih / 4);
  int px = cx + (int)e.pupilX - pw / 2;
  int py = cy + (int)e.pupilY - ph / 2;
  if (px < ix) px = ix;
  if (px + pw > ix + iw) px = ix + iw - pw;
  if (py < iy) py = iy;
  if (py + ph > iy + ih) py = iy + ih - ph;
  display.fillRoundRect(px, py, pw, ph, r / 2, SSD1306_BLACK);
  if (iw > 15 && ih > 15) display.fillCircle(px + pw - 4, py + 4, 2, SSD1306_WHITE);

  if (animationVariant == 3) {
    int browY = iy - 2;
    display.drawLine(ix + 2, browY, cx - 2, browY - 2, SSD1306_WHITE);
    display.drawLine(cx + 2, browY - 2, ix + iw - 2, browY, SSD1306_WHITE);
  } else if (animationVariant == 4) {
    int sparkleX = isLeft ? ix + 5 : ix + iw - 5;
    display.drawPixel(sparkleX, iy + 4, SSD1306_WHITE);
    display.drawPixel(sparkleX + (isLeft ? 1 : -1), iy + 5, SSD1306_WHITE);
  }
  if (currentMood == MOOD_SUSPICIOUS) {
    int browY = iy - 2;
    display.drawLine(ix + 3, browY, ix + iw - 3, browY + (isLeft ? -3 : 1), SSD1306_WHITE);
  } else if (currentMood == MOOD_CONFUSED) {
    int browY = iy - 3;
    display.drawLine(ix + 2, browY + (isLeft ? 2 : 0), cx, browY - 2, SSD1306_WHITE);
    display.drawLine(cx, browY - 2, ix + iw - 2, browY + (isLeft ? 0 : 2), SSD1306_WHITE);
  }

  if (currentMood == MOOD_HAPPY || currentMood == MOOD_EXCITED ||
      currentMood == MOOD_LOVE || currentMood == MOOD_LAUGHING)
    display.fillRect(ix, iy + ih - 10, iw, 12, SSD1306_BLACK);
  else if (currentMood == MOOD_SLEEPY)
    display.fillRect(ix, iy, iw, ih / 2, SSD1306_BLACK);
  else if (currentMood == MOOD_BORED || currentMood == MOOD_GRUMPY)
    display.fillRect(ix, iy, iw, currentMood == MOOD_GRUMPY ? ih / 2 : ih / 3, SSD1306_BLACK);
  else if (currentMood == MOOD_SAD) {
    if (isLeft)
      for (int i = 0; i < 8; i++) display.drawLine(ix, iy+i, ix+iw, iy+4+i, SSD1306_BLACK);
    else
      for (int i = 0; i < 8; i++) display.drawLine(ix, iy+4+i, ix+iw, iy+i, SSD1306_BLACK);
  }

  if (currentMood == MOOD_STARSTRUCK) {
    display.drawLine(px + pw / 2, py + 2, px + pw / 2, py + ph - 2, SSD1306_WHITE);
    display.drawLine(px + 2, py + ph / 2, px + pw - 2, py + ph / 2, SSD1306_WHITE);
  }
}

void drawMouth() {
  const int mx = 64;
  const int midY = 54;
  switch (currentMood) {
    case MOOD_HAPPY:
    case MOOD_LOVE:
    case MOOD_WINK:
    case MOOD_SHY:
    case MOOD_EMBARRASSED:
      for (int thickness = 0; thickness < 2; thickness++) {
        int previousX = mx - 10;
        int previousY = midY + 4 + thickness;
        for (int x = -9; x <= 10; x++) {
          int y = midY + 4 + (x * x) / 40 + thickness;
          display.drawLine(previousX, previousY, mx + x, y, SSD1306_WHITE);
          previousX = mx + x;
          previousY = y;
        }
      }
      break;

    case MOOD_EXCITED:
    case MOOD_LAUGHING:
      display.fillRoundRect(mx - 7, midY - 5, 14, 11, 5, SSD1306_WHITE);
      display.fillRoundRect(mx - 4, midY - 2, 8, 5, 3, SSD1306_BLACK);
      break;

    case MOOD_SAD:
    case MOOD_CRYING:
      for (int thickness = 0; thickness < 2; thickness++) {
        int previousX = mx - 9;
        int previousY = midY + 1 - thickness;
        for (int x = -8; x <= 9; x++) {
          int y = midY + 1 - (x * x) / 35 - thickness;
          display.drawLine(previousX, previousY, mx + x, y, SSD1306_WHITE);
          previousX = mx + x;
          previousY = y;
        }
      }
      break;

    case MOOD_SURPRISED:
    case MOOD_SCARED:
      display.fillRoundRect(mx - 4, midY - 5, 9, 12, 4, SSD1306_WHITE);
      display.fillRoundRect(mx - 2, midY - 3, 5, 8, 2, SSD1306_BLACK);
      break;

    case MOOD_SLEEPY:
      display.drawLine(mx - 5, midY, mx + 5, midY, SSD1306_WHITE);
      display.drawPixel(mx - 4, midY + 1, SSD1306_WHITE);
      display.drawPixel(mx + 4, midY + 1, SSD1306_WHITE);
      break;

    case MOOD_ANGRY: {
      const int mouthX[] = {-8, -5, -2, 1, 4, 7};
      const int mouthY[] = {0, 3, 0, 3, 0, 3};
      for (int i = 0; i < 5; i++) {
        display.drawLine(mx + mouthX[i], midY + mouthY[i],
                         mx + mouthX[i + 1], midY + mouthY[i + 1],
                         SSD1306_WHITE);
      }
      break;
    }

    case MOOD_SUSPICIOUS:
    case MOOD_COOL:
      display.drawLine(mx - 6, midY + 1, mx + 6, midY - 1, SSD1306_WHITE);
      display.drawLine(mx - 4, midY + 2, mx + 4, midY, SSD1306_WHITE);
      break;

    case MOOD_DIZZY:
    case MOOD_CONFUSED:
      display.drawLine(mx - 7, midY, mx - 3, midY + 2, SSD1306_WHITE);
      display.drawLine(mx - 3, midY + 2, mx + 1, midY - 1, SSD1306_WHITE);
      display.drawLine(mx + 1, midY - 1, mx + 5, midY + 2, SSD1306_WHITE);
      break;

    case MOOD_BORED:
      display.drawLine(mx - 5, midY + 1, mx + 5, midY + 1, SSD1306_WHITE);
      break;

    case MOOD_STARSTRUCK:
      display.fillCircle(mx, midY, 4, SSD1306_WHITE);
      display.drawPixel(mx - 1, midY - 1, SSD1306_BLACK);
      display.drawPixel(mx + 1, midY - 1, SSD1306_BLACK);
      break;

    case MOOD_GRUMPY:
      display.drawLine(mx - 6, midY - 1, mx, midY + 2, SSD1306_WHITE);
      display.drawLine(mx, midY + 2, mx + 6, midY - 1, SSD1306_WHITE);
      break;

    case MOOD_SNEEZING:
      display.fillRoundRect(mx - 6, midY - 4, 12, 9, 4, SSD1306_WHITE);
      display.drawLine(mx - 3, midY, mx + 3, midY, SSD1306_BLACK);
      break;

    case MOOD_NORMAL:
    default:
      display.drawLine(mx - 5, midY, mx + 5, midY, SSD1306_WHITE);
      display.drawPixel(mx - 4, midY - 1, SSD1306_WHITE);
      display.drawPixel(mx + 4, midY - 1, SSD1306_WHITE);
      break;
  }
}

void updateEyePhysics() {
  unsigned long now = millis();
  breathVal = sin(now / 800.0) * 1.5;

  const int blinkDelay = 120;
  const int blinkMin = 2000;
  const int blinkMax = 6000;

  if (now > leftEye.nextBlinkTime) {
    leftEye.blinking = rightEye.blinking = true;
    leftEye.lastBlink = now;
    leftEye.nextBlinkTime = now + random(blinkMin, blinkMax);
  }
  if (leftEye.blinking) {
    leftEye.targetH = rightEye.targetH = 2;
    if (now - leftEye.lastBlink > blinkDelay) {
      leftEye.blinking = rightEye.blinking = false;
      leftEye.targetH  = rightEye.targetH  = 32;
    }
  }
  leftEye.update();
  rightEye.update();
}

void drawEmoPage() {
  updateEyePhysics();
  if (currentMood == MOOD_DIZZY) {
    drawDizzyEye(true);
    drawDizzyEye(false);
  } else if (currentMood == MOOD_ANGRY) {
    drawAngryEye(true);
    drawAngryEye(false);
  } else {
    drawNormalEye(true);
    drawNormalEye(false);
  }

  if (currentMood == MOOD_COOL) {
    display.fillRoundRect(27, 24, 34, 17, 4, SSD1306_BLACK);
    display.fillRoundRect(77, 24, 34, 17, 4, SSD1306_BLACK);
    display.drawLine(60, 30, 78, 30, SSD1306_WHITE);
    display.drawLine(32, 27, 41, 27, SSD1306_WHITE);
    display.drawLine(82, 27, 91, 27, SSD1306_WHITE);
  } else if (currentMood == MOOD_CRYING) {
    display.drawBitmap(42, 42, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
    display.drawBitmap(91, 42, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
  } else if (currentMood == MOOD_SHY || currentMood == MOOD_EMBARRASSED) {
    display.drawLine(28, 48, 37, 46, SSD1306_WHITE);
    display.drawLine(30, 51, 39, 49, SSD1306_WHITE);
    display.drawLine(89, 46, 98, 48, SSD1306_WHITE);
    display.drawLine(87, 49, 96, 51, SSD1306_WHITE);
  } else if (currentMood == MOOD_STARSTRUCK) {
    display.drawBitmap(7, 4, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
    display.drawBitmap(105, 4, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
  } else if (currentMood == MOOD_SNEEZING) {
    display.drawLine(52, 43, 48, 46, SSD1306_WHITE);
    display.drawLine(76, 43, 80, 46, SSD1306_WHITE);
    display.drawLine(49, 51, 45, 54, SSD1306_WHITE);
  }

  drawMouth();

  if (currentMood == MOOD_LOVE)
    display.drawBitmap(56, 0, bmp_heart, 16, 16, SSD1306_WHITE);
  else if (currentMood == MOOD_SLEEPY)
    display.drawBitmap(110, 0, bmp_zzz, 16, 16, SSD1306_WHITE);
}

void resetSceneParticles() {
  for (int i = 0; i < SCENE_PARTICLE_COUNT; i++) {
    sceneParticles[i].x = random(SCREEN_WIDTH);
    sceneParticles[i].y = random(SCREEN_HEIGHT);
    sceneParticles[i].speed = random(10, 36) / 10.0f;
    sceneParticles[i].size = random(2, 5);
  }
}

void drawSceneAnimation() {
  const unsigned long now = millis();

  if (animationSceneIndex == 7) {
    for (int layer = 0; layer < 3; layer++) {
      int previousY = 16 + layer * 12 + sin(now / 500.0f + layer) * 4;
      for (int x = 4; x < SCREEN_WIDTH; x += 4) {
        int y = 16 + layer * 12 + sin(now / 500.0f + x / 18.0f + layer) * 4;
        display.drawLine(x - 4, previousY, x, y, SSD1306_WHITE);
        previousY = y;
      }
    }
    return;
  }

  if (animationSceneIndex == 10) {
    for (int burst = 0; burst < 3; burst++) {
      const int centersX[] = { 24, 64, 104 };
      const int centersY[] = { 30, 23, 34 };
      int radius = ((now / 70 + burst * 8) % 24) + 2;
      for (int ray = 0; ray < 8; ray++) {
        float angle = ray * 0.785398f + now / 1800.0f + burst;
        int endX = centersX[burst] + cos(angle) * radius;
        int endY = centersY[burst] + sin(angle) * radius;
        display.drawLine(centersX[burst], centersY[burst], endX, endY, SSD1306_WHITE);
      }
      display.drawPixel(centersX[burst], centersY[burst], SSD1306_WHITE);
    }
    return;
  }

  for (int i = 0; i < SCENE_PARTICLE_COUNT; i++) {
    SceneParticle& particle = sceneParticles[i];
    int x = (int)particle.x;
    int y = (int)particle.y;

    switch (animationSceneIndex) {
      case 0:
        display.drawPixel(x, y, SSD1306_WHITE);
        display.drawPixel(x + 1, y + 1, SSD1306_WHITE);
        particle.y += particle.speed;
        particle.x += sin((now / 180.0f) + i) * 0.25f;
        if (particle.y > SCREEN_HEIGHT) {
          particle.y = 0;
          particle.x = random(SCREEN_WIDTH);
        }
        break;

      case 1:
        display.drawLine(x, y, x - 2, y + 5, SSD1306_WHITE);
        particle.y += particle.speed * 1.8f;
        if (particle.y > SCREEN_HEIGHT) {
          particle.y = 0;
          particle.x = random(SCREEN_WIDTH);
        }
        break;

      case 2:
        display.fillRoundRect(x, y + 3, 25, 8, 4, SSD1306_WHITE);
        display.fillCircle(x + 7, y + 3, 5, SSD1306_WHITE);
        display.fillCircle(x + 13, y, 7, SSD1306_WHITE);
        display.fillCircle(x + 19, y + 3, 5, SSD1306_WHITE);
        particle.x += particle.speed * 0.35f;
        if (particle.x > SCREEN_WIDTH) {
          particle.x = -28;
          particle.y = random(5, SCREEN_HEIGHT - 12);
        }
        break;

      case 3:
        if (((now / 140 + i * 7) % 5) != 0) {
          display.drawPixel(x, y, SSD1306_WHITE);
          display.drawPixel(x - 2, y, SSD1306_WHITE);
          display.drawPixel(x + 2, y, SSD1306_WHITE);
          display.drawPixel(x, y - 2, SSD1306_WHITE);
          display.drawPixel(x, y + 2, SSD1306_WHITE);
        }
        break;

      case 4:
        display.drawCircle(x, y, particle.size, SSD1306_WHITE);
        display.drawPixel(x - 1, y - 1, SSD1306_WHITE);
        particle.y -= particle.speed * 0.65f;
        if (particle.y < -particle.size * 2) {
          particle.y = SCREEN_HEIGHT + random(8);
          particle.x = random(SCREEN_WIDTH);
        }
        break;

      case 5:
        display.drawBitmap(x, y, bmp_small_heart, 8, 8, SSD1306_WHITE);
        particle.y -= particle.speed * 0.55f;
        if (particle.y < -8) {
          particle.y = SCREEN_HEIGHT + random(8);
          particle.x = random(SCREEN_WIDTH - 8);
        }
        break;

      case 6:
        if (((now / 90 + i * 11) % 7) < 4) {
          display.drawCircle(x, y, 2, SSD1306_WHITE);
          display.drawPixel(x, y, SSD1306_WHITE);
          display.drawPixel(x - 3, y, SSD1306_WHITE);
          display.drawPixel(x + 3, y, SSD1306_WHITE);
        }
        particle.x += sin((now / 500.0f) + i * 2) * 0.35f;
        particle.y += cos((now / 600.0f) + i) * 0.3f;
        if (particle.x < 0) particle.x = SCREEN_WIDTH - 1;
        if (particle.x >= SCREEN_WIDTH) particle.x = 0;
        if (particle.y < 0) particle.y = SCREEN_HEIGHT - 1;
        if (particle.y >= SCREEN_HEIGHT) particle.y = 0;
        break;

      case 8:
        display.drawLine(x, y, x + 5, y + 8, SSD1306_WHITE);
        display.drawCircle(x + 2, y + 4, 4, SSD1306_WHITE);
        particle.x += sin(now / 350.0f + i) * 0.8f;
        particle.y += particle.speed * 0.8f;
        if (particle.y > SCREEN_HEIGHT + 8) {
          particle.y = -8;
          particle.x = random(SCREEN_WIDTH);
        }
        break;

      case 9:
        display.drawLine(x, y, x - 8, y + 4, SSD1306_WHITE);
        display.drawPixel(x, y, SSD1306_WHITE);
        particle.x -= particle.speed * 1.8f;
        particle.y += particle.speed * 0.45f;
        if (particle.x < -8 || particle.y > SCREEN_HEIGHT) {
          particle.x = SCREEN_WIDTH + random(24);
          particle.y = random(SCREEN_HEIGHT / 2);
        }
        break;
    }
  }
}

void drawAnimationMode() {
  drawSceneAnimation();
}

void drawClockPage() {
  struct tm t;
  if (!getLocalTime(&t)) {
    display.setFont(NULL);
    display.setCursor(30, 30);
    display.print("Syncing...");
    return;
  }
  String ampm = (t.tm_hour >= 12) ? "PM" : "AM";
  int h12 = t.tm_hour % 12;
  if (h12 == 0) h12 = 12;

  display.setTextColor(SSD1306_WHITE);
  display.setFont(NULL);
  display.setCursor(114, 0);
  display.print(ampm);

  display.setFont(&FreeSansBold18pt7b);
  char timeStr[24];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", h12, t.tm_min);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 42);
  display.print(timeStr);

  display.setFont(&FreeSans9pt7b);
  char dateStr[20];
  strftime(dateStr, 20, "%a, %b %d", &t);
  display.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 60);
  display.print(dateStr);
}

void drawWorldClockPage() {
  struct tm t;
  if (!getLocalTime(&t)) return;
  time_t now; time(&now);
  time_t londonEpoch   = now;
  time_t newyorkEpoch  = now - (4 * 3600);
  struct tm* localtm   = &t;
  struct tm* londontm  = gmtime(&londonEpoch);
  struct tm* nytm      = gmtime(&newyorkEpoch);

  display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  display.setFont(NULL);
  display.setTextColor(SSD1306_BLACK);
  String title = "WORLD CLOCK";
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 4);
  display.print(title);
  display.setTextColor(SSD1306_WHITE);

  display.drawLine(42, 16, 42, 63, SSD1306_WHITE);
  display.drawLine(85, 16, 85, 63, SSD1306_WHITE);
  display.setFont(NULL);
  display.setCursor(12, 18); display.print("JPR");
  display.setCursor(52, 18); display.print("LDN");
  display.setCursor(94, 18); display.print("NYC");

  char jStr[10]; sprintf(jStr, "%02d:%02d", localtm->tm_hour, localtm->tm_min);
  display.getTextBounds(jStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(21 - w/2, 48); display.print(jStr);

  char lStr[10]; sprintf(lStr, "%02d:%02d", londontm->tm_hour, londontm->tm_min);
  display.getTextBounds(lStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(64 - w/2, 48); display.print(lStr);

  char nStr[10]; sprintf(nStr, "%02d:%02d", nytm->tm_hour, nytm->tm_min);
  display.getTextBounds(nStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(106 - w/2, 48); display.print(nStr);
}

void drawWeatherPage() {
  if (WiFi.status() != WL_CONNECTED) {
    display.setFont(NULL); display.setCursor(0, 0); display.print("No WiFi");
    return;
  }

  display.setFont(&FreeSansBold9pt7b);
  String city = CITY;
  city.toUpperCase();
  if (city.length() > 9) city = city.substring(0, 8) + ".";
  display.setCursor(0, 14);
  display.print(city);

  display.drawBitmap(92, 15, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);

  display.setFont(&FreeSansBold18pt7b);
  String tempText = String((int)temperature);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(tempText, 0, 48, &x1, &y1, &w, &h);
  display.setCursor(0, 48);
  display.print(tempText);
  display.fillCircle(x1 + w + 5, 28, 3, SSD1306_WHITE);

  display.setFont(NULL);
  display.setCursor(x1 + w + 12, 27);
  display.print("C");
  String shortDesc = weatherDesc;
  if (shortDesc.length() > 19) shortDesc = shortDesc.substring(0, 18) + ".";
  display.setCursor(0, 57);
  display.print(shortDesc);
  display.drawBitmap(93, 49, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
  display.setCursor(103, 51);
  display.print(humidity);
  display.print("%");
}

void drawForecastPage() {
  display.fillRect(0, 0, 128, 14, SSD1306_WHITE);
  display.setFont(NULL);
  display.setTextColor(SSD1306_BLACK);
  String title = "3-DAY FORECAST";
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 4);
  display.print(title);
  display.setTextColor(SSD1306_WHITE);

  display.drawLine(42, 16, 42, 63, SSD1306_WHITE);
  display.drawLine(85, 16, 85, 63, SSD1306_WHITE);

  for (int i = 0; i < 3; i++) {
    int xStart = i * 43, centerX = xStart + 21;
    display.setFont(NULL);
    String d = fcast[i].dayName;
    if (d == "") d = "WAIT";
    display.setCursor(centerX - (d.length() * 3), 18);
    display.print(d);
    display.drawBitmap(centerX - 8, 26, getMiniIcon(fcast[i].iconType), 16, 16, SSD1306_WHITE);
    display.setFont(&FreeSansBold9pt7b);
    display.getTextBounds(String(fcast[i].temp).c_str(), 0, 0, &x1, &y1, &w, &h);
    display.setCursor(centerX - w/2 - 2, 58);
    display.print(fcast[i].temp);
    display.fillCircle(centerX + w/2 + 1, 50, 2, SSD1306_WHITE);
  }
}

void drawMenu() {
  static const char* mainLabels[] = {
    "Clock faces", "Weather", "Animations", "Expressions", "Dino game", "Auto mode"
  };
  static const char* clockLabels[] = {
    "12-hour digital", "Analog clock", "24-hour digital"
  };
  static const char* weatherLabels[] = {
    "Current weather", "Weather summary", "3-day forecast"
  };
  static const char* animationLabels[] = {
    "Snow", "Rain", "Clouds", "Bubbles", "Stars", "Hearts",
    "Fireflies", "Aurora", "Falling leaves", "Comets", "Fireworks"
  };
  static const char* expressionLabels[] = {
    "Happy", "Love", "Surprised", "Sleepy", "Angry", "Sad",
    "Excited", "Suspicious", "Dizzy", "Neutral", "Wink", "Crying",
    "Laughing", "Confused", "Bored", "Cool", "Shy", "Starstruck",
    "Scared", "Grumpy", "Embarrassed", "Sneezing"
  };

  const char* const* labels = mainLabels;
  if (menuDepth == 1) {
    if (currentMode == MODE_CLOCK_FACES) labels = clockLabels;
    else if (currentMode == MODE_WEATHER_FACE) labels = weatherLabels;
    else if (currentMode == MODE_ANIMATION) labels = animationLabels;
    else if (currentMode == MODE_FACE_EXPRESSIONS) labels = expressionLabels;
  }
  const int count = getMenuOptionCount();

  display.setFont(NULL);

  const int visibleRows = min(count, 5);
  int firstItem = selectedMenuOption - (visibleRows / 2);
  if (firstItem < 0) firstItem = 0;
  if (firstItem + visibleRows > count) firstItem = count - visibleRows;

  for (int row = 0; row < visibleRows; row++) {
    const int item = firstItem + row;
    const int y = 2 + row * 10;
    const bool selected = item == selectedMenuOption;
    if (selected) {
      display.fillRoundRect(4, y, 120, 9, 3, SSD1306_WHITE);
      display.setTextColor(SSD1306_BLACK);
    }
    display.setCursor(11, y + 1);
    display.print(labels[item]);
    display.setTextColor(SSD1306_WHITE);
    if (selected) display.setCursor(112, y + 1);
    if (selected) display.print(">");
  }

  display.drawLine(4, 53, 124, 53, SSD1306_WHITE);
  display.setCursor(9, 56);
  if (menuDepth == 0) display.print("TAP NEXT  2X OPEN");
  else display.print("TAP NEXT  2X SELECT");
}

void drawAnalogClock() {
  struct tm t;
  if (!getLocalTime(&t)) {
    display.setFont(NULL);
    display.setCursor(30, 30);
    display.print("Syncing...");
    return;
  }

  const int cx = 64, cy = 32, radius = 23;
  display.drawCircle(cx, cy, radius, SSD1306_WHITE);
  display.drawCircle(cx, cy, radius - 1, SSD1306_WHITE);
  for (int mark = 0; mark < 12; mark++) {
    float angle = mark * 0.523599f - 1.570796f;
    int x1 = cx + cos(angle) * (radius - 3);
    int y1 = cy + sin(angle) * (radius - 3);
    int x2 = cx + cos(angle) * (radius - (mark % 3 == 0 ? 7 : 5));
    int y2 = cy + sin(angle) * (radius - (mark % 3 == 0 ? 7 : 5));
    display.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
  }

  float minuteAngle = t.tm_min * 0.10472f - 1.570796f;
  float hourAngle = (t.tm_hour % 12 + t.tm_min / 60.0f) * 0.523599f - 1.570796f;
  display.drawLine(cx, cy, cx + cos(hourAngle) * 11, cy + sin(hourAngle) * 11, SSD1306_WHITE);
  display.drawLine(cx, cy, cx + cos(minuteAngle) * 18, cy + sin(minuteAngle) * 18, SSD1306_WHITE);
  display.fillCircle(cx, cy, 2, SSD1306_WHITE);

  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a %d %b", &t);
  display.setFont(NULL);
  display.setCursor(43, 59);
  display.print(dateStr);
}

void drawDigitalClock24() {
  struct tm t;
  if (!getLocalTime(&t)) {
    display.setFont(NULL);
    display.setCursor(30, 30);
    display.print("Syncing...");
    return;
  }

  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t.tm_hour, t.tm_min);
  display.setFont(&FreeSansBold18pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 39);
  display.print(timeStr);
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%A, %d %b", &t);
  display.setFont(NULL);
  display.getTextBounds(dateStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 57);
  display.print(dateStr);
}

void drawClockStyle() {
  if (clockStyle == 0) drawClockPage();
  else if (clockStyle == 1) drawAnalogClock();
  else drawDigitalClock24();
}

void drawWeatherSummary() {
  display.setFont(NULL);
  display.setCursor(0, 1);
  String city = CITY;
  city.toUpperCase();
  if (city.length() > 13) city = city.substring(0, 12) + ".";
  display.print(city);
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  display.drawBitmap(90, 17, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);
  display.setFont(&FreeSansBold18pt7b);
  String tempText = String((int)temperature) + "C";
  display.setCursor(4, 40);
  display.print(tempText);
  display.setFont(NULL);

  String desc = weatherDesc;
  if (desc.length() > 12) desc = desc.substring(0, 11) + ".";
  display.setCursor(4, 48);
  display.print(desc);
  display.setCursor(4, 56);
  display.print("Humidity ");
  display.print(humidity);
  display.print("%");
}

void drawWeatherStyle() {
  if (weatherStyle == 0) drawWeatherPage();
  else if (weatherStyle == 1) drawWeatherSummary();
  else drawForecastPage();
}

void updateDinoGame() {
  if (gameOver) return;
  unsigned long now = millis();
  if (lastGameUpdate == 0) lastGameUpdate = now;
  float frameScale = constrain((now - lastGameUpdate) / 20.0f, 0.5f, 2.5f);
  lastGameUpdate = now;

  dinoY += dinoVelocityY * frameScale;
  dinoVelocityY += 0.32f * frameScale;
  if (dinoY > 48) {
    dinoY = 48;
    dinoVelocityY = 0;
  }

  obstacleX -= 2.0f * frameScale;
  if (obstacleX < -8) {
    obstacleX = SCREEN_WIDTH;
    gameScore++;
  }

  if (obstacleX < 32 && obstacleX > 14 && dinoY > 35) gameOver = true;
}

void drawDinoGame() {
  updateDinoGame();
  display.setFont(NULL);
  display.setCursor(2, 1);
  display.print("DINO");
  display.setCursor(88, 1);
  display.print("SCORE ");
  display.print(gameScore);
  display.drawLine(0, 51, SCREEN_WIDTH, 51, SSD1306_WHITE);

  int dinoTop = (int)dinoY - 13;
  display.fillRect(18, dinoTop + 5, 10, 8, SSD1306_WHITE);
  display.fillRect(24, dinoTop, 8, 8, SSD1306_WHITE);
  display.drawPixel(29, dinoTop + 2, SSD1306_BLACK);
  display.fillRect(20, (int)dinoY - 2, 3, 4, SSD1306_WHITE);
  int legOffset = ((millis() / 120) % 2 == 0 && dinoY >= 47) ? 2 : 0;
  display.drawLine(21, (int)dinoY - 1, 19 + legOffset, 49, SSD1306_WHITE);
  display.drawLine(26, (int)dinoY - 1, 27 - legOffset, 49, SSD1306_WHITE);

  if (obstacleX < SCREEN_WIDTH && obstacleX > -8) {
    display.fillRect((int)obstacleX, 37, 7, 14, SSD1306_WHITE);
    display.fillRect((int)obstacleX - 3, 42, 3, 4, SSD1306_WHITE);
  }

  if (gameOver) {
    display.fillRect(25, 19, 78, 28, SSD1306_BLACK);
    display.drawRect(25, 19, 78, 28, SSD1306_WHITE);
    display.setCursor(40, 23);
    display.print("GAME OVER");
    display.setCursor(7, 35);
    display.print("Tap retry / 2x menu");
  }
}

void drawAutoMode() {
  unsigned long elapsed = (millis() - autoModeStarted) % 25000UL;
  if (elapsed < 10000UL) {
    drawClockPage();
  } else if (elapsed < 20000UL) {
    drawWeatherPage();
  } else {
    drawEmoPage();
  }
}

void playBootAnimation() {
  display.setTextColor(SSD1306_WHITE);
  int cx = 64, cy = 32;
  for (int r = 0; r < 80; r += 4) {
    display.clearDisplay(); display.fillCircle(cx, cy, r, SSD1306_WHITE); display.display(); delay(8);
  }
  for (int r = 0; r < 80; r += 4) {
    display.clearDisplay();
    display.fillCircle(cx, cy, 80, SSD1306_WHITE);
    display.fillCircle(cx, cy, r, SSD1306_BLACK);
    display.display(); delay(8);
  }
  display.setFont(&FreeSansBold9pt7b);
  String bootText = "N I M O";
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(bootText, 0, 0, &x1, &y1, &w, &h);
  display.clearDisplay();
  display.setCursor((SCREEN_WIDTH - w) / 2, 36);
  display.print(bootText);
  display.display();
  delay(2000);
}

// ==================================================
// SETUP
// ==================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  randomSeed(micros());

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 initialization failed");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  settings.begin("deskbuddy", false);
  clockStyle = constrain(settings.getInt("clock", 0), 0, 2);
  weatherStyle = constrain(settings.getInt("weather", 0), 0, 2);
  selectedAnimation = constrain(settings.getInt("animation", 0), 0, 10);
  animationSceneIndex = selectedAnimation;
  setFaceExpression(constrain(settings.getInt("expression", 1), 0, FACE_EXPRESSION_COUNT - 1));
  currentMode = MODE_AUTO;

  leftEye.init(28, 18, 32, 32);
  rightEye.init(80, 18, 32, 32);
  resetSceneParticles();

  playBootAnimation();

  display.clearDisplay();
  display.setFont(NULL);
  display.setCursor(20, 30);
  display.print("Connecting WiFi");
  display.display();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long wifiStart = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - wifiStart < 15000)) {
    delay(500);
    display.print(".");
    display.display();
  }
  if (WiFi.status() == WL_CONNECTED) {
    display.clearDisplay(); display.setCursor(25, 30); display.print("WiFi Connected!"); display.display(); delay(1000);
    configTime(0, 0, "pool.ntp.org");
    setenv("TZ", TIMEZONE, 1); tzset();
    getWeatherAndForecast();
    lastWeatherUpdate = millis();
  } else {
    display.clearDisplay(); display.setCursor(15, 30); display.print("WiFi Failed!"); display.display(); delay(2000);
  }

  rawPinState = false;
  lastPinState = false;
  pendingTap = false;
  buttonArmed = false;
  buttonReleaseStart = 0;
  buttonReadyAt = millis() + 1200;
  autoModeStarted = millis();
}

// ==================================================
// LOOP
// ==================================================
void loop() {
  handleButton();

  if (petMoodEnd != 0 && millis() > petMoodEnd) {
    currentMood = petPreviousMood;
    petMoodEnd = 0;
  }

  if (millis() - lastWeatherUpdate > 600000 && WiFi.status() == WL_CONNECTED) {
    getWeatherAndForecast();
    lastWeatherUpdate = millis();
  }

  display.clearDisplay();

  if (menuOpen) {
    drawMenu();
  } else {
    switch (currentMode) {
      case MODE_CLOCK_FACES:
        drawClockStyle();
        break;
      case MODE_WEATHER_FACE:
        drawWeatherStyle();
        break;
      case MODE_ANIMATION:
        drawAnimationMode();
        break;
      case MODE_FACE_EXPRESSIONS:
        drawEmoPage();
        break;
      case MODE_GAMES:
        drawDinoGame();
        break;
      case MODE_AUTO:
        drawAutoMode();
        break;
    }
  }

  display.display();
  delay(20);
}