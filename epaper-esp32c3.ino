
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
// #include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Adafruit_NeoPixel.h>

/*
  DC   = 1
  RES  = 2
  BUSY = 3
  SCL  = 4
  CS   = 5
  SDA  = 6
*/

GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> display(GxEPD2_213_BN(SS, 1, 2, 3));
// GxEPD2_3C<GxEPD2_213_Z98c, GxEPD2_213_Z98c::HEIGHT> display(GxEPD2_213_Z98c(SS, 1, 2, 3));

void setup()
{
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);

    display.init(115200, true, 50, false);
    helloWorld();

    display.hibernate();

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);
    strip.begin(); // Initialize the LED strip
    strip.clear(); // Turn off all LEDs
    strip.show();  // Send the updated color to the strip
}

void helloWorld()
{
    display.setRotation(1);
    display.setFont(&FreeMonoBold12pt7b);
    display.setTextColor(GxEPD_BLACK);

    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds("Hello, world!", 0, 0, &tbx, &tby, &tbw, &tbh);

    // center the bounding box by transposition of the origin:
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = ((display.height() - tbh) / 2) - tby;

    display.setFullWindow();
    display.firstPage();

    do
    {
        display.fillScreen(GxEPD_WHITE);

        display.setCursor(x, y);
        display.print("Hello, world!");

        // display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        // display.getTextBounds(lineTwo, 0, 0, &tbx, &tby, &tbw, &tbh);
        // x = ((display.width() - tbw) / 2) - tbx;
        // display.setCursor(x, y + tbh);
        // display.print("This is cool");
    } while (display.nextPage());
}

void loop()
{
}
