/*
 Copyright (C) 2017 Ronan Gaillard <ronan.gaillard@live.fr>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
*/
#include "ludevice.h"

#ifdef EEPROM_SUPPORT
#include <EEPROM.h>
#endif

ludevice::ludevice() : ludevice(DEFAULT_CE_PIN, DEFAULT_CS_PIN)
{
}

ludevice::ludevice(uint8_t _cepin, uint8_t _cspin) : radio(_cepin, _cspin)
{
}

void ludevice::setAddress(uint64_t address)
{
    setAddress((uint8_t *)&address);
}

void ludevice::setAddress(uint8_t *address)
{
    uint8_t address_dongle[5];

    // printf("Setting address: %s\r\n", hexa(address, 5));

    memcpy(address_dongle, address, 4);
    address_dongle[0] = 0;

    radio.stopListening();
    radio.openReadingPipe(2, address_dongle);
    radio.openReadingPipe(1, address);
    radio.openWritingPipe(address);
}

bool ludevice::begin()
{
    uint8_t init_status = radio.begin();

    // radio.printDetails();

    if (init_status == 0 || init_status == 0xff)
    {
        return false;
    }

    // aes_base = 0x5897AF67;
    // aes_base = 0x5897AF60;
    aes_base = random(0xfffffff + 1) << 4;

    EEPROM.begin(1 + 5 + 16);
    EEPROM.get(MAC_ADDRESS_EEPROM_ADDRESS + 0, current_channel);
    EEPROM.get(MAC_ADDRESS_EEPROM_ADDRESS + 1, rf_address);
    EEPROM.get(MAC_ADDRESS_EEPROM_ADDRESS + 1 + 5, device_key);

    radio.stopListening();
    if (1)
    {
        radio.setAutoAck(1);
        radio.setRetries(3, 1);
        radio.setPayloadSize(PAYLOAD_SIZE);
        radio.enableDynamicPayloads();
        radio.enableAckPayload();
        radio.enableDynamicAck();
        radio.openWritingPipe(PAIRING_MAC_ADDRESS);
        radio.openReadingPipe(1, PAIRING_MAC_ADDRESS);
        changeChannel();
        radio.setDataRate(RF24_2MBPS);
        {
            // writeRegister(SETUP_AW, 0x03); // Reset addr size to 5 bytes
            digitalWrite(DEFAULT_CS_PIN, LOW);
            SPI.transfer(W_REGISTER | (REGISTER_MASK & 0x3));
            SPI.transfer(0x03);
            digitalWrite(DEFAULT_CS_PIN, HIGH);
        }
    }
    radio.stopListening();

    // radio.openWritingPipe(PAIRING_MAC_ADDRESS);
    // radio.openReadingPipe(1, PAIRING_MAC_ADDRESS);
    // radio.setAutoAck(1);

    // radio.setPALevel(RF24_PA_MAX);

    // radio.setDataRate(RF24_2MBPS);
    // radio.setPayloadSize(PAYLOAD_SIZE);
    // radio.enableDynamicPayloads();
    // radio.enableAckPayload();
    // radio.enableDynamicAck();
    // radio.setRetries(3, 1);
    // changeChannel();

    radio.stopListening();

    return true;
}

void ludevice::setChecksum(uint8_t *payload, uint8_t len)
{
    uint8_t checksum = 0;

    for (uint8_t i = 0; i < (len - 1); i++)
        checksum += payload[i];

    payload[len - 1] = -checksum;
}

