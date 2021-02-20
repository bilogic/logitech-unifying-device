/*
    Copyright (C) 2020 bilogic
        Heavily modified with the aim to be fully compatible with logitech unifying protocol

    Copyright (C) 2017 Ronan Gaillard <ronan.gaillard@live.fr>

        This program is free software; you can redistribute it and/or
        modify it under the terms of the GNU General Public License
        version 2 as published by the Free Software Foundation.
*/

#ifndef LOGITECH_MOUSE
#define LOGITECH_MOUSE

#define PAIRING_MARKER_PHASE_1 0xe1
#define PAIRING_MARKER_PHASE_2 0xe2
#define PAIRING_MARKER_PHASE_3 0xe3

#define LOGITACKER_DEVICE_PROTOCOL_UNIFYING 0x04
#define LOGITACKER_DEVICE_PROTOCOL_LIGHTSPEED 0x0C
#define LOGITACKER_DEVICE_PROTOCOL_G700 0x07

#define LOGITACKER_DEVICE_UNIFYING_TYPE_UNKNOWN 0x00
#define LOGITACKER_DEVICE_UNIFYING_TYPE_KEYBOARD 0x01
#define LOGITACKER_DEVICE_UNIFYING_TYPE_MOUSE 0x02
#define LOGITACKER_DEVICE_UNIFYING_TYPE_NUMPAD 0x03
#define LOGITACKER_DEVICE_UNIFYING_TYPE_PRESENTER 0x04
#define LOGITACKER_DEVICE_UNIFYING_TYPE_REMOTE 0x07
#define LOGITACKER_DEVICE_UNIFYING_TYPE_TRACKBALL 0x08
#define LOGITACKER_DEVICE_UNIFYING_TYPE_TOUCHPAD 0x09
#define LOGITACKER_DEVICE_UNIFYING_TYPE_TABLET 0x0a
#define LOGITACKER_DEVICE_UNIFYING_TYPE_GAMEPAD 0x0b
#define LOGITACKER_DEVICE_UNIFYING_TYPE_JOYSTICK 0x0c

#define LOGITACKER_DEVICE_USABILITY_INFO_RESERVED 0x0
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_BASE 0x1
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_TOP_CASE 0x2
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_EDGE_OF_TOP_RIGHT_CORNER 0x3
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_OTHER 0x4
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_TOP_LEFT_CORNER 0x5
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_BOTTOM_LEFT_CORNER 0x6
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_TOP_RIGHT_CORNER 0x7
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_BOTTOM_RIGHT_CORNER 0x8
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_TOP_EDGE 0x9
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_RIGHT_EDGE 0xa
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_LEFT_EDGE 0xb
#define LOGITACKER_DEVICE_USABILITY_INFO_PS_LOCATION_ON_THE_BOTTOM_EDGE 0xc

#define LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD 0x1
#define LOGITACKER_DEVICE_REPORT_TYPES_MOUSE 0x2
#define LOGITACKER_DEVICE_REPORT_TYPES_MULTIMEDIA 0x3
#define LOGITACKER_DEVICE_REPORT_TYPES_POWER_KEYS 0x4
#define LOGITACKER_DEVICE_REPORT_TYPES_MEDIA_CENTER 0x8
#define LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD_LED 0xe
#define LOGITACKER_DEVICE_REPORT_TYPES_SET_KEEP_ALIVE 0xf
#define LOGITACKER_DEVICE_REPORT_TYPES_SHORT_HIDPP 0x10
#define LOGITACKER_DEVICE_REPORT_TYPES_LONG_HIDPP 0x11
#define LOGITACKER_DEVICE_REPORT_TYPES_ENCRYPTED_KEYBOARD 0x13
#define LOGITACKER_DEVICE_REPORT_TYPES_ENCRYPTED_HIDPP_LONG 0x1b
#define LOGITACKER_DEVICE_REPORT_TYPES_PAIRING 0x1f
#define LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE 0x40

#define LOGITACKER_DEVICE_CAPS_LINK_ENCRYPTION 0x1     // (1 << 0)
#define LOGITACKER_DEVICE_CAPS_BATTERY_STATUS 0x2      // (1 << 1)
#define LOGITACKER_DEVICE_CAPS_UNIFYING_COMPATIBLE 0x4 // (1 << 2)
#define LOGITACKER_DEVICE_CAPS_UNKNOWN1 0x8            // (1 << 3)

