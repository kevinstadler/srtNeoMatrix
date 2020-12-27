#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

class Scene {
  public:
    uint16_t w, h;
    virtual void draw(Adafruit_NeoMatrix *matrix, int16_t x, int16_t y); // = 0
};

class AniMatrix : public Adafruit_NeoMatrix {
  private:
    unsigned long frameLength;
    unsigned long lastRender;
    Scene *sceneQueue[2];
    byte nScenes;
    byte firstScene;
    int16_t firstSceneOffset;

  public:
    bool circular = false;
    AniMatrix(int w, int h, uint8_t pin, float fps) : Adafruit_NeoMatrix(w, h, pin, 
        //NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
        NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG) { // , NEO_GRB            + NEO_KHZ800
      this->begin();
      this->clear();
      this->show();
      this->firstSceneOffset = this->width();
      this->setTextWrap(false);
      this->setBrightness(35);
      this->frameLength = 1000 / fps;
      this->getUsage();
    };

    // returns any scenes which are dropped from the queue from the completion of this frame
    Scene* drawFrame() {
      this->clear();
      int16_t offset = this->firstSceneOffset;
      for (byte i = 0; i < this->nScenes; i++) {
        byte index = (this->firstScene + i) % 2;
        sceneQueue[index]->draw(this, offset, 0);
        offset += this->sceneQueue[index]->w;
        if (offset >= this->width()) {
          // scenes so far already fill up screen, so stop drawing more
          break;
        }
      }

      unsigned long timeSinceLastRender = millis() - this->lastRender;
      if (timeSinceLastRender < frameLength) {
//        Serial.printf("Waiting for %d\n", frameLength - timeSinceLastRender);
        delay(frameLength - timeSinceLastRender);
//      } else {
//        Serial.printf("Coming back here after %dms (%dms too late)\n", timeSinceLastRender, timeSinceLastRender-frameLength);
      }
      lastRender = millis();
      this->show();
      firstSceneOffset -= 1;
      Scene *expired = NULL;
      if (firstSceneOffset == -sceneQueue[firstScene]->w) {
        if (!this->circular) {
          expired = sceneQueue[firstScene];
          this->nScenes--;
        }
        firstSceneOffset += sceneQueue[firstScene]->w;
        firstScene = (firstScene + 1) % 2;
      }
      return expired;
    };

    bool queueScene(Scene *scene) {
      if (this->nScenes == 2) {
        return false;
      }
      this->sceneQueue[(this->firstScene + this->nScenes) % 2] = scene;
      this->nScenes++;
      // TODO if this is the first scene that was added, set the current offset to width
      return true;
    }

    // estimated current draw by the current scene (in mA)
    int getUsage() {
      // for 8x64 pixels (512 leds === 1536 bytes) the sum can be up to 391680
      // any 0 value actually consumes 1 mA as well, so start with that offset
      unsigned long sum = 14 * this->numBytes;
      for (int i = 0; i < this->numBytes; i++) {
        sum += this->pixels[i];
        // alternatively
        // sum += map(this->pixels[i], 0, 255, 1, 20); // truncates too early
//        sum += map(this->pixels[i], 0, 255, 13, 255); // even an off LED takes 1uA, so put 13 in
      }
      // a fully on RGB led (sum of 765) is equivalent to 60mA, so divide by 12.75
//      return sum / 12.75;
      // if we've aadded the initial 1uA for off LEDS, gotta scale down from the original 255+13 to 255 (take down another 5%)
      return sum / 13.39;
    }
};

// dummies for getTextBounds();
int16_t x, y;

class TextScene : public Scene {
  private:
//    Adafruit_NeoMatrix *matrix;
    String message;
    uint16_t color;
    const GFXfont *font;
  public:
    TextScene(Adafruit_NeoMatrix *matrix, String message, uint16_t color, const GFXfont *font) {
//      this->matrix = matrix;
      this->message = message;
      this->color = color;
      // TODO could pass this one instead matrix->setPassThruColor(24 bit version!);
//      matrix->setPassThruColor(0xFF0000); // RGB?
      // in the end it all boils down to ->drawPixel() which is overridden by NeoMatrix to call NeoPixel::setPixelColor(24bit)
      this->font = font;
//      matrix->setFont(font);
      matrix->getTextBounds(message, 0, 0, &x, &y, &this->w, &this->h);
    };
    void draw(Adafruit_NeoMatrix *matrix, int16_t x, int16_t y) {
//      matrix->setFont(font);
      matrix->setCursor(x, y);//+6
      matrix->setTextColor(this->color);
      // assume 7 pixels per character
//      int spaceLeft = (matrix->width() - x) / 7;
//      matrix->print(this->message.length() > spaceLeft ? this->message.substring(0, spaceLeft) : this->message);
      matrix->print(this->message);
    };
};
