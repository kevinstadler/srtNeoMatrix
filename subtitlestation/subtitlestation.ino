#define LED_PIN 0
#include "subtitles.h"

void setup() {
  Serial.begin(115200);
  setupSubtitles("/love.srt");
}

void loop() {
  drawSubtitles();
}