#define LOGITACKER_UNIFYING_PAIRING_REQ1_OFFSET_DEVICE_WPID 9
#define LOGITACKER_UNIFYING_PAIRING_REQ1_OFFSET_DEVICE_TYPE 13
#define LOGITACKER_UNIFYING_PAIRING_REQ1_OFFSET_DEVICE_CAPS 14

#define LOGITACKER_UNIFYING_PAIRING_RSP1_OFFSET_DONGLE_WPID 9
#define LOGITACKER_UNIFYING_PAIRING_RSP1_OFFSET_BASE_ADDR 3
#define LOGITACKER_UNIFYING_PAIRING_RSP1_OFFSET_ADDR_PREFIX 7

#define LOGITACKER_UNIFYING_PAIRING_REQ2_OFFSET_DEVICE_NONCE 3
#define LOGITACKER_UNIFYING_PAIRING_REQ2_OFFSET_DEVICE_SERIAL 7
#define LOGITACKER_UNIFYING_PAIRING_REQ2_OFFSET_DEVICE_REPORT_TYPES_LE 11 //little endian 32bit uint
#define LOGITACKER_UNIFYING_PAIRING_REQ2_OFFSET_DEVICE_USABILITY_INFO 15

#define LOGITACKER_UNIFYING_PAIRING_RSP2_OFFSET_DONGLE_NONCE 3

#define LOGITACKER_UNIFYING_PAIRING_REQ3_OFFSET_DEVICE_NAME_LEN 4
#define LOGITACKER_UNIFYING_PAIRING_REQ3_OFFSET_DEVICE_NAME 5

#define ECB 1

#include "aes.h"
#include <nRF24L01.h>
#include <RF24.h>
#include <elapsedMillis.h>

#define DEFAULT_CE_PIN D4
#define DEFAULT_CS_PIN D3
#define CHANNEL 5
#define PAYLOAD_SIZE 22
#define PAIRING_MAC_ADDRESS 0xBB0ADCA575LL
#define EEPROM_SUPPORT
#define MAC_ADDRESS_EEPROM_ADDRESS 0

#ifdef EEPROM_SUPPORT
#include <EEPROM.h>
#endif

class ludevice
{
private:
    RF24 radio;

    void setChecksum(uint8_t *payload, uint8_t len);
    void setAddress(uint8_t *address);
    void setAddress(uint64_t address);

    struct AES_ctx ctx;

    bool is_pairing = false;
    uint8_t current_channel;
    uint8_t channel_pairing_id = -1;
    uint8_t channel_tx_id = -1;
    uint8_t channel_pairing[11] = {62, 8, 35, 65, 14, 41, 71, 17, 44, 74, 5};
    uint8_t channel_tx[25] = {5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47, 50, 53, 56, 59, 62, 65, 68, 71, 74, 77};

    bool lock_channel = false;
    bool is_connected = false;
    char *success = "success";
    char *failed = "failed";
    char *status;
    uint8_t aes_counter = 0;
    // uint32_t aes_base = 0x171df9f0;
    uint32_t aes_base = 0xed3456ed;

    uint8_t *wakeup_packet;
    char _hexs[66];
    uint8_t _read_buffer[22];

    uint32_t stay_alive_counter = 0;

    uint8_t rf_address[5];
    uint8_t send_address[5];
    uint8_t recv_address[5][5];

    // uint8_t rf_address[5];
    uint8_t device_key[16];
    uint8_t device_raw_key_material[16];

    elapsedMillis send_alive_timer;
    elapsedMillis idle_timer;

    // uint32_t firmware_version = 0x22000017; // K800
    uint32_t firmware_version = 0x35000017; // K270
    // char *device_name = "K800";
    // char *device_name = "K270";
    char *device_name = "KespB";

    // keyboard at 20ms, 0x14
    // mouse keep alive at 8ms interval
    uint16_t keep_alive = 0x14;

