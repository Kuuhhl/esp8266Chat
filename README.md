<br />
<p align="center">
  <img src="icon.png" alt="Logo" width="100" height="100">

  <h3 align="center">esp8266Chat</h3>
  <p align="center">a simple chat server for the esp8266.</p>
</p>

## Screenshots
![Screenshot](screenshot.png)


## Installation (for NodeMCU)
1. Download the [latest release](https://github.com/Kuuhhl/esp8266Chat/releases/) `.bin` file.
2. Download and install [esptool.py](https://github.com/espressif/esptool).
3. Flash the `.bin` file using this command: `esptool.py -p /dev/ttyUSB0 write_flash -fm dout 0x0000 nodemcu.bin`

## Usage
1. Connect the esp8266 to a powerbank.
2. Connect to the newly created Wifi-Hotspot `Chat Server`.
3. Open any URL in your browser, it should automatically redirect you. (it doesn't work? try to replace `https://` in your URL with `http://`.)

## Endpoints
* `/` default landing page.
* `/styles.css` css styles for the landing page.
* `/scripts.js` js scripts for the landing page.
* `/sendText` POST endpoint to send a new message. Parameters: `nickname`, `text`.
* `/showText` content of messages.txt file.
* `/clear` clear all messages.
