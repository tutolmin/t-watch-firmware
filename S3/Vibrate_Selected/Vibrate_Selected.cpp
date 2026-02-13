/**
 * @file      DRV2605_Basic.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2025  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2025-04-23
 *
 */
//#include <Arduino.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>

lv_obj_t *slider;

// Массив разрешенных эффектов + 0 для выключения
const uint8_t allowed_effects[] = {0, 1, 10, 12, 14, 15, 15, 17, 47, 58, 59, 64, 70, 71, 82, 83, 88, 89, 94, 95, 106, 107, 118};
const uint8_t effects_count = sizeof(allowed_effects) / sizeof(allowed_effects[0]);

// Константы для быстрых эффектов
const uint8_t EFFECT_82 = 82;
const uint8_t EFFECT_107 = 107;
const uint8_t EFFECT_58 = 58;
const uint8_t EFFECT_71 = 71;
const uint8_t EFFECT_15 = 15;
const uint8_t EFFECT_89 = 89;

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
    uint8_t effect = (uint8_t)(uintptr_t)lv_event_get_user_data(e);
    
    // Воспроизводим выбранный эффект
    instance.drv.setWaveform(0, effect);
    instance.drv.run();
}

void vibration_feedback_effect_handler(lv_timer_t *time)
{
    // get the effect value
    int16_t value = lv_slider_get_value(slider);
    
    // Проверяем, что индекс не выходит за пределы массива
    if(value >= 0 && value < effects_count) {
        uint8_t effect = allowed_effects[value];
        if(effect != 0) {
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

    lv_timer_create(vibration_feedback_effect_handler, 2000, NULL);

    // Верхние кнопки (три в ряд над слайдером) - теперь с увеличенным отступом
    // Кнопка для эффекта 15 - левая
    lv_obj_t *button_15_top = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_15_top, 40, 40);
    lv_obj_align_to(button_15_top, slider, LV_ALIGN_OUT_TOP_LEFT, 0, -40); // увеличенный отступ сверху
    lv_obj_add_event_cb(button_15_top, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_15);
    lv_obj_t *label_15_top = lv_label_create(button_15_top);
    lv_label_set_text(label_15_top, "15");
    lv_obj_center(label_15_top);

    // Кнопка для эффекта 82 - центральная
    lv_obj_t *button_82_top = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_82_top, 40, 40);
    lv_obj_align_to(button_82_top, slider, LV_ALIGN_OUT_TOP_MID, 0, -40); // увеличенный отступ сверху
    lv_obj_add_event_cb(button_82_top, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_82);
    lv_obj_t *label_82_top = lv_label_create(button_82_top);
    lv_label_set_text(label_82_top, "82");
    lv_obj_center(label_82_top);

    // Кнопка для эффекта 89 - правая
    lv_obj_t *button_89_top = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_89_top, 40, 40);
    lv_obj_align_to(button_89_top, slider, LV_ALIGN_OUT_TOP_RIGHT, 0, -40); // увеличенный отступ сверху
    lv_obj_add_event_cb(button_89_top, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_89);
    lv_obj_t *label_89_top = lv_label_create(button_89_top);
    lv_label_set_text(label_89_top, "89");
    lv_obj_center(label_89_top);

    // Нижние кнопки (три в ряд под слайдером, с увеличенным отступом)
    // Кнопка для эффекта 58 - левая
    lv_obj_t *button_58_bottom = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_58_bottom, 40, 40);
    lv_obj_align_to(button_58_bottom, slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 50); // увеличенный отступ снизу
    lv_obj_add_event_cb(button_58_bottom, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_58);
    lv_obj_t *label_58_bottom = lv_label_create(button_58_bottom);
    lv_label_set_text(label_58_bottom, "58");
    lv_obj_center(label_58_bottom);

    // Кнопка для эффекта 71 - центральная
    lv_obj_t *button_71_bottom = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_71_bottom, 40, 40);
    lv_obj_align_to(button_71_bottom, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 50); // увеличенный отступ снизу
    lv_obj_add_event_cb(button_71_bottom, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_71);
    lv_obj_t *label_71_bottom = lv_label_create(button_71_bottom);
    lv_label_set_text(label_71_bottom, "71");
    lv_obj_center(label_71_bottom);

    // Кнопка для эффекта 107 - правая
    lv_obj_t *button_107_bottom = lv_btn_create(lv_scr_act());
    lv_obj_set_size(button_107_bottom, 40, 40);
    lv_obj_align_to(button_107_bottom, slider, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 50); // увеличенный отступ снизу
    lv_obj_add_event_cb(button_107_bottom, button_click_handler, LV_EVENT_CLICKED, (void*)(uintptr_t)EFFECT_107);
    lv_obj_t *label_107_bottom = lv_label_create(button_107_bottom);
    lv_label_set_text(label_107_bottom, "107");
    lv_obj_center(label_107_bottom);

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