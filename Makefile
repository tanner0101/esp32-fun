.PHONY: compile upload monitor

# ESP32_BOARD=esp32:esp32:esp32
# ESP32_BOARD=esp32:esp32:esp32c6
# DEV_PATH=/dev/ttyUSB0
ESP32_BOARD=esp32:esp32:upesy_esp32c3_mini
DEV_PATH=/dev/ttyACM0

build:
	arduino-cli compile -b $(ESP32_BOARD)

flash:
	arduino-cli upload -p $(DEV_PATH) -b $(ESP32_BOARD)

log:
	arduino-cli monitor -p $(DEV_PATH) -b $(ESP32_BOARD) --config baudrate=115200