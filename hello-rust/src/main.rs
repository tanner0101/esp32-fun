#![no_std]
#![no_main]

use embedded_hal::spi::SpiBus;
use esp_backtrace as _;
use esp_hal::{
    clock::ClockControl,
    delay::Delay,
    gpio::Io,
    peripherals::Peripherals,
    prelude::*,
    spi::{master::Spi, SpiMode},
    system::SystemControl,
};

use smart_leds::{
    colors::{CYAN, MAGENTA, YELLOW},
    RGB8,
};

struct Ws2812<'a, Spi>
where
    Spi: SpiBus<u8>,
{
    spi: &'a mut Spi,
}

impl<'a, Spi> Ws2812<'a, Spi>
where
    Spi: SpiBus<u8>,
{
    fn new(spi: &'a mut Spi) -> Self {
        Self { spi }
    }

    fn send_byte(&mut self, byte: u8) -> Result<(), Spi::Error> {
        const PATTERNS: [u8; 4] = [0b1000_1000, 0b1000_1110, 0b11101000, 0b11101110];
        let mut current = byte.clone();
        for _ in 0..4 {
            let bits = (current & 0b1100_0000) >> 6;
            self.spi.write(&[PATTERNS[bits as usize]])?;
            current <<= 2;
        }
        Ok(())
    }

    fn send_color(&mut self, color: RGB8) -> Result<(), Spi::Error> {
        self.send_byte(color.g)?;
        self.send_byte(color.r)?;
        self.send_byte(color.b)?;
        Ok(())
    }
}

#[entry]
fn main() -> ! {
    let peripherals = Peripherals::take();
    let system = SystemControl::new(peripherals.SYSTEM);

    let clocks = ClockControl::max(system.clock_control).freeze();
    let delay = Delay::new(&clocks);

    esp_println::logger::init_logger_from_env();

    let io = Io::new(peripherals.GPIO, peripherals.IO_MUX);
    let mut spi = Spi::new(peripherals.SPI2, 3_333_333.Hz(), SpiMode::Mode1, &clocks)
        .with_mosi(io.pins.gpio8);

    let mut led = Ws2812::new(&mut spi);

    let mut i = 0;
    loop {
        log::info!("Hello, world! {}", i);
        i += 1;

        led.send_color(CYAN).unwrap();
        delay.delay(500.millis());
        led.send_color(MAGENTA).unwrap();
        delay.delay(500.millis());
        led.send_color(YELLOW).unwrap();
        delay.delay(500.millis());
    }
}
