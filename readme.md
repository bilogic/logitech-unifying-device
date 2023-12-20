# Library to build Logitech Unifying compatible devices
I have always wanted an 84 key standard layout, wireless, mechanical keyboard with RGB backlight and rotary knobs plus media control buttons near the spacebar. Anyway, it did not take long for me to realize that bluetooth keyboards are notoriously unreliable, intermittently disconnecting every so often. However, Unifying ones are much better.

I never planned to publish this code, thus it is not the cleanest. But since I'm stuck, I thought someone might put it to good use and maybe figure out the problem. If you can make a more secured Unifying compatible protocol, all the better.

### Many thanks to:
- Ronan Gaillard https://github.com/ronangaillard/logitech-mouse/issues/5
- RoganDawes and Marcus Meng https://github.com/RoganDawes/LOGITacker/issues/55
- Code to perform AES ECB mode, will gladly attribute it if someone knows its origin

## Compatibility
- Keystrokes A-Z working up to 012.010.00019
- Keystrokes A-Z not working for 012.010.00032
- Keystrokes A-Z not working for 024.001.00023

## Hardware
- Logitech ```C-U0007``` dongle with firmware ```012.001.00019```, dongles with newer firmware are doing something to reject our keystrokes.
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