void ludevice::hidpp10(uint8_t *rf_payload, uint8_t payload_size)
{
    uint8_t rf_response[22] = {0};
    uint8_t reply = 22;
    char *name = "UNKNOWN PACKET!!! PLEASE TEST AT DONGLE SIDE!!!";

    rf_response[0] = rf_payload[0];
    rf_response[1] = 0x40 | rf_payload[1]; // report ID
    rf_response[2] = rf_payload[2];        // device index
    rf_response[3] = rf_payload[3];        // sub id
    rf_response[4] = rf_payload[4];        // address

    uint32_t feature_id = (rf_payload[5] << 8) + (rf_payload[6]);
    uint32_t addr_param = (rf_payload[4] << 8) + (rf_payload[5]);

    // request sub_id is [2 + 1]
    switch (rf_payload[3])
    {
    case 0x80: // SET_REGISTER
        // reply = 10;
        printf("SETTING REGISTER!!!!!!!!!");
        break;
    case 0x81: // GET_REGISTER
        reply = 10;
        switch (addr_param)
        {
        case 0xf101:
            // firmware major
            name = "Firmware Major";
            rf_response[5] = rf_payload[5];
            rf_response[6] = (firmware_version >> 24) & 0xff;
            rf_response[7] = (firmware_version >> 16) & 0xff;
            break;
        case 0xf102:
            name = "Firmware Minor";
            rf_response[5] = rf_payload[5];
            rf_response[6] = (firmware_version >> 8) & 0xff;
            rf_response[7] = (firmware_version >> 0) & 0xff;
            break;
        case 0xf103:
            name = "Firmware 0x03";
            rf_response[5] = rf_payload[5];
            rf_response[6] = 0x01;
            rf_response[7] = 0x02;
            break;
        case 0xf104:
            name = "Firmware 0x04";
            rf_response[5] = rf_payload[5];
            rf_response[6] = 0x02;
            rf_response[7] = 0x14;
            break;
        case 0x700:
            name = "HIDPP_REG_BATTERY_STATUS bad";
            rf_response[5] = 0x7; //rf_payload[5];
            rf_response[6] = 3;   // capacity
            rf_response[7] = 0x0; // level 1-7
            rf_response[8] = 0x0; // discharing
            break;
        case 0xd00:
            // reply = 0;
            name = "HIDPP_REG_BATTERY_MILEAGE";
            rf_response[5] = 50;     // capacity: 0 - 100
            rf_response[6] = 0;      // nothing: 0
            rf_response[7] = 0 << 6; // status: 0 - discharging, 1 - charging
            break;
        default:
            reply = 0;
            name = "UNKNOWN";
            break;
        }
        break;
    default:
        switch (feature_id)
        {
        case 0x03:
            // 00 10 9E 00 02 00 03 00 B6 97
            // 00 51 9E 00 02 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D - response
            if (addr_param = 0x200)
            {
                name = "copied from K270 (2)";
                reply = 22;
                rf_response[3] = 0x02;
                rf_response[4] = 0x02;
                rf_response[5] = 0x00;
            }
            break;
        case 0x3f13:
            // 00 10 0E 00 12 3F 13 00 00 7E
            // 00 51 0E 00 12 02 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 8D - response
            name = "copied from K270 (1)";
            reply = 22;
            rf_response[3] = 0x00;
            rf_response[4] = 0x12;
            rf_response[5] = 0x02;
            break;
        case 0x0000: // 00 10 0E 00 12 00 00 00 B6 1A
            name = "we don't understand HID++ 2.0";
            reply = 10;
            // HID++ 1.0
            rf_response[3] = 0x8f;
            rf_response[4] = 0;
            // RF rf_response Results start from [4 + 1]
            rf_response[5] = 0x10 + (rf_payload[4] & 0xf);
            rf_response[6] = 1;
            rf_response[7] = 0;
            rf_response[8] = 0;
        }
        break;
    }

    if (reply && (rf_payload[1] == 0x10 || rf_payload[1] == 0x11))
    {
        if (reply == 10)
            rf_response[1] = 0x50;
        if (reply == 22)
            rf_response[1] = 0x51;
        radiowrite(rf_response, reply, name, 5);
    }
}

