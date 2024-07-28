
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
// #include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Adafruit_NeoPixel.h>
#include <ctime>
#include <string>

constexpr auto EPD_DC = 1;
constexpr auto EPD_RES = 2;
constexpr auto EPD_BUSY = 3;
// constexpr auto EPD_SCL = 4;
// constexpr auto EPD_SDA = 6;
constexpr auto EPD_CS = 7;

constexpr auto DEBUG = true;

constexpr auto HEIGHT = GxEPD2_213_BN::HEIGHT;
constexpr auto WIDTH = GxEPD2_213_BN::WIDTH_VISIBLE;
GxEPD2_BW<GxEPD2_213_BN, HEIGHT> display(GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY));

// constexpr auto HEIGHT = GxEPD2_213_Z19c::HEIGHT;
// constexpr auto WIDTH = GxEPD2_213_Z19c::WIDTH_VISIBLE;
// GxEPD2_3C<GxEPD2_213_Z19c, GxEPD2_213_Z19c::HEIGHT> display(GxEPD2_213_Z19c(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY));

constexpr auto BAUD = 115200;

constexpr auto BUTTON_OUT = 8;
constexpr auto BUTTON_IN = 10;

class EPD32
{
    enum class State
    {
        init,
        menuIdle,
        startProgram,
        runProgram
    };

    State state{State::init};

    enum class Program
    {
        mandelbrot,
        hypnotize,
        gameOfLife
    };

    Program program{Program::mandelbrot};

    bool buttonDown()
    {
        return digitalRead(BUTTON_IN);
    }

    const char *programName(Program program)
    {
        switch (program)
        {
        case Program::mandelbrot:
            return "mandelbrot";
        case Program::hypnotize:
            return "hypnotize";
        case Program::gameOfLife:
            return "game of life";
        default:
            return "unknown";
        }
    }

    Program nextProgram(Program current)
    {
        switch (current)
        {
        case Program::mandelbrot:
            return Program::hypnotize;
        case Program::hypnotize:
            return Program::gameOfLife;
        case Program::gameOfLife:
            return Program::mandelbrot;
        default:
            return Program::mandelbrot;
        }
    }

    bool isInMandelbrot(int16_t x, int16_t y, int max_iteration)
    {
        constexpr float X_SCALE = 1.5f / WIDTH;
        constexpr float Y_SCALE = 2.0f / HEIGHT;
        constexpr float X_OFFSET = -1.25f;
        constexpr float Y_OFFSET = -1.0f;

        float real = static_cast<float>(x) * X_SCALE + X_OFFSET;
        float imag = static_cast<float>(y) * Y_SCALE + Y_OFFSET;
        float zr = 0.0f;
        float zi = 0.0f;
        float zr2 = 0.0f;
        float zi2 = 0.0f;
        int iteration = 0;

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

    int i{0};

    void startMandelbrot()
    {
        i = 3;
        display.fillScreen(GxEPD_WHITE);
    }

    void runMandelbrot()
    {
        for (int16_t y = 0; y < HEIGHT; y++)
        {
            for (int16_t x = 0; x < WIDTH; x++)
            {
                const auto color = isInMandelbrot(x, y, i) ? GxEPD_BLACK : GxEPD_WHITE;
                display.writePixel(x, y, color);
            }
        }
        Serial.printf("mandelbrot iter = %d\n", i);
        display.display(true);
        ++i;
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

        int density = std::max(std::rand() % 20, 2);
        for (int x = 0; x < WIDTH; ++x)
        {
            for (int y = 0; y < HEIGHT; ++y)
            {
                grid[x][y] = std::rand() % density == 0;
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

    void hypnotize()
    {
        int r = 5;
        uint16_t x = display.width() / 2;
        uint16_t y = display.height() / 2;

        int z = 0;
        while (true)
        {
            display.fillScreen(GxEPD_WHITE);

            for (int i = 0; i < 28; ++i)
            {
                display.drawCircle(x, y, i * r + z, GxEPD_BLACK);
            }

            ++z;
            if (z >= r)
            {
                z = 0;
            }

            display.display(true);
        }
    }

    void helloWorld()
    {
        showText("Hello, world!");
    }

    void showText(const char *text)
    {
        display.setRotation(1);
        display.setFont(&FreeMonoBold12pt7b);
        display.setTextColor(GxEPD_BLACK);

        int16_t tbx, tby;
        uint16_t tbw, tbh;
        display.getTextBounds(text, 0, 0, &tbx, &tby, &tbw, &tbh);

        // center the bounding box by transposition of the origin:
        uint16_t x = ((display.width() - tbw) / 2) - tbx;
        uint16_t y = ((display.height() - tbh) / 2) - tby;

        display.fillScreen(GxEPD_WHITE);

        display.setCursor(x, y);
        display.print(text);
        display.display(true);
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

    void _run()
    {

        switch (state)
        {
        case State::init:
        {
            program = Program::mandelbrot;
            showText(programName(program));
            state = State::menuIdle;
        }
        break;
        case State::menuIdle:
        {
            if (buttonDown())
            {
                delay(1000);
                if (!buttonDown())
                {
                    program = nextProgram(program);
                    Serial.printf("next program %s\n", programName(program));
                    showText(programName(program));
                }
                else
                {
                    Serial.printf("start program %s\n", programName(program));
                    state = State::startProgram;
                }
            }
        }
        break;
        case State::startProgram:
        {
            switch (program)
            {
            case Program::mandelbrot:
            {
                startMandelbrot();
                state = State::runProgram;
            }
            break;
            default:
                break;
            };
        }
        break;
        case State::runProgram:
        {
            switch (program)
            {
            case Program::mandelbrot:
            {
                runMandelbrot();
            }
            break;
            default:
                break;
            };
        }
        break;
        default:
        {
            state = State::init;
        }
        break;
        }
    }

public:
    void run()
    {
        _run();
    }
};

EPD32 epd32{};

void setup()
{
    pinMode(BUTTON_OUT, OUTPUT);
    digitalWrite(BUTTON_OUT, HIGH);
    pinMode(BUTTON_IN, INPUT);

    Serial.begin(BAUD);

    display.init(BAUD, true, 50, false);

    Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();
    strip.setPixelColor(0, strip.Color(255, 0, 0));

    // clearWindow();
    // helloWorld();
    // hypnotize();
    // fuzz();
    // gameOfLife();
    // drawMandelbrot();
}

void loop()
{
    epd32.run();
}
