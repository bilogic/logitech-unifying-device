#include <Arduino.h>
#include <elapsedMillis.h>
#include "ludevice.h"

elapsedMillis move_timer;
ludevice kespb(D4, D3);

float mouseSpeed = 10.0f;
float degreesToRadians = 2.0f * 3.14f / 360.0f;
bool keydown = false;

void setup()
{
    int retcode;
    Serial.setDebugOutput(true);
    Serial.begin(921600);
    Serial.println("Starting");

    kespb.begin();
    while (1)
    {
        if (kespb.reconnect())
        {
            printf("Reconnected!\r\n");
            break;
        }
        else
        {
            retcode = kespb.pair();
            if (retcode == true)
            {
                if (kespb.register_device())
                {
                    printf("Paired and connected\r\n");
                    break;
                }
            }
            else
            {
                if (retcode == false)
                {
                    printf("No dongle wants to pair\r\n");
                    delay(5000); // sleep for 5 seconds before trying to pair again
                }
            }
        }
        yield();
    }
}

void loop()
{
    // kespb.loop();
    // return;
    if ((move_timer > 5000))
    {
        // keydown
        if (!keydown)
        {
            kespb.typee(4, 5, 6, 7, 8, 9);
            // kespb.typep(4, 5, 6, 7, 8, 9);
            // kespb.typem(0x192, 0); // calculator
            // kespb.typem(0x183, 0); // video player

            kespb.typem(0xe2); // toggle mute
            // kespb.typem(0xe9); // volume up
            // kespb.typem(0xea); // volume down
            // kespb.typem(0xb6); // rev
            // kespb.typem(0xcd); // play/pause
            // kespb.typem(0xb5); // fwd

            keydown = true;
        }
        else
        {
            kespb.typep();
            kespb.typee();
            kespb.typem();
            keydown = false;
            move_timer = 0;
        }
    }
    kespb.loop();
    return;

    if (move_timer > 1000)
    {
        int x, y = 0;

        Serial.println("moving mouse ");
        for (x = 0; x < 360; x += 5)
        {
            Serial.print(".");

            kespb.move((uint16_t)(mouseSpeed * cos(((float)x) * degreesToRadians)),
                       (uint16_t)(mouseSpeed * sin(((float)x) * degreesToRadians)));

            // delay(1000);
            // kespb.typee();
        }
        Serial.println("");
        move_timer = 0;
    }
}
