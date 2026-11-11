#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

// ==================================================
// YOUR CREDENTIALS - EDIT THESE
// ==================================================
const char* WIFI_SSID = "Hafiz";
const char* WIFI_PASSWORD = "Tom@hunt01";
const char* OPENWEATHER_API_KEY = "761a2b9fac1a8feb9a92c93cfeb95724";
const char* CITY = "Sirajganj";
const char* COUNTRY_CODE = "BD";
const char* TIMEZONE = "BDT-6";
const char* SETUP_AP_SSID = "NIMO";
const char* SETUP_AP_PASSWORD = "nimo-setup-24";
const unsigned long WIFI_RETRY_INTERVAL_MS = 15000;
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 30000;
const int ANIMATION_SCENE_COUNT = 22;

// ==================================================
// HARDWARE CONFIGURATION
// ==================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SDA_PIN 5
#define SCL_PIN 6
#define BUTTON_PIN 7
#define TOUCH_ACTIVE_LEVEL HIGH

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
const unsigned long DOUBLE_TAP_DELAY = 550;
const unsigned long TOUCH_PRESS_DEBOUNCE_TIME = 40;
const unsigned long TOUCH_RELEASE_DEBOUNCE_TIME = 35;

enum DeskMode {
  MODE_CLOCK_FACES,
  MODE_WEATHER_FACE,
  MODE_ANIMATION,
  MODE_FACE_EXPRESSIONS,
  MODE_GAMES,
  MODE_AUTO,
  MODE_TIMER
};

DeskMode currentMode = MODE_AUTO;
DeskMode modeBeforeMenu = MODE_ANIMATION;
bool menuOpen = false;
int menuDepth = 0;
int selectedMenuOption = 0;
int clockStyle = 0;
int weatherStyle = 0;
int selectedAnimation = 0;
int selectedExpression = 1;
bool pendingTap = false;
bool doubleTapCandidate = false;
unsigned long petMoodEnd = 0;
int petPreviousMood = 1;
unsigned long autoModeStarted = 0;
int autoModePage = 0;
int animationSceneIndex = 0;
int timerPage = 0;
bool stopwatchRunning = false;
unsigned long stopwatchElapsedMs = 0;
unsigned long stopwatchStartedAt = 0;
bool countdownRunning = false;
bool countdownFinished = false;
int countdownMinutes = 5;
unsigned long countdownRemainingMs = 5UL * 60UL * 1000UL;
unsigned long countdownStartedAt = 0;

#define SCENE_PARTICLE_COUNT 14
struct SceneParticle {
  float x;
  float y;
  float speed;
  int size;
};
SceneParticle sceneParticles[SCENE_PARTICLE_COUNT];
void resetSceneParticles();

float dinoY = 48;
float dinoVelocityY = 0;
float obstacleX = 128;
unsigned long lastGameUpdate = 0;
unsigned long gameScore = 0;
bool gameOver = false;

bool accessPointMode = false;
WebServer controlServer(80);
unsigned long networkInfoUntil = 0;
String pendingWifiSsid;
String pendingWifiPassword;
String configuredWifiSsid;
String configuredWifiPassword;
String wifiConnectionMessage;
bool wifiConnectPending = false;
bool stationWasConnected = false;
bool wifiScanInProgress = false;
bool savedNetworkFailureReported = false;
unsigned long wifiConnectStartedAt = 0;
unsigned long stationConnectStartedAt = 0;
unsigned long lastWifiRetryAt = 0;

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
#define MOOD_CURIOUS    22
#define MOOD_PROUD      23
#define MOOD_NERVOUS    24
#define MOOD_SILLY      25
#define MOOD_FOCUSED    26
#define MOOD_GRATEFUL   27
#define MOOD_PLAYFUL    28
#define MOOD_SHOCKED    29

int currentMood = MOOD_HAPPY;
int weatherMood = MOOD_HAPPY;
const int FACE_EXPRESSION_COUNT = 30;

int expressionMoodForIndex(int index) {
  static const int moods[FACE_EXPRESSION_COUNT] = {
    MOOD_HAPPY, MOOD_LOVE, MOOD_SURPRISED, MOOD_SLEEPY, MOOD_ANGRY,
    MOOD_SAD, MOOD_EXCITED, MOOD_SUSPICIOUS, MOOD_DIZZY, MOOD_NORMAL,
    MOOD_WINK, MOOD_CRYING, MOOD_LAUGHING, MOOD_CONFUSED, MOOD_BORED,
    MOOD_COOL, MOOD_SHY, MOOD_STARSTRUCK, MOOD_SCARED, MOOD_GRUMPY,
    MOOD_EMBARRASSED, MOOD_SNEEZING, MOOD_CURIOUS, MOOD_PROUD,
    MOOD_NERVOUS, MOOD_SILLY, MOOD_FOCUSED, MOOD_GRATEFUL,
    MOOD_PLAYFUL, MOOD_SHOCKED
  };
  return moods[constrain(index, 0, FACE_EXPRESSION_COUNT - 1)];
}

void setFaceExpression(int index) {
  selectedExpression = constrain(index, 0, FACE_EXPRESSION_COUNT - 1);
  currentMood = expressionMoodForIndex(selectedExpression);
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
unsigned long blinkStartedAt = 0;
bool faceBlinking = false;
float blinkAmount = 0;
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
  if (menuDepth == 0) return 7;
  if (currentMode == MODE_CLOCK_FACES) return 6;
  if (currentMode == MODE_WEATHER_FACE) return 6;
  if (currentMode == MODE_ANIMATION) return ANIMATION_SCENE_COUNT;
  if (currentMode == MODE_FACE_EXPRESSIONS) return FACE_EXPRESSION_COUNT;
  if (currentMode == MODE_TIMER) return 2;
  return 1;
}

void enterSelectedMenuOption() {
  if (menuDepth == 0) {
    currentMode = (DeskMode)selectedMenuOption;
    if (currentMode == MODE_CLOCK_FACES ||
        currentMode == MODE_WEATHER_FACE ||
        currentMode == MODE_ANIMATION ||
        currentMode == MODE_FACE_EXPRESSIONS ||
        currentMode == MODE_TIMER) {
      menuDepth = 1;
      selectedMenuOption = currentMode == MODE_CLOCK_FACES ? clockStyle :
                           currentMode == MODE_WEATHER_FACE ? weatherStyle :
                           currentMode == MODE_FACE_EXPRESSIONS ? selectedExpression :
                           currentMode == MODE_TIMER ? timerPage :
                           selectedAnimation;
      return;
    }

    menuOpen = false;
    if (currentMode == MODE_AUTO) {
      autoModePage = 0;
      autoModeStarted = millis();
    }
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
  } else if (currentMode == MODE_TIMER) {
    timerPage = selectedMenuOption;
  }
  menuOpen = false;
  menuDepth = 0;
}

void jumpDino() {
  if (!gameOver && dinoY >= 47) dinoVelocityY = -7.0f;
}

void resetCountdownToSetting() {
  countdownRunning = false;
  countdownFinished = false;
  countdownRemainingMs = (unsigned long)countdownMinutes * 60UL * 1000UL;
}

void handleTimerSingleTap() {
  if (timerPage == 0) {
    if (stopwatchRunning) {
      stopwatchElapsedMs += millis() - stopwatchStartedAt;
      stopwatchRunning = false;
    } else {
      stopwatchStartedAt = millis();
      stopwatchRunning = true;
    }
    return;
  }

  if (countdownRunning) {
    unsigned long elapsed = millis() - countdownStartedAt;
    countdownRemainingMs = elapsed >= countdownRemainingMs ? 0 : countdownRemainingMs - elapsed;
    countdownRunning = false;
    countdownFinished = countdownRemainingMs == 0;
    return;
  }

  countdownMinutes = countdownMinutes >= 60 ? 1 : countdownMinutes + 1;
  resetCountdownToSetting();
}

void handleTimerDoubleTap() {
  if (timerPage == 0) {
    stopwatchRunning = false;
    stopwatchElapsedMs = 0;
    stopwatchStartedAt = 0;
    return;
  }

  if (countdownRunning) {
    resetCountdownToSetting();
  } else {
    if (countdownFinished || countdownRemainingMs == 0) resetCountdownToSetting();
    countdownStartedAt = millis();
    countdownRunning = true;
    countdownFinished = false;
  }
}

