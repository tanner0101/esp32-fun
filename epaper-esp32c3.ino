
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Adafruit_NeoPixel.h>
#include <cstdlib>
#include <ctime>

/*
  DC   = 1
  RES  = 2
  BUSY = 3
  SCL  = 4
  CS   = 5
  SDA  = 6
*/

constexpr auto DEBUG = false;

constexpr auto HEIGHT = GxEPD2_213_BN::HEIGHT;
constexpr auto WIDTH = GxEPD2_213_BN::WIDTH_VISIBLE;

GxEPD2_BW<GxEPD2_213_BN, HEIGHT> display(GxEPD2_213_BN(SS, 1, 2, 3));

void setup()
{
    pinMode(8, OUTPUT);
    digitalWrite(8, HIGH);

    int baud = DEBUG ? 115200 : 0;
    display.init(baud, true, 50, false);

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    // clearWindow();
    // hellloWorld();
    // circles();
    // fuzz();

    gameOfLife();
}

std::array<std::array<bool, HEIGHT>, WIDTH> grid{0};
std::array<std::array<bool, HEIGHT>, WIDTH> newGrid{0};

int countNeighbors(int x, int y)
{
    int count = 0;
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx != 0 || dy != 0)
            {
                int nx = (x + dx + WIDTH) % WIDTH;
                int ny = (y + dy + HEIGHT) % HEIGHT;
                count += grid[nx][ny];
            }
        }
    }
    return count;
}

void rPentominoSeed()
{
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;

    grid[centerX][centerY] = true;
    grid[centerX + 1][centerY] = true;
    grid[centerX - 1][centerY + 1] = true;
    grid[centerX][centerY + 1] = true;
    grid[centerX][centerY + 2] = true;
}

void randomSeed()
{
    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned>(std::time(0)));

    // Randomly populate the grid
    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            grid[x][y] = std::rand() % 10 == 0;
        }
    }
}

void gameOfLife()
{
    randomSeed();

    while (true)
    {
        display.fillScreen(GxEPD_WHITE);

        for (int x = 0; x < WIDTH; ++x)
        {
            for (int y = 0; y < HEIGHT; ++y)
            {
                int neighbors = countNeighbors(x, y);
                if (grid[x][y])
                {
                    newGrid[x][y] = (neighbors == 2 || neighbors == 3);
                }
                else
                {
                    newGrid[x][y] = (neighbors == 3);
                }
                display.writePixel(x, y, newGrid[x][y] ? GxEPD_BLACK : GxEPD_WHITE);
            }
        }
        grid.swap(newGrid);
        display.display(true);
    }
}

void fuzz()
{
    display.fillScreen(GxEPD_WHITE);
    uint16_t color = GxEPD_BLACK;
    for (auto w = 0; w < display.width(); w++)
    {
        for (auto h = 0; h < display.height(); h++)
        {
            display.writePixel(w, h, color);
            color = (color == GxEPD_BLACK) ? GxEPD_WHITE : GxEPD_BLACK;
        }
        color = (color == GxEPD_BLACK) ? GxEPD_WHITE : GxEPD_BLACK;
        display.display(true);
    }
}

void circles()
{
    display.fillScreen(GxEPD_WHITE);

    int16_t r = 5;
    uint16_t x = display.width() / 2;
    uint16_t y = display.height() / 2;

    while (true)
    {
        for (const auto color : {GxEPD_BLACK, GxEPD_WHITE})
        {
            for (int i = 0; i < 27; ++i)
            {
                display.drawCircle(x, y, r + i * 5, color);
                display.display(true);
            }
        }
    }

    display.hibernate();
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
        display.print("Hello, world");

        // display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
        // display.getTextBounds(lineTwo, 0, 0, &tbx, &tby, &tbw, &tbh);
        // x = ((display.width() - tbw) / 2) - tbx;
        // display.setCursor(x, y + tbh);
        // display.print("This is cool");
    } while (display.nextPage());
}

void clearWindow()
{
    display.setFullWindow();
    display.firstPage();
    do
    {
        display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());
}

void loop()
{
}
