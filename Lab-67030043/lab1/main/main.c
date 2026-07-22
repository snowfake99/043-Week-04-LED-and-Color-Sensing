#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
 
static const char *TAG = "LAB1_RGB_TIMING";
 
/* กำหนดขา GPIO ตามผังวงจรของบอร์ด ESP32-WROOM-32
* หมายเหตุ: ใช้ GPIO18 แทน GPIO6 เดิมในใบงาน
* เพราะบอร์ดนี้ GPIO6-11 ถูกใช้เชื่อมกับ Flash Memory ภายใน ห้ามใช้เป็น GPIO ทั่วไป */
#define LED_R_GPIO        GPIO_NUM_4
#define LED_G_GPIO        GPIO_NUM_5
#define LED_B_GPIO        GPIO_NUM_18
 
/* ตั้งเป็น 1 ถ้า LED เป็นแบบ Common Anode (ขา COM ต่อ 3V3, สั่งติดด้วย logic 0)
* ตั้งเป็น 0 ถ้าเป็น Common Cathode (ขา COM ต่อ GND, สั่งติดด้วย logic 1) */
#define LED_COMMON_ANODE   1
 
// กำหนดเวลาหน่วง (หน่วยมิลลิวินาที)
#define TIME_ON_MS         2000  // ติดไฟ 2 วินาที
#define TIME_OFF_MS        500  // ดับไฟ 0.5 วินาที ก่อนข้ามไปสีถัดไป
#define TIME_REST_MS      1000  // เว้นระยะพักรอบวงลูป (หลังครบ 3 สี) 1 วินาที
 
static inline int on_level(void)  { return LED_COMMON_ANODE ? 0 : 1; }
static inline int off_level(void) { return LED_COMMON_ANODE ? 1 : 0; }
 
void init_rgb_gpio(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_R_GPIO) | (1ULL << LED_G_GPIO) | (1ULL << LED_B_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
 
    // เริ่มต้นให้ LED ทุกสีดับสนิท
    gpio_set_level(LED_R_GPIO, off_level());
    gpio_set_level(LED_G_GPIO, off_level());
    gpio_set_level(LED_B_GPIO, off_level());
}
 
void app_main(void)
{
    init_rgb_gpio();
    ESP_LOGI(TAG, "RGB LED Timing System Started.");
 
    while (1) {
        // -----------------------------------------------------------
        // เฟสที่ 1: จ่ายแสงสีแดง (Red Phase) - ติด 0.5วิ ดับ 0.5วิ
        // -----------------------------------------------------------
        ESP_LOGI(TAG, "Phase R: ON");
        gpio_set_level(LED_R_GPIO, on_level());
        vTaskDelay(pdMS_TO_TICKS(TIME_ON_MS));
 
        gpio_set_level(LED_R_GPIO, off_level());
        ESP_LOGI(TAG, "Phase R: OFF");
        vTaskDelay(pdMS_TO_TICKS(TIME_OFF_MS));
 
        // -----------------------------------------------------------
        // เฟสที่ 2: จ่ายแสงสีเขียว (Green Phase) - ติด 0.5วิ ดับ 0.5วิ
        // -----------------------------------------------------------
        ESP_LOGI(TAG, "Phase G: ON");
        gpio_set_level(LED_G_GPIO, on_level());
        vTaskDelay(pdMS_TO_TICKS(TIME_ON_MS));
 
        gpio_set_level(LED_G_GPIO, off_level());
        ESP_LOGI(TAG, "Phase G: OFF");
        vTaskDelay(pdMS_TO_TICKS(TIME_OFF_MS));
 
        // -----------------------------------------------------------
        // เฟสที่ 3: จ่ายแสงสีน้ำเงิน (Blue Phase) - ติด 0.5วิ ดับ 0.5วิ
        // -----------------------------------------------------------
        ESP_LOGI(TAG, "Phase B: ON");
        gpio_set_level(LED_B_GPIO, on_level());
        vTaskDelay(pdMS_TO_TICKS(TIME_ON_MS));
 
        gpio_set_level(LED_B_GPIO, off_level());
        ESP_LOGI(TAG, "Phase B: OFF");
        vTaskDelay(pdMS_TO_TICKS(TIME_OFF_MS));
 
        // -----------------------------------------------------------
        // เฟสที่ 4: ระยะพักระบบ (Rest Phase / Discharge Window)
        // -----------------------------------------------------------
        ESP_LOGI(TAG, "Entering Rest Phase... Waiting for residual charge to dissipate.");
        vTaskDelay(pdMS_TO_TICKS(TIME_REST_MS));
 
        printf("-----------------------------------------------------------\n");
    }
}