void ludevice::hidpp20(uint8_t *rf_payload, uint8_t payload_size)
{
    // https://initrd.net/stuff/mousejack/doc/pdf/DEFCON-24-Marc-Newlin-MouseJack-Injecting-Keystrokes-Into-Wireless-Mice.slides.pdf
    // https://drive.google.com/file/d/0B4Pb6jGAmjoKQ3hlZDFxUHVqRkU/view

    // [16.922] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [16.923] 9D:65:CB:58:4D // ACK
    // [17.015] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [17.015] 9D:65:CB:58:4D // ACK
    // [17.108] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [17.108] 9D:65:CB:58:4D // ACK
    // [17.201] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [17.201] 9D:65:CB:58:4D // ACK
    // [17.294] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [17.294] 9D:65:CB:58:4D 00:10:4D:00:14:00:00:00:00:8F // ACK payload; requesting HID++ version
    //                         00:10:ce:00:12:3f:13:00:00:be
    //                         00:10:ce:00:12:00:00:00:11:ff
    // [17.302] 9D:65:CB:58:4D 00:51:4D:00:14:04:05:0000000000000000000000000000:45 // response (HID++ 4.5)
    // [17.302] 9D:65:CB:58:4D // ACK
    // [17.387] 9D:65:CB:58:4D 0040006E52 // keepalive, 110ms interval
    // [17.387] 9D:65:CB:58:4D // ACK
    // https://lekensteyn.nl/files/logitech/logitech_hidpp_2.0_specification_draft_2012-06-04.pdf
    // https://github.com/mame82/UnifyingVulnsDisclosureRepo/blob/master/talk/phishbot_2019_redacted3.pdf
    // https://raw.githubusercontent.com/torvalds/linux/master/drivers/hid/hid-logitech-hidpp.c

    // [0] 00 - device index
    // [1] 10 - Report ID
    //          0x10, 7 bytes UNIFYING_RF_REPORT_HIDPP_SHORT, 0x0E = UNIFYING_RF_REPORT_LED
    //          0x11, 20 bytes REPORT_ID_HIDPP_VERY_LONG
    // [2] CE - Device Index / RF prefix
    // [3] 00 - Sub ID
    // ---
    // [4] 12 - Address
    // [5] xx - value 0
    // [6] xx - value 1
    // [7] xx - value 2
    // [8] 00 -
    // [9] xx - checksum

    uint8_t rf_response[22] = {0};
    uint8_t reply = 22;
    char *name;

    rf_response[0] = rf_payload[0];
    rf_response[1] = 0x51;
    rf_response[2] = rf_payload[2];
    rf_response[3] = rf_payload[3];
    rf_response[4] = rf_payload[4];

    uint32_t feature_id = (rf_payload[5] << 8) + (rf_payload[6]);
    // https://lekensteyn.nl/files/logitech/logitech_hidpp_2.0_specification_draft_2012-06-04.pdf
    switch (feature_id)
    {
    default:
        reply = 0;
        break;
    case 0x0000: // root
        name = "root 0x0000";
        reply = 10;
        // HID++ 2.0, 4.5
        // rf_response[3] = 0x0;
        // rf_response[4] = 0x10 + (ack_payload[4] & 0xf);
        // RF rf_response Results start from [4 + 1]
        rf_response[5] = 0x2;
        rf_response[6] = 0x0;
        rf_response[7] = rf_payload[8]; // ping
        rf_response[8] = 0;
        break;
    case 0x0003: //device info

        // request parms starts from ack_payload[4]
        name = "firmware 0x0003";
        // RF rf_response Results start from [4 + 1]
        rf_response[5] = 0x0;
        rf_response[6] = 'a';
        rf_response[7] = 'b';
        rf_response[8] = 'c';
        rf_response[9] = 0x33;
        rf_response[10] = 0x44;
        rf_response[11] = 0x1;
        rf_response[12] = 0x1;
        rf_response[13] = 0x0; //xx
        rf_response[14] = 'K';
        rf_response[15] = 'S';
        rf_response[16] = 'B';

        break;
    case 0x1000: // battery
        name = "battery 0x1000";
        // RF rf_response Results start from [4 + 1]
        rf_response[5] = 80; // BatteryDischargeLevel
        rf_response[6] = 70; // BatteryDischargeNextLevel
        rf_response[7] = 2;  // 0 - charging
        break;
    case 0x1d4b: //wireless device status
        name = "wireless 0x1d4b";
        rf_response[5] = 0;
        rf_response[6] = 0;
        rf_response[7] = 0;
        break;
    }
    if (reply && rf_payload[1] == 0x10)
    {
        if (reply == 10)
            rf_response[1] = 0x50;
        if (reply == 22)
            rf_response[1] = 0x51;
        radiowrite(rf_response, reply, name, 1);
    }
}

void ludevice::loop(void)
{
    uint8_t response_size = 0;
    if (!is_connected)
        return;

    uint8_t *rf_payload;

    if (radio.available())
    {
        response_size = read(rf_payload);
        hidpp10(rf_payload, response_size);
        // hidpp20(rf_payload, response_size);
    }
    // stay_alive_mouse();
    stay_alive_keyboard();
}

void ludevice::stay_alive_keyboard(void)
{
    uint8_t retry = 5;
    uint16_t send_interval;
    bool silent = true;
    char buffer[30];

    // 8ms for movement, 110ms for 5 seoncds when movement stops, 1200ms after
    if (1)
    {
        switch (keep_alive)
        {
        case 278:
            if (idle_timer > 60000)
            {
                update_keep_alive(1200, retry, silent);
                return;
            }
            break;
        case 1200:
            if (idle_timer > (1000 * 5 * 60))
            {
                printf("- 5 minutes of idle, TODO: go to sleep\r\n");
                idle_timer = 6000;
            }
            break;
        default:
            if (idle_timer > 30000)
            {
                update_keep_alive(278, retry, silent);
                return;
            }
            break;
        }

        send_interval = keep_alive;
        switch (keep_alive)
        {
        case 278:
            send_interval = 250;
            break;
        case 1200:
            send_interval = 1100;
            break;
        }
    }

    if (send_alive_timer > send_interval)
    {
        unsigned long t = idle_timer;
        sprintf(buffer, "%dms keep alive (idle: %d)", keep_alive, t);
        radiowrite_ex(keep_alive_packet, sizeof(keep_alive_packet), buffer, retry, silent);
        send_alive_timer = 0;
        stay_alive_counter++;
    }
}

