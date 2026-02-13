/**
 * @file      DRV2605_Basic.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2025-04-23
 *
 */
#include <LilyGoLib.h>
#include <LV_Helper.h>

lv_obj_t *slider;

// Массив разрешенных эффектов + 0 для выключения
const uint8_t allowed_effects[] = {0, 1, 10, 12, 14, 15, 15, 17, 47, 58, 59, 64, 70, 71, 82, 88, 89, 94, 95, 106, 107, 118};
const uint8_t effects_count = sizeof(allowed_effects) / sizeof(allowed_effects[0]);

// Константы для быстрых эффектов
const uint8_t EFFECT_82 = 82;
const uint8_t EFFECT_107 = 107;
const uint8_t EFFECT_58 = 58;
const uint8_t EFFECT_89 = 89;
const uint8_t EFFECT_47 = 47;

void value_changed_event_cb(lv_event_t *e)
{
    lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    int16_t value = lv_slider_get_value(slider);
    
    // Проверяем, что индекс не выходит за пределы массива
    if(value >= 0 && value < effects_count) {
        uint8_t effect = allowed_effects[value];
        if(effect == 0) {
            lv_label_set_text_fmt(label, "Off:%d", value);
        } else {
            lv_label_set_text_fmt(label, "Effect:%d (%d)", value, effect);
        }
    } else {
        lv_label_set_text_fmt(label, "Effect:%d", value);
    }
}

void button_click_handler(lv_event_t *e)
{
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_current_target(e);
    uint8_t button_type = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    Serial.print("Button clicked: ");
    Serial.println(button_type);
    
    switch(button_type) {
        case 1: // Кнопка для последовательности 89-47
            Serial.println("Starting sequence 89->47");
            instance.drv.setWaveform(0, 89);
            instance.drv.run();
            Serial.println("First effect 89 started");
            delay(1000); // ждем завершения первого эффекта
            Serial.println("Delay after first effect completed");
            delay(500); // пауза 400мс
            Serial.println("Pause completed");
            instance.drv.setWaveform(0, 47);
            instance.drv.run();
            Serial.println("Second effect 47 started");
            break;
            
        case 2: // Кнопка для эффекта 82
            Serial.println("Playing effect 82");
            instance.drv.setWaveform(0, EFFECT_82);
            instance.drv.run();
            break;
            
        case 3: // Кнопка для сигнала err (82, 70, 82)
            Serial.println("Playing error signal: 82->70->82");
            instance.drv.setWaveform(0, 82);
            instance.drv.run();
            delay(1000); // ждем завершения первого эффекта
            Serial.println("First effect 82 completed");
            delay(600); // пауза 400мс
            Serial.println("Pause between effects completed");
            instance.drv.setWaveform(0, 15);
            instance.drv.run();
            delay(1000); // ждем завершения второго эффекта
            Serial.println("Effect 70 completed");
            delay(600); // пауза 400мс
            Serial.println("Pause before third effect completed");
            instance.drv.setWaveform(0, 82);
            instance.drv.run();
            Serial.println("Third effect 82 started");
            break;
            
        case 4: // Кнопка для эффекта 58
            Serial.println("Playing effect 58");
            instance.drv.setWaveform(0, EFFECT_58);
            instance.drv.run();
            break;
            
        case 5: // Кнопка для последовательности 71-71 с паузой
            Serial.println("Starting sequence 71->pause->71");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            Serial.println("First effect 71 started");
            delay(1000); // ждем завершения первого эффекта
            Serial.println("Delay after first effect completed");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            Serial.println("Second effect 71 started");
            break;
            
        case 6: // Кнопка для двойной последовательности (dbl)
            Serial.println("Playing double signal sequence");
            // 2 раза вибрация 71 с интервалом 400мс
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            delay(1000); // ждем завершения первого эффекта
            Serial.println("First effect 71 of first pair completed");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            delay(1000); // ждем завершения второго эффекта
            Serial.println("Second effect 71 of first pair completed");
            delay(1000); // пауза 1500мс
            Serial.println("Main pause completed");
            // 4 раза вибрация 71 с интервалом 400мс
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            delay(1000); // ждем завершения первого эффекта
            Serial.println("First effect 71 of second sequence completed");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            delay(1000); // ждем завершения второго эффекта
            Serial.println("Second effect 71 of second sequence completed");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            delay(1000); // ждем завершения третьего эффекта
            Serial.println("Third effect 71 of second sequence completed");
            instance.drv.setWaveform(0, 71);
            instance.drv.run();
            Serial.println("Fourth effect 71 of second sequence started");
            break;
    }
    
    Serial.println("Button handler completed");
}

