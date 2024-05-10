# Library to build Logitech Unifying compatible devices
I have always wanted an 84 key standard layout, wireless, mechanical keyboard with RGB backlight and rotary knobs plus media control buttons near the spacebar. Anyway, it did not take long for me to realize that bluetooth keyboards are notoriously unreliable, intermittently disconnecting every so often. However, Unifying ones are much better.

I never planned to publish this code, thus it is not the cleanest. ~~But since I'm stuck, I thought someone might put it to good use and maybe figure out the problem. If you can make a more secured Unifying compatible protocol, all the better.~~

### Many thanks to:
- Ronan Gaillard https://github.com/ronangaillard/logitech-mouse/issues/5
- RoganDawes and Marcus Meng https://github.com/RoganDawes/LOGITacker/issues/55
- Code to perform AES ECB mode, will gladly attribute it if someone knows its origin

## C-U0007 mitigation effort from firmware `012.010.00032` onwards
In an effort to prevent keys injection, the receiver's firmware will reject packets that start with a full buffer of 6 keystrokes, so start by sending packets slowly (like a human). If you know of how it is done exactly, feel free to PR a write up.

## Hardware
- Logitech ```C-U0007``` dongle
- ESP8266, NRF24L01+ and a 5v power board for the NRF24L01+ module, connect them as follows:
  - WEMOS D1 mini D3 <-> power board CS
  - WEMOS D1 mini D4 <-> power board CE
  - WEMOS D1 mini D5 <-> power board SCK
  - WEMOS D1 mini D6 <-> power board MISO
  - WEMOS D1 mini D7 <-> power board MOSI
  - WEMOS D1 mini 5V <-> power board VCC
  - WEMOS D1 mini GND <-> power board GND

## Software
- Open ```logitech-unifying-device.code-workspace``` in VSCode with PlatformIO installed
- Press ```ctrl + alt + u``` to compile
- If it is your first time using PlatformIO, **wait** until the icon appears on the activity bar at the left of VSCode before compiling, it should just work
