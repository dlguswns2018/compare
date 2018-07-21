/* Google translation device example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "esp_http_client.h"
#include "sdkconfig.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "audio_hal.h"
#include "esp_peripherals.h"
#include "periph_button.h"
#include "periph_wifi.h"
#include "periph_led.h"
#include "google_tts.h"
#include "google_sr.h"
#include "google_translate.h"
#include "board.h"

static const char *TAG = "GOOGLE_TRANSLATION_EXAMPLE";

#define GOOGLE_SR_LANG "cmn-Hans-CN"            // https://cloud.google.com/speech-to-text/docs/languages
#define GOOGLE_TRANSLATE_LANG_FROM "zh-CN"      //https://cloud.google.com/translate/docs/languages
#define GOOGLE_TRANSLATE_LANG_TO "en"           //https://cloud.google.com/translate/docs/languages
//#define GOOGLE_TTS_LANG "en-US-Wavenet-D"       //https://cloud.google.com/text-to-speech/docs/voices
#define GOOGLE_TTS_LANG "ko-KR-Standard-A"       //https://cloud.google.com/text-to-speech/docs/voices

#define EXAMPLE_RECORD_PLAYBACK_SAMPLE_RATE (16000)

#define CONFIG_WIFI_SSID "G6"  // set your value
#define CONFIG_WIFI_PASSWORD "555555555"  // set your value
#define CONFIG_GOOGLE_API_KEY "djskljgkjagiAjdkajgkAdjkfjkadsjgijoag"  // set your value

esp_periph_handle_t led_handle = NULL;
void google_sr_begin(google_sr_handle_t sr)
{
    if (led_handle) {
        periph_led_blink(led_handle, GPIO_LED_GREEN, 500, 500, true, -1);
    }
    ESP_LOGW(TAG, "Start speaking now");
}

void translate_task(void *pv)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_LOGI(TAG, "translate_task");

    
    tcpip_adapter_init();
    
    ESP_LOGI(TAG, "[ 1 ] Initialize Buttons & Connect to Wi-Fi network, ssid=%s", CONFIG_WIFI_SSID);
    // Initialize peripherals management
    esp_periph_config_t periph_cfg = { 0 };
    esp_periph_init(&periph_cfg);

    periph_wifi_cfg_t wifi_cfg = {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
    };
    
    esp_periph_handle_t wifi_handle = periph_wifi_init(&wifi_cfg);
    
    // Initialize Button peripheral
    periph_button_cfg_t btn_cfg = {
        .gpio_mask = GPIO_SEL_MODE | GPIO_SEL_REC,
    };
    esp_periph_handle_t button_handle = periph_button_init(&btn_cfg);

    periph_led_cfg_t led_cfg = {
        .led_speed_mode = LEDC_LOW_SPEED_MODE,
        .led_duty_resolution = LEDC_TIMER_10_BIT,
        .led_timer_num = LEDC_TIMER_0,
        .led_freq_hz = 5000,
    };
    led_handle = periph_led_init(&led_cfg);


    // Start wifi & button peripheral
    esp_periph_start(button_handle);
    esp_periph_start(wifi_handle);
    esp_periph_start(led_handle); 
    
    periph_wifi_wait_for_connected(wifi_handle, portMAX_DELAY);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_hal_codec_config_t audio_hal_codec_cfg =  AUDIO_HAL_MAX98357_DEFAULT();
    audio_hal_codec_cfg.i2s_iface.samples = AUDIO_HAL_08K_SAMPLES;
    audio_hal_handle_t hal = audio_hal_init(&audio_hal_codec_cfg, 2);
    ESP_LOGI(TAG, "audio_hal_init finish");
    //audio_hal_ctrl_codec(hal, AUDIO_HAL_CODEC_MODE_BOTH, AUDIO_HAL_CTRL_START);
    
    /*google_sr_config_t sr_config = {
        .api_key = CONFIG_GOOGLE_API_KEY,
        .lang_code = GOOGLE_SR_LANG,
        .record_sample_rates = EXAMPLE_RECORD_PLAYBACK_SAMPLE_RATE,
        .encoding = ENCODING_LINEAR16,
        .on_begin = google_sr_begin,
    };
    google_sr_handle_t sr = google_sr_init(&sr_config);*/
    
    google_tts_config_t tts_config = {
        .api_key = CONFIG_GOOGLE_API_KEY,
        .playback_sample_rate = EXAMPLE_RECORD_PLAYBACK_SAMPLE_RATE,
    };
    google_tts_handle_t tts = google_tts_init(&tts_config);

    ESP_LOGI(TAG, "[ 4 ] Setup event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from the pipeline");
    //googe_sr_set_listener(sr, evt);
    googe_tts_set_listener(tts, evt);

    ESP_LOGI(TAG, "[4.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_get_event_iface(), evt);

    ESP_LOGI(TAG, "[ 5 ] Listen for all pipeline events");

	  google_tts_start(tts, "기상청에 따르면 이날 서울 낮 최고기온은 36.9도에 달해 기록적인 폭염이 닥쳤던 1994년 이후 7월 기온으로 가장 높았다. ", GOOGLE_TTS_LANG);	
    while (1) {
        audio_event_iface_msg_t msg;
        
        ESP_LOGW(TAG, "while start ........... ");
                 
        if (audio_event_iface_listen(evt, &msg, portMAX_DELAY) != ESP_OK) {
            ESP_LOGW(TAG, "[ * ] Event process failed: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                 msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);
            continue;
        }

        ESP_LOGI(TAG, "[ * ] Event received: src_type:%d, source:%p cmd:%d, data:%p, data_len:%d",
                 msg.source_type, msg.source, msg.cmd, msg.data, msg.data_len);

        if (google_tts_check_event_finish(tts, &msg)) {
            ESP_LOGI(TAG, "[ * ] TTS Finish");
            continue;
        }

		/*

        if (msg.source_type != PERIPH_ID_BUTTON) {
            continue;
        }

        // It's MODE button
        if ((int)msg.data == GPIO_MODE) {
            break;
        }

        if ((int)msg.data != GPIO_REC) {
            continue;
        }

        if (msg.cmd == PERIPH_BUTTON_PRESSED) {
            google_tts_stop(tts);
            ESP_LOGI(TAG, "[ * ] Resuming pipeline");
            google_sr_start(sr);
        } else if (msg.cmd == PERIPH_BUTTON_RELEASE || msg.cmd == PERIPH_BUTTON_LONG_RELEASE) {
            ESP_LOGI(TAG, "[ * ] Stop pipeline");

            periph_led_stop(led_handle, GPIO_LED_GREEN);

            char *original_text = google_sr_stop(sr);
            if (original_text == NULL) {
                continue;
            }
            ESP_LOGI(TAG, "Original text = %s", original_text);
            char *translated_text = google_translate(original_text, GOOGLE_TRANSLATE_LANG_FROM, GOOGLE_TRANSLATE_LANG_TO, CONFIG_GOOGLE_API_KEY);
            if (translated_text == NULL) {
                continue;
            }
            ESP_LOGI(TAG, "Translated text = %s", translated_text);
            google_tts_start(tts, translated_text, GOOGLE_TTS_LANG);
        }

		*/

    }
    ESP_LOGI(TAG, "[ 6 ] Stop audio_pipeline");
    //google_sr_destroy(sr);
    google_tts_destroy(tts);
    /* Stop all periph before removing the listener */
    esp_periph_stop_all();
    audio_event_iface_remove_listener(esp_periph_get_event_iface(), evt);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);
    esp_periph_destroy();
    
    vTaskDelete(NULL);
}

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    xTaskCreate(translate_task, "gg_translation", 2 * 4096, NULL, 5, NULL);
}
