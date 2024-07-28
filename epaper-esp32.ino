
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_BW.h>
// #include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Adafruit_NeoPixel.h>
#include <ctime>
#include <string>
#include <esp_sleep.h>

class EPD32
{
    enum class State
    {
        init,
        menuIdle,
        startProgram,
        runProgram,
        idle
    };

    State state{State::init};

    enum class Program
    {
        mandelbrot,
        hypnotize,
        gameOfLife
    };

    Program program{Program::mandelbrot};

    static constexpr auto EPD_DC = 1;
    static constexpr auto EPD_RES = 2;
    static constexpr auto EPD_BUSY = 3;
    // constexpr auto EPD_SCL = 4;
    // constexpr auto EPD_SDA = 6;
    static constexpr auto EPD_CS = 7;

    static constexpr auto DEBUG = true;

    static constexpr auto HEIGHT = GxEPD2_213_BN::HEIGHT;
    static constexpr auto WIDTH = GxEPD2_213_BN::WIDTH_VISIBLE;

    // constexpr auto HEIGHT = GxEPD2_213_Z19c::HEIGHT;
    // constexpr auto WIDTH = GxEPD2_213_Z19c::WIDTH_VISIBLE;
    // GxEPD2_3C<GxEPD2_213_Z19c, GxEPD2_213_Z19c::HEIGHT> display(GxEPD2_213_Z19c(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY));

    static constexpr auto BAUD = 115200;

    static constexpr auto BUTTON_IN = 0;

    GxEPD2_BW<GxEPD2_213_BN, HEIGHT> display{GxEPD2_213_BN(EPD_CS, EPD_DC, EPD_RES, EPD_BUSY)};

    bool ignoreButtonDown{false};

    bool buttonDown()
    {
        if (ignoreButtonDown)
        {
            return false;
        }

        return digitalRead(BUTTON_IN);
    }

    bool buttonUp()
    {
        return !digitalRead(BUTTON_IN);
    }

    const char *programName(Program &program)
    {
        switch (program)
        {
        case Program::mandelbrot:
            return "mandelbrot";
        case Program::gameOfLife:
            return "game of life";
        case Program::hypnotize:
            return "hypnotize";
        default:
            return "unknown";
        }
    }

    Program nextProgram(Program current)
    {
        switch (current)
        {
        case Program::mandelbrot:
            return Program::gameOfLife;
        case Program::gameOfLife:
            return Program::hypnotize;
        case Program::hypnotize:
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
        clearWindow(true);
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
                if (EPD32::cancel)
                {
                    EPD32::cancel = false;
                    Serial.println("cancel mandelbrot");
                    return;
                }
            }
        }
        Serial.printf("mandelbrot iter = %d\n", i);
        display.display(true);
        ++i;

        if (i >= 30)
        {
            state = State::idle;
        }
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

    void startGameOfLife()
    {
        clearWindow(true);
        randomSeed();
    }

    void runGameOfLife()
    {
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
                if (EPD32::cancel)
                {
                    EPD32::cancel = false;
                    Serial.println("cancel game of life");
                    return;
                }
            }
        }
        grid.swap(newGrid);
        display.display(true);
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

    void startHypnotize()
    {
        i = 0;
        clearWindow(false);
    }

    void runHypnotize()
    {
        display.fillScreen(GxEPD_WHITE);

        const int r = 5;
        const uint16_t x = display.width() / 2;
        const uint16_t y = display.height() / 2;

        for (int j = 0; j < 28; ++j)
        {
            display.drawCircle(x, y, j * r + i, GxEPD_BLACK);
        }

        ++i;
        if (i >= r)
        {
            i = 0;
        }

        display.display(true);
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

    void clearWindow(bool partial)
    {
        display.setFullWindow();
        display.setRotation(0);
        display.setCursor(0, 0);
        display.firstPage();
        do
        {
            display.fillScreen(GxEPD_WHITE);
        } while (display.nextPage() && !partial);
    }

    void sleep()
    {

        esp_deep_sleep_enable_gpio_wakeup(BUTTON_IN, ESP_GPIO_WAKEUP_GPIO_HIGH);
        Serial.println("Going to sleep now");
        delay(1000);
        esp_deep_sleep_start();
    }

    void _run()
    {
        if (buttonUp())
        {
            ignoreButtonDown = false;
        }

        switch (state)
        {
        case State::init:
        {
            delay(1000);
            EPD32::cancel = false;
            Serial.println("init");
            showText(programName(program));
            state = State::menuIdle;
        }
        break;
        case State::menuIdle:
        {
            if (buttonDown())
            {
                delay(500);
                if (buttonDown())
                {
                    Serial.printf("start program %s\n", programName(program));
                    state = State::startProgram;
                    ignoreButtonDown = true;
                }
                else
                {
                    program = nextProgram(program);
                    Serial.printf("next program %s\n", programName(program));
                    showText(programName(program));
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
            case Program::gameOfLife:
            {
                startGameOfLife();
                state = State::runProgram;
            }
            break;
            case Program::hypnotize:
            {
                startHypnotize();
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
            if (buttonDown())
            {
                ignoreButtonDown = true;
                state = State::init;
                return;
            }

            switch (program)
            {
            case Program::mandelbrot:
            {
                runMandelbrot();
            }
            break;
            case Program::gameOfLife:
            {
                runGameOfLife();
            }
            break;
            case Program::hypnotize:
            {
                runHypnotize();
            }
            break;
            default:
                break;
            };
        }
        break;
        case State::idle:
        {
            if (buttonDown())
            {
                ignoreButtonDown = true;
                state = State::init;
                return;
            }

            // sleep();
        }
        break;
        default:
        {
            state = State::init;
        }
        break;
        }
    }

    static volatile bool cancel;

public:
    void setup()
    {
        pinMode(BUTTON_IN, INPUT_PULLDOWN);
        attachInterrupt(digitalPinToInterrupt(BUTTON_IN), handleButton, RISING);

        Serial.begin(BAUD);

        display.init(BAUD, true, 50, false);

        Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, 7, NEO_GRB + NEO_KHZ800);
        strip.begin();
        strip.clear();
        strip.show();
    }

    static void IRAM_ATTR handleButton()
    {
        EPD32::cancel = true;
        Serial.println("button");
    }

    void run()
    {
        _run();
    }
};

volatile bool EPD32::cancel = false;

EPD32 epd32{};

void setup()
{
    // clearWindow();
    // hypnotize();
    // fuzz();
    // gameOfLife();
    // drawMandelbrot();

    epd32.setup();
}

void loop()
{
    epd32.run();
}