    // https://github.com/pwr-Solaar/Solaar/blob/master/lib/logitech_receiver/descriptors.py
    // uint16_t device_wpid = 0x2010; // actual K800, p=1.0
    uint16_t device_wpid = 0x4003; // actual k270
    // uint16_t device_wpid = 0x406E; // K800 new, p=4.5
    // uint16_t device_wpid = 0x4024; // Anywhere MX
    // uint16_t device_wpid = 0x200F; // MK320, p=1.0
    // uint16_t device_wpid = 0x1337; // custom
    uint8_t protocol = LOGITACKER_DEVICE_PROTOCOL_UNIFYING;         // unifying
    uint8_t device_type = LOGITACKER_DEVICE_UNIFYING_TYPE_KEYBOARD; // 1
    uint8_t caps =
        LOGITACKER_DEVICE_CAPS_LINK_ENCRYPTION | // 0001, 1
        // LOGITACKER_DEVICE_CAPS_BATTERY_STATUS |   // 0010, 2
        LOGITACKER_DEVICE_CAPS_UNIFYING_COMPATIBLE | // 0100, 4
        LOGITACKER_DEVICE_CAPS_UNKNOWN1 |            // 1000, 8
        0;

    uint8_t pp1_unknown = 0x1A; // 0010 1010, K270
    // uint8_t pp1_unknown = 0x00; // 0000 0000 K800
    // uint8_t pp1_unknown = 0x01; // 0000 0001 Hacker

    /* Pre-defined pairing packets */
    uint8_t pairing_packet_1_bis[5] = {
        0xF0,
        LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x01, 0x84,
        0x26};
    // [CH: 17]  BB:0A:DC:A5:75  05 5F 01 49 16 90 09 F2 14 40 03 04 00 01 0D 00 00 00 00 00 2A 1E (22 bytes)
    uint8_t pairing_packet_1[22] = {
        0xF0,
        LOGITACKER_DEVICE_REPORT_TYPES_PAIRING | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x01,
        0xfa, 0xde, 0x11, 0x11, 0x07,  // rfaddress
        keep_alive,                    // default keep_alive
        ((device_wpid & 0xff00) >> 8), // wireless PID MSB
        ((device_wpid & 0x00ff) >> 0), // wireless PID LSB
        protocol, 0x00,
        device_type,
        caps,
        0x00, 0x00, 0x00, 0x00, 0x00,
        pp1_unknown,
        0xEC};

    uint32_t nonce = 0xDF850991;
    uint32_t serial = 0xA58094B6; // K270
    uint32_t rt =
        LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD |
        // LOGITACKER_DEVICE_REPORT_TYPES_MOUSE |
        LOGITACKER_DEVICE_REPORT_TYPES_MULTIMEDIA |
        LOGITACKER_DEVICE_REPORT_TYPES_POWER_KEYS |
        // LOGITACKER_DEVICE_REPORT_TYPES_MEDIA_CENTER |
        LOGITACKER_DEVICE_REPORT_TYPES_KEYBOARD_LED |
        // LOGITACKER_DEVICE_REPORT_TYPES_SHORT_HIDPP |
        // LOGITACKER_DEVICE_REPORT_TYPES_LONG_HIDPP |
        0;