void handleSingleTap() {
  if (menuOpen) {
    selectedMenuOption = (selectedMenuOption + 1) % getMenuOptionCount();
    return;
  }

  switch (currentMode) {
    case MODE_CLOCK_FACES:
      clockStyle = (clockStyle + 1) % 6;
      settings.putInt("clock", clockStyle);
      break;
    case MODE_WEATHER_FACE:
      weatherStyle = (weatherStyle + 1) % 6;
      settings.putInt("weather", weatherStyle);
      break;
    case MODE_ANIMATION:
      animationSceneIndex = (animationSceneIndex + 1) % ANIMATION_SCENE_COUNT;
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
      autoModePage = (autoModePage + 1) % 3;
      autoModeStarted = millis();
      break;
    case MODE_TIMER:
      handleTimerSingleTap();
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

  if (currentMode == MODE_TIMER) {
    handleTimerDoubleTap();
    return;
  }

  openMenu();
}

void handleButton() {
  unsigned long now = millis();
  if (now < buttonReadyAt) return;

  bool reading = digitalRead(BUTTON_PIN) == TOUCH_ACTIVE_LEVEL;
  if (!buttonArmed) {
    if (!reading) {
      if (buttonReleaseStart == 0) buttonReleaseStart = now;
      if (now - buttonReleaseStart >= TOUCH_RELEASE_DEBOUNCE_TIME) {
        buttonArmed = true;
        rawPinState = false;
        lastPinState = false;
        pendingTap = false;
        doubleTapCandidate = false;
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
    if (reading && pendingTap && now - lastTapTime <= DOUBLE_TAP_DELAY) {
      doubleTapCandidate = true;
    } else if (!reading && !lastPinState) {
      doubleTapCandidate = false;
    }
  }
  unsigned long debounceTime = rawPinState
    ? TOUCH_PRESS_DEBOUNCE_TIME
    : TOUCH_RELEASE_DEBOUNCE_TIME;
  if (now - lastPinChange < debounceTime) return;

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
      doubleTapCandidate = false;
      isLongPressHandled = true;
    }
  } else if (!currentPinState && lastPinState) {
    if ((now - pressStartTime < LONG_PRESS_TIME) && !isLongPressHandled) {
      if (doubleTapCandidate && pendingTap) {
        pendingTap = false;
        doubleTapCandidate = false;
        handleDoubleTap();
      } else {
        pendingTap = true;
        lastTapTime = now;
        if (currentMode == MODE_GAMES && !gameOver) jumpDino();
      }
    }
    doubleTapCandidate = false;
  }
  lastPinState = currentPinState;

  bool secondTouchDebouncing = rawPinState && pendingTap &&
    now - lastTapTime <= DOUBLE_TAP_DELAY + TOUCH_PRESS_DEBOUNCE_TIME;
  if (pendingTap && !doubleTapCandidate && !secondTouchDebouncing &&
      now - lastTapTime > DOUBLE_TAP_DELAY) {
    pendingTap = false;
    if (currentMode != MODE_GAMES || gameOver) handleSingleTap();
  }
}

// ==================================================
// LOCAL WI-FI CONTROL PANEL
// ==================================================
const char CONTROL_PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#f5f6fa"><title>NIMO | Desk Buddy</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#f4f5f9;color:#202538;font:15px/1.5 system-ui,-apple-system,"Segoe UI",sans-serif}
header{height:68px;background:#fff;border-bottom:1px solid #e6e8ef;display:flex;align-items:center;padding:0 max(18px,calc((100vw - 960px)/2));gap:12px}
.logo{width:36px;height:36px;border-radius:11px;background:#6558d3;color:white;display:grid;place-items:center;font-size:19px;font-weight:800}
.brand{font-weight:750}.brand small{display:block;font-size:10px;letter-spacing:1px;color:#81889a}.status{margin-left:auto;color:#687086;font-size:12px}
main{max-width:960px;margin:auto;padding:26px 18px 55px}.tabs{display:flex;gap:5px;width:max-content;background:#e9eaf1;border-radius:11px;padding:5px;margin-bottom:24px}
.tab{border:0;background:none;color:#747b8f;border-radius:8px;padding:9px 16px;font-weight:650}.tab.active{background:#fff;color:#202538;box-shadow:0 2px 6px #20253812}
.panel{display:none}.panel.active{display:block}h1{font-size:26px;letter-spacing:-.6px;margin:0 0 3px}h2{font-size:16px;margin:0}p{color:#7b8294;margin:4px 0 18px;font-size:13px}
.card{background:white;border:1px solid #e7e9ef;border-radius:15px;padding:20px;margin:14px 0;box-shadow:0 8px 22px #20253808}
.grid{display:grid;grid-template-columns:repeat(4,1fr);gap:10px;margin-top:18px}.mode{min-height:78px;text-align:left;background:white;border:1px solid #e2e4ec;border-radius:11px;padding:12px;color:#343a4d;font-weight:650}
.mode:hover,.mode.active{border-color:#7569dc;background:#f3f2ff;color:#4035a3}.mode b{display:block;color:#6558d3;font-size:18px;margin-bottom:5px}
label{display:block;color:#60677a;font-size:12px;font-weight:650;margin:15px 0 6px}select,input{width:100%;height:44px;background:white;border:1px solid #dfe2e9;border-radius:9px;padding:0 12px;color:#202538;font:inherit}
.row{display:grid;grid-template-columns:1fr 1fr;gap:12px}.btn{border:0;border-radius:9px;height:42px;padding:0 16px;background:#eeeef4;color:#30364a;font:inherit;font-weight:680;cursor:pointer}
.primary{background:#6558d3;color:white}.danger{background:#fff0f1;color:#b5404b}.btnline{display:flex;align-items:center;gap:11px;margin-top:16px;flex-wrap:wrap}
.msg{font-size:12px;color:#21865a;min-height:18px;margin-top:8px}.msg.bad{color:#b5404b}.hint{font-size:12px;color:#83899a}
.address{font:600 13px ui-monospace,monospace;color:#5046b0}.network{display:flex;justify-content:space-between;align-items:center;border-top:1px solid #eceef2;padding-top:13px;margin-top:14px}
.network strong,.network small{display:block}.network small{color:#7b8294;margin-top:2px}.notice{background:#f3f2ff;color:#625b89;padding:12px;border-radius:9px;font-size:12px;margin-top:14px}
[hidden]{display:none!important}.style-options{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:16px}
.style-option{min-height:56px;border:1px solid #e2e4ec;border-radius:11px;background:#fff;color:#343a4d;font:inherit;font-weight:650;cursor:pointer}
.style-option:hover,.style-option.active{border-color:#7569dc;background:#f3f2ff;color:#4035a3}
.timer-display{font:700 clamp(32px,8vw,48px)/1.2 ui-monospace,SFMono-Regular,Consolas,monospace;letter-spacing:1px;color:#292d43;margin:18px 0}
.timer-type{display:flex;gap:8px;flex-wrap:wrap}.timer-type .btn.active{background:#eeeaff;color:#5046b0;box-shadow:inset 0 0 0 1px #8b81e4}
.game-controls{display:flex;align-items:center;gap:14px;flex-wrap:wrap}.jump-button{min-width:170px;height:58px;font-size:18px;touch-action:manipulation;user-select:none;-webkit-user-select:none}
.game-score{font-weight:750;color:#5046b0}
.tool-panel{scroll-margin-top:88px}
canvas{display:none}@media(max-width:620px){header{padding:0 14px}main{padding:20px 13px 40px}.grid{grid-template-columns:repeat(2,1fr)}.row{grid-template-columns:1fr}.tabs{display:grid;width:100%;grid-template-columns:repeat(3,1fr)}.tab{padding:8px 4px}.card{padding:16px}}
body{min-height:100vh;background:radial-gradient(ellipse at 12% 0%,#eeecff 0,transparent 36%),linear-gradient(180deg,#f8f9fc 0%,#f1f3f8 100%);color:#20243a}
header{position:sticky;top:0;z-index:5;height:72px;background:#ffffffed;backdrop-filter:blur(16px);box-shadow:0 1px 0 #e7e9f0}
.logo{box-shadow:0 5px 12px #6558d344}
.status{display:flex;align-items:center;gap:8px;border:1px solid #e8eaf1;border-radius:999px;background:#fff;padding:7px 12px;font-weight:650}
.status:before{content:"";width:8px;height:8px;border-radius:50%;background:#a4a9b8;box-shadow:0 0 0 3px #a4a9b822}
.status[data-connection="online"]:before{background:#20a66a;box-shadow:0 0 0 3px #20a66a22}
.status[data-connection="connecting"]:before{background:#e5a12b;box-shadow:0 0 0 3px #e5a12b22;animation:status-pulse 1.4s ease-in-out infinite}
@keyframes status-pulse{50%{box-shadow:0 0 0 6px #e5a12b10}}
main{max-width:1040px;padding-top:32px}
.tabs{border:1px solid #e3e5ee;background:#e9eaf1aa;backdrop-filter:blur(8px)}
.tab,.mode,.btn,select,input{transition:background-color .18s ease,border-color .18s ease,color .18s ease,box-shadow .18s ease,transform .18s ease}
.tab,.btn,.mode{cursor:pointer}
.tab:focus-visible,.mode:focus-visible,.btn:focus-visible,select:focus-visible,input:focus-visible{outline:3px solid #8e85ef88;outline-offset:2px}
.panel.active{animation:panel-in .24s ease both}
@keyframes panel-in{from{opacity:0;transform:translateY(5px)}to{opacity:1;transform:translateY(0)}}
h1{font-size:clamp(25px,4vw,32px);letter-spacing:-.9px}
.card{border-color:#e5e7ef;border-radius:17px;padding:22px;box-shadow:0 10px 28px #2025380a,0 1px 2px #20253808}
.card>h2{letter-spacing:-.2px}
.grid{gap:12px}
.mode{min-height:88px;border-color:#e5e7ef;border-radius:13px;box-shadow:0 2px 6px #20253806}
.mode b{width:30px;height:30px;display:grid;place-items:center;border-radius:9px;background:#f1efff;font-size:17px}
.mode:hover{transform:translateY(-2px);border-color:#a9a2ed;box-shadow:0 8px 18px #5046b01a}
.mode.active{border-color:#7569dc;box-shadow:inset 0 0 0 1px #7569dc,0 6px 16px #5046b014}
.btn{min-height:44px;border-radius:10px;box-shadow:0 1px 2px #2025380a}
.btn:hover:not(:disabled){transform:translateY(-1px);filter:brightness(.98);box-shadow:0 5px 12px #20253816}
.btn:active:not(:disabled),.mode:active{transform:translateY(0) scale(.985)}
.btn:disabled{opacity:.6;cursor:wait}
.primary{box-shadow:0 4px 10px #6558d333}
select,input{border-color:#dfe2eb;border-radius:10px}
select:hover,input:hover{border-color:#b8bbcb}
.network{gap:14px}
.address{white-space:nowrap;border:1px solid #e6e3ff;border-radius:9px;background:#f5f3ff;padding:7px 10px}
.notice{border:1px solid #e7e3ff;background:linear-gradient(135deg,#f5f3ff,#f0f3ff)}
.msg{transition:color .18s ease}
@media(max-width:620px){header{height:64px}.status{max-width:58vw;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;padding:6px 9px}main{padding-top:23px}.card{padding:17px;border-radius:14px}.mode{min-height:80px}}
@media(prefers-reduced-motion:reduce){*,*::before,*::after{scroll-behavior:auto!important;animation-duration:.01ms!important;animation-iteration-count:1!important;transition-duration:.01ms!important}}
</style></head><body><header><div class="logo">N</div><div class="brand">NIMO<small>DESK BUDDY</small></div><div class="status" id="status" aria-live="polite">Connecting…</div></header>
<main><nav class="tabs"><button class="tab active" data-tab="home">Overview</button><button class="tab" data-tab="look">Appearance</button><button class="tab" data-tab="device">Device</button></nav>
<section class="panel active" id="home"><h1>Your desk, your buddy.</h1><p>Choose what NIMO should show right now.</p>
<div class="card"><h2>Display mode</h2><p>Choose a mode to open its options or launch it. Appearance also has style controls.</p><div class="grid">
<button class="mode" data-mode="0"><b>◷</b>Clock</button><button class="mode" data-mode="1"><b>☀</b>Weather</button>
<button class="mode" data-mode="2"><b>✳</b>Animation</button><button class="mode" data-mode="3"><b>☺</b>Expression</button>
<button class="mode" data-mode="4"><b>▸</b>Dino game</button><button class="mode" data-mode="5"><b>↻</b>Auto</button>
<button class="mode" data-mode="6"><b>◴</b>Timer</button></div><div class="msg" id="modeMsg"></div></div>
<div class="card tool-panel" id="faceOptions" hidden><h2 id="faceOptionsTitle">Choose a style</h2><p id="faceOptionsHint">Select a style to show on NIMO.</p><div class="style-options" id="faceOptionsList"></div><div class="msg" id="faceMsg"></div></div>
<div class="card tool-panel" id="timerPanel" hidden><h2>Timer & stopwatch</h2><p>Choose a tool, then control it from your phone or the desk buddy touch sensor.</p>
<div class="timer-type"><button class="btn" id="stopwatchTab">Stopwatch</button><button class="btn" id="countdownTab">Countdown</button></div>
<div class="timer-display" id="timerDisplay">00:00.0</div>
<div id="stopwatchControls"><div class="btnline"><button class="btn primary" id="stopwatchToggle">Start</button><button class="btn" id="stopwatchReset">Reset</button></div></div>
<div id="countdownControls" hidden><label for="countdownMinutesInput">Duration (minutes)</label><div class="row"><input id="countdownMinutesInput" type="number" min="1" max="60" step="1" value="5"><button class="btn" id="setCountdown">Set duration</button></div><div class="btnline"><button class="btn primary" id="countdownToggle">Start timer</button><button class="btn" id="countdownReset">Reset</button></div></div>
<div class="msg" id="timerMsg"></div></div>
<div class="card tool-panel" id="gamePanel" hidden><h2>Dino game</h2><p>Tap jump to clear each cactus. The game is shown on NIMO’s display.</p><div class="game-controls"><button class="btn primary jump-button" id="jumpButton">Jump</button><button class="btn" id="restartGame">Restart game</button><span class="game-score" id="gameStatus">Score 0</span></div><div class="msg" id="gameMsg"></div></div></section>
<section class="panel" id="look"><h1>Appearance</h1><p>Explore each collection without mixing its options with other controls.</p><div class="card"><h2>Choose a style</h2><p>Pick a collection, then select one of its styles.</p>
<label for="collection">Collection</label><select id="collection"><option value="clock">Clock faces</option><option value="weather">Weather faces</option><option value="animation">Animations</option><option value="expression">Expressions</option></select>
<label for="style">Style</label><select id="style"></select><div class="btnline"><button class="btn primary" id="applyStyle">Show this style</button></div><div class="msg" id="styleMsg"></div></div></section>
<section class="panel" id="device"><h1>Device settings</h1><p>Connection, local time, and device actions.</p>
<div class="card"><h2>Connection</h2><p id="connectionDescription">Keep this dashboard available on NIMO's setup network while connecting it to your Wi-Fi.</p><div class="network"><div><strong id="networkName">Starting NIMO…</strong><small id="networkHint">Setup network</small></div><span class="address" id="address">192.168.1.4</span></div>
<div id="offline" class="notice">Connect your phone to <strong>NIMO</strong>, password <strong>nimo-setup-24</strong>, then open <strong>http://192.168.1.4</strong>.</div>
<label for="wifiList">Available Wi-Fi networks</label><div class="row"><select id="wifiList"><option value="">Scan to find networks</option></select><button class="btn" id="scanWifi">Scan networks</button></div><div class="hint">NIMO connects to 2.4 GHz Wi-Fi networks.</div>
<label for="wifiPassword">Wi-Fi password</label><input id="wifiPassword" type="password" autocomplete="current-password" placeholder="Enter the selected network password">
<div class="btnline"><button class="btn primary" id="connectWifi">Connect and save</button><span class="hint" id="wifiStatus">Saved network will reconnect automatically.</span></div><div class="msg" id="wifiMsg"></div></div>
<div class="card"><h2>Set date and time</h2><p>Set the local time manually when internet time sync is unavailable.</p><div class="row"><div><label for="date">Date</label><input id="date" type="date"></div><div><label for="time">Time</label><input id="time" type="time"></div></div>
<div class="btnline"><button class="btn primary" id="setTime">Set device time</button><span class="hint" id="timeState">Checking device time…</span></div><div class="msg" id="timeMsg"></div>
<div class="notice">Without internet sync or a battery-backed RTC, the clock keeps time only while powered. Set it again after a power cycle.</div></div>
<div class="card"><h2>Restart device</h2><p>Restart NIMO and reconnect to its dashboard after it comes back online.</p><button class="btn danger" id="reboot">Restart NIMO</button><div class="msg" id="deviceMsg"></div></div></section></main>
<script>
const $=id=>document.getElementById(id);
const choices={clock:['Big 12-hour','Big 24-hour','Split digital','Seconds bar','World clock','Minimal 12-hour'],weather:['Current weather','Weather summary','3-day forecast','Conditions card','Feels-like face','Icon weather'],animation:['Snow','Rain','Clouds','Bubbles','Stars','Hearts','Fireflies','Aurora','Falling leaves','Comets','Fireworks','Spiral','Tunnel','DNA'],expression:['Happy','Love','Surprised','Sleepy','Angry','Sad','Excited','Suspicious','Dizzy','Neutral','Wink','Crying','Laughing','Confused','Bored','Cool','Shy','Starstruck','Scared','Grumpy','Embarrassed','Sneezing']};
choices.animation.push('Orbiting stars','Comet shower','Petal breeze','Heartbeat','Ripple rings','Twinkle field','Orbiting planet','Confetti');
choices.expression.push('Curious','Proud','Nervous','Silly','Focused','Grateful','Playful','Shocked');
const selected={clock:0,weather:0,animation:0,expression:1};
function msg(id,text,bad=false){$(id).textContent=text;$(id).classList.toggle('bad',bad)}
async function post(path,data){const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)});const text=await r.text();if(!r.ok)throw Error(text||('HTTP '+r.status));return text}
function selectMode(value){document.querySelectorAll('.mode').forEach(b=>b.classList.toggle('active',b.dataset.mode===String(value)))}
document.querySelectorAll('.tab').forEach(b=>b.onclick=()=>{document.querySelectorAll('.tab,.panel').forEach(e=>e.classList.remove('active'));b.classList.add('active');$(b.dataset.tab).classList.add('active')});
let timerState={page:0,stopwatchMs:0,stopwatchRunning:false,countdownMs:300000,countdownRunning:false,countdownFinished:false,countdownMinutes:5,stamp:performance.now()};
let countdownInputDirty=false;
function hideModePanels(){['faceOptions','timerPanel','gamePanel'].forEach(id=>$(id).hidden=true)}
function showFaceOptions(kind){const titles={clock:'Clock faces',weather:'Weather faces',animation:'Animations',expression:'Expressions'};const modes={clock:0,weather:1,animation:2,expression:3};hideModePanels();$('faceOptions').hidden=false;$('faceOptionsTitle').textContent=titles[kind];$('faceOptionsHint').textContent='Choose what to display on NIMO.';const list=$('faceOptionsList');list.replaceChildren(...choices[kind].map((name,index)=>{const button=document.createElement('button');button.type='button';button.className='style-option'+(selected[kind]===index?' active':'');button.textContent=name;button.onclick=async()=>{try{msg('faceMsg',await post('/api/control',{command:kind,value:index}));selected[kind]=index;selectMode(modes[kind]);showFaceOptions(kind)}catch(e){msg('faceMsg',e.message,true)}};return button}));$('faceOptions').scrollIntoView({behavior:'smooth',block:'nearest'})}
async function selectModeOnDevice(value){await post('/api/control',{command:'mode',value});selectMode(value)}
document.querySelectorAll('.mode').forEach(b=>b.onclick=async()=>{const mode=Number(b.dataset.mode);msg('modeMsg','');hideModePanels();if(mode>=0&&mode<=3){const kind=['clock','weather','animation','expression'][mode];showFaceOptions(kind);return}if(mode===6){selectMode(mode);$('timerPanel').hidden=false;$('timerPanel').scrollIntoView({behavior:'smooth',block:'nearest'});try{await post('/api/timer',{action:timerState.page===1?'select-countdown':'select-stopwatch'});await refreshStatus()}catch(e){msg('timerMsg',e.message,true)}return}if(mode===4){selectMode(mode);$('gamePanel').hidden=false;$('gamePanel').scrollIntoView({behavior:'smooth',block:'nearest'});try{await selectModeOnDevice(mode);msg('gameMsg','Game ready. Tap Jump whenever a cactus approaches.');refreshStatus()}catch(e){msg('gameMsg',e.message,true)}return}try{await selectModeOnDevice(mode);msg('modeMsg','Display mode changed.')}catch(e){msg('modeMsg',e.message,true)}});
function fillStyles(){const kind=$('collection').value;$('style').replaceChildren(...choices[kind].map((n,i)=>new Option(n,i)));$('style').value=selected[kind]}
$('collection').onchange=fillStyles;
$('applyStyle').onclick=async()=>{const kind=$('collection').value,value=$('style').value;try{msg('styleMsg',await post('/api/control',{command:kind,value}));selected[kind]=Number(value);selectMode(({clock:0,weather:1,animation:2,expression:3})[kind])}catch(e){msg('styleMsg',e.message,true)}};
async function timerAction(action,extra={}){try{msg('timerMsg',await post('/api/timer',{action,...extra}));if(action==='countdown-set')countdownInputDirty=false;await refreshStatus()}catch(e){msg('timerMsg',e.message,true)}}
$('stopwatchTab').onclick=()=>timerAction('select-stopwatch');
$('countdownTab').onclick=()=>timerAction('select-countdown');
$('stopwatchToggle').onclick=()=>timerAction(timerState.stopwatchRunning?'stopwatch-pause':'stopwatch-start');
$('stopwatchReset').onclick=()=>timerAction('stopwatch-reset');
$('countdownToggle').onclick=()=>timerAction(timerState.countdownRunning?'countdown-pause':'countdown-start');
$('countdownReset').onclick=()=>timerAction('countdown-reset');
$('setCountdown').onclick=()=>timerAction('countdown-set',{minutes:$('countdownMinutesInput').value});
$('countdownMinutesInput').oninput=()=>countdownInputDirty=true;
$('jumpButton').onclick=async()=>{try{msg('gameMsg',await post('/api/game',{action:'jump'}));refreshStatus()}catch(e){msg('gameMsg',e.message,true)}};
$('restartGame').onclick=async()=>{try{msg('gameMsg',await post('/api/game',{action:'restart'}));refreshStatus()}catch(e){msg('gameMsg',e.message,true)}};
window.addEventListener('keydown',e=>{if(e.code==='Space'&&!$('gamePanel').hidden&&!['BUTTON','INPUT','SELECT'].includes(document.activeElement.tagName)){e.preventDefault();$('jumpButton').click()}});
function formatStopwatch(ms){const tenths=Math.floor(ms/100)%10,totalSeconds=Math.floor(ms/1000),seconds=totalSeconds%60,minutes=Math.floor(totalSeconds/60)%60,hours=Math.floor(totalSeconds/3600);return hours?`${String(hours).padStart(2,'0')}:${String(minutes).padStart(2,'0')}:${String(seconds).padStart(2,'0')}`:`${String(minutes).padStart(2,'0')}:${String(seconds).padStart(2,'0')}.${tenths}`}
function paintTimer(){const elapsed=performance.now()-timerState.stamp;let ms=timerState.page===0?timerState.stopwatchMs+(timerState.stopwatchRunning?elapsed:0):Math.max(0,timerState.countdownMs-(timerState.countdownRunning?elapsed:0));$('timerDisplay').textContent=timerState.page===0?formatStopwatch(ms):`${String(Math.floor(ms/60000)).padStart(2,'0')}:${String(Math.ceil(ms/1000)%60).padStart(2,'0')}`;$('stopwatchToggle').textContent=timerState.stopwatchRunning?'Pause':'Start';$('countdownToggle').textContent=timerState.countdownRunning?'Pause':timerState.countdownFinished?'Start again':'Start timer';$('stopwatchTab').classList.toggle('active',timerState.page===0);$('countdownTab').classList.toggle('active',timerState.page===1);$('stopwatchControls').hidden=timerState.page!==0;$('countdownControls').hidden=timerState.page!==1}
setInterval(paintTimer,100);
let timeInputDirty=false;
$('setTime').onclick=async()=>{if(!$('date').value||!$('time').value){msg('timeMsg','Select both the date and time.',true);return}try{msg('timeMsg',await post('/api/time',{date:$('date').value,time:$('time').value}));timeInputDirty=false;$('timeState').textContent='Device time is set.';refreshStatus()}catch(e){msg('timeMsg',e.message,true)}};
$('date').oninput=$('time').oninput=()=>timeInputDirty=true;
$('reboot').onclick=async()=>{if(!confirm('Restart NIMO now?'))return;try{msg('deviceMsg',await post('/api/reboot',{}));$('reboot').disabled=true}catch(e){msg('deviceMsg',e.message,true)}};
async function scanNetworks(){const button=$('scanWifi');button.disabled=true;$('wifiList').replaceChildren(new Option('Scanning for networks…',''));msg('wifiMsg','Searching nearby Wi-Fi networks…');try{let result;do{const r=await fetch('/api/wifi/scan');result=await r.json();if(!r.ok)throw Error(result.error||('HTTP '+r.status));if(result.scanning)await new Promise(resolve=>setTimeout(resolve,1200))}while(result.scanning);if(result.error)throw Error(result.error);if(!result.networks.length){$('wifiList').replaceChildren(new Option('No networks found — scan again',''));msg('wifiMsg','No networks were found. Make sure NIMO is not too far from the router.',true);return}const options=result.networks.map(n=>{const label=`${n.ssid}${n.secured?'  ·  secured':''}  ·  ${n.rssi} dBm`;return new Option(label,n.ssid)});$('wifiList').replaceChildren(...options);msg('wifiMsg',`${options.length} network${options.length===1?'':'s'} found. Choose yours.`)}catch(e){$('wifiList').replaceChildren(new Option('Scan to find networks',''));msg('wifiMsg',e.message,true)}finally{button.disabled=false}}
$('scanWifi').onclick=scanNetworks;
$('connectWifi').onclick=async()=>{const selectedSsid=$('wifiList').value;const selectedPassword=$('wifiPassword').value;if(!selectedSsid){msg('wifiMsg','Scan and select a Wi-Fi network first.',true);return}try{msg('wifiMsg',await post('/api/wifi/connect',{ssid:selectedSsid,password:selectedPassword}));$('wifiStatus').textContent='Connecting. NIMO setup Wi-Fi stays available.'}catch(e){msg('wifiMsg',e.message,true)}};
function loadStatus(s){
  $('status').textContent=s.online?'Connected to '+s.ssid:s.connecting?'Connecting to Wi-Fi...':s.ap?'NIMO setup Wi-Fi active':'Wi-Fi unavailable';
  $('status').dataset.connection=s.online?'online':s.connecting?'connecting':'offline';
  $('networkName').textContent=s.online?(s.ssid||'Wi-Fi connected'):s.ap?'NIMO setup network':'Wi-Fi unavailable';
  $('networkHint').textContent=s.online?'Home Wi-Fi address - open this IP on the same network':s.ap?'Setup Wi-Fi is ready while NIMO connects':'Check device Wi-Fi';
  $('address').textContent=s.online&&s.stationIp?s.stationIp:s.ap?(s.apIp||'192.168.1.4'):'-';
  $('offline').hidden=s.online||!s.ap;
  $('connectionDescription').textContent=s.online?'Use this home Wi-Fi address from a device on the same network. NIMO setup Wi-Fi remains available for recovery.':'NIMO setup Wi-Fi stays available while you connect it to your home Wi-Fi.';
  if(s.wifiMessage){const statusMessage=s.wifiMessage.toLowerCase();msg('wifiMsg',s.wifiMessage,statusMessage.includes('failed')||statusMessage.includes('could not save'))}
  if(s.online){$('wifiStatus').textContent=s.wifiMessage.startsWith('Connected and saved.')?'Network saved. NIMO reconnects automatically when it is in range.':'Connected to '+s.ssid+'.';if(s.wifiMessage.startsWith('Connected and saved.'))$('wifiPassword').value=''}
  selectMode(s.mode);selected.clock=s.clock;selected.weather=s.weather;selected.animation=s.animation;selected.expression=s.expression;
  timerState={page:s.timerPage,stopwatchMs:s.stopwatchMs,stopwatchRunning:s.stopwatchRunning,countdownMs:s.countdownMs,countdownRunning:s.countdownRunning,countdownFinished:s.countdownFinished,countdownMinutes:s.countdownMinutes,stamp:performance.now()};
  if(!countdownInputDirty&&document.activeElement!==$('countdownMinutesInput'))$('countdownMinutesInput').value=s.countdownMinutes;
  paintTimer();$('gameStatus').textContent=s.gameOver?'Game over - score '+s.gameScore:'Score '+s.gameScore;$('jumpButton').disabled=s.gameOver;$('jumpButton').textContent=s.gameOver?'Game over':'Jump';fillStyles();
  if(s.timeValid&&!timeInputDirty){$('date').value=s.date;$('time').value=s.time;$('timeState').textContent=s.date+' '+s.time}else if(!s.timeValid)$('timeState').textContent='Time is not synchronized.'
}
function refreshStatus(){return fetch('/api/status').then(r=>{if(!r.ok)throw Error('HTTP '+r.status);return r.json()}).then(loadStatus).catch(e=>{$('status').textContent='Device unavailable';$('status').dataset.connection='offline';msg('modeMsg',e.message,true)})}
refreshStatus();setInterval(refreshStatus,4000);
</script></body></html>
)rawliteral";

bool readControlValue(int& value) {
  if (!controlServer.hasArg("value")) return false;
  String text = controlServer.arg("value");
  if (text.length() == 0) return false;
  char* end = nullptr;
  long parsed = strtol(text.c_str(), &end, 10);
  if (*end != '\0' || parsed < 0 || parsed > 1000) return false;
  value = (int)parsed;
  return true;
}

void handleControlPage() {
  controlServer.send_P(200, "text/html; charset=utf-8", CONTROL_PAGE);
}

void appendJsonString(String& output, const String& value) {
  output += '"';
  for (size_t i = 0; i < value.length(); i++) {
    uint8_t c = (uint8_t)value[i];
    if (c == '"' || c == '\\') {
      output += '\\';
      output += (char)c;
    } else if (c < 0x20) {
      char escaped[7];
      snprintf(escaped, sizeof(escaped), "\\u%04x", c);
      output += escaped;
    } else {
      output += (char)c;
    }
  }
  output += '"';
}

void handleControlStatus() {
  struct tm localTime;
  bool timeValid = getLocalTime(&localTime, 10) && localTime.tm_year >= 120;
  unsigned long stopwatchMs = stopwatchElapsedMs;
  if (stopwatchRunning) stopwatchMs += millis() - stopwatchStartedAt;
  unsigned long countdownMs = countdownRemainingMs;
  if (countdownRunning) {
    unsigned long elapsed = millis() - countdownStartedAt;
    countdownMs = elapsed >= countdownRemainingMs ? 0 : countdownRemainingMs - elapsed;
  }
  char dateText[11] = "";
  char timeText[6] = "";
  if (timeValid) {
    strftime(dateText, sizeof(dateText), "%Y-%m-%d", &localTime);
    strftime(timeText, sizeof(timeText), "%H:%M", &localTime);
  }
  String json = "{\"mode\":" + String((int)currentMode)
    + ",\"expression\":" + String(selectedExpression)
    + ",\"animation\":" + String(selectedAnimation)
    + ",\"clock\":" + String(clockStyle)
    + ",\"weather\":" + String(weatherStyle)
    + ",\"timerPage\":" + String(timerPage)
    + ",\"stopwatchMs\":" + String(stopwatchMs)
    + ",\"stopwatchRunning\":" + String(stopwatchRunning ? "true" : "false")
    + ",\"countdownMs\":" + String(countdownMs)
    + ",\"countdownRunning\":" + String(countdownRunning ? "true" : "false")
    + ",\"countdownFinished\":" + String(countdownFinished ? "true" : "false")
    + ",\"countdownMinutes\":" + String(countdownMinutes)
    + ",\"gameScore\":" + String(gameScore)
    + ",\"gameOver\":" + String(gameOver ? "true" : "false")
    + ",\"online\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false")
    + ",\"ap\":" + String(accessPointMode ? "true" : "false")
    + ",\"connecting\":" + String(wifiConnectPending ? "true" : "false")
    + ",\"ssid\":";
  appendJsonString(json, WiFi.status() == WL_CONNECTED ? WiFi.SSID() : configuredWifiSsid);
  json += ",\"stationIp\":";
  appendJsonString(json, WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : String(""));
  json += ",\"apIp\":";
  appendJsonString(json, accessPointMode ? WiFi.softAPIP().toString() : String(""));
  json += ",\"wifiMessage\":";
  appendJsonString(json, wifiConnectionMessage);
  json += ",\"timeValid\":" + String(timeValid ? "true" : "false")
    + ",\"date\":\"" + dateText + "\",\"time\":\"" + timeText + "\"}";
  controlServer.send(200, "application/json", json);
}

void handleWifiScan() {
  if (!wifiScanInProgress) {
    WiFi.scanDelete();
    int startResult = WiFi.scanNetworks(true, false);
    if (startResult == WIFI_SCAN_FAILED) {
      controlServer.send(503, "application/json", "{\"scanning\":false,\"error\":\"Could not start Wi-Fi scan.\"}");
      return;
    }
    wifiScanInProgress = true;
  }

  int networkCount = WiFi.scanComplete();
  if (networkCount == WIFI_SCAN_RUNNING) {
    controlServer.send(200, "application/json", "{\"scanning\":true}");
    return;
  }
  wifiScanInProgress = false;
  if (networkCount == WIFI_SCAN_FAILED) {
    WiFi.scanDelete();
    controlServer.send(503, "application/json", "{\"scanning\":false,\"error\":\"Wi-Fi scan failed. Please try again.\"}");
    return;
  }

  String json = "{\"scanning\":false,\"networks\":[";
  bool first = true;
  for (int i = 0; i < networkCount; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) continue;
    if (!first) json += ',';
    first = false;
    json += "{\"ssid\":";
    appendJsonString(json, ssid);
    json += ",\"rssi\":" + String(WiFi.RSSI(i));
    json += ",\"secured\":" + String(WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "false" : "true") + "}";
  }
  json += "]}";
  WiFi.scanDelete();
  controlServer.send(200, "application/json", json);
}

void handleWifiConnect() {
  if (!controlServer.hasArg("ssid")) {
    controlServer.send(400, "text/plain", "Choose a Wi-Fi network first.");
    return;
  }
  String ssid = controlServer.arg("ssid");
  String password = controlServer.arg("password");
  if (ssid.length() == 0 || ssid.length() > 32 ||
      password.length() > 63 ||
      (password.length() > 0 && password.length() < 8)) {
    controlServer.send(400, "text/plain", "SSID must be 1–32 characters; Wi-Fi passwords must be empty or 8–63 characters.");
    return;
  }
  if (wifiConnectPending) {
    controlServer.send(409, "text/plain", "A Wi-Fi connection attempt is already in progress.");
    return;
  }

  if (wifiScanInProgress) {
    WiFi.scanDelete();
    wifiScanInProgress = false;
  }
  pendingWifiSsid = ssid;
  pendingWifiPassword = password;
  wifiConnectPending = true;
  wifiConnectStartedAt = millis();
  wifiConnectionMessage = "Connecting to " + ssid + "…";
  stationWasConnected = WiFi.status() == WL_CONNECTED;
  WiFi.disconnect(false, false);
  if (password.length() == 0) WiFi.begin(pendingWifiSsid.c_str());
  else WiFi.begin(pendingWifiSsid.c_str(), pendingWifiPassword.c_str());
  controlServer.send(202, "text/plain", "Connecting to " + ssid + ". Keep the NIMO setup network connected.");
}

void handleControlCommand() {
  int value;
  if (!readControlValue(value)) {
    controlServer.send(400, "text/plain", "Missing or invalid control value.");
    return;
  }

  String command = controlServer.arg("command");
  if (command == "mode" && value <= MODE_TIMER) {
    currentMode = (DeskMode)value;
    menuOpen = false;
    menuDepth = 0;
    if (currentMode == MODE_AUTO) {
      autoModePage = 0;
      autoModeStarted = millis();
    } else if (currentMode == MODE_GAMES) {
      dinoY = 48;
      dinoVelocityY = 0;
      obstacleX = SCREEN_WIDTH;
      gameScore = 0;
      gameOver = false;
      lastGameUpdate = millis();
    }
  } else if (command == "expression" && value < FACE_EXPRESSION_COUNT) {
    setFaceExpression(value);
    currentMode = MODE_FACE_EXPRESSIONS;
    settings.putInt("expression", selectedExpression);
  } else if (command == "animation" && value < ANIMATION_SCENE_COUNT) {
    selectedAnimation = value;
    animationSceneIndex = value;
    currentMode = MODE_ANIMATION;
    resetSceneParticles();
    settings.putInt("animation", selectedAnimation);
  } else if (command == "clock" && value < 6) {
    clockStyle = value;
    currentMode = MODE_CLOCK_FACES;
    settings.putInt("clock", clockStyle);
  } else if (command == "weather" && value < 6) {
    weatherStyle = value;
    currentMode = MODE_WEATHER_FACE;
    settings.putInt("weather", weatherStyle);
  } else {
    controlServer.send(400, "text/plain", "Unknown command or out-of-range value.");
    return;
  }
  menuOpen = false;
  menuDepth = 0;
  controlServer.send(200, "text/plain", "Control applied.");
}

void handleTimerCommand() {
  if (!controlServer.hasArg("action")) {
    controlServer.send(400, "text/plain", "Timer action is required.");
    return;
  }

  String action = controlServer.arg("action");
  if (action == "select-stopwatch") {
    currentMode = MODE_TIMER;
    timerPage = 0;
  } else if (action == "select-countdown") {
    currentMode = MODE_TIMER;
    timerPage = 1;
  } else if (action == "stopwatch-start") {
    if (!stopwatchRunning) {
      stopwatchStartedAt = millis();
      stopwatchRunning = true;
    }
    timerPage = 0;
    currentMode = MODE_TIMER;
  } else if (action == "stopwatch-pause") {
    if (stopwatchRunning) {
      stopwatchElapsedMs += millis() - stopwatchStartedAt;
      stopwatchRunning = false;
    }
    timerPage = 0;
    currentMode = MODE_TIMER;
  } else if (action == "stopwatch-reset") {
    stopwatchRunning = false;
    stopwatchElapsedMs = 0;
    stopwatchStartedAt = 0;
    timerPage = 0;
    currentMode = MODE_TIMER;
  } else if (action == "countdown-start") {
    if (!countdownRunning) {
      if (countdownFinished || countdownRemainingMs == 0) resetCountdownToSetting();
      countdownStartedAt = millis();
      countdownRunning = true;
      countdownFinished = false;
    }
    timerPage = 1;
    currentMode = MODE_TIMER;
  } else if (action == "countdown-pause") {
    if (countdownRunning) {
      unsigned long elapsed = millis() - countdownStartedAt;
      countdownRemainingMs = elapsed >= countdownRemainingMs ? 0 : countdownRemainingMs - elapsed;
      countdownRunning = false;
      countdownFinished = countdownRemainingMs == 0;
    }
    timerPage = 1;
    currentMode = MODE_TIMER;
  } else if (action == "countdown-reset") {
    resetCountdownToSetting();
    timerPage = 1;
    currentMode = MODE_TIMER;
  } else if (action == "countdown-set") {
    if (!controlServer.hasArg("minutes")) {
      controlServer.send(400, "text/plain", "Countdown duration is required.");
      return;
    }
    String minutesText = controlServer.arg("minutes");
    char* end = nullptr;
    long minutes = strtol(minutesText.c_str(), &end, 10);
    if (minutesText.length() == 0 || *end != '\0' || minutes < 1 || minutes > 60) {
      controlServer.send(400, "text/plain", "Countdown duration must be from 1 to 60 minutes.");
      return;
    }
    countdownMinutes = (int)minutes;
    resetCountdownToSetting();
    timerPage = 1;
    currentMode = MODE_TIMER;
  } else {
    controlServer.send(400, "text/plain", "Unknown timer action.");
    return;
  }

  menuOpen = false;
  menuDepth = 0;
  controlServer.send(200, "text/plain", "Timer updated.");
}

void handleGameCommand() {
  if (!controlServer.hasArg("action")) {
    controlServer.send(400, "text/plain", "Game action is required.");
    return;
  }
  String action = controlServer.arg("action");
  if (action == "restart") {
    currentMode = MODE_GAMES;
    dinoY = 48;
    dinoVelocityY = 0;
    obstacleX = SCREEN_WIDTH;
    gameScore = 0;
    gameOver = false;
    lastGameUpdate = millis();
  } else if (action == "jump") {
    if (currentMode != MODE_GAMES) {
      controlServer.send(409, "text/plain", "Select the Dino game before jumping.");
      return;
    }
    if (gameOver) {
      controlServer.send(409, "text/plain", "Game over. Restart the game to play again.");
      return;
    }
    jumpDino();
  } else {
    controlServer.send(400, "text/plain", "Unknown game action.");
    return;
  }
  menuOpen = false;
  menuDepth = 0;
  controlServer.send(200, "text/plain", action == "jump" ? "Jump!" : "Game restarted.");
}

void handleSetDeviceTime() {
  if (!controlServer.hasArg("date") || !controlServer.hasArg("time")) {
    controlServer.send(400, "text/plain", "Date and time are required.");
    return;
  }

  String dateValue = controlServer.arg("date");
  String timeValue = controlServer.arg("time");
  int year, month, day, hour, minute;
  bool numericDateTime = dateValue.length() == 10 && timeValue.length() == 5;
  for (size_t i = 0; numericDateTime && i < dateValue.length(); i++) {
    if (i != 4 && i != 7 && (dateValue[i] < '0' || dateValue[i] > '9')) {
      numericDateTime = false;
    }
  }
  for (size_t i = 0; numericDateTime && i < timeValue.length(); i++) {
    if (i != 2 && (timeValue[i] < '0' || timeValue[i] > '9')) {
      numericDateTime = false;
    }
  }
  if (!numericDateTime ||
      sscanf(dateValue.c_str(), "%4d-%2d-%2d", &year, &month, &day) != 3 ||
      sscanf(timeValue.c_str(), "%2d:%2d", &hour, &minute) != 2 ||
      dateValue[4] != '-' || dateValue[7] != '-' || timeValue[2] != ':' ||
      year < 2020 || year > 2099 || month < 1 || month > 12 ||
      hour < 0 || hour > 23 || minute < 0 || minute > 59) {
    controlServer.send(400, "text/plain", "Invalid date or time.");
    return;
  }

  static const uint8_t daysPerMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  int maxDay = daysPerMonth[month - 1];
  bool leapYear = (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
  if (month == 2 && leapYear) maxDay = 29;
  if (day < 1 || day > maxDay) {
    controlServer.send(400, "text/plain", "Invalid calendar date.");
    return;
  }

  struct tm local = {};
  local.tm_year = year - 1900;
  local.tm_mon = month - 1;
  local.tm_mday = day;
  local.tm_hour = hour;
  local.tm_min = minute;
  local.tm_isdst = -1;
  time_t epoch = mktime(&local);
  if (epoch < 0) {
    controlServer.send(400, "text/plain", "Could not convert local date and time.");
    return;
  }
  struct timeval now = {};
  now.tv_sec = epoch;
  if (settimeofday(&now, nullptr) != 0) {
    controlServer.send(500, "text/plain", "Could not set device time.");
    return;
  }
  controlServer.send(200, "text/plain", "Device date and time updated.");
}

void handleReboot() {
  controlServer.send(200, "text/plain", "Restarting NIMO…");
  delay(250);
  ESP.restart();
}

void startControlServer() {
  controlServer.on("/", HTTP_GET, handleControlPage);
  controlServer.on("/api/status", HTTP_GET, handleControlStatus);
  controlServer.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  controlServer.on("/api/wifi/connect", HTTP_POST, handleWifiConnect);
  controlServer.on("/api/control", HTTP_POST, handleControlCommand);
  controlServer.on("/api/timer", HTTP_POST, handleTimerCommand);
  controlServer.on("/api/game", HTTP_POST, handleGameCommand);
  controlServer.on("/api/time", HTTP_POST, handleSetDeviceTime);
  controlServer.on("/api/reboot", HTTP_POST, handleReboot);
  controlServer.onNotFound([]() {
    controlServer.send(404, "text/plain", "Not found.");
  });
  controlServer.begin();
  Serial.println("Local control panel started on port 80.");
}

// ==================================================
// DRAW FUNCTIONS
// ==================================================
void drawBlinkMask(int ix, int iy, int iw, int ih);

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

  drawBlinkMask(ix, iy, iw, ih);
  int starOffset = (now / 80) % 4;
  display.drawBitmap(6 - starOffset, 0, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
  display.drawBitmap(106 + starOffset, 0, bmp_dizzy_stars, 16, 16, SSD1306_WHITE);
}

void drawBlinkMask(int ix, int iy, int iw, int ih) {
  if (blinkAmount <= 0) return;
  const int covered = constrain((int)(ih * blinkAmount + 0.5f), 0, ih);
  if (covered >= ih) {
    display.fillRoundRect(ix, iy, iw, ih, min(9, ih / 2), SSD1306_BLACK);
    return;
  }
  const int topCover = (covered * 9 + 9) / 10;
  const int bottomCover = covered - topCover;
  if (topCover > 0)
    display.fillRoundRect(ix, iy, iw, topCover, min(iw, topCover) / 2, SSD1306_BLACK);
  if (bottomCover > 0)
    display.fillRoundRect(ix, iy + ih - bottomCover, iw, bottomCover,
                          min(iw, bottomCover) / 2, SSD1306_BLACK);
}

void drawHeartEye(int cx, int cy, int size) {
  const int radius = max(2, size / 2);
  display.fillCircle(cx - radius, cy - radius / 2, radius, SSD1306_WHITE);
  display.fillCircle(cx + radius, cy - radius / 2, radius, SSD1306_WHITE);
  display.fillTriangle(cx - size, cy - radius / 3,
                       cx + size, cy - radius / 3,
                       cx, cy + size, SSD1306_WHITE);
}

void drawAngryEye(bool isLeft) {
  Eye& e = isLeft ? leftEye : rightEye;
  int ix = (int)e.x, iy = (int)e.y, iw = (int)e.w, ih = (int)e.h;

  display.fillRoundRect(ix, iy, iw, ih, min(8, ih / 2), SSD1306_WHITE);
  int cx = ix + iw / 2, cy = iy + ih / 2;
  const int pupilW = max(8, iw / 3);
  const int pupilH = max(9, ih / 2);
  const int px = constrain(cx + (int)e.pupilX - pupilW / 2, ix + 2, ix + iw - pupilW - 2);
  const int py = constrain(cy + (int)e.pupilY - pupilH / 2, iy + 2, iy + ih - pupilH - 2);
  display.fillRoundRect(px, py, pupilW, pupilH, min(pupilW, pupilH) / 2, SSD1306_BLACK);
  display.drawPixel(px + pupilW - 3, py + 3, SSD1306_WHITE);

  const int slant = max(6, ih / 3 + (int)(2.0f * sin(millis() / 170.0f)));
  if (isLeft) {
    display.fillTriangle(ix - 1, iy - 1, ix + iw + 1, iy - 1,
                         ix + iw + 1, iy + slant, SSD1306_BLACK);
    display.drawBitmap(ix - 12, iy - 6, bmp_angry_mark, 16, 16, SSD1306_WHITE);
  } else {
    display.fillTriangle(ix - 1, iy - 1, ix + iw + 1, iy - 1,
                         ix - 1, iy + slant, SSD1306_BLACK);
    display.drawBitmap(ix + iw - 4, iy - 6, bmp_angry_mark, 16, 16, SSD1306_WHITE);
  }
  drawBlinkMask(ix, iy, iw, ih);
}

void drawClosedEye(int ix, int cy, int width, int curve) {
  const int halfWidth = max(1, width / 2);
  int previousX = ix;
  int previousY = cy + curve;
  for (int x = 1; x <= width; x++) {
    const int offset = x - halfWidth;
    const int y = cy + curve - (curve * offset * offset) / (halfWidth * halfWidth);
    const int nextX = ix + x;
    display.drawLine(previousX, previousY, nextX, y, SSD1306_WHITE);
    display.drawLine(previousX, previousY + 1, nextX, y + 1, SSD1306_WHITE);
    previousX = nextX;
    previousY = y;
  }
}

void drawNormalEye(bool isLeft) {
  Eye& e = isLeft ? leftEye : rightEye;
  const int ix = (int)e.x, iy = (int)e.y, iw = max(8, (int)e.w), ih = max(8, (int)e.h);
  const int cx = ix + iw / 2, cy = iy + ih / 2;

  if (currentMood == MOOD_LOVE) {
    const int heartSize = 12 + (int)(2.0f * sin(millis() / 220.0f));
    drawHeartEye(cx + (int)(e.pupilX * 0.25f), cy + (int)(e.pupilY * 0.25f), heartSize);
    drawBlinkMask(ix, iy, iw, ih);
    return;
  }

  const bool closed =
    (currentMood == MOOD_WINK && isLeft) ||
    (currentMood == MOOD_SILLY && !isLeft) ||
    currentMood == MOOD_LAUGHING || currentMood == MOOD_GRATEFUL ||
    currentMood == MOOD_SNEEZING;
  if (closed) {
    drawClosedEye(ix + 2, cy, iw - 4, currentMood == MOOD_SNEEZING ? -3 : 3);
    drawBlinkMask(ix, iy, iw, ih);
    return;
  }

  const int radius = min(9, ih / 2);
  display.fillRoundRect(ix, iy, iw, ih, radius, SSD1306_WHITE);

  int pupilW = max(7, iw / 3);
  int pupilH = max(8, ih / 2);
  if (currentMood == MOOD_CURIOUS || currentMood == MOOD_SHOCKED) {
    pupilW = max(9, iw / 2);
    pupilH = max(10, ih / 2 + 2);
  } else if (currentMood == MOOD_SCARED || currentMood == MOOD_NERVOUS) {
    pupilW = max(6, iw / 4);
    pupilH = max(7, ih / 3);
  } else if (currentMood == MOOD_SUSPICIOUS || currentMood == MOOD_BORED ||
             currentMood == MOOD_GRUMPY || currentMood == MOOD_FOCUSED ||
             currentMood == MOOD_SLEEPY) {
    pupilH = max(5, ih / 3);
  }
  const int pupilX = constrain(cx + (int)e.pupilX - pupilW / 2,
                               ix + 2, ix + iw - pupilW - 2);
  const int pupilY = constrain(cy + (int)e.pupilY - pupilH / 2,
                               iy + 2, iy + ih - pupilH - 2);
  display.fillRoundRect(pupilX, pupilY, pupilW, pupilH,
                        min(pupilW, pupilH) / 2, SSD1306_BLACK);
  if (pupilW >= 8 && pupilH >= 8) {
    display.fillCircle(pupilX + pupilW - 3, pupilY + 3, 1, SSD1306_WHITE);
    if (pupilH > 11) display.drawPixel(pupilX + pupilW - 2, pupilY + 5, SSD1306_WHITE);
  }

  if (currentMood == MOOD_SUSPICIOUS) {
    display.drawLine(ix + 3, iy - 2, ix + iw - 3, iy - 2 + (isLeft ? -3 : 1), SSD1306_WHITE);
  } else if (currentMood == MOOD_CONFUSED) {
    display.drawLine(ix + 2, iy - 3 + (isLeft ? 2 : 0), cx, iy - 5, SSD1306_WHITE);
    display.drawLine(cx, iy - 5, ix + iw - 2, iy - 3 + (isLeft ? 0 : 2), SSD1306_WHITE);
  } else if (currentMood == MOOD_CURIOUS) {
    display.drawLine(ix + 3, iy - 2 + (isLeft ? 0 : 3),
                     ix + iw - 3, iy - 2 + (isLeft ? 3 : 0), SSD1306_WHITE);
  } else if (currentMood == MOOD_FOCUSED || currentMood == MOOD_PROUD ||
             currentMood == MOOD_ANGRY || currentMood == MOOD_GRUMPY) {
    const int browDrop = currentMood == MOOD_FOCUSED ? 3 : 1;
    display.drawLine(ix + 2, iy - 2 + browDrop, cx, iy - 4, SSD1306_WHITE);
    display.drawLine(cx, iy - 4, ix + iw - 2, iy - 2, SSD1306_WHITE);
  } else if (currentMood == MOOD_NERVOUS) {
    display.drawLine(ix + 3, iy - 1, cx, iy - 4, SSD1306_WHITE);
    display.drawLine(cx, iy - 4, ix + iw - 3, iy - 1, SSD1306_WHITE);
  } else if (currentMood == MOOD_SCARED || currentMood == MOOD_SHOCKED) {
    display.drawLine(ix + 4, iy - 3, ix + iw - 4, iy - 3, SSD1306_WHITE);
  }

  if (currentMood == MOOD_HAPPY || currentMood == MOOD_EXCITED ||
      currentMood == MOOD_PLAYFUL || currentMood == MOOD_STARSTRUCK) {
    display.drawLine(ix + 3, iy + ih - 4, cx, iy + ih - 1, SSD1306_BLACK);
    display.drawLine(cx, iy + ih - 1, ix + iw - 3, iy + ih - 4, SSD1306_BLACK);
  } else if (currentMood == MOOD_SLEEPY || currentMood == MOOD_BORED ||
             currentMood == MOOD_FOCUSED) {
    const float sway = currentMood == MOOD_SLEEPY
      ? 0.5f + 0.5f * sin(millis() / 520.0f) : 0.0f;
    const int droop = currentMood == MOOD_SLEEPY
      ? (int)(ih * (0.25f + 0.12f * sway))
      : (currentMood == MOOD_FOCUSED ? ih / 5 : ih / 6);
    display.fillRect(ix, iy, iw, droop, SSD1306_BLACK);
    display.drawLine(ix + 2, iy + droop, cx, iy + droop + 2, SSD1306_WHITE);
    display.drawLine(cx, iy + droop + 2, ix + iw - 2, iy + droop, SSD1306_WHITE);
  } else if (currentMood == MOOD_GRUMPY) {
    display.fillRect(ix, iy, iw, ih / 4, SSD1306_BLACK);
  } else if (currentMood == MOOD_SAD || currentMood == MOOD_CRYING) {
    const int slant = max(5, ih / 3 + (int)(2.0f * sin(millis() / 420.0f)));
    if (isLeft)
      display.fillTriangle(ix - 1, iy - 1, ix + iw + 1, iy - 1,
                           ix - 1, iy + slant, SSD1306_BLACK);
    else
      display.fillTriangle(ix - 1, iy - 1, ix + iw + 1, iy - 1,
                           ix + iw + 1, iy + slant, SSD1306_BLACK);
  }

  if (currentMood == MOOD_STARSTRUCK || currentMood == MOOD_PLAYFUL) {
    const int sparkleX = isLeft ? ix + 6 : ix + iw - 6;
    display.drawLine(sparkleX - 2, cy, sparkleX + 2, cy, SSD1306_WHITE);
    display.drawLine(sparkleX, cy - 2, sparkleX, cy + 2, SSD1306_WHITE);
  }

  drawBlinkMask(ix, iy, iw, ih);
}

void drawMouthCurve(int cx, int cy, int halfWidth, int curve, int thickness = 2) {
  const int widthSquared = halfWidth * halfWidth;
  int previousX = cx - halfWidth;
  int previousY = cy + curve;
  for (int x = -halfWidth + 1; x <= halfWidth; x++) {
    const int y = cy + (curve * x * x) / widthSquared;
    for (int line = 0; line < thickness; line++)
      display.drawLine(previousX, previousY + line, cx + x, y + line, SSD1306_WHITE);
    previousX = cx + x;
    previousY = y;
  }
}

void drawOpenMouth(int cx, int cy, int width, int height, bool tongue = false) {
  const int radius = min(width, height) / 2;
  display.fillRoundRect(cx - width / 2, cy - height / 2, width, height, radius, SSD1306_WHITE);
  if (width > 5 && height > 5)
    display.fillRoundRect(cx - width / 2 + 2, cy - height / 2 + 2,
                          width - 4, height - 4, max(1, radius - 2), SSD1306_BLACK);
  if (tongue && width > 8 && height > 7)
    display.fillRoundRect(cx + 1, cy + height / 2 - 5, width / 3, 4, 2, SSD1306_WHITE);
}

void drawMouth() {
  const int mx = 64;
  const int midY = 54;
  const float pulse = sin(millis() / 190.0f);
  switch (currentMood) {
    case MOOD_HAPPY:
      drawMouthCurve(mx, midY, 10, 4);
      break;
    case MOOD_LOVE:
      drawMouthCurve(mx, midY, 8, 3);
      break;
    case MOOD_WINK:
      drawMouthCurve(mx + 2, midY, 8, 4);
      break;
    case MOOD_SHY:
      drawMouthCurve(mx, midY, 7, 2);
      break;
    case MOOD_EMBARRASSED:
      drawMouthCurve(mx, midY + 1, 6, 1);
      display.drawPixel(mx - 7, midY + 2, SSD1306_WHITE);
      display.drawPixel(mx + 7, midY + 2, SSD1306_WHITE);
      break;
    case MOOD_PROUD:
      drawMouthCurve(mx + 1, midY, 8, 3);
      display.drawPixel(mx + 9, midY, SSD1306_WHITE);
      break;
    case MOOD_GRATEFUL:
      drawMouthCurve(mx, midY, 8, 3);
      break;
    case MOOD_PLAYFUL:
      drawMouthCurve(mx, midY - 1, 8, 3);
      display.fillRoundRect(mx + 1, midY + 2, 5, 4, 2, SSD1306_WHITE);
      break;
    case MOOD_EXCITED:
      drawOpenMouth(mx, midY, 17, 12 + (int)(2.0f * pulse));
      display.drawLine(mx - 5, midY - 3, mx + 5, midY - 3, SSD1306_WHITE);
      break;
    case MOOD_LAUGHING:
      drawOpenMouth(mx, midY, 19, 13 + (int)(2.0f * pulse));
      display.drawLine(mx - 6, midY - 3, mx + 6, midY - 3, SSD1306_WHITE);
      break;
    case MOOD_SILLY:
      drawOpenMouth(mx, midY, 13, 11, true);
      break;
    case MOOD_SAD:
      drawMouthCurve(mx, midY, 9, -4);
      break;
    case MOOD_CRYING:
      drawMouthCurve(mx, midY, 8, -4);
      display.drawLine(mx - 4, midY - 1, mx + 4, midY - 1, SSD1306_WHITE);
      break;
    case MOOD_SURPRISED:
      drawOpenMouth(mx, midY, 8, 12);
      break;
    case MOOD_SCARED:
      drawOpenMouth(mx, midY, 11, 14);
      break;
    case MOOD_SHOCKED:
      drawOpenMouth(mx, midY, 13, 15 + (int)(pulse > 0.6f));
      break;
    case MOOD_SLEEPY:
      drawOpenMouth(mx, midY + 1, 8, 4 + (int)(3.0f * (0.5f + 0.5f * pulse)));
      break;
    case MOOD_ANGRY:
      drawMouthCurve(mx, midY, 8, -2, 2);
      break;
    case MOOD_SUSPICIOUS:
      display.drawLine(mx - 7, midY + 1, mx + 5, midY - 1, SSD1306_WHITE);
      display.drawLine(mx - 5, midY + 2, mx + 5, midY, SSD1306_WHITE);
      break;
    case MOOD_COOL:
      display.drawLine(mx - 7, midY + 1, mx + 7, midY + 1, SSD1306_WHITE);
      break;
    case MOOD_DIZZY:
      display.drawLine(mx - 8, midY, mx - 4, midY + 2, SSD1306_WHITE);
      display.drawLine(mx - 4, midY + 2, mx, midY - 1, SSD1306_WHITE);
      display.drawLine(mx, midY - 1, mx + 4, midY + 2, SSD1306_WHITE);
      display.drawLine(mx + 4, midY + 2, mx + 8, midY, SSD1306_WHITE);
      break;
    case MOOD_CONFUSED:
      display.drawCircle(mx + 2, midY, 3, SSD1306_WHITE);
      break;
    case MOOD_BORED:
      display.drawLine(mx - 6, midY + 1, mx + 6, midY + 1, SSD1306_WHITE);
      break;
    case MOOD_STARSTRUCK:
      drawMouthCurve(mx, midY, 8, 3);
      display.drawPixel(mx - 3, midY + 5, SSD1306_WHITE);
      break;
    case MOOD_GRUMPY:
      drawMouthCurve(mx, midY, 7, -3);
      break;
    case MOOD_SNEEZING:
      drawOpenMouth(mx, midY, 11, 7 + (int)(2.0f * pulse));
      break;
    case MOOD_CURIOUS:
      display.drawCircle(mx, midY, 3, SSD1306_WHITE);
      display.drawPixel(mx, midY + 1, SSD1306_WHITE);
      break;
    case MOOD_NERVOUS:
      display.drawLine(mx - 7, midY - 1, mx - 4, midY + 1, SSD1306_WHITE);
      display.drawLine(mx - 4, midY + 1, mx - 1, midY - 1, SSD1306_WHITE);
      display.drawLine(mx - 1, midY - 1, mx + 2, midY + 1, SSD1306_WHITE);
      display.drawLine(mx + 2, midY + 1, mx + 5, midY - 1, SSD1306_WHITE);
      break;
    case MOOD_FOCUSED:
      display.drawLine(mx - 6, midY + 1, mx + 6, midY + 1, SSD1306_WHITE);
      break;
    case MOOD_NORMAL:
    default:
      drawMouthCurve(mx, midY, 6, 1);
      break;
  }
}

void updateEyePhysics() {
  const unsigned long now = millis();
  const float time = now / 1000.0f;
  breathVal = sin(time * 1.25f) * 1.2f;

  if (now - lastSaccade >= saccadeInterval) {
    float gazeX = (float)random(-6, 7);
    float gazeY = (float)random(-4, 5);
    if (random(0, 5) == 0) gazeX = gazeY = 0;
    if (currentMood == MOOD_ANGRY) {
      gazeX *= 0.45f;
      gazeY *= 0.45f;
    } else if (currentMood == MOOD_SLEEPY) {
      gazeY = max(1.0f, gazeY);
    } else if (currentMood == MOOD_SAD) {
      gazeY = max(1.0f, gazeY);
    } else if (currentMood == MOOD_LOVE) {
      gazeX *= 0.35f;
      gazeY *= 0.35f;
    }
    leftEye.targetPupilX = gazeX;
    rightEye.targetPupilX = gazeX + random(-1, 2);
    leftEye.targetPupilY = gazeY;
    rightEye.targetPupilY = gazeY + random(-1, 2);
    lastSaccade = now;
    saccadeInterval = random(1200, 3200);
  }

  float height = 31.0f;
  float width = 32.0f;
  float centerOffset = breathVal;
  if (currentMood == MOOD_SURPRISED || currentMood == MOOD_SHOCKED ||
      currentMood == MOOD_SCARED || currentMood == MOOD_CURIOUS) {
    height = currentMood == MOOD_SHOCKED ? 34.0f : 33.0f;
    width = currentMood == MOOD_CURIOUS ? 34.0f : 33.0f;
    centerOffset -= 1.0f;
  } else if (currentMood == MOOD_ANGRY) {
    height = 30.0f;
    centerOffset += 1.0f;
  } else if (currentMood == MOOD_SLEEPY) {
    height = 29.0f + 0.6f * sin(time * 1.6f);
    centerOffset += 1.0f;
  } else if (currentMood == MOOD_SAD || currentMood == MOOD_CRYING) {
    height = 30.0f;
    centerOffset += 1.0f;
  } else if (currentMood == MOOD_BORED || currentMood == MOOD_GRUMPY ||
             currentMood == MOOD_FOCUSED) {
    height = 28.0f;
    width = 31.0f;
    centerOffset += 0.5f;
  } else if (currentMood == MOOD_EXCITED || currentMood == MOOD_STARSTRUCK) {
    height = 32.0f + 0.7f * sin(time * 2.0f);
    width = 33.0f;
  } else if (currentMood == MOOD_SUSPICIOUS || currentMood == MOOD_COOL) {
    height = 29.0f;
    width = 33.0f;
  } else if (currentMood == MOOD_NERVOUS || currentMood == MOOD_EMBARRASSED) {
    height = 30.0f;
    width = 31.0f;
  }
  leftEye.targetH = rightEye.targetH = height;
  leftEye.targetW = rightEye.targetW = width;
  leftEye.targetX = 44.0f - width / 2.0f;
  rightEye.targetX = 96.0f - width / 2.0f;
  const float eyeCenterY = 30.0f + centerOffset;
  leftEye.targetY = rightEye.targetY = eyeCenterY - height / 2.0f;

  if (!faceBlinking && (long)(now - leftEye.nextBlinkTime) >= 0) {
    faceBlinking = true;
    blinkStartedAt = now;
    leftEye.blinking = rightEye.blinking = true;
  }
  if (faceBlinking) {
    const unsigned long elapsed = now - blinkStartedAt;
    const float progress = constrain(elapsed / 180.0f, 0.0f, 1.0f);
    blinkAmount = 0.5f - 0.5f * cos(progress * TWO_PI);
    if (elapsed >= 180) {
      faceBlinking = false;
      blinkAmount = 0;
      leftEye.blinking = rightEye.blinking = false;
      leftEye.nextBlinkTime = rightEye.nextBlinkTime = now + random(2200, 5800);
      if (random(0, 5) == 0)
        leftEye.nextBlinkTime = rightEye.nextBlinkTime = now + 190;
    }
  } else {
    blinkAmount = 0;
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
  } else if (currentMood == MOOD_SAD) {
    const int tearY = 42 + (int)((millis() / 110) % 8);
    display.drawBitmap(42, tearY, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
    display.drawBitmap(91, tearY, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
  } else if (currentMood == MOOD_SHY || currentMood == MOOD_EMBARRASSED) {
    display.drawLine(28, 48, 37, 46, SSD1306_WHITE);
    display.drawLine(30, 51, 39, 49, SSD1306_WHITE);
    display.drawLine(89, 46, 98, 48, SSD1306_WHITE);
    display.drawLine(87, 49, 96, 51, SSD1306_WHITE);
  } else if (currentMood == MOOD_LOVE || currentMood == MOOD_NERVOUS ||
             currentMood == MOOD_GRATEFUL || currentMood == MOOD_PLAYFUL) {
    display.drawPixel(30, 48, SSD1306_WHITE);
    display.drawPixel(34, 46, SSD1306_WHITE);
    display.drawPixel(38, 48, SSD1306_WHITE);
    display.drawPixel(90, 48, SSD1306_WHITE);
    display.drawPixel(94, 46, SSD1306_WHITE);
    display.drawPixel(98, 48, SSD1306_WHITE);
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
  else if (currentMood == MOOD_SLEEPY) {
    const int drift = (millis() / 180) % 5;
    display.drawBitmap(108 + drift, 4 - drift, bmp_zzz, 16, 16, SSD1306_WHITE);
  }
  else if (currentMood == MOOD_NERVOUS) {
    display.drawPixel(58, 12, SSD1306_WHITE);
    display.drawPixel(70, 9, SSD1306_WHITE);
  } else if (currentMood == MOOD_SHOCKED) {
    display.drawPixel(18, 12, SSD1306_WHITE);
    display.drawPixel(110, 12, SSD1306_WHITE);
  } else if (currentMood == MOOD_GRATEFUL) {
    display.drawBitmap(56, 1, bmp_small_heart, 8, 8, SSD1306_WHITE);
  }
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

  if (animationSceneIndex == 11) {
    const int centerX = SCREEN_WIDTH / 2;
    const int centerY = SCREEN_HEIGHT / 2;
    for (int i = 0; i < 24; i++) {
      float angle = now / 180.0f + i * 0.48f;
      int radius = (now / 40 + i * 3) % 48;
      int x = centerX + cos(angle) * radius;
      int y = centerY + sin(angle) * radius * 0.48f;
      display.fillCircle(x, y, (i % 5 == 0) ? 2 : 1, SSD1306_WHITE);
    }
    return;
  }

  if (animationSceneIndex == 12) {
    const int centerX = SCREEN_WIDTH / 2;
    const int centerY = SCREEN_HEIGHT / 2;
    for (int layer = 0; layer < 7; layer++) {
      int phase = (now / 35 + layer * 12) % 64 + 1;
      int halfWidth = phase * 2;
      int halfHeight = phase / 2;
      display.drawRoundRect(centerX - halfWidth, centerY - halfHeight,
                            halfWidth * 2, halfHeight * 2, 4, SSD1306_WHITE);
    }
    return;
  }

  if (animationSceneIndex == 13) {
    int previousX = 0;
    int previousTopY = 32;
    int previousBottomY = 32;
    for (int x = 4; x < SCREEN_WIDTH; x += 4) {
      float wave = sin(x / 11.0f - now / 260.0f);
      int topY = 32 + wave * 20;
      int bottomY = 32 - wave * 20;
      display.drawPixel(x, topY, SSD1306_WHITE);
      display.drawPixel(x, bottomY, SSD1306_WHITE);
      if (previousX != 0) {
        display.drawLine(previousX, previousTopY, x, topY, SSD1306_WHITE);
        display.drawLine(previousX, previousBottomY, x, bottomY, SSD1306_WHITE);
      }
      if ((x / 4) % 2 == 0) display.drawLine(x, topY, x, bottomY, SSD1306_WHITE);
      previousX = x;
      previousTopY = topY;
      previousBottomY = bottomY;
    }
    return;
  }

  if (animationSceneIndex == 14) {
    const int centerX = SCREEN_WIDTH / 2;
    const int centerY = SCREEN_HEIGHT / 2;
    display.drawCircle(centerX, centerY, 8, SSD1306_WHITE);
    for (int orbit = 0; orbit < 3; orbit++) {
      float angle = now / (700.0f + orbit * 260.0f) + orbit * 2.1f;
      int radius = 15 + orbit * 8;
      int x = centerX + cos(angle) * radius;
      int y = centerY + sin(angle) * radius * 0.55f;
      display.drawCircle(x, y, orbit == 1 ? 2 : 1, SSD1306_WHITE);
      display.drawPixel(x, y, SSD1306_WHITE);
    }
    return;
  }

  if (animationSceneIndex == 15) {
    for (int comet = 0; comet < 5; comet++) {
      int x = SCREEN_WIDTH - ((now / (20 + comet * 4) + comet * 31) % (SCREEN_WIDTH + 24));
      int y = 8 + (comet * 13 + (now / 180 + comet * 7) % 17) % 48;
      display.drawLine(x, y, x + 7, y - 3, SSD1306_WHITE);
      display.drawLine(x + 2, y + 1, x + 6, y - 1, SSD1306_WHITE);
      display.drawPixel(x, y, SSD1306_WHITE);
    }
    return;
  }

  if (animationSceneIndex == 16) {
    for (int petal = 0; petal < 7; petal++) {
      int x = (petal * 23 + now / 35) % (SCREEN_WIDTH + 12) - 6;
      int y = (petal * 17 + now / 55) % (SCREEN_HEIGHT + 12) - 6;
      x += sin(now / 430.0f + petal) * 5;
      display.drawPixel(x, y, SSD1306_WHITE);
      display.drawPixel(x - 1, y - 1, SSD1306_WHITE);
      display.drawPixel(x + 1, y - 1, SSD1306_WHITE);
      display.drawPixel(x - 1, y + 1, SSD1306_WHITE);
      display.drawPixel(x + 1, y + 1, SSD1306_WHITE);
    }
    return;
  }

  if (animationSceneIndex == 17) {
    const int baseline = 33;
    const int pulse = (now / 90) % 18;
    int previousX = 0;
    int previousY = baseline;
    for (int x = 0; x < SCREEN_WIDTH; x += 2) {
      int phase = (x + (now / 8) % SCREEN_WIDTH) % SCREEN_WIDTH;
      int y = baseline;
      if (phase > 49 && phase < 77) {
        static const int beat[] = { 0, -3, 10, -20, 13, -5, 0, 0, 0, 0, 0, 0, 0, 0 };
        int beatIndex = phase - 50;
        if (beatIndex < (int)(sizeof(beat) / sizeof(beat[0]))) y += beat[beatIndex];
      } else {
        y += (int)(sin((phase + pulse) / 8.0f) * 1.5f);
      }
      if (x > 0) display.drawLine(previousX, previousY, x, y, SSD1306_WHITE);
      previousX = x;
      previousY = y;
    }
    return;
  }

  if (animationSceneIndex == 18) {
    int radius = (now / 55) % 34;
    display.drawCircle(64, 32, radius, SSD1306_WHITE);
    display.drawCircle(64, 32, (radius + 14) % 34, SSD1306_WHITE);
    display.drawCircle(64, 32, (radius + 25) % 34, SSD1306_WHITE);
    return;
  }

  if (animationSceneIndex == 19) {
    static const int starX[] = { 9, 28, 47, 68, 89, 113, 18, 55, 78, 101, 38, 121 };
    static const int starY[] = { 9, 38, 16, 51, 11, 34, 57, 28, 7, 54, 43, 16 };
    for (int star = 0; star < 12; star++) {
      int phase = (now / 110 + star * 5) % 17;
      if (phase < 9) {
        int x = starX[star];
        int y = starY[star];
        display.drawPixel(x, y, SSD1306_WHITE);
        if (phase < 3) {
          display.drawLine(x - 2, y, x + 2, y, SSD1306_WHITE);
          display.drawLine(x, y - 2, x, y + 2, SSD1306_WHITE);
        }
      }
    }
    return;
  }

  if (animationSceneIndex == 20) {
    const int centerX = SCREEN_WIDTH / 2;
    const int centerY = SCREEN_HEIGHT / 2;
    display.drawCircle(centerX, centerY, 10, SSD1306_WHITE);
    display.drawCircle(centerX - 3, centerY - 2, 2, SSD1306_WHITE);
    display.drawRoundRect(centerX - 27, centerY - 12, 54, 24, 11, SSD1306_WHITE);
    float angle = now / 450.0f;
    int moonX = centerX + cos(angle) * 27;
    int moonY = centerY + sin(angle) * 12;
    display.fillCircle(moonX, moonY, 3, SSD1306_WHITE);
    return;
  }

  if (animationSceneIndex == 21) {
    for (int confetti = 0; confetti < 16; confetti++) {
      int x = (confetti * 29 + now / (18 + confetti % 5)) % SCREEN_WIDTH;
      int y = (confetti * 19 + now / (28 + confetti % 7)) % SCREEN_HEIGHT;
      if (confetti % 3 == 0) display.drawLine(x, y, x + 2, y + 3, SSD1306_WHITE);
      else if (confetti % 3 == 1) display.drawRect(x, y, 3, 3, SSD1306_WHITE);
      else display.drawPixel(x, y, SSD1306_WHITE);
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
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %b %d", &t);
  display.setCursor(4, 8);
  display.print(dateStr);
  display.setCursor(108, 8);
  display.print(ampm);
  display.setFont(&FreeSansBold18pt7b);
  char timeStr[24];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", h12, t.tm_min);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 49);
  display.print(timeStr);
}

void drawWorldClockPage() {
  struct tm t;
  if (!getLocalTime(&t)) return;
  time_t now; time(&now);
  time_t londonEpoch   = now;
  time_t newyorkEpoch  = now - (4 * 3600);
  struct tm londonTime = *gmtime(&londonEpoch);
  struct tm newYorkTime = *gmtime(&newyorkEpoch);
  struct tm* localtm = &t;

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

  int localHour12 = localtm->tm_hour % 12;
  if (localHour12 == 0) localHour12 = 12;
  int londonHour12 = londonTime.tm_hour % 12;
  if (londonHour12 == 0) londonHour12 = 12;
  int newYorkHour12 = newYorkTime.tm_hour % 12;
  if (newYorkHour12 == 0) newYorkHour12 = 12;
  char jStr[10]; sprintf(jStr, "%02d:%02d", localHour12, localtm->tm_min);
  display.getTextBounds(jStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(21 - w/2, 39); display.print(jStr);
  display.setCursor(12, 51); display.print(localtm->tm_hour >= 12 ? "PM" : "AM");

  char lStr[10]; sprintf(lStr, "%02d:%02d", londonHour12, londonTime.tm_min);
  display.getTextBounds(lStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(64 - w/2, 39); display.print(lStr);
  display.setCursor(55, 51); display.print(londonTime.tm_hour >= 12 ? "PM" : "AM");

  char nStr[10]; sprintf(nStr, "%02d:%02d", newYorkHour12, newYorkTime.tm_min);
  display.getTextBounds(nStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(106 - w/2, 39); display.print(nStr);
  display.setCursor(97, 51); display.print(newYorkTime.tm_hour >= 12 ? "PM" : "AM");
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
  display.setCursor(0, 12);
  display.print(city);

  display.drawLine(0, 16, 124, 16, SSD1306_WHITE);
  display.drawBitmap(92, 19, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);

  display.setFont(&FreeSansBold18pt7b);
  String tempText = String((int)temperature);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(tempText, 0, 46, &x1, &y1, &w, &h);
  display.setCursor(0, 46);
  display.print(tempText);
  display.fillCircle(x1 + w + 5, 28, 3, SSD1306_WHITE);

  display.setFont(NULL);
  display.setCursor(x1 + w + 12, 31);
  display.print("C");
  String shortDesc = weatherDesc;
  if (shortDesc.length() > 12) shortDesc = shortDesc.substring(0, 11) + ".";
  display.setCursor(0, 54);
  display.print(shortDesc);
  display.drawBitmap(93, 48, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
  display.setCursor(103, 53);
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
    "Clock faces", "Weather", "Animations", "Expressions", "Dino game", "Auto mode", "Timer"
  };
  static const char* clockLabels[] = {
    "Big 12-hour", "Big 24-hour", "Split digital", "Seconds bar", "World clock",
    "Minimal 12-hour"
  };
  static const char* weatherLabels[] = {
    "Current weather", "Weather summary", "3-day forecast",
    "Conditions card", "Feels-like face", "Icon weather"
  };
  static const char* animationLabels[] = {
    "Snow", "Rain", "Clouds", "Bubbles", "Stars", "Hearts",
    "Fireflies", "Aurora", "Falling leaves", "Comets", "Fireworks",
    "Spiral", "Tunnel", "DNA",
    "Orbiting stars", "Comet shower", "Petal breeze", "Heartbeat",
    "Ripple rings", "Twinkle field", "Orbiting planet", "Confetti"
  };
  static const char* timerLabels[] = { "Stopwatch", "Set timer" };
  static const char* expressionLabels[] = {
    "Happy", "Love", "Surprised", "Sleepy", "Angry", "Sad",
    "Excited", "Suspicious", "Dizzy", "Neutral", "Wink", "Crying",
    "Laughing", "Confused", "Bored", "Cool", "Shy", "Starstruck",
    "Scared", "Grumpy", "Embarrassed", "Sneezing", "Curious", "Proud",
    "Nervous", "Silly", "Focused", "Grateful", "Playful", "Shocked"
  };

  const char* const* labels = mainLabels;
  if (menuDepth == 1) {
    if (currentMode == MODE_CLOCK_FACES) labels = clockLabels;
    else if (currentMode == MODE_WEATHER_FACE) labels = weatherLabels;
    else if (currentMode == MODE_ANIMATION) labels = animationLabels;
    else if (currentMode == MODE_FACE_EXPRESSIONS) labels = expressionLabels;
    else if (currentMode == MODE_TIMER) labels = timerLabels;
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

void drawSplitClock() {
  struct tm t;
  if (!getLocalTime(&t)) {
    display.setFont(NULL);
    display.setCursor(30, 30);
    display.print("Syncing...");
    return;
  }

  display.setFont(NULL);
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %d %b", &t);
  display.setCursor(4, 4);
  display.print(dateStr);
  display.drawLine(4, 14, 124, 14, SSD1306_WHITE);

  char hourText[3], minuteText[3];
  snprintf(hourText, sizeof(hourText), "%02d", t.tm_hour % 12 == 0 ? 12 : t.tm_hour % 12);
  snprintf(minuteText, sizeof(minuteText), "%02d", t.tm_min);
  display.drawRoundRect(5, 19, 52, 37, 5, SSD1306_WHITE);
  display.drawRoundRect(71, 19, 52, 37, 5, SSD1306_WHITE);
  display.setFont(&FreeSansBold18pt7b);
  display.setCursor(10, 47);
  display.print(hourText);
  display.setCursor(76, 47);
  display.print(minuteText);
  display.fillCircle(64, 31, 2, SSD1306_WHITE);
  display.fillCircle(64, 43, 2, SSD1306_WHITE);
  display.setFont(NULL);
  display.setCursor(106, 4);
  display.print(t.tm_hour >= 12 ? "PM" : "AM");
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
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %d %b", &t);
  display.setFont(NULL);
  display.setCursor(4, 8);
  display.print(dateStr);
  display.setFont(&FreeSansBold18pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 49);
  display.print(timeStr);
}

void drawSecondsClock() {
  struct tm t;
  if (!getLocalTime(&t)) {
    display.setFont(NULL);
    display.setCursor(30, 30);
    display.print("Syncing...");
    return;
  }

  char timeStr[6];
  int hour12 = t.tm_hour % 12;
  if (hour12 == 0) hour12 = 12;
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour12, t.tm_min);
  char dateStr[20];
  strftime(dateStr, sizeof(dateStr), "%a, %b %d", &t);
  display.setFont(NULL);
  display.setCursor(4, 4);
  display.print(dateStr);
  char secondsText[3];
  snprintf(secondsText, sizeof(secondsText), "%02d", t.tm_sec);
  int16_t x1, y1;
  uint16_t w, h;
  display.setFont(NULL);
  display.getTextBounds(secondsText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor(106, 4);
  display.print(t.tm_hour >= 12 ? "PM" : "AM");
  display.setCursor(SCREEN_WIDTH - w - 4, 51);
  display.print(secondsText);
  display.setFont(&FreeSansBold18pt7b);
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 47);
  display.print(timeStr);

  display.drawRect(4, 59, 120, 3, SSD1306_WHITE);
  int progressWidth = (t.tm_sec * 118) / 59;
  if (progressWidth > 0) display.fillRect(5, 60, progressWidth, 1, SSD1306_WHITE);
}

void drawMinimalClock12() {
  struct tm t;
  if (!getLocalTime(&t)) return;

  int hour12 = t.tm_hour % 12;
  if (hour12 == 0) hour12 = 12;
  char timeStr[6];
  snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour12, t.tm_min);

  display.setFont(&FreeSansBold18pt7b);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(timeStr, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2 - x1,
                    (SCREEN_HEIGHT - h) / 2 - y1);
  display.print(timeStr);
}

void drawClockStyle() {
  switch (clockStyle) {
    case 0: drawClockPage(); break;
    case 1: drawDigitalClock24(); break;
    case 2: drawSplitClock(); break;
    case 3: drawSecondsClock(); break;
    case 4: drawWorldClockPage(); break;
    case 5: drawMinimalClock12(); break;
  }
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

void drawWeatherConditions() {
  String condition = weatherMain;
  if (condition.length() > 9) condition = condition.substring(0, 9);
  display.setFont(NULL);
  display.setCursor(3, 3);
  display.print(condition);
  display.setCursor(96, 3);
  display.print("FEELS");
  display.drawLine(3, 12, 125, 12, SSD1306_WHITE);

  display.drawBitmap(5, 20, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);
  display.setFont(&FreeSansBold18pt7b);
  char temperatureText[6];
  snprintf(temperatureText, sizeof(temperatureText), "%d", (int)temperature);
  int16_t temperatureX, temperatureY;
  uint16_t temperatureWidth, temperatureHeight;
  display.getTextBounds(temperatureText, 40, 47, &temperatureX, &temperatureY,
                       &temperatureWidth, &temperatureHeight);
  display.setCursor(40, 47);
  display.print(temperatureText);
  display.fillCircle(temperatureX + temperatureWidth + 3, 29, 2, SSD1306_WHITE);

  display.setFont(NULL);
  display.setCursor(temperatureX + temperatureWidth + 8, 29);
  display.print("C");
  display.setCursor(94, 24);
  display.print((int)feelsLike);
  display.print(" C");
  display.drawBitmap(91, 39, bmp_tiny_drop, 8, 8, SSD1306_WHITE);
  display.setCursor(102, 39);
  display.print(humidity);
  display.print("%");

  String description = weatherDesc;
  if (description.length() > 19) description = description.substring(0, 18) + ".";
  display.setCursor(3, 54);
  display.print(description);
}

void drawWeatherFeelsLike() {
  display.setFont(NULL);
  String city = CITY;
  city.toUpperCase();
  display.setCursor(4, 4);
  display.print(city);
  display.drawLine(4, 13, 124, 13, SSD1306_WHITE);

  display.setFont(NULL);
  display.setCursor(7, 16);
  display.print("FEELS LIKE");
  display.setFont(&FreeSansBold18pt7b);
  char feelsText[8];
  snprintf(feelsText, sizeof(feelsText), "%dC", (int)feelsLike);
  display.setCursor(7, 51);
  display.print(feelsText);

  display.drawBitmap(91, 17, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);
  display.setFont(NULL);
  display.setCursor(7, 55);
  display.print("ACT ");
  display.print((int)temperature);
  display.print("C  RH ");
  display.print(humidity);
  display.print("%");
}

void drawWeatherIconFace() {
  display.setFont(NULL);
  display.setCursor(4, 4);
  display.print(CITY);
  String condition = weatherMain;
  if (condition.length() > 5) condition = condition.substring(0, 5);
  display.setCursor(94, 4);
  display.print(condition);
  display.drawLine(4, 13, 124, 13, SSD1306_WHITE);

  display.drawBitmap(12, 19, getBigIcon(weatherMain), 32, 32, SSD1306_WHITE);
  display.setFont(&FreeSansBold18pt7b);
  char temperatureText[8];
  snprintf(temperatureText, sizeof(temperatureText), "%dC", (int)temperature);
  display.setCursor(52, 45);
  display.print(temperatureText);
  display.setFont(NULL);
  display.setCursor(53, 53);
  String description = weatherDesc;
  if (description.length() > 11) description = description.substring(0, 10) + ".";
  display.print(description);
}

void drawWeatherStyle() {
  switch (weatherStyle) {
    case 0: drawWeatherPage(); break;
    case 1: drawWeatherSummary(); break;
    case 2: drawForecastPage(); break;
    case 3: drawWeatherConditions(); break;
    case 4: drawWeatherFeelsLike(); break;
    case 5: drawWeatherIconFace(); break;
  }
}

void drawTimerPage() {
  display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setFont(NULL);
  const char* heading = timerPage == 0 ? "STOPWATCH" : "COUNTDOWN";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(heading, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 4);
  display.print(heading);
  display.setTextColor(SSD1306_WHITE);

  unsigned long shownMs;
  if (timerPage == 0) {
    shownMs = stopwatchElapsedMs;
    if (stopwatchRunning) shownMs += millis() - stopwatchStartedAt;
  } else {
    shownMs = countdownRemainingMs;
    if (countdownRunning) {
      unsigned long elapsed = millis() - countdownStartedAt;
      if (elapsed >= countdownRemainingMs) {
        shownMs = 0;
        countdownRemainingMs = 0;
        countdownRunning = false;
        countdownFinished = true;
      } else {
        shownMs -= elapsed;
      }
    }
  }

  unsigned long totalSeconds = shownMs / 1000UL;
  char timeText[16];
  if (timerPage == 0 && totalSeconds >= 3600UL) {
    snprintf(timeText, sizeof(timeText), "%02lu:%02lu:%02lu",
             totalSeconds / 3600UL, (totalSeconds / 60UL) % 60UL, totalSeconds % 60UL);
    display.setFont(&FreeSansBold9pt7b);
  } else {
    snprintf(timeText, sizeof(timeText), "%02lu:%02lu",
             totalSeconds / 60UL, totalSeconds % 60UL);
    display.setFont(&FreeSansBold18pt7b);
  }
  display.getTextBounds(timeText, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, 42);
  display.print(timeText);

  display.setFont(NULL);
  display.setCursor(4, 56);
  if (timerPage == 0) {
    display.print(stopwatchRunning ? "TAP PAUSE   2X RESET" : "TAP START   2X RESET");
  } else if (countdownFinished) {
    display.print("TIME UP!  2X RESTART");
  } else if (countdownRunning) {
    display.print("TAP PAUSE 2X RESET");
  } else {
    display.print(countdownMinutes);
    display.print("m +1 TAP  2X GO");
  }
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
  } else if (dinoY < 25) {
    dinoY = 25;
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
  char scoreText[20];
  snprintf(scoreText, sizeof(scoreText), "SCORE %lu", gameScore);
  int16_t scoreX, scoreY;
  uint16_t scoreWidth, scoreHeight;
  display.getTextBounds(scoreText, 0, 0, &scoreX, &scoreY, &scoreWidth, &scoreHeight);
  display.setCursor(SCREEN_WIDTH - scoreWidth - 4, 9);
  display.print(scoreText);
  display.drawLine(0, 51, SCREEN_WIDTH, 51, SSD1306_WHITE);
  for (int x = 5; x < SCREEN_WIDTH; x += 19) {
    int groundX = (x - (millis() / 35) % 19 + 19) % SCREEN_WIDTH;
    display.drawPixel(groundX, 54, SSD1306_WHITE);
    display.drawPixel(groundX + 5, 58, SSD1306_WHITE);
  }

  const int baseY = (int)dinoY;
  const int topY = baseY - 22;
  const bool runningFrame = dinoY >= 47 && (millis() / 110) % 2 == 0;
  display.fillRoundRect(16, topY + 10, 20, 12, 4, SSD1306_WHITE);
  display.fillRoundRect(25, topY + 2, 13, 11, 3, SSD1306_WHITE);
  display.fillRoundRect(32, topY + 7, 9, 5, 2, SSD1306_WHITE);
  display.fillTriangle(16, topY + 11, 9, topY + 14, 17, topY + 16, SSD1306_WHITE);
  display.drawLine(23, topY + 8, 25, topY + 11, SSD1306_WHITE);
  display.drawLine(28, topY + 6, 30, topY + 9, SSD1306_WHITE);
  display.drawPixel(35, topY + 5, SSD1306_BLACK);
  display.drawPixel(38, topY + 10, SSD1306_BLACK);
  display.drawLine(29, topY + 15, 33, topY + 16, SSD1306_BLACK);
  display.drawLine(27, topY + 18, 30, topY + 16, SSD1306_BLACK);

  if (runningFrame) {
    display.drawLine(22, baseY - 3, 18, baseY, SSD1306_WHITE);
    display.drawLine(18, baseY, 22, baseY, SSD1306_WHITE);
    display.drawLine(31, baseY - 3, 34, baseY - 1, SSD1306_WHITE);
    display.drawLine(34, baseY - 1, 32, baseY, SSD1306_WHITE);
  } else {
    display.drawLine(22, baseY - 3, 22, baseY, SSD1306_WHITE);
    display.drawLine(22, baseY, 26, baseY, SSD1306_WHITE);
    display.drawLine(31, baseY - 3, 31, baseY, SSD1306_WHITE);
    display.drawLine(31, baseY, 35, baseY, SSD1306_WHITE);
  }

  if (obstacleX < SCREEN_WIDTH && obstacleX > -15) {
    const int cactusX = (int)obstacleX;
    const int cactusBase = 51;
    display.fillRoundRect(cactusX + 4, 34, 7, cactusBase - 34, 3, SSD1306_WHITE);
    display.fillRoundRect(cactusX, 39, 5, 8, 2, SSD1306_WHITE);
    display.fillRoundRect(cactusX - 2, 34, 7, 7, 2, SSD1306_WHITE);
    display.drawLine(cactusX + 2, 38, cactusX + 2, 42, SSD1306_BLACK);
    display.fillRoundRect(cactusX + 10, 42, 5, 7, 2, SSD1306_WHITE);
    display.fillRoundRect(cactusX + 12, 38, 6, 6, 2, SSD1306_WHITE);
    display.drawLine(cactusX + 14, 42, cactusX + 14, 45, SSD1306_BLACK);
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
  static const unsigned long pageDurations[] = { 10000UL, 10000UL, 5000UL };
  unsigned long now = millis();
  if (now - autoModeStarted >= pageDurations[autoModePage]) {
    autoModePage = (autoModePage + 1) % 3;
    autoModeStarted = now;
  }

  if (autoModePage == 0) {
    drawClockStyle();
  } else if (autoModePage == 1) {
    drawWeatherStyle();
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

void showNetworkAddress(const char* networkLabel, const IPAddress& address,
                        const char* setupPassword) {
  display.clearDisplay();
  display.setFont(NULL);
  display.setTextColor(SSD1306_WHITE);
  const char* title = "NIMO CONTROL PANEL";
  int16_t x, y;
  uint16_t width, height;
  display.getTextBounds(title, 0, 0, &x, &y, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 4);
  display.print(title);

  display.getTextBounds(networkLabel, 0, 0, &x, &y, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, 17);
  display.print(networkLabel);

  String ipText = address.toString();
  display.getTextBounds(ipText, 0, 0, &x, &y, &width, &height);
  display.setCursor((SCREEN_WIDTH - width) / 2, setupPassword ? 30 : 39);
  display.print(ipText);
  if (setupPassword) {
    display.setCursor(12, 42);
    display.print("PW: ");
    display.print(setupPassword);
  }
  display.setCursor(1, 55);
  display.print("OPEN IN PHONE BROWSER");
  display.display();
}

void updateWifiConnection() {
  const bool connected = WiFi.status() == WL_CONNECTED;
  const unsigned long now = millis();

  if (connected) {
    if (!stationWasConnected) {
      stationWasConnected = true;
      lastWifiRetryAt = now;
      savedNetworkFailureReported = false;
      if (wifiConnectPending) {
        configuredWifiSsid = pendingWifiSsid;
        configuredWifiPassword = pendingWifiPassword;
        size_t savedSsidBytes = settings.putString("wifi_ssid", configuredWifiSsid);
        size_t savedPasswordBytes = settings.putString("wifi_password", configuredWifiPassword);
        pendingWifiSsid = "";
        pendingWifiPassword = "";
        wifiConnectPending = false;
        if (savedSsidBytes > 0 && savedPasswordBytes > 0) {
          wifiConnectionMessage = "Connected and saved. NIMO will reconnect when this network is in range.";
        } else {
          wifiConnectionMessage = "Connected, but could not save Wi-Fi settings. They may need to be entered again after restart.";
          Serial.println(wifiConnectionMessage);
        }
      } else {
        wifiConnectionMessage = "Connected to " + WiFi.SSID() + ".";
      }
      Serial.print("Wi-Fi connected to ");
      Serial.print(WiFi.SSID());
      Serial.print("; IP: ");
      Serial.println(WiFi.localIP());
      configTime(0, 0, "pool.ntp.org");
      setenv("TZ", TIMEZONE, 1);
      tzset();
      lastWeatherUpdate = now - 600001UL;
      networkInfoUntil = now + 10000;
    }
    return;
  }

  if (stationWasConnected) {
    stationWasConnected = false;
    stationConnectStartedAt = now;
    savedNetworkFailureReported = false;
    wifiConnectionMessage = wifiConnectPending
      ? "Connecting to " + pendingWifiSsid + "…"
      : "Wi-Fi disconnected. NIMO will retry the saved network.";
  }

  if (wifiConnectPending && now - wifiConnectStartedAt >= WIFI_CONNECT_TIMEOUT_MS) {
    String failedSsid = pendingWifiSsid;
    pendingWifiSsid = "";
    pendingWifiPassword = "";
    wifiConnectPending = false;
    wifiConnectionMessage = "Connection to " + failedSsid + " failed. Check the password or signal and try again.";
    Serial.println(wifiConnectionMessage);
    if (configuredWifiSsid.length() > 0) {
      WiFi.disconnect(false, false);
      if (configuredWifiPassword.length() == 0) WiFi.begin(configuredWifiSsid.c_str());
      else WiFi.begin(configuredWifiSsid.c_str(), configuredWifiPassword.c_str());
      stationConnectStartedAt = now;
      savedNetworkFailureReported = false;
      lastWifiRetryAt = now;
    }
  } else if (!wifiConnectPending && configuredWifiSsid.length() > 0 &&
             now - lastWifiRetryAt >= WIFI_RETRY_INTERVAL_MS) {
    WiFi.reconnect();
    lastWifiRetryAt = now;
  }
  if (!wifiConnectPending && !connected && !savedNetworkFailureReported &&
      configuredWifiSsid.length() > 0 && now - stationConnectStartedAt >= WIFI_CONNECT_TIMEOUT_MS) {
    wifiConnectionMessage = "Saved network is unavailable. NIMO is retrying; choose another network below if needed.";
    savedNetworkFailureReported = true;
  }
}

// ==================================================
// SETUP
// ==================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  randomSeed(micros());

  pinMode(BUTTON_PIN, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 initialization failed");
    while (true) delay(1000);
  }
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();

  settings.begin("deskbuddy", false);
  configuredWifiSsid = settings.getString("wifi_ssid", String(WIFI_SSID));
  configuredWifiPassword = settings.getString("wifi_password", String(WIFI_PASSWORD));
  clockStyle = constrain(settings.getInt("clock", 0), 0, 5);
  weatherStyle = constrain(settings.getInt("weather", 0), 0, 5);
  int savedAnimation = settings.getInt("animation", 0);
  if (savedAnimation >= 16) savedAnimation -= 2;
  else if (savedAnimation >= 14) savedAnimation = 14;
  selectedAnimation = constrain(savedAnimation, 0, ANIMATION_SCENE_COUNT - 1);
  animationSceneIndex = selectedAnimation;
  setFaceExpression(constrain(settings.getInt("expression", 1), 0, FACE_EXPRESSION_COUNT - 1));
  currentMode = MODE_AUTO;
  setenv("TZ", TIMEZONE, 1);
  tzset();

  leftEye.init(28, 18, 32, 32);
  rightEye.init(80, 18, 32, 32);
  resetSceneParticles();

  playBootAnimation();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setAutoReconnect(true);
  IPAddress setupIP(192, 168, 1, 4);
  IPAddress gateway(192, 168, 1, 4);
  IPAddress subnet(255, 255, 255, 0);
  bool addressConfigured = WiFi.softAPConfig(setupIP, gateway, subnet);
  accessPointMode = addressConfigured &&
    WiFi.softAP(SETUP_AP_SSID, SETUP_AP_PASSWORD);
  if (accessPointMode) {
    wifiConnectionMessage = "Setup Wi-Fi is ready. Scan for your home network.";
    Serial.print("Setup Wi-Fi SSID: ");
    Serial.println(SETUP_AP_SSID);
    Serial.print("Setup Wi-Fi IP: ");
    Serial.println(WiFi.softAPIP());
    showNetworkAddress("NIMO SETUP WI-FI", WiFi.softAPIP(), SETUP_AP_PASSWORD);
  } else {
    wifiConnectionMessage = "Could not start setup Wi-Fi. Check the AP settings.";
    Serial.println("ERROR: Could not start setup Wi-Fi access point.");
    display.clearDisplay();
    display.setFont(NULL);
    display.setCursor(8, 24);
    display.print("Setup Wi-Fi failed");
    display.setCursor(8, 39);
    display.print("Check AP settings");
    display.display();
  }

  if (configuredWifiSsid.length() > 0) {
    wifiConnectionMessage = "Connecting to saved network " + configuredWifiSsid + "…";
    if (configuredWifiPassword.length() == 0) WiFi.begin(configuredWifiSsid.c_str());
    else WiFi.begin(configuredWifiSsid.c_str(), configuredWifiPassword.c_str());
    stationConnectStartedAt = millis();
    lastWifiRetryAt = millis();
  } else {
    wifiConnectionMessage = "No saved Wi-Fi network. Scan and choose one below.";
  }
  startControlServer();
  if (accessPointMode) {
    Serial.print("Open the control panel at http://");
    Serial.println(WiFi.softAPIP());
    showNetworkAddress("NIMO SETUP WI-FI", WiFi.softAPIP(), SETUP_AP_PASSWORD);
    networkInfoUntil = millis() + 5000;
  }

  rawPinState = false;
  lastPinState = false;
  pendingTap = false;
  doubleTapCandidate = false;
  buttonArmed = false;
  buttonReleaseStart = 0;
  buttonReadyAt = millis() + 1200;
  autoModeStarted = millis();
}

// ==================================================
// LOOP
// ==================================================
void loop() {
  updateWifiConnection();
  controlServer.handleClient();
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

  if (millis() < networkInfoUntil) {
    const bool stationConnected = WiFi.status() == WL_CONNECTED;
    IPAddress panelIP = stationConnected ? WiFi.localIP() : WiFi.softAPIP();
    showNetworkAddress(stationConnected ? "HOME WI-FI CONNECTED" : "NIMO SETUP WI-FI",
                       panelIP, stationConnected ? nullptr : SETUP_AP_PASSWORD);
  } else if (menuOpen) {
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
      case MODE_TIMER:
        drawTimerPage();
        break;
    }
  }

  display.display();
  delay(20);
}
