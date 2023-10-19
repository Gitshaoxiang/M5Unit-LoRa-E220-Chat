
#ifndef NATIVE_PLATFORM

#include "M5Cardputer.h"
#include "string.h"
#include "M5_LoRa_E220_JP.h"
#include <Arduino.h>

M5Canvas canvas(&M5.Display);

LoRa_E220_JP lora;
struct LoRaConfigItem_t config;
struct RecvFrame_t lora_data;

/** prototype declaration **/
void LoRaRecvTask(void *pvParameters);
void LoRaSendTask(void *pvParameters);

#else

#include "M5Unified.h"
#include "string.h"
M5Canvas canvas(&M5.Display);

#define millis SDL_GetTicks
#define delay  SDL_Delay

#endif

#include "M5GFX.h"
#include "background.h"
#include <vector>

struct lora_msg_t {
    bool is_send;
    std::string msg;
};

std::vector<lora_msg_t> msg_history;  // print key + space, enter, del

void updateBackground() {
    // image_data_bg
    canvas.pushImage(0, 0, 240, 135, image_data_background);
}

void refreshPage() {
    canvas.pushSprite(0, 0);
}

void updateSignal(uint8_t level) {
    uint8_t margin = 2;
    uint8_t width  = 5;

    if (level < 1) {
        canvas.fillRect(1 * margin, 16, width, 6, BLACK);
    }

    if (level < 2) {
        canvas.fillRect(2 * margin + width, 10, width, 12, BLACK);
    }

    if (level < 3) {
        canvas.fillRect(3 * margin + 2 * width, 6, width, 16, BLACK);
    }

    if (level < 4) {
        canvas.fillRect(4 * margin + 3 * width, 2, width, 20, BLACK);
    }
}

void updateBat(uint8_t level) {
    uint8_t margin = 2;
    uint8_t width  = 8;
    uint8_t height = 16;
    uint8_t posx   = 31;
    uint8_t posy   = 4;

    if (level < 5) {
        canvas.fillRect(1 * margin + posx, posy, width, height, BLACK);
    }
    if (level < 20) {
        canvas.fillRect(2 * margin + posx + width, posy, width, height, BLACK);
    }
    if (level < 40) {
        canvas.fillRect(3 * margin + posx + 2 * width, posy, width, height,
                        BLACK);
    }
    if (level < 60) {
        canvas.fillRect(4 * margin + posx + 3 * width, posy, width, height,
                        BLACK);
    }
    if (level < 80) {
        canvas.fillRect(5 * margin + posx + 4 * width, posy, width, height,
                        BLACK);
    }
}

void updateConfig(char *str) {
    canvas.fillRect(87, 3, 150, 18, BLACK);
    canvas.drawString(str, 90, 4);
}

void updateInput(const char *str) {
    canvas.fillRect(3, 110, 234, 21, BLACK);
    canvas.drawString(str, 6, 111);
}

void updateHistory() {
    uint8_t posy       = 26;
    uint8_t posx       = 5;
    uint8_t height     = 15;
    uint8_t margin     = 2;
    uint8_t font_width = 7;

    uint8_t msg_count = 0;

    for (auto i : msg_history) {
        std::string msg = i.msg;
        int msg_len     = msg.length();
        int msg_width   = msg_len * font_width;
        if (i.is_send) {
            canvas.fillSmoothRoundRect(240 - 5 - msg_width - 2,
                                       posy + height * msg_count, msg_width + 2,
                                       height, 2, 0x1c9f);
            canvas.drawString(msg.c_str(), 240 - 5 - msg_width - 1,
                              posy + height * msg_count);
        } else {
            canvas.fillSmoothRoundRect(5, posy + height * msg_count,
                                       msg_width + 2, height, 2, 0xFDA0);
            canvas.drawString(msg.c_str(), 6, posy + height * msg_count);
        }
        msg_count++;
        posy++;
    }
}

