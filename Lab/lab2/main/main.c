#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
 
static const char *TAG = "LAB2_ADC_SETTLING";
 
// กำหนดขาภาคส่ง RGB LED
#define TX_LED_R_GPIO        GPIO_NUM_4
#define TX_LED_G_GPIO        GPIO_NUM_5
#define TX_LED_B_GPIO        GPIO_NUM_18   // ใช้แทน GPIO6 เพราะ ESP32-WROOM-32 ใช้ GPIO6-11 กับ Flash
 
// หลอดขาว 4 ขาเป็นแบบ Common Anode (COM ต่อ 3V3) -> ต้องกลับ logic การติด/ดับ
// ติดไฟ = ส่ง 0 (ดึงขั้วลง GND ให้กระแสไหลจาก 3V3 ผ่านหลอด)
// ดับไฟ = ส่ง 1 (เท่ากับฝั่ง COM ไม่มีแรงดันตกคร่อม)
#define TX_ON   0
#define TX_OFF  1
 
// กำหนดขาภาครับอนาล็อก (ESP32: ADC1_CH6 คือ GPIO34)
#define RX_ADC_UNIT          ADC_UNIT_1
#define RX_ADC_CHANNEL       ADC_CHANNEL_6
 
#define NUM_SAMPLES          20
#define SAMPLING_DELAY_MS    150   // 3000ms / 20 samples = 150ms
 
void init_hardware(adc_oneshot_unit_handle_t *adc_handle)
{
    // 1. ตั้งค่าขาเอาต์พุตดิจิทัลสำหรับควบคุม LED RGB
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TX_LED_R_GPIO) | (1ULL << TX_LED_G_GPIO) | (1ULL << TX_LED_B_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
 
    // ดับไฟเริ่มต้น
    gpio_set_level(TX_LED_R_GPIO, TX_OFF);
    gpio_set_level(TX_LED_G_GPIO, TX_OFF);
    gpio_set_level(TX_LED_B_GPIO, TX_OFF);
 
    // 2. ตั้งค่าหน่วย ADC Unit 1 ธรรมดา (ไม่มีการ Calibrate เพื่อดูบิตดิบ)
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = RX_ADC_UNIT,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, adc_handle));
 
    // 3. ตั้งค่าขาสัญญาณอนาล็อก ความละเอียดเริ่มต้น (12 บิต: 0 - 4095)
    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, // รองรับช่วงระดับแรงดันเต็มพิกัด 3.3V
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(*adc_handle, RX_ADC_CHANNEL, &chan_config));
}
 
// ฟังก์ชันจำลองวงจรอ่านค่าดิบแบบอนุกรมเวลาในช่วงสลับสีไฟ
void sample_and_print(adc_oneshot_unit_handle_t adc_handle, const char* phase_name)
{
    printf("Color %s:\n", phase_name);
    printf("No, ADC Raw\n");
 
    // ทำการสุ่มอ่าน 20 แซมเปิ้ล โดยเก็บค่า adc ต่อเนื่องทุก 150ms
    for (int i = 1; i <= NUM_SAMPLES; i++) {
        int raw_value = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, RX_ADC_CHANNEL, &raw_value));
 
        // พิมพ์ค่าดิบในรูปแบบ CSV ฟอร์แมตตามข้อกำหนด
        printf("%d, %d\n", i, raw_value);
 
        vTaskDelay(pdMS_TO_TICKS(SAMPLING_DELAY_MS));
    }
}
 
void app_main(void)
{
    adc_oneshot_unit_handle_t adc1_handle;
    init_hardware(&adc1_handle);
 
    ESP_LOGI(TAG, "Transient Observation System Online.");
    printf("==============================================================\n");
 
    while (1) {
        // --- รอบไฟสีแดง ---
        gpio_set_level(TX_LED_R_GPIO, TX_ON);
        vTaskDelay(pdMS_TO_TICKS(2500)); // เปล่งแสงนาน 2.5 วินาที
        gpio_set_level(TX_LED_R_GPIO, TX_OFF); // ดับไฟเข้าสู่จังหวะพัก (Rest Phase)
        sample_and_print(adc1_handle, "RED");
        printf("--------------------------------------------------------------\n");
 
        // --- รอบไฟสีเขียว ---
        gpio_set_level(TX_LED_G_GPIO, TX_ON);
        vTaskDelay(pdMS_TO_TICKS(2500));
        gpio_set_level(TX_LED_G_GPIO, TX_OFF);
        sample_and_print(adc1_handle, "GREEN");
        printf("--------------------------------------------------------------\n");
 
        // --- รอบไฟสีน้ำเงิน ---
        gpio_set_level(TX_LED_B_GPIO, TX_ON);
        vTaskDelay(pdMS_TO_TICKS(2500));
        gpio_set_level(TX_LED_B_GPIO, TX_OFF);
        sample_and_print(adc1_handle, "BLUE");
        printf("==============================================================\n");
    }
}