void ludevice::stay_alive_mouse(void)
{
    uint8_t retry = 5;
    uint16_t send_interval;
    bool silent = false;
    char buffer[30];

    // 8ms for movement, 110ms for 5 seoncds when movement stops, 1200ms after

    switch (keep_alive)
    {
    case 110:
        if (idle_timer > 5000)
        {
            update_keep_alive(1200, retry, silent);
            return;
        }
        break;
    case 1200:
        if (idle_timer > (1000 * 5 * 60))
        {
            printf("- 5 minutes of idle, TODO: go to sleep\r\n");
            idle_timer = 6000;
        }
        break;
    default:
        if (idle_timer > 80)
        {
            update_keep_alive(110, retry, silent);
            return;
        }
        break;
    }

    send_interval = keep_alive;
    switch (keep_alive)
    {
    case 110:
        send_interval = 100;
        break;
    case 1200:
        send_interval = 1100;
        break;
    }

    if (send_alive_timer > send_interval)
    {
        unsigned long t = idle_timer;
        sprintf(buffer, "%dms keep alive (idle: %d)", keep_alive, t);
        radiowrite_ex(keep_alive_packet, sizeof(keep_alive_packet), buffer, retry, silent);
        send_alive_timer = 0;
        stay_alive_counter++;
    }
}

bool ludevice::update_keep_alive(uint16_t timeout, uint8_t retry, bool silent)
{
    char buffer[30];
    keep_alive_packet[2] = ((timeout & 0xff00) >> 8); // timeout
    keep_alive_packet[3] = ((timeout & 0x00ff));      // timeout
    setChecksum(keep_alive_packet, 5);

    keep_alive_change_packet[3] = ((timeout & 0xff00) >> 8); // timeout
    keep_alive_change_packet[4] = ((timeout & 0x00ff));      // timeout
    setChecksum(keep_alive_change_packet, 10);

    retry = 10;
    sprintf(buffer, "set keep alive to %d ms", timeout);
    if (radiowrite_ex(keep_alive_change_packet, sizeof(keep_alive_change_packet), buffer, retry, silent))
    {
        // uint8_t *response;
        // read(response);
        keep_alive = timeout;
        return true;
    }
    return false;
}

bool ludevice::pair_response(uint8_t *packet, char *name, uint8_t retry)
{
    while (retry)
    {
        if (!radiowrite(packet, 5, name, 1))
        {
            retry--;
            if (retry == 0)
                return false;
        }
        else
        {
            if (radio.available())
                break;
        }
    }
}

int ludevice::pair()
{
    bool passed;
    uint8_t retry = 10;
    uint8_t bis_retry;
    uint8_t response_size;
    uint8_t *response;
    uint8_t prefix;

    is_pairing = true;
    setAddress(PAIRING_MAC_ADDRESS);

    {
        // Send REQ1
        prefix = PAIRING_MARKER_PHASE_1; //random(256);
        pairing_packet_1[0] = prefix;
        pairing_packet_1[3] = rf_address[4];
        pairing_packet_1[4] = rf_address[3];
        pairing_packet_1[5] = rf_address[2];
        pairing_packet_1[6] = rf_address[1];
        pairing_packet_1[7] = rf_address[0];

        // lock_channel = false;
        if (!radiowrite(pairing_packet_1, 22, "REQ1", retry))
            return -10;

        lock_channel = true;

        memcpy(device_raw_key_material, pairing_packet_1 + LOGITACKER_UNIFYING_PAIRING_RSP1_OFFSET_BASE_ADDR, 4);       //REQ1 device_rf_address
        memcpy(device_raw_key_material + 4, pairing_packet_1 + LOGITACKER_UNIFYING_PAIRING_REQ1_OFFSET_DEVICE_WPID, 2); //REQ1 device_wpid

        // sending REQ1 success, try sending BIS1 to get a response from dongle
        pairing_packet_1_bis[0] = prefix;
        pairing_packet_1_bis[3] = pairing_packet_1[3];
        bis_retry = 10;
        while (bis_retry)
        {
            if (radiowrite(pairing_packet_1_bis, sizeof(pairing_packet_1_bis), "BIS1", 1))
            {
                response_size = read(response);
                if (response_size > 0)
                {
                    if (response[0] != prefix)
                    {
                        printf("Wrong prefix\r\n");
                    }
                    else
                        break;
                }
                else
                    printf("Empty response\r\n");
            }
            bis_retry--;
        }
        if (bis_retry == 0)
            return false;

        // extract info from BIS1 response
        {
            memcpy(device_raw_key_material + 6, response + LOGITACKER_UNIFYING_PAIRING_RSP1_OFFSET_DONGLE_WPID, 2); //RSP1 dongle_wpid
            for (int i = 0; i < 5; i++)
                rf_address[i] = response[(3 + (4 - i))];
            setAddress(rf_address);
        }
    }

    {
        // Send REQ2
        prefix = PAIRING_MARKER_PHASE_2; //0; //random(256);
        pairing_packet_2[0] = prefix;

        nonce = random(0xffffffff);
        pairing_packet_2[3] = ((nonce & 0xff000000) >> 24); // device nonce MSB
        pairing_packet_2[4] = ((nonce & 0x00ff0000) >> 16); // device nonce
        pairing_packet_2[5] = ((nonce & 0x0000ff00) >> 8);  // device nonce
        pairing_packet_2[6] = ((nonce & 0x000000ff) >> 0);  // device nonce LSB

        serial = random(0xffffffff);
        pairing_packet_2[7] = ((serial & 0xff000000) >> 24); // device serial MSB
        pairing_packet_2[8] = ((serial & 0x00ff0000) >> 16); // device serial
        pairing_packet_2[9] = ((serial & 0x0000ff00) >> 8);  // device serial
        pairing_packet_2[10] = ((serial & 0x000000ff) >> 0); // device serial LSB
        if (!radiowrite(pairing_packet_2, 22, "REQ2", retry))
            return false;

        memcpy(device_raw_key_material + 8, pairing_packet_2 + LOGITACKER_UNIFYING_PAIRING_REQ2_OFFSET_DEVICE_NONCE, 4); //REQ2 device_nonce

        // sending REQ2 success, try sending BIS2 to get a response from dongle
        pairing_packet_2_bis[0] = prefix;
        pairing_packet_2_bis[3] = pairing_packet_2[3];
        bis_retry = 10;
        while (bis_retry)
        {
            if (radiowrite(pairing_packet_2_bis, sizeof(pairing_packet_2_bis), "BIS2", 1))
            {
                response_size = read(response);
                if (response_size > 0)
                {
                    if (response[0] != prefix)
                    {
                        printf("Wrong prefix\r\n");
                    }
                    else
                        break;
                }
                else
                    printf("Empty response\r\n");
            }
            bis_retry--;
        }
        if (bis_retry == 0)
            return false;

        // extract info from BIS2 response
        memcpy(device_raw_key_material + 12, response + LOGITACKER_UNIFYING_PAIRING_RSP2_OFFSET_DONGLE_NONCE, 4); //RSP2 dongle_nonce
    }

    {
        prefix = PAIRING_MARKER_PHASE_3;
        pairing_packet_3[0] = prefix;
        pairing_packet_3[4] = strlen(device_name);
        memcpy(pairing_packet_3 + 5, device_name, pairing_packet_3[4]);

        if (!radiowrite(pairing_packet_3, 22, "REQ3", retry))
            return false;

        pairing_packet_3_bis[0] = prefix;
        if (!pair_response(pairing_packet_3_bis, "BIS3", retry))
        {
            printf("BIS3 failed");
        }

        response_size = read(response);
        if (response_size == 0)
        {
            printf("No response\r\n");
            return false;
        }
    }

    {
        if (!radiowrite(pairing_packet_4, 10, "Final", retry))
            return false;
    }

#ifdef EEPROM_SUPPORT
    /* Save address to eeprom */
    device_key[2] = device_raw_key_material[0];
    device_key[1] = device_raw_key_material[1] ^ 0xFF;
    device_key[5] = device_raw_key_material[2] ^ 0xFF;
    device_key[3] = device_raw_key_material[3];
    device_key[14] = device_raw_key_material[4];
    device_key[11] = device_raw_key_material[5];
    device_key[9] = device_raw_key_material[6];
    device_key[0] = device_raw_key_material[7];
    device_key[8] = device_raw_key_material[8];
    device_key[6] = device_raw_key_material[9] ^ 0x55;
    device_key[4] = device_raw_key_material[10];
    device_key[15] = device_raw_key_material[11];
    device_key[10] = device_raw_key_material[12] ^ 0xFF;
    device_key[12] = device_raw_key_material[13];
    device_key[7] = device_raw_key_material[14];
    device_key[13] = device_raw_key_material[15] ^ 0x55;

    printf("- Given RF Address:   %s\r\n", hexa(rf_address, 5));
    printf("- Device Key Raw:     %s\r\n", hexs(device_raw_key_material, 16));
    printf("- Device Key Derived: %s\r\n", hexs(device_key, 16));
    printf("- CHANNEL:            %d\r\n", current_channel);

    EEPROM.put(MAC_ADDRESS_EEPROM_ADDRESS + 0, current_channel);
    EEPROM.put(MAC_ADDRESS_EEPROM_ADDRESS + 1, rf_address);
    EEPROM.put(MAC_ADDRESS_EEPROM_ADDRESS + 1 + 5, device_key);
    EEPROM.commit();
#endif

    lock_channel = false;
    AES_init_ctx(&ctx, device_key);
    return true;
}