void setup() {
#ifndef NATIVE_PLATFORM

    auto cfg = M5.config();
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(1);

    // M5Cardputer.Power.setExtOutput(true);

    canvas.createSprite(240, 135);
    canvas.setTextColor(WHITE);
    canvas.setTextFont(&fonts::efontCN_14);
    canvas.setTextSize(1);

    // lora.Init(&Serial2, 9600, SERIAL_8N1, 2, 1);
    lora.Init(&Serial2, 9600, SERIAL_8N1, 1, 2);
    // Serial.begin(9600);

    lora.SetDefaultConfigValue(config);

    config.own_address              = 0x0000;
    config.baud_rate                = BAUD_9600;
    config.air_data_rate            = BW125K_SF9;
    config.subpacket_size           = SUBPACKET_200_BYTE;
    config.rssi_ambient_noise_flag  = RSSI_AMBIENT_NOISE_ENABLE;
    config.transmitting_power       = TX_POWER_13dBm;
    config.own_channel              = 0x00;
    config.rssi_byte_flag           = RSSI_BYTE_ENABLE;
    config.transmission_method_type = UART_P2P_MODE;
    config.lbt_flag                 = LBT_DISABLE;
    config.wor_cycle                = WOR_2000MS;
    config.encryption_key           = 0x1234;
    config.target_address           = 0x0000;
    config.target_channel           = 0x00;

    delay(100);
    if (lora.InitLoRaSetting(config) != 0) {
        Serial.println("init error, pls pull the M0,M1 to 1");
        Serial.println("or click Btn to skip");
        // msg_history.push_back({true, "Config Error"});
    } else {
        // msg_history.push_back({true, "Config OK"});
    }

    // マルチタスク
    xTaskCreateUniversal(LoRaRecvTask, "LoRaRecvTask", 8192, NULL, 1, NULL,
                         APP_CPU_NUM);

#else
    auto cfg = M5.config();
    M5.Display.begin();

    canvas.createSprite(240, 135);
    canvas.setTextColor(WHITE);
    canvas.setTextFont(&fonts::efontCN_14);
    canvas.setTextSize(1);

    // max 5
    msg_history.push_back({true, "msg: 11111"});
    msg_history.push_back({false, "msg: 222222"});
    msg_history.push_back({true, "msg: 11111"});
    msg_history.push_back({false, "msg: 11111"});
    msg_history.push_back({true, "msg: 11111"});

#endif
}

String data          = "";
uint8_t signal_level = 4;

void loop() {
    M5Cardputer.update();
    updateBackground();
    updateConfig("SF:7/BW:500K/CH:0");
    int8_t bat_level = M5.Power.getBatteryLevel();
    updateBat(bat_level);
    updateSignal(signal_level);
    updateHistory();
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            M5Cardputer.Speaker.tone(5000, 20);
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            for (auto i : status.word) {
                data += i;
            }
            if (status.del) {
                data.remove(data.length() - 1);
            }
            if (status.enter) {
                if (msg_history.size() >= 5) {
                    msg_history.erase(msg_history.begin());
                }
                if (lora.SendFrame(config, (uint8_t *)data.c_str(),
                                   strlen(data.c_str())) == 0) {
                    Serial.println("send succeeded.");
                    Serial.println("");
                    msg_history.push_back({true, data.c_str()});
                    data = "";
                } else {
                    msg_history.push_back({true, "send failed."});
                    Serial.println("send failed.");
                    Serial.println("");
                }
            }
        }
    }
    updateInput(data.c_str());
    refreshPage();
}

#ifndef NATIVE_PLATFORM

void LoRaRecvTask(void *pvParameters) {
    String msg;
    while (1) {
        if (lora.RecieveFrame(&lora_data) == 0) {
            M5Cardputer.Speaker.tone(5000, 20);
            msg = "";
            Serial.println("recv data:");
            for (int i = 0; i < lora_data.recv_data_len; i++) {
                Serial.printf("%c", lora_data.recv_data[i]);
                msg += char(lora_data.recv_data[i]);
            }
            Serial.println("");
            Serial.println("hex dump:");
            for (int i = 0; i < lora_data.recv_data_len; i++) {
                Serial.printf("%02x ", lora_data.recv_data[i]);
            }
            Serial.printf("\n");
            Serial.printf("RSSI: %d dBm\n", lora_data.rssi);
            Serial.printf("\n");
            msg += "/RSSI:";
            msg += String(lora_data.rssi);
            msg += "dBm";
            signal_level = 0;
            if (lora_data.rssi > -80) {
                signal_level = 1;
            }
            if (lora_data.rssi > -60) {
                signal_level = 2;
            }
            if (lora_data.rssi > -40) {
                signal_level = 3;
            }
            if (lora_data.rssi > -20) {
                signal_level = 4;
            }

            if (msg_history.size() >= 5) {
                msg_history.erase(msg_history.begin());
            }
            msg_history.push_back({false, msg.c_str()});
        }
        delay(1);
    }
}

#endif