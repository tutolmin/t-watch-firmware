/*
Modified SetTimeFromBLE.ino: Uses BLE to set T-Watch RTC time and turns off the display after inactivity.
Display turns back on on BLE activity (connect, disconnect, write) OR pressing the side button (PEK_KEY).
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "config.h"
#include <esp_wifi.h>

TTGOClass *ttgo;
TFT_eSPI *tft;
AXP20X_Class *power; // ← глобально
PCF8563_Class *rtc;

bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t interval = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define TFT_GREY            0x5AEB

// Добавьте глобальные переменные для сервера и рекламы
BLEServer *pServer = nullptr;
BLEAdvertising *pAdvertising = nullptr;
// ← ДОБАВЬТЕ ЭТУ СТРОКУ:
BLECharacteristic *pCharacteristic = nullptr;  // <-- глобальная!

// Глобальные переменные
unsigned long lastBatteryUpdate = 0;
int batteryPercent = 100;

// Переменные для управления состоянием дисплея
bool displayIsOn = true; // Предполагаем, что дисплей включен при старте
uint32_t lastActivityTimestamp = 0;
const uint32_t DISPLAY_OFF_TIMEOUT_MS = 20000; // 30 seconds of inactivity before turning off display

// Флаг для прерывания от кнопки
volatile bool pekKeyPressed = false;

void IRAM_ATTR handlePEKKeyPress() {
  // Устанавливаем флаг в обработчике прерывания
  pekKeyPressed = true;
}

void drawSTATUS(bool status);
void drawBatteryLevel(int percent);

bool setDateTimeFormBLE(const char *str)
{
    // Work delay emulation    
    delay(3000);

    uint16_t year;
    uint8_t month, day, hour, min, sec;
    String temp, data;
    int r1, r2;
    if (str == NULL) return false;

    data = str;

    r1 = data.indexOf(',');
    if (r1 < 0) return false;
    temp = data.substring(0, r1);
    year = (uint16_t)temp.toInt();

    r1 += 1;
    r2 = data.indexOf(',', r1);
    if (r2 < 0) return false;
    temp = data.substring(r1, r2);
    month = (uint16_t)temp.toInt();

    r1 = r2 + 1;
    r2 = data.indexOf(',', r1);
    if (r2 < 0) return false;
    temp = data.substring(r1, r2);
    day = (uint16_t)temp.toInt();

    r1 = r2 + 1;
    r2 = data.indexOf(',', r1);
    if (r2 < 0) return false;
    temp = data.substring(r1, r2);
    hour = (uint16_t)temp.toInt();

    r1 = r2 + 1;
    r2 = data.indexOf(',', r1);
    if (r2 < 0) return false;
    temp = data.substring(r1, r2);
    min = (uint16_t)temp.toInt();

    r1 = r2 + 1;
    temp = data.substring(r1);
    sec = (uint16_t)temp.toInt();

    // No parameter check, please set the correct time
    Serial.printf("SET:%u/%u/%u %u:%u:%u\n", year, month, day, hour, min, sec);
    rtc->setDateTime(year, month, day, hour, min, sec);

    // Update timestamp on successful time set
    lastActivityTimestamp = millis();
    // Turn on display if it was off
    if (!displayIsOn) {
         ttgo->openBL();
         displayIsOn = true;
         Serial.println("Display turned ON due to time setting.");
    }

    return true;
}

class MyCallbacks: public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic)
    {
        Serial.println("Start working ...");
        ttgo->motor->onec(500);
        delay(800);
        ttgo->motor->onec();
        delay(500);

        std::string value = pCharacteristic->getValue();

        if (value.length() > 0) {
            Serial.println("*********");
            Serial.print("New value: ");
            for (int i = 0; i < value.length(); i++)
                Serial.print(value[i]);
            Serial.println();
            Serial.println("*********");
        }

        if (value.length() <= 0) {
            return;
        }
        if (!setDateTimeFormBLE(value.c_str())) {
            Serial.println("DateTime format error ...");
            ttgo->motor->onec();
            delay(500);
            ttgo->motor->onec();
            delay(500);
            ttgo->motor->onec();
            delay(500);

            ttgo->motor->onec(500);
            delay(1000);
            ttgo->motor->onec(500);
            delay(1000);
            ttgo->motor->onec(500);
            delay(1000);

            ttgo->motor->onec();
            delay(500);
            ttgo->motor->onec();
            delay(500);
            ttgo->motor->onec();
            delay(500);
            }
        // Update timestamp on any write activity (already done in setDateTimeFormBLE)
    }
};

class MyServerCallback : public BLEServerCallbacks
{
    void onConnect(BLEServer *pServer)
    {
        deviceConnected = true;
        Serial.println("onConnect");
        // Update timestamp on connection
        lastActivityTimestamp = millis();
        // Turn on display if it was off
        if (!displayIsOn) {
             ttgo->openBL();
             displayIsOn = true;
             Serial.println("Display turned ON due to connection.");
        }
        // Вибрация при подключении
        if(ttgo) { // Проверка, что ttgo инициализирован
             Serial.println("Vibration ON connect triggered.");
             ttgo->motor->onec(500);
             delay(800);
             ttgo->motor->onec();
             delay(500);
             ttgo->motor->onec();
             delay(500);
             ttgo->motor->onec();
             delay(500);
        }
    }

    void onDisconnect(BLEServer *pServer)
    {
        deviceConnected = false;
        Serial.println("onDisconnect");
        // Update timestamp on disconnection
        lastActivityTimestamp = millis();
        // Turn on display if it was off
        if (!displayIsOn) {
             ttgo->openBL();
             displayIsOn = true;
             Serial.println("Display turned ON due to disconnection.");
        }
        // --- НАЧАЛО ИЗМЕНЕНИЙ ---
        // Перезапустить advertisings, чтобы быть видимым для новых подключений
        if(pAdvertising) {
            pAdvertising->start();
        }
        Serial.println("Advertising restarted after disconnection.");
        // --- КОНЕЦ ИЗМЕНЕНИЙ ---

        // Вибрация при отключении
        if(ttgo) { // Проверка, что ttgo инициализирован
             Serial.println("Vibration ON disconnect triggered.");
             ttgo->motor->onec(500);
             delay(800);
             ttgo->motor->onec();
             delay(500);
             ttgo->motor->onec();
             delay(500);
             ttgo->motor->onec();
             delay(500);
        }
    }
};

void setupBLE(void)
{

    BLEDevice::init("LilyGo-Watch");
    pServer = BLEDevice::createServer(); // Сохраняем указатель

    pServer->setCallbacks(new MyServerCallback);

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
            CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE);

    pCharacteristic->setCallbacks(new MyCallbacks());

    // Опционально: добавить дескриптор для уведомлений (CCCD)
//    pCharacteristic->addDescriptor(new BLE2902());

    pCharacteristic->setValue("Format: YY,MM,DD,h,m,s");
    pService->start();

    pAdvertising = pServer->getAdvertising(); // Сохраняем указатель
//    pAdvertising->setMinInterval(1600); // 1600 * 0.625 мс = 1000 мс
//    pAdvertising->setMaxInterval(1600);    
    pAdvertising->start();
}

void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("1. Download <LightBlue> on Google Play");
    Serial.println("2. Connect the Bluetooth device named <LilyGo-Watch >");
    Serial.println("3. Slide to the bottom of the page, the attribute UUID is <beb5483e-36e1-4688-b7f5-ea07361b26a8>, then click it");
    Serial.println("4. Select < UTF-8 String > in the Data format of the page");
    Serial.println("5. Click the READ AGAIN button, it will return to the format requirements set for the first time,the default format is YY, MM, DD, H, M, S");
    Serial.println("6. Under <WRITTEN VALUES>, fill in the time you need to set, such as : 2020, 08, 07, 11, 20, 30");
    Serial.println("7. Click the <WRITE> button on the right, the time will be written into the watch");
    Serial.println();

    //Get watch instance
    ttgo = TTGOClass::getWatch();
    // Initialize the hardware
    ttgo->begin();
    power = ttgo->power;
    power->adc1Enable(
    AXP202_BATT_VOL_ADC1 |   // Напряжение аккумулятора — ОБЯЗАТЕЛЬНО
    AXP202_BATT_CUR_ADC1,    // Ток заряда/разряда — опционально, но полезно
    true                     // true = включить
    );
    // --- НАЧАЛО ИЗМЕНЕНИЙ ---
    // Инициализация мотора сразу после begin()
    ttgo->motor_begin();
    Serial.println("Motor initialized.");
    ttgo->motor->onec(500);
    delay(800);
    // --- КОНЕЦ ИЗМЕНЕНИЙ ---
    // Turn on the backlight
    ttgo->openBL();
    displayIsOn = true; // Mark display as on after initialization
    Serial.println("Display turned ON initially.");
    //  Receive as a local variable for easy writing
    rtc = ttgo->rtc;
    tft = ttgo->tft;


    // Time check will be done, if the time is incorrect, it will be set to compile time
    rtc->check();

    // Some settings of BLE
    setupBLE();

    // Setup interrupt for PEK_KEY (side button)
    pinMode(AXP202_INT, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(AXP202_INT), handlePEKKeyPress, FALLING); // FALLING edge usually triggers on button press

    // Use ext1 for external wakeup
//    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);

    // Clear any pending interrupts on AXP202
    power->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
    power->clearIRQ();

    Serial.println("PEK_KEY interrupt configured.");

    setCpuFrequencyMhz(60); // или даже 40 МГц

    esp_wifi_set_mode(WIFI_MODE_NULL);
    esp_wifi_stop();

    // Draw initial connection status
    drawSTATUS(false);

    // Initialize the activity timestamp
    lastActivityTimestamp = millis();
}

void drawSTATUS(bool status)
{
    String str = status ? "Connection" : "Disconnect";
    int16_t cW = tft->textWidth("Connection", 2);
    int16_t dW = tft->textWidth("Disconnect", 2);
    int16_t w = cW > dW ? cW : dW;
    w += 6;
    int16_t x = 160;
    int16_t y = 20;
    int16_t h = tft->fontHeight(2) + 4;
    uint16_t col = status ? TFT_GREEN : TFT_GREY;
    tft->fillRoundRect(x, y, w, h, 3, col);
    tft->setTextColor(TFT_BLACK, col);
    tft->setTextFont(2);
    tft->drawString(str, x + 2, y);
}

void drawBatteryLevel(int percent) {
    // Фиксированная строка для расчёта максимальной ширины: "100%"
    String maxStr = "100%";
    int16_t w = tft->textWidth(maxStr, 2) + 6;  // ширина под самый длинный вариант
    int16_t h = tft->fontHeight(2) + 4;
    int16_t x = 2;
    int16_t y = 2;

    // Цвет фона — чёрный (как в drawSTATUS)
    tft->fillRoundRect(x, y, w, h, 3, TFT_BLACK);

    // Цвет текста в зависимости от заряда
    uint16_t textColor =
        percent > 30 ? TFT_GREEN :
        percent > 10 ? TFT_YELLOW : TFT_RED;

    tft->setTextColor(textColor, TFT_BLACK);
    tft->setTextFont(2);  // крупный шрифт, как в drawSTATUS
    tft->drawString(String(percent) + "%", x + 2, y);

//    tft->drawString(String(power->getBattDischargeCurrent()),x+5,y+2);
}

void loop() {

    static uint32_t lastUpdate = 0;
    if (millis() - lastUpdate > 5000) {
        float current_mA = power->getBattDischargeCurrent(); // ток разрядки в мА
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f mA", current_mA);

        // Записываем в характеристику как строку
        pCharacteristic->setValue(buffer);

        lastUpdate = millis();
    }

    // Check for connection status changes
    if (!deviceConnected && oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Draw deviceDisconnected");
        drawSTATUS(false);
    }

    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
        Serial.println("Draw deviceConnected");
        drawSTATUS(true);
    }

    // Handle PEK_KEY press interrupt flag
    if (pekKeyPressed) {
        Serial.println("PEK_KEY pressed interrupt received.");
        pekKeyPressed = false; // Сбросить флаг

        // Update timestamp on button press
        lastActivityTimestamp = millis();

        // Turn on display if it was off
        if (!displayIsOn) {
             power->setPowerOutPut(AXP202_LDO4, true);
             power->setPowerOutPut(AXP202_LDO3, true);
             power->setPowerOutPut(AXP202_LDO2, true);
             ttgo->openBL();
             displayIsOn = true;
             Serial.println("Display turned ON due to PEK_KEY press.");
/*
             // Optional: Vibrate to confirm wake-up
             if (ttgo) {
                 ttgo->motor->onec();
                 Serial.println("Vibration ON due to PEK_KEY press.");
             }
*/
        }
        // Clear AXP202 IRQ status after handling
        power->clearIRQ();
    }

    // Update time display every second IF display is on
    if (displayIsOn && (millis() - interval > 1000)) {
        interval = millis();
        tft->setTextColor(TFT_YELLOW, TFT_BLACK);
        tft->drawString(rtc->formatDateTime(PCF_TIMEFORMAT_DD_MM_YYYY), 50, 200, 4);
        tft->drawString(rtc->formatDateTime(PCF_TIMEFORMAT_HMS), 5, 118, 7);
    }

    // Update battery level every 10 seconds IF display is on
    if (displayIsOn && (millis() - lastBatteryUpdate > 10000)) {
        if (power->isBatteryConnect()) {
            batteryPercent = power->getBattPercentage();
            // Ограничиваем диапазон (иногда возвращает >100)
            batteryPercent = constrain(batteryPercent, 0, 100);
        } else {
            batteryPercent = -1; // батарея не подключена
        }
        lastBatteryUpdate = millis();
    }

    // Handle display off logic based on timeout
    if (displayIsOn && (millis() - lastActivityTimestamp > DISPLAY_OFF_TIMEOUT_MS)) {
        Serial.printf("Inactivity detected (%lu ms), turning display OFF...\n", millis() - lastActivityTimestamp);
        ttgo->closeBL(); // Turn off the backlight
        power->setPowerOutPut(AXP202_LDO2, false); // ← дисплей + подсветка
        power->setPowerOutPut(AXP202_LDO3, false); // ← акселерометр
        power->setPowerOutPut(AXP202_LDO4, false); // ← акселерометр
        // The following power channels are not used
        power->setPowerOutPut(AXP202_EXTEN, false);
        power->setPowerOutPut(AXP202_DCDC2, false);
//        ttgo->displaySleep(); 

//        esp_light_sleep_start(); // Проснется по таймеру или кнопке

        displayIsOn = false;
    }


    // Update UI elements (only if display is on, otherwise it's redundant)
    if (displayIsOn) {
        drawSTATUS(deviceConnected);
        if (batteryPercent >= 0) {
            drawBatteryLevel(batteryPercent);
        } else {
            // Can show "NO BAT" or hide
        }
    }

    // Small delay to prevent busy-waiting and allow proper timing checks
    delay(250);
}