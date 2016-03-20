#include <FastLED.h>

#define BLUE_BUTTON_PIN 3
#define BPM 120
#define CLOCK_PIN 13
#define DATA_PIN 12
#define DEFAULT_BRIGHTNESS 191
#define DEFAULT_HUE 5
#define DEFAULT_SAT 223
#define FPS 120
#define LED_GROUP_SIZE (NUM_LEDS / 12)
#define MAX_BRIGHTNESS 223
#define MAX_SAT 255
#define MIN_BRIGHTNESS 63
#define MIN_SAT 191
#define NUM_HUE_DISTANCES 6
#define NUM_LEDS 180
#define RED_BUTTON_PIN 2
#define SPEED 10


CRGB leds[NUM_LEDS];
CHSV hsvs[NUM_LEDS];


const uint16_t LEFT_CENTER = (
    LED_GROUP_SIZE % 2 == 0 ? (LED_GROUP_SIZE / 2) - 1 : LED_GROUP_SIZE / 2);
const uint16_t RIGHT_CENTER = LED_GROUP_SIZE / 2;
const uint16_t DIST_CENTER = (
    LED_GROUP_SIZE % 2 == 0 ? LED_GROUP_SIZE / 2 : (LED_GROUP_SIZE / 2) + 1);
const uint8_t HUE_DISTANCES[NUM_HUE_DISTANCES] = {0, 127, 42, 170, 234, 191};