uint8_t ludevice::read(uint8_t *&packet)
{
    uint8_t packet_size = 22;

    if (radio.available())
    {
        packet = _read_buffer;
        radio.read(packet, packet_size);
        if (1)
        {
            if ((packet[19] == packet[20]) && (packet[20] == packet[21]))
                packet_size = 10;
            if (packet_size == 10 && ((packet[7] == packet[8]) && (packet[8] == packet[9])))
                packet_size = 5;
        }
        if (packet[1] != 0xe)
        {
            // printf("IN [%2d]: %2d                   ", packet_size, current_channel);
            printf("IN [%2d]:                  %2d   ", packet_size, current_channel);
            printf("%s\r\n", hexs(packet, packet_size));
        }
        return packet_size;
    }
    return 0;
}

bool ludevice::radiowrite(uint8_t *packet, uint8_t packet_size, char *name, uint8_t retry)
{
    return radiowrite_ex(packet, packet_size, name, retry, false);
}

bool ludevice::radiowrite_ex(uint8_t *packet, uint8_t packet_size, char *name, uint8_t retry, bool silent)
{
    char outcome;

    // retry = 1;
    outcome = '!';
    while (retry)
    {
        setChecksum(packet, packet_size);
        if (radio.write(packet, packet_size))
            outcome = ' ';
        else
            retry--;

        if (!silent)
        {
            // printf("OUT[%2d]: %2d  %s %c ", packet_size, current_channel, hexa(rf_address, 5), outcome);
            // printf("%d OUT[%2d]:  %s  %2d %c ", millis(), packet_size, hexa(rf_address, 5), current_channel, outcome);
            printf("OUT[%2d]:  %s  %2d %c ", packet_size, hexa(rf_address, 5), current_channel, outcome);
            printf("%s", hexs(packet, packet_size));
            if (name != NULL)
                printf(" - %s\r\n", name);
            else
                printf("\r\n");
        }

        if (outcome == '!')
        {
            if (!lock_channel)
            {
                changeChannel();
            }
        }
        else
            break;
    };

    if (outcome == '!')
        return false;

    return true;
}

void ludevice::changeChannel()
{
    if (is_pairing)
    {
        channel_pairing_id++;
        if (channel_pairing_id > sizeof(channel_tx))
            channel_pairing_id = 0;
        current_channel = channel_pairing[channel_pairing_id];
    }
    else
    {
        channel_tx_id++;
        if (channel_tx_id > sizeof(channel_tx))
            channel_tx_id = 0;
        current_channel = channel_pairing[channel_tx_id];
    }
    current_channel = 32;
    radio.setChannel(current_channel);
}

bool ludevice::reconnect()
{
    return register_device();
}

bool ludevice::register_device()
{
#ifndef EEPROM_SUPPORT
#warning "EEPROM support is not enabled"
    return false;
#else
    uint8_t prefix;
    uint8_t *response;
    bool failed;
    uint8_t packet_size;

    is_pairing = false;

    EEPROM.get(MAC_ADDRESS_EEPROM_ADDRESS + 1, rf_address);

    prefix = rf_address[0];

    rf_address[0] = 0;
    setAddress(rf_address);

    register1[0] = prefix;
    register1[2] = prefix;
    packet_size = sizeof(register1);
    if (!radiowrite(register1, packet_size, "register1", 5))
        return false;

    register2[0] = prefix;
    packet_size = sizeof(register2);
    if (!radiowrite(register2, packet_size, "register2", 5))
        return false;

    rf_address[0] = prefix;
    setAddress(rf_address);

    hello[2] = prefix;
    packet_size = sizeof(hello);
    if (!radiowrite(hello, packet_size, "hello", 5))
        return false;

    if (!update_keep_alive(110, 5, false))
        return false;

    is_connected = true;
    AES_init_ctx(&ctx, device_key);
    return true;
#endif
}