void vibration_feedback_effect_handler(lv_timer_t *time)
{
    // get the effect value
    int16_t value = lv_slider_get_value(slider);
    
    // Проверяем, что индекс не выходит за пределы массива
    if(value >= 0 && value < effects_count) {
        uint8_t effect = allowed_effects[value];
        if(effect != 0) {
            Serial.print("Slider effect: ");
            Serial.println(effect);
            // set the effect to play
            instance.drv.setWaveform(0, effect);  // play allowed effect
            // play the effect!
            instance.drv.run();
        }
        // Если effect == 0, то вибрация отключена
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Setup starting...");

    instance.begin();

    beginLvglHelper(instance);

    /*Create a slider in the center of the display*/
    slider = lv_slider_create(lv_scr_act());
    lv_obj_set_width(slider, LV_PCT(80));
    lv_obj_set_height(slider, LV_PCT(10));
    lv_slider_set_range(slider, 0, effects_count - 1);  // Изменяем диапазон на количество разрешенных эффектов + 1 для выключения
    lv_obj_center(slider);

    // Устанавливаем начальное значение слайдера в 0 (выключено)
    lv_slider_set_value(slider, 0, LV_ANIM_OFF);

    // Create input device group , only T-LoRa-Pager need.
    lv_group_t *group = lv_group_create();
    lv_set_default_group(group);
    lv_group_add_obj(lv_group_get_default(), slider);

    /*Create a label below the slider*/
    lv_obj_t *slider_label = lv_label_create(lv_scr_act());
    lv_label_set_text(slider_label, "Off:0");
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_add_event_cb(slider, value_changed_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);

    lv_timer_create(vibration_feedback_effect_handler, 1000, NULL);

    // Верхние кнопки (три в ряд над слайдером)
    // Кнопка для последовательности 89-47 - левая
    lv_obj_t *button_seq_89_47 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_seq_89_47, 40, 40);
    lv_obj_align_to(button_seq_89_47, slider, LV_ALIGN_OUT_TOP_LEFT, 0, -40);
    lv_obj_add_event_cb(button_seq_89_47, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)1);
    lv_obj_t *label_seq_89_47 = lv_label_create(button_seq_89_47);
    lv_label_set_text(label_seq_89_47, "Go");
    lv_obj_center(label_seq_89_47);

    // Кнопка для эффекта 82 - центральная
    lv_obj_t *button_82_top = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_82_top, 40, 40);
    lv_obj_align_to(button_82_top, slider, LV_ALIGN_OUT_TOP_MID, 0, -40);
    lv_obj_add_event_cb(button_82_top, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)2);
    lv_obj_t *label_82_top = lv_label_create(button_82_top);
    lv_label_set_text(label_82_top, "Skip");
    lv_obj_center(label_82_top);

    // Кнопка для сигнала err (82, 70, 82) - правая
    lv_obj_t *button_err = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_err, 40, 40);
    lv_obj_align_to(button_err, slider, LV_ALIGN_OUT_TOP_RIGHT, 0, -40);
    lv_obj_add_event_cb(button_err, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)3);
    lv_obj_t *label_err = lv_label_create(button_err);
    lv_label_set_text(label_err, "err");
    lv_obj_center(label_err);

    // Нижние кнопки (три в ряд под слайдером, с увеличенным отступом)
    // Кнопка для эффекта 58 - левая
    lv_obj_t *button_58_bottom = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_58_bottom, 40, 40);
    lv_obj_align_to(button_58_bottom, slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50);
    lv_obj_add_event_cb(button_58_bottom, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)4);
    lv_obj_t *label_58_bottom = lv_label_create(button_58_bottom);
    lv_label_set_text(label_58_bottom, "58");
    lv_obj_center(label_58_bottom);

    // Кнопка для последовательности 71-71 с паузой - центральная
    lv_obj_t *button_seq_71_71 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_seq_71_71, 40, 40);
    lv_obj_align_to(button_seq_71_71, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 50);
    lv_obj_add_event_cb(button_seq_71_71, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)5);
    lv_obj_t *label_seq_71_71 = lv_label_create(button_seq_71_71);
    lv_label_set_text(label_seq_71_71, "Sngl");
    lv_obj_center(label_seq_71_71);

    // Кнопка для двойной последовательности - правая
    lv_obj_t *button_dbl = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_dbl, 40, 40);
    lv_obj_align_to(button_dbl, slider, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 50);
    lv_obj_add_event_cb(button_dbl, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)6);
    lv_obj_t *label_dbl = lv_label_create(button_dbl);
    lv_label_set_text(label_dbl, "dbl");
    lv_obj_center(label_dbl);

    Serial.println("Setup completed");

    // Set brightness to MAX
    // T-LoRa-Pager brightness level is 0 ~ 16
    // T-Watch-S3 , T-Watch-S3-Plus , T-Watch-Ultra brightness level is 0 ~ 255
    instance.setBrightness(DEVICE_MAX_BRIGHTNESS_LEVEL);
}


void loop()
{
    lv_task_handler();
    delay(5);
}