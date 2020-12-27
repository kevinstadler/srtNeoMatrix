## srtNeoMatrix

Arduino sketch for reading `.srt` subtitles from SPIFFS and displaying them on a NeoMatrix LED panel

## `AniMatrix` scene animation/management for NeoMatrix

A more comprehensive (and maintained) version of the consecutive scene animation/management library used by this sketch can be found at https://github.com/kevinstadler/AniMatrix/

## Automatic brightness

Based on surrounding brightness readings from a TEMT6000 sensor, the output brightness is automatically adjusted to improve readability. The following settings were determined empirically:

TEMT6000 reading 70 lux (small artificial light in a dark room) -> LED brightness 30
TEMT6000 reading 1024+ (sunlit room) -> LED brightness 200-250

The sketch uses a linear transform mapping the [0, 1024] range of `analogRead()` from the TEMT6000 to [1, 220] output brightness.

Note that the `Adafruit_NeoMatrix` library automatically applies extra gamma correction after that!

## Subtitles for `Love Actually`

* txt: https://www.opensubtitles.org/en/subtitleserve/sub/65631 
* srt: https://www.opensubtitles.org/en/subtitleserve/sub/153247
* script (but no times): https://transcripts.fandom.com/wiki/Love_Actually
* `_script.doc` file which has not just scene numbers but *locations* (and role names in CAPS)