void ludevice::move(uint16_t x_move, uint16_t y_move)
{
    move(x_move, y_move, false, false);
}

void ludevice::move(uint16_t x_move, uint16_t y_move, bool leftClick, bool rightClick)
{
    move(x_move, y_move, 0, 0, false, false);
}

void ludevice::move(uint16_t x_move, uint16_t y_move, uint8_t scroll_v, uint8_t scroll_h)
{
    move(x_move, y_move, scroll_v, scroll_h, false, false);
}

void ludevice::move(uint16_t x_move, uint16_t y_move, uint8_t scroll_v, uint8_t scroll_h, bool leftClick, bool rightClick)
{
    idle_timer = 0;

    uint8_t mouse_payload[] = {0x00, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint32_t cursor_velocity;

    cursor_velocity = ((uint32_t)y_move & 0xFFF) << 12 | (x_move & 0xFFF);

    memcpy(mouse_payload + 4, &cursor_velocity, 3);

    if (leftClick)
        mouse_payload[2] = 1;

    if (rightClick)
        mouse_payload[2] |= 2; //1 << 1;

    mouse_payload[7] = scroll_v;
    mouse_payload[8] = scroll_h;

    setChecksum(mouse_payload, 10);
    while (!radio.write(mouse_payload, 10, 0))
        ;

    radio.flush_rx();
}

void ludevice::click(bool leftClick, bool rightClick)
{
    move(0, 0, leftClick, rightClick);
}

void ludevice::scroll(uint8_t scroll_v, uint8_t scroll_h)
{
    move(0, 0, scroll_v, scroll_h, false, false);
}

void ludevice::scroll(uint8_t scroll_v)
{
    scroll(scroll_v, 0);
}

void ludevice::wipe_pairing(void)
{
    uint8_t erase[15 + 6] = {0};
    EEPROM.put(MAC_ADDRESS_EEPROM_ADDRESS, erase);
    EEPROM.commit();
}

char *ludevice::hexs_ex(uint8_t *x, uint8_t length, bool reverse, char separator)
{
    uint8_t c;
    _hexs[0] = 0;
    for (int i = 0; i < length; i++)
    {
        c = x[i];
        if (reverse)
            c = x[length - 1 - i];
        sprintf(_hexs + (i * 3), "%02X", c);
        if (i < length - 1)
            sprintf(_hexs + (i * 3) + 2, "%c", separator);
    }

    return _hexs;
}

char *ludevice::hexa(uint8_t *x, uint8_t length)
{
    return hexs_ex(x, length, true, ':');
}

char *ludevice::hexs(uint8_t *x, uint8_t length)
{
    return hexs_ex(x, length, false, ' ');
}

void ludevice::typep(uint8_t scan1, uint8_t scan2, uint8_t scan3, uint8_t scan4, uint8_t scan5, uint8_t scan6)
{
    idle_timer = 0;

    uint8_t key_payload[] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE | 0x80,
        0x00, // [2] modifier
        0x00,
        0x00, // [4] scancode
        0x00, 0x00, 0x00, 0x00,
        0x00};

    key_payload[3] = 0x4;  // send 'a'
    key_payload[4] = 0x37; // send '.'
    key_payload[3] = scan1;
    key_payload[4] = scan2;
    key_payload[5] = scan3;
    key_payload[6] = scan4;
    key_payload[7] = scan5;
    key_payload[8] = scan6;
    setChecksum(key_payload, 10);
    radiowrite(key_payload, 10, "plain key", 5);
    return;
    while (1)
    {
        status = failed;
        if (radio.write(key_payload, 10, 0))
            status = success;

        printf("- plain keyboard: %s, %s\r\n", hexs(key_payload, 10), status);
        break;
    }
}

