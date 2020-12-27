#include <FS.h>
#include "AniMatrix.h"
#include "fonts.h"

//typedef struct Role {
//  String name;
//  uint16_t color;
//} Role;

// groups
// BILLY, JOE, DJ(=DEC), MIKE, ANT?
// JAMIE, KATYA, CHRIS, ELEANORE
// DANIEL, KAREN, DAISY, SAM
// COLIN, MARK, NANCY, TONY
// film studio: JOHN, JUDY, TONY
// PETER, MARK, VICAR, JULIET
// pm: ANNIE, PM, TERENCE, PAT, NATALIE, ALEX, JEREMY, PRESIDENT
// DANIEL, SARAH, MARK
// MIA(=MIY), HARRY, SARAH
//Role roles[] = { { "OTHER", matrix.Color(255, 0, 0) },
//  { "PINK", matrix.Color(255, 50, 50) }
//};

AniMatrix *matrix;

uint16_t colors[2];

//int getRoleIndex(String role) {
//  for (byte i = 1; i < sizeof(roles)/sizeof(roles[0]); i++) {
//    if (role.equalsIgnoreCase(roles[i].name)) {
//      return i;
//    }
//  }
//  return 0;
//}


File subs;

String readLine() {
  return subs.readStringUntil('\n');
}


long parseMs(String hhMMssmmm) {
  long t = hhMMssmmm.substring(0, 2).toInt(); // hours
  t = 60*t + hhMMssmmm.substring(3, 5).toInt(); // minutes
  t = 60*t + hhMMssmmm.substring(6, 8).toInt(); // seconds
  return 1000*t + hhMMssmmm.substring(9, 12).toInt(); // ms
}

long previousEnding;

String readSRTLine() {
  readLine(); // discard int line
  String s = readLine();
  long t0 = parseMs(s.substring(0, 12));
  long t1 = parseMs(s.substring(17));
  s = readLine(); // actual text line
  if (s.startsWith("- ")) {
    s.remove(0, 1);
  }
  // sometimes titles are multi-line
  while (subs.peek() != 13) { // newline
    s += " " + readLine();
  }
  s.replace(" - ", "   ");
  int i = -1;
  while ((i = s.indexOf('(', i+1)) != -1) {
    int j = s.indexOf(')', i);

    // definitely filter out if it's a song title
    int e = s.indexOf(':', i);

    // if it's not a song title, gotta check.
    if (e == -1 || e > j) {
      // don't filter out funny acting instructions. only remove "(2nd )Man", "girlfriend", "Director", "Karen", "Daniel", "Dec", "Billy" 
      // SOLUTION: only leave things that end in -s or -ing
      e = s.indexOf(' ', i);
      if (e == -1 || e > j) {
        e = j;
      }
      if ((s[e-1] == 's' && s[e-2] != 's') || s.substring(e-3, e).equals("ing") || s[e-1] == ',') {
        Serial.println("Keeping because first word ends with s, ing, or ',': " + s);
        continue;
      }
      // keep "Russian", "Pidgin Portuguese", "Portuguese", "Italian", "With English accent", "Gentle growl" // TODO add "Car" for Car horn??
      String firstWord = s.substring(i+1, e);
      if (firstWord.equals("Russian") || firstWord.equals("Pidgin") || firstWord.equals("Portuguese") || firstWord.equals("Italian") ||
          firstWord.equals("With") || firstWord.equals("Gentle") || firstWord.equals("Inaudible")) {
        Serial.println("Keeping because of magic word: " + s);
        continue;
      }
      // check second word (if there is one)
      if (e != j) {
        Serial.println("Checking second word: " + s.substring(e));
        e = min(j, s.indexOf(' ', e+1));
        if ((s[e-1] == 's' && s[e-2] != 's') || s.substring(e-3, e).equals("ing") || s[e-1] == ',') {
          Serial.println("Keeping because second word ends with s, ing, or ',': " + s);
          continue;
        }
      }
      
    }
    Serial.println("Removing " + s.substring(i, j + 1));
    s.remove(i, j - i + 1);
  }
  // FIXME check if anything of string is left at all? special treatment if length is 0??
  
  // now consume two newlines
  subs.read();
  subs.read();

  if (previousEnding != 0) {
    // PREpend as many spaces as warranted by time between end of previous and this.
    // minimum 1 space for 47ms, add an extra space for every 100ms? but 15 spaces max
    long nspaces = min(15l, 1 + (t0 - previousEnding) / 200);
    // if first letter is a uppercase, add an extra space or two in front
    if (nspaces == 1 && (isUpperCase(s[0]) || s[0] == '(')) {
//      Serial.println("Adding 2nd and 3rd space before upper case or bracket: " + s);
      nspaces = 3;
    }

    for (byte i = 0; i < nspaces; i++) {
      s = " " + s;
    }
  }
  previousEnding = t1;
  // check if end of file reached, if so seek to beginning, reset previousEnding
  // FIXME this check is too sensitive, leading to a stuck/blocking read on the next call
  if (subs.position() >= subs.size()) {
    subs.seek(0);
    previousEnding = 0;
  }
  return s;
}

Scene* SRTScene(String s) {
  // could check if trimmed string starts with slash but trim changes string in place so....
  int italic = s.indexOf('\'');
  if (italic != -1) {
    if (italic == 0 || s[italic-1] == ' ') {
      s.remove(italic, 1);
    } else {
      italic = -1; // don't change font
    }
  }
  // dogicapixel4pt7b wide dogica4pt7b EVEN WIDER
//  const GFXfont *font = &( italic != -1 ? dogicapixel4pt7b : dogicapixel4pt7b);
//  return new TextScene(matrix, s, italic != -1 ? colors[1] : colors[0], NULL);
  return new TextScene(matrix, s, colors[0], NULL);
}

void setupSubtitles(String filename) {
//  matrix = new Adafruit_NeoMatrix(64, 8, 0, NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
//        NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
//        NEO_GRB            + NEO_KHZ800);
  matrix = new AniMatrix(72, 8, 0, 50.0); // 50fps is pushing it
  colors[0] = matrix->Color(255, 0, 0);
  colors[1] = matrix->Color(255, 100, 100);

  if (SPIFFS.begin()) {
    Serial.println("FS mounted.");
  }
  subs = SPIFFS.open(filename, "r");
  if (subs) {
    Serial.println("File opened.");
  }

  // seek to end position
//  subs.seek(subs.size() - 400);
  // start at random position
  subs.seek(random(subs.size() - 400), SeekSet); // bool
  subs.readStringUntil('\n');
  while (subs.peek() != 13) {
    subs.readStringUntil('\n');
  }
  subs.read();
  subs.read();

  // put two on stack
  matrix->queueScene(SRTScene(readSRTLine()));
  matrix->queueScene(SRTScene(readSRTLine()));
}

void drawSubtitles() {
  // TODO analogRead only takes 100us (that's .1ms), but only do it sometimes anyway
  matrix->setBrightness(map(analogRead(A0), 0, 1024, 1, 220));
  if (subs) {
    Scene *expired = matrix->drawFrame();
    if (expired != NULL) {
      delete expired;
      matrix->queueScene(SRTScene(readSRTLine()));
    }
  }
//  Serial.println(matrix->getUsage());
}
