#include <FastLED.h>

#define BUTTON_PIN 2
#define CLOCK_PIN 13
#define DATA_PIN 12
#define DEFAULT_BRIGHTNESS 191
#define DEFAULT_HUE 5
#define DEFAULT_SAT 191
#define FPS 120
#define LED_GROUP_SIZE (NUM_LEDS / 12)
#define MAX_BRIGHTNESS 223
#define MAX_SAT 255
#define MIN_BRIGHTNESS 63
#define MIN_SAT 191
#define NUM_HUE_DISTANCES 6
#define NUM_LEDS 180
#define SPEED 10

CRGB leds[NUM_LEDS];
CHSV hsvs[NUM_LEDS];

const uint16_t LEFT_CENTER = (
    LED_GROUP_SIZE % 2 == 0 ? (LED_GROUP_SIZE / 2) - 1 : LED_GROUP_SIZE / 2);
const uint16_t RIGHT_CENTER = LED_GROUP_SIZE / 2;
const uint16_t DIST_CENTER = (
    LED_GROUP_SIZE % 2 == 0 ? LED_GROUP_SIZE / 2 : (LED_GROUP_SIZE / 2) + 1);
const uint8_t HUE_DISTANCES[NUM_HUE_DISTANCES] = {0, 127, 42, 170, 234, 191};

boolean flashRed = false;
uint8_t buttonState;
uint8_t currentPattern = 0;
uint8_t lastButtonState = LOW;
uint8_t rainbowHue = 0;
uint64_t debounceDelay = 50;
uint64_t lastDebounceTime;

uint8_t calcHueForGroup(uint8_t baseHue, uint16_t group) {
    return baseHue + HUE_DISTANCES[group % NUM_HUE_DISTANCES];
}

void turnOffLights() {
    for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        hsvs[i].val = 0;
    }
    hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
    FastLED.show();
}

void setupHuesForGroups() {
    uint8_t hue;
    uint16_t groupNum;
    for (uint16_t i = 0; i < NUM_LEDS; i += LED_GROUP_SIZE) {
        groupNum = i / LED_GROUP_SIZE;
        hue = calcHueForGroup(DEFAULT_HUE + (63 * groupNum), groupNum);
        for (uint16_t j = i; j < i + LED_GROUP_SIZE; ++j) {
            hsvs[j] = CHSV(hue, DEFAULT_SAT, 0);
        }
    }
}

void twinkleLoop() {
    uint8_t wave, brightUp, brightDown;
    setupHuesForGroups();
    for (uint16_t fade = 0; fade <= 255; ++fade) {
        wave = sin8(fade);
        brightUp = lerp8by8(MIN_BRIGHTNESS, MAX_BRIGHTNESS, wave);
        brightDown = lerp8by8(MAX_BRIGHTNESS, MIN_BRIGHTNESS, wave);
        for (uint16_t dot = 0; dot < NUM_LEDS; ++dot) {
            if (dot % 2 == 0) {
                hsvs[dot].val = brightDown;
            } else {
                hsvs[dot].val = brightUp;
            }
            hsvs[dot].sat = lerp8by8(MIN_SAT, MAX_SAT, wave);
        }
        hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
        FastLED.show();
        delay(10 / SPEED);
    }
}

void movingPattern() {
    uint8_t hue;
    setupHuesForGroups();
    for (uint16_t i = 0; i < LED_GROUP_SIZE; ++i) {
        for (uint16_t j = 0; j < NUM_LEDS; j += LED_GROUP_SIZE) {
            hsvs[j + i].hue += 16;
            hsvs[j + i].sat = MAX_SAT;
            hsvs[j + i].val = DEFAULT_BRIGHTNESS;
            for (uint16_t k = j; k < j + i; ++k) {
                hsvs[k].hue += 16;
                hsvs[k].sat = lerp8by8(0, hsvs[k].sat, 224);
                hsvs[k].val = lerp8by8(0, hsvs[k].val, 192);
            }
        }
        hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
        FastLED.show();
        delay(1000 / SPEED);
    }
}

void panningAnimation(uint16_t left, uint16_t right, uint16_t len, boolean out) {
    for (uint16_t i = 0; i < len; ++i) {
        if (i != 0) {
            if (out) {
                --left, ++right;
            } else {
                ++left, --right;
            }
        }
        if (i == len - 1 && !out) {
            continue;
        }
        for (uint16_t j = 0; j < NUM_LEDS; j += LED_GROUP_SIZE) {
            if (out) {
                hsvs[j + left].sat = MAX_SAT;
                hsvs[j + left].val = DEFAULT_BRIGHTNESS;
                hsvs[j + right].sat = MAX_SAT;
                hsvs[j + right].val = DEFAULT_BRIGHTNESS;
            } else {
                hsvs[j + left].val = 0;
                hsvs[j + right].val = 0;
            }
        }
        for (uint16_t j = 0; j < NUM_LEDS; ++j) {
            hsvs[j].hue += 32;
            if (i % 2 == 0) {
                hsvs[j].sat -= 32;
            } else {
                hsvs[j].sat += 32;
            }
        }
        hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
        FastLED.show();
        delay(2000 / SPEED);
    }
}