void ludevice::typem(uint16_t scan1, uint16_t scan2)
{
    idle_timer = 0;

    uint8_t key_payload[] = {
        0x00, 0xC3,
        0x00, // [2] scancode
        0x00,
        0x00, // [4] scancode
        0x00,
        0x00, 0x00, 0x00, // unused
        0x00};

    // PAGE UP (0x4B)
    // PAGE DOWN (0x4E)
    // ESC (0x29)
    // F5 (0x3E)
    // PERIOD (0x37)
    // B (0x05)

    // 00 C3 E2 00 00 00 00 00 00 5B (10 bytes) // toggle mute

    key_payload[2] = ((scan1 & 0x00ff) >> 0);
    key_payload[3] = ((scan1 & 0xff00) >> 8);
    key_payload[4] = ((scan2 & 0x00ff) >> 0);
    key_payload[5] = ((scan2 & 0xff00) >> 8);

    radiowrite(key_payload, 10, "media key", 5);
}

void ludevice::typee(uint8_t scan1, uint8_t scan2, uint8_t scan3, uint8_t scan4, uint8_t scan5, uint8_t scan6)
{
    uint32_t temp_counter;
    bool ret;

    uint8_t rf_frame[22] = {0};
    uint8_t plain_payload[8] = {0};

    idle_timer = 0;

    plain_payload[1] = scan1;
    plain_payload[2] = scan2;
    plain_payload[3] = scan3;
    plain_payload[4] = scan4;
    plain_payload[5] = scan5;
    plain_payload[6] = scan6;

    temp_counter = (aes_counter & 0xf);
    temp_counter = aes_base + (aes_counter & 0xf);
    logitacker_unifying_crypto_encrypt_keyboard_frame(rf_frame, plain_payload, temp_counter);

    if (scan1 == 0 && scan2 == 0 && scan3 == 0 && scan4 == 0 && scan5 == 0 && scan6 == 0)
        ret = radiowrite(rf_frame, 22, "encrypted key up", 1);
    else
        ret = radiowrite(rf_frame, 22, "encrypted key down", 1);

    if (ret)
        // aes_counter++;
        aes_base++;
}

void ludevice::update_little_known_secret_counter(uint8_t *counter_bytes)
{
    memcpy(little_known_secret + 7, counter_bytes, 4);
}

void ludevice::logitacker_unifying_crypto_calculate_frame_key(uint8_t *ciphertext, uint8_t *counter_bytes, bool silent)
{
    if (!silent)
        printf("1. last plain l_k_s:                 %s\r\n", hexs(little_known_secret, 16));
    update_little_known_secret_counter(counter_bytes); // copy counter_bytes into little_known_secret
    if (!silent)
        printf("2. plain l_k_s+counter:              %s\r\n", hexs(little_known_secret, 16));

    if (!silent)
        printf("3. device_key:                       %s\r\n", hexs(device_key, 16));
    memcpy(ciphertext, little_known_secret, 16); // copy little_known_secret into ciphertext
    AES_ECB_encrypt(&ctx, ciphertext);           // encrypt ciphertext

    if (!silent)
        printf("4. frame_key:                        %s\r\n", hexs(ciphertext, 16));
    return;
}

void ludevice::logitacker_unifying_crypto_encrypt_keyboard_frame(uint8_t *rf_frame, uint8_t *plain_payload, uint32_t counter)
{
    rf_frame[1] = LOGITACKER_DEVICE_REPORT_TYPES_ENCRYPTED_KEYBOARD | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE | 0x80;
    bool silent = false;

    uint8_t counter_bytes[4] = {0};
    // K800
    // counter_bytes[3] = (uint8_t)((counter & 0xff000000) >> 24);
    // counter_bytes[2] = (uint8_t)((counter & 0x00ff0000) >> 16);
    // counter_bytes[1] = (uint8_t)((counter & 0x0000ff00) >> 8);
    // counter_bytes[0] = (uint8_t)((counter & 0x000000ff) >> 0);

    // K270
    counter_bytes[0] = (uint8_t)((counter & 0xff000000) >> 24);
    counter_bytes[1] = (uint8_t)((counter & 0x00ff0000) >> 16);
    counter_bytes[2] = (uint8_t)((counter & 0x0000ff00) >> 8);
    counter_bytes[3] = (uint8_t)((counter & 0x000000ff) >> 0);
    memcpy(rf_frame + 10, counter_bytes, 4);

    uint8_t frame_key[16] = {0};
    logitacker_unifying_crypto_calculate_frame_key(frame_key, counter_bytes, silent);

    plain_payload[7] = 0xC9;
    memcpy(rf_frame + 2, plain_payload, 8);

    if (!silent)
        printf("5. plain rf_frame:             %s\r\n", hexs(rf_frame, 22));

    for (int i = 0; i < 8; i++)
        rf_frame[2 + i] ^= frame_key[i];

    setChecksum(rf_frame, 22);

    if (!silent)
        printf("6. encrypted rf_frame:         %s\r\n", hexs(rf_frame, 22));
}