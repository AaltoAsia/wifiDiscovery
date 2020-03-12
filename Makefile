
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


ESP_ADDR = wifitest
ESP_PWD = insert_pwd
OTA_ADDR = ESP_ADDR
#192.168.4.1
OTA_PWD = ${ESP_PWD}

BUILD_EXTRA_FLAGS+=-DESP_ADDR="\"${ESP_ADDR}\""
BUILD_EXTRA_FLAGS+=-DESP_PWD="\"${ESP_PWD}\""


include $(HOME)/Arduino/makeEspArduino/makeEspArduino.mk
