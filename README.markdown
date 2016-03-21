# LED Octahedron

This project is the code for creating several animations for my LED octahedron
project.

## Physical Components

* 6 meters of [Adafruit Dotstar LED's (APA102, 30 LED/m)][dotstars].
  * To cover every edge of the box, you'll need 204"/~5.18m (17" per edge * 12
    edges) of LED's.
* [Catchitecture's Octacat Box][octacat]
* An Arduino. There are many to choose from, but I used an [Uno][uno] for mine.
  Namely that the clock speed still worked great, there were no power
  conversions from 3.3v to 5v as is necessary on faster/more powerful
  Arduino's, and it's cheap. However, FastLED supports a [variety of different
  microcontrollers][fastled_controllers], and this code can likely work on any
  controller that supports interrupts for the button pressing modes (which
  you will likely have to refactor the button code if your board doesn't
  support them).
* Some push buttons, some resistors, power, breadboard, proto shield if you
  want to solder (highly suggested), and many other items that are up to you
  and you can find out more details about elsewhere. I would highly suggest
  reading [Adafruit's Guide to Dotstar's][dotstar_guide], as well as consider
  getting an Arduino starter kit.
* Essentially you build a circuit with a clock and data pin from the Arduino to
  the LED strip, power separately, and attach 2 interrupt pins to push buttons
  (with a little bit of resistance in between).

## Software

* Uses [FastLED 3.1][fastled_release]. See FastLED's [website][fastled_site]
  and [documentation][fastled_docs] for more information.
* Read the comments in the code for adding patterns and general modification.

[dotstar_guide]: https://learn.adafruit.com/adafruit-dotstar-leds
[dotstars]: https://www.adafruit.com/products/2237
[fastled_controllers]: https://github.com/FastLED/FastLED/wiki/Overview#supported-platforms
[fastled_docs]: https://github.com/FastLED/FastLED/wiki/Overview
[fastled_release]: https://github.com/FastLED/FastLED/releases/tag/v3.1.0
[fastled_site]: http://fastled.io/
[octacat]: http://www.catchitecture.com/#!product-page/cba4/9d8d564f-df7c-08bb-7562-52b1b8a3086f
[uno]: https://www.arduino.cc/en/Main/ArduinoBoardUno

## Great Job!

Don't let d12 shaped boxes go to waste! Make them rave-d12 boxes!\*

\* Cat not included, but highly recommended.
