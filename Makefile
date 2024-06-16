.PHONY: compile upload monitor

compile:
	arduino-cli compile -b esp32:esp32:upesy_esp32c3_mini

upload: compile
	arduino-cli upload -p /dev/ttyACM0 -b esp32:esp32:upesy_esp32c3_mini

monitor:
	arduino-cli monitor -p /dev/ttyACM0 -b esp32:esp32:upesy_esp32c3_mini
