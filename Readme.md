WiFi Discovery test
==================

Test WiFi discovery mechanism where the first device is configured with a web page on AP captive portal and from the second device onwards they can request the wifi credentials from the first device if in range.

Building
---------

1. Clone the repo with `git clone --recursive `
2. Install [makeEspArduino](https://github.com/plerup/makeEspArduino) to `${HOME}/Arduino/makeEspArduino/makeEspArduino.mk` (or change paths in Makefile)
3. Connect the ESP device
3. Change `UPLOAD_PORT` (and `ESP_ROOT` if needed) in Makefile, check other settings also
4. `make upload`