const CHSVPalette16 OctocatColorsPalette_p(
    CHSV(0, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(127, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(170, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(234, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(191, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(64, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(191, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(106, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(234, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(255, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(128, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(255, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(170, DEFAULT_SAT, MAX_BRIGHTNESS),
    CHSV(42, DEFAULT_SAT, MAX_BRIGHTNESS)
);

CHSVPalette16 hsvPalettes[] = {
    OctocatColorsPalette_p
};

CRGBPalette16 rgbPalettes[] = {
    OctocatColorsPalette_p, LavaColors_p, PartyColors_p, OceanColors_p
};


boolean beatMode = false;
boolean flashingMode = false;
uint8_t currentPattern = 0;
uint8_t currentHSVPalette = 0;
uint8_t currentRGBPalette = 0;
uint8_t rainbowHue = 0;
uint64_t debounceDelay = 200;
uint64_t lastFlashDebounceTime = 0;
uint64_t lastBeatDebounceTime = 0;


uint8_t getGradientHue(uint8_t led) {
    return (led * 255) / (NUM_LEDS - 1);
}


uint8_t getGroupHue(uint8_t led) {
    return ((led / LED_GROUP_SIZE) * 255) / 11;
}


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
        hue = calcHueForGroup(DEFAULT_HUE + (64 * groupNum), groupNum);
        for (uint16_t j = i; j < i + LED_GROUP_SIZE; ++j) {
            hsvs[j] = CHSV(hue, DEFAULT_SAT, 0);
        }
    }
}


void twinklePattern() {
    uint8_t bpm = 30;
    uint8_t beat = beatsin8(bpm, 0, 255);
    CHSVPalette16 palette = OctocatColorsPalette_p;
    for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        CHSV hsv = ColorFromPalette(
            palette, getGroupHue(i), DEFAULT_BRIGHTNESS);
        hsv.sat = lerp8by8(MIN_SAT, MAX_SAT, beat);
        if (i % 2 == 0) {
            hsv.val = lerp8by8(MAX_BRIGHTNESS, MIN_BRIGHTNESS, beat);
        } else {
            hsv.val = lerp8by8(MIN_BRIGHTNESS, MAX_BRIGHTNESS, beat);
        }
        leds[i] = hsv;
    }
}


void cometPatternRGB() {
    static uint8_t offset = 0;

    uint8_t groupOffset = offset % LED_GROUP_SIZE;
    CRGBPalette16 palette = rgbPalettes[currentRGBPalette];

    fadeToBlackBy(leds, NUM_LEDS, 127);
    for (uint16_t i = groupOffset; i < NUM_LEDS; i += LED_GROUP_SIZE) {
        leds[i] = ColorFromPalette(palette, getGradientHue(i), MAX_BRIGHTNESS);
    }
    ++offset;
    delay(50);
}


void cometPatternHSV() {
    static uint8_t offset = 0;

    uint8_t groupOffset = offset % LED_GROUP_SIZE;
    CHSVPalette16 palette = hsvPalettes[currentHSVPalette];

    fadeToBlackBy(leds, NUM_LEDS, 127);
    for (uint16_t i = groupOffset; i < NUM_LEDS; i += LED_GROUP_SIZE) {
        CHSV hsv = ColorFromPalette(
            palette, getGradientHue(i), MAX_BRIGHTNESS);
        hsv.hue += 16 * groupOffset;
        leds[i] = hsv;
    }
    ++offset;
    delay(50);
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
            DEFAULT_HUE + (64 * (pos / LED_GROUP_SIZE)), pos / LED_GROUP_SIZE);
    } else {
        hue = rainbowHue;
    }
    leds[pos] += CHSV(hue + random8(64), DEFAULT_SAT, MAX_BRIGHTNESS);
}


void glitterPattern() {
    uint8_t bpm = 60;
    uint8_t beat = beatsin8(bpm, 159, MAX_BRIGHTNESS);
    CHSV hsv = CHSV(rainbowHue, DEFAULT_SAT, beat);
    for (uint16_t i = 0; i < NUM_LEDS; ++i) {
        hsvs[i] = hsv;
        hsv.hue += 7;
    }
    hsv2rgb_rainbow(hsvs, leds, NUM_LEDS);
    if(random8() < 64) {
        leds[random16(NUM_LEDS)] += CRGB::White;
    }
}


void pulsingPattern() {
    uint8_t bpm = 60;
    uint8_t beat = beatsin8(bpm, 64, 255);
    uint8_t hue;
    CRGBPalette16 palette = rgbPalettes[currentRGBPalette];

    for(uint16_t i = 0; i < NUM_LEDS; i++) {
        uint8_t brightnessFactor = lerp8by8(8, 20, sin8(rainbowHue));
        uint8_t brightness = beat - (rainbowHue + (i * brightnessFactor));
        leds[i] = ColorFromPalette(
            palette, rainbowHue + (i * 2), brightness);
    }
}


void beatSyncMultiplesPattern() {
    fadeToBlackBy(leds, NUM_LEDS, 20);
    CRGBPalette16 palette = rgbPalettes[currentRGBPalette];
    for(uint16_t i = 0; i < 8; i++) {
        uint16_t index = beatsin16(i * 2, 0, NUM_LEDS);
        leds[index] |= ColorFromPalette(palette, i * 32, MAX_BRIGHTNESS);
    }
}


/* Main Logic */

typedef void (*PatternArray[])();
PatternArray patterns = {
    panningLoop
    /* cometPatternRGB, */
    /* cometPatternHSV, */
    /* twinklePattern, */
    /* randomSparklesGroupPattern, */
    /* randomSparklesRainbowPattern, */
    /* pulsingPattern, */
    /* glitterPattern, */
    /* beatSyncMultiplesPattern */
};

void nextPalette() {
    currentRGBPalette = (
        (currentRGBPalette + 1) %
        (sizeof(rgbPalettes) / sizeof(rgbPalettes[0])));
    currentHSVPalette = (
        (currentHSVPalette + 1) %
        (sizeof(hsvPalettes) / sizeof(hsvPalettes[0])));
}


void nextPattern() {
    currentPattern = (
        (currentPattern + 1) % (sizeof(patterns) / sizeof(patterns[0])));
}


void toggleFlashingMode() {
    uint64_t interruptTime = millis();
    if (interruptTime - lastFlashDebounceTime > debounceDelay) {
        Serial.println("Toggling Flashing Mode");
        flashingMode = !flashingMode;
    }
    if (flashingMode) {
        beatMode = false;
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    }
    lastFlashDebounceTime = interruptTime;
}

void toggleBeatMode() {
    uint64_t interruptTime = millis();
    if (interruptTime - lastBeatDebounceTime > debounceDelay) {
        Serial.println("Toggling Beat Mode");
        beatMode = !beatMode;
    }
    if (!beatMode) {
        FastLED.setBrightness(DEFAULT_BRIGHTNESS);
    }
    lastBeatDebounceTime = interruptTime;
}

void setup() {
    Serial.begin(9600);
    pinMode(RED_BUTTON_PIN, INPUT);
    pinMode(BLUE_BUTTON_PIN, INPUT);
    attachInterrupt(
        digitalPinToInterrupt(RED_BUTTON_PIN), toggleFlashingMode, RISING);
    attachInterrupt(
        digitalPinToInterrupt(BLUE_BUTTON_PIN), toggleBeatMode, RISING);
    FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN>(
        leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setTemperature(CarbonArc);
    FastLED.setBrightness(DEFAULT_BRIGHTNESS);
}

void loop() {
    if (beatMode) {
        FastLED.setBrightness(beatsin8(BPM, 0, 255));
    }
    if (flashingMode) {
        blinkRedLights();
    } else {
        patterns[currentPattern]();
    }
    FastLED.show();
    FastLED.delay(1000 / FPS);
    EVERY_N_SECONDS(12) { nextPattern(); }
    EVERY_N_SECONDS(3) { nextPalette(); }
    EVERY_N_MILLISECONDS(20) { ++rainbowHue; }
}
