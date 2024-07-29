#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>

constexpr auto WIDTH = 128; // OLED display width, in pixels
constexpr auto HEIGHT = 32; // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

bool isInMandelbrot(int16_t x, int16_t y, int zoom)
{
    // Using exponential decay for the zoom factor to keep the zoom speed constant
    float factor = pow(0.95, zoom);

    // Coordinates of the center of the zoomed area
    float centerX = -0.749f;
    float centerY = 0.1008f;

    float X_SCALE = (4.0f * factor) / WIDTH;
    float X_OFFSET = centerX - (2.0f * factor);

    float Y_SCALE = (1.0f * factor) / HEIGHT;
    float Y_OFFSET = centerY - (0.5f * factor);

    float real = static_cast<float>(x) * X_SCALE + X_OFFSET;
    float imag = static_cast<float>(y) * Y_SCALE + Y_OFFSET;
    float zr = 0.0f;
    float zi = 0.0f;
    float zr2 = 0.0f;
    float zi2 = 0.0f;
    int iteration = 0;

    const auto max_iteration = 100;
    while (zr2 + zi2 < 4.0f && iteration < max_iteration)
    {
        zi = 2.0f * zr * zi + imag;
        zr = zr2 - zi2 + real;
        zr2 = zr * zr;
        zi2 = zi * zi;
        iteration++;
    }

    return iteration == max_iteration;
}

void startMandelbrot()
{
    display.fillScreen(WHITE);
}

int zoom = 1;
int incr = 1;

void runMandelbrot()
{
    for (int16_t y = 0; y < HEIGHT; y++)
    {
        for (int16_t x = 0; x < WIDTH; x++)
        {
            const auto color = isInMandelbrot(x, y, zoom) ? WHITE : BLACK;
            display.writePixel(x, y, color);
        }
    }
    Serial.printf("mandelbrot zoom = %d\n", zoom);
    display.display();
    zoom += incr;
    if (zoom > 300)
    {
        incr = -1;
    }
    if (zoom <= 1)
    {
        incr = 1;
    }
}

void setup()
{
    Serial.begin(115200);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    display.display();
    delay(2000); // Pause for 2 seconds

    // Clear the buffer
    display.clearDisplay();

    // Draw a single pixel in white
    display.drawPixel(10, 10, WHITE);

    startMandelbrot();
}

void loop()
{
    runMandelbrot();
}