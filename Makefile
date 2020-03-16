
#SKETCH=led_control.ino

#mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
#current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
#ESP_ROOT=$(HOME)/Arduino/esp8266


## download esp32 core:
#cd ~                                                   
#git clone https://github.com/espressif/arduino-esp32.git  esp32
#cd esp32
#git submodule update --init
#cd tools     
#python get.py


UPLOAD_PORT = /dev/ttyUSB0
CHIP=esp32
ESP_ROOT=$(HOME)/Arduino/hardware/espressif/esp32/
#BOARD=nodemcuv2
#nodemcu #nodemcuv2

# Didn't like /tmp ???
# BUILD_DIR=bin/

FLASH_DEF=4M2M
#FS_DIR=data
#UPLOAD_SPEED=460800

# EXTRAFLAGS+=-DDEBUG_ESP_PORT=Serial


DEVICEID = 1
HOSTNAME = "O-MI-${DEVICEID}"
PWD = "theuploadpass"

OTA_ADDR = ${HOSTNAME}
#192.168.8.1
OTA_PWD = ${PWD}

BUILD_EXTRA_FLAGS+=-DHOSTNAME="\"${HOSTNAME}\""
BUILD_EXTRA_FLAGS+=-DPWD="\"${PWD}\""


include $(HOME)/Arduino/makeEspArduino/makeEspArduino.mk
