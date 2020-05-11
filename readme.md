# Library to build Logitech Unifying compatible devices
I have always wanted an 84 key standard layout, wireless, mechanical keyboard with RGB backlight and rotary knobs plus media control buttons near the spacebar. Anyway, it did not take long for me to realize that bluetooth keyboards are notoriously unreliable, intermittently disconnecting every so often. However, Unifying ones are much better.

I never planned to publish this code, thus it is not the cleanest. But since I'm stuck, I thought someone might put it to good use and maybe figure out the problem. If you can make a more secured Unifying compatible protocol, all the better.

### Many thanks to:
- Ronan Gaillard https://github.com/ronangaillard/logitech-mouse/issues/5
- RoganDawes and Marcus Meng https://github.com/RoganDawes/LOGITacker/issues/55
- Code to perform AES ECB mode, will gladly attribute it if someone knows its origin

## Hardware
- Logitech ```C-U0007``` dongle with firmware ```012.001.00019```, newer versions do something to reject keystrokes.
- ESP8266 and NRF24L01+, connect them as follows:
  - WEMOS D1 mini D3 <-> NRF24L01+ CS
  - WEMOS D1 mini D4 <-> NRF24L01+ CE
  - WEMOS D1 mini D5 <-> NRF24L01+ SCK
  - WEMOS D1 mini D6 <-> NRF24L01+ MISO
  - WEMOS D1 mini D7 <-> NRF24L01+ MOSI

## Software
- Open ```logitech-unifying-device.code-workspace``` in VSCode with PlatformIO installed
- Press ```ctrl + alt + u``` to compile
- If it is your first time using PlatformIO, **wait** until the icon appears on the activity bar at the left of VSCode before compiling, it should just work

## Known issues:
- Encrypted keystrokes **are not accepted** by the latest firmware ```012.010.00032``` for ```C-U0007``` dongles
    - See https://github.com/bilogic/logitech-unifying-device/issues/1