void panningLoop() {
    setupHuesForGroups();
    panningAnimation(LEFT_CENTER, RIGHT_CENTER, DIST_CENTER, true);
    panningAnimation(0, LED_GROUP_SIZE - 1, DIST_CENTER, false);
    panningAnimation(LEFT_CENTER, RIGHT_CENTER, DIST_CENTER, true);
}

void pingPongSide(uint16_t lower, uint16_t upper, int16_t inc) {
    uint8_t dir = lower + inc;
    boolean out = true;

    while (dir != lower) {
        for (uint16_t j = 0; j < NUM_LEDS; j += LED_GROUP_SIZE) {
            if (out) {
                hsvs[j + dir].sat = MAX_SAT;
                hsvs[j + dir].val = DEFAULT_BRIGHTNESS;
            } else {
                hsvs[j + dir].val = 0;
            }
        }
        if (dir == upper) {
            if (out) {
                out = false;
            } else {
                dir -= inc;
            }
        } else {
            if (out) {
                dir += inc;
            } else {
                dir -= inc;
            }
        }
        for (uint16_t i = 0; i < NUM_LEDS; ++i) {
            hsvs[i].hue += 32;
        }
        hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
        FastLED.show();
        delay(2000 / SPEED);
    }
}

void pingPongAnimation(uint16_t left, uint16_t right) {
    for (uint16_t j = 0; j < NUM_LEDS; j += LED_GROUP_SIZE) {
        CHSV curHsv = CHSV(
            calcHueForGroup(DEFAULT_HUE, j),
            MAX_SAT, DEFAULT_BRIGHTNESS);
        hsvs[j + left].sat = hsvs[j + right].sat = MAX_SAT;
        hsvs[j + left].val = hsvs[j + right].val = DEFAULT_BRIGHTNESS;
    }
    hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
    FastLED.show();
    // Right
    pingPongSide(right, LED_GROUP_SIZE - 1, 1);
    // Left
    pingPongSide(left, 0, -1);
}

void pingPongLoop() {
    setupHuesForGroups();
    pingPongAnimation(LEFT_CENTER, RIGHT_CENTER);
}

void blinkRedLights() {
    for (uint8_t i = 0; i < 4; ++i) {
        uint16_t dot = 0;
        for (dot = 0; dot < NUM_LEDS; ++dot) {
            leds[dot] = CRGB::Red;
        }
        FastLED.show();
        delay(1000 / SPEED);
        for (dot = 0; dot < NUM_LEDS; ++dot) {
            leds[dot] = CRGB::Black;
        }
        FastLED.show();
        delay(1000 / SPEED);
    }
}

void randomSparklesGroupPattern() {
    randomSparklesPattern(true);
}

void randomSparklesRainbowPattern() {
    randomSparklesPattern(false);
}

void randomSparklesPattern(boolean groupHues) {
    uint16_t pos = random16(NUM_LEDS);
    uint8_t hue;

    fadeToBlackBy(leds, NUM_LEDS, 10);
    if (groupHues) {
        hue = calcHueForGroup(
            DEFAULT_HUE + (63 * (pos / LED_GROUP_SIZE)), pos / LED_GROUP_SIZE);
    } else {
        hue = rainbowHue;
    }
    leds[pos] += CHSV(hue + random8(64), DEFAULT_SAT, MAX_BRIGHTNESS);
}

/* Main Logic */

typedef void (*PatternArray[])();
PatternArray patterns = {
    randomSparklesGroupPattern, randomSparklesRainbowPattern, movingPattern
};

void nextPattern() {
    currentPattern = (
        (currentPattern + 1) % (sizeof(patterns) / sizeof(patterns[0])));
}

void changeMode() {
    uint8_t buttonReading = digitalRead(BUTTON_PIN);
    if (buttonReading != lastButtonState) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (buttonReading != buttonState) {
            buttonState = buttonReading;
            if (buttonState == HIGH) {
                flashRed = !flashRed;
            }
        }
    }
    flashRed = !flashRed;
}

void setup() {
    Serial.begin(9600);
    pinMode(BUTTON_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), changeMode, FALLING);
    FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN>(
        leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setTemperature(CarbonArc);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void loop() {
    if (flashRed) {
        blinkRedLights();
    } else {
        patterns[currentPattern]();
    }
    FastLED.delay(1000 / FPS);
    EVERY_N_SECONDS(5) { nextPattern(); }
    EVERY_N_MILLISECONDS(20) { ++rainbowHue; }
}