    uint8_t pairing_packet_2_bis[5] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x02, 0x01,
        0xbd};
    uint8_t pairing_packet_2[22] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_PAIRING | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x02,
        ((nonce & 0xff000000) >> 24),  // device nonce MSB
        ((nonce & 0x00ff0000) >> 16),  // device nonce
        ((nonce & 0x0000ff00) >> 8),   // device nonce
        ((nonce & 0x000000ff) >> 0),   // device nonce LSB
        ((serial & 0xff000000) >> 24), // device serial MSB
        ((serial & 0x00ff0000) >> 16), // device serial
        ((serial & 0x0000ff00) >> 8),  // device serial
        ((serial & 0x000000ff) >> 0),  // device serial LSB
        ((rt & 0x000000ff) >> 0),      // device report types
        ((rt & 0x0000ff00) >> 8),      // device report types
        ((rt & 0x00ff0000) >> 16),     // device report types
        ((rt & 0xff000000) >> 24),     // device report types
        0x02,                          // device_usability_info
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x79};

    uint8_t pairing_packet_3_bis[5] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x03, 0x01,
        0x0f};
    uint8_t pairing_packet_3[22] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_PAIRING | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE, // 0x40 is device to dongle
        0x03,                                                                               // step 3
        0x1,                                                                                // number of reports fixed to 1
        0x3,                                                                                // length of device name
        65, 66, 67, 0, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xB6};

    uint8_t pairing_packet_4[10] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_SET_KEEP_ALIVE | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x06,
        0x01, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0xED};

    uint8_t keep_alive_change_packet[10] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_SET_KEEP_ALIVE | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE, // 0x40 is device to dongle
        0x00,                                                                                      // unused
        0x00, keep_alive,                                                                          // timeout, 00:6E is 110ms, 01:00 is 256ms, 04:B0 is 1200ms
        0x00, 0x00, 0x00, 0x00,                                                                    // unused
        0xEA                                                                                       // checksum
    };
    uint8_t keep_alive_packet[5] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x01, 0x00, // timeout, 00:6E is 110ms, 01:00 is 256ms, 04:B0 is 1200ms
        0xEA        // checksum
    };
    /* Enf of pre-defined pairing packets */

    uint8_t register1[22] = {
        0x62, // RF of device
        LOGITACKER_DEVICE_REPORT_TYPES_LONG_HIDPP | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x62, // RF of device
        0x07, 0x00, 0x01, 0x01,
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xE1};

    uint8_t register2[10] = {
        0x62, // RF of device
        LOGITACKER_DEVICE_REPORT_TYPES_SET_KEEP_ALIVE | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x07,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x48};

    uint8_t hello[22] = {
        0x00,
        LOGITACKER_DEVICE_REPORT_TYPES_LONG_HIDPP | LOGITACKER_DEVICE_REPORT_TYPES_KEEP_ALIVE,
        0x62, // RF of device
        0x04, 0x00, 0x46, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xEF};

    uint8_t little_known_secret[16] = {
        //                            81B4  81B5  81B6  81B7  81B8  81B9
        // 0     1     2     3     4     5     6   (7)   (8)   (9)   (A)     B     C     D     E     F
        0x04, 0x14, 0x1d, 0x1f, 0x27, 0x28, 0x0d, 0xde, 0xad, 0xbe, 0xef, 0x0a, 0x0d, 0x13, 0x26, 0x0e};

public:
    ludevice(uint8_t _cepin, uint8_t _cspin);
    ludevice();

    bool begin();

    int pair();
    bool pairing();
    bool pair_response(uint8_t *packet, char *name, uint8_t retry);
    bool reconnect();
    bool register_device();

    void move(uint16_t x_move, uint16_t y_move);
    void move(uint16_t x_move, uint16_t y_move, bool leftClick, bool rightClick);
    void move(uint16_t x_move, uint16_t y_move, uint8_t scroll_v, uint8_t scroll_h);
    void move(uint16_t x_move, uint16_t y_move, uint8_t scroll_v, uint8_t scroll_h, bool leftClick, bool rightClick);
    void click(bool leftClick, bool rightClick);
    void scroll(uint8_t scroll_v, uint8_t scroll_h);
    void scroll(uint8_t scroll_v);
    char *hexs(uint8_t *x, uint8_t length);
    char *hexa(uint8_t *x, uint8_t length);
    char *hexs_ex(uint8_t *x, uint8_t length, bool reverse, char separator);
    void typep(uint8_t scan1 = 0, uint8_t scan2 = 0, uint8_t scan3 = 0, uint8_t scan4 = 0, uint8_t scan5 = 0, uint8_t scan6 = 0);
    void typee(uint8_t scan1 = 0, uint8_t scan2 = 0, uint8_t scan3 = 0, uint8_t scan4 = 0, uint8_t scan5 = 0, uint8_t scan6 = 0);
    void typem(uint16_t scan1 = 0, uint16_t scan2 = 0);

    void changeChannel();

    void wipe_pairing(void);
    void loop(void);
    void stay_alive_mouse(void);
    void stay_alive_keyboard(void);
    bool update_keep_alive(uint16_t timeout, uint8_t retry, bool silent);
    bool radiowrite(uint8_t *packet, uint8_t packet_size, char *name, uint8_t retry);
    bool radiowrite_ex(uint8_t *packet, uint8_t packet_size, char *name, uint8_t retry, bool silent);
    uint8_t read(uint8_t *&packet);

    void hidpp10(uint8_t *rf_payload, uint8_t payload_size);
    void hidpp20(uint8_t *rf_payload, uint8_t payload_size);

    void logitacker_unifying_crypto_encrypt_keyboard_frame(uint8_t *encrypted, uint8_t *plain, uint32_t counter);
    void logitacker_unifying_crypto_calculate_frame_key(uint8_t *frame_key, uint8_t *counter_bytes, bool silent);
    void update_little_known_secret_counter(uint8_t *counter);
    bool connected();
};

#endif
