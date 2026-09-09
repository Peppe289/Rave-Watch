#include "hal/adc_types.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "voltage.h"

static adc_oneshot_unit_handle_t adc1_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;

void battery_adc_init(void) {
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = BATTERY_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, BATTERY_ADC_CHANNEL, &config));

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = BATTERY_ADC_UNIT,
        .chan = BATTERY_ADC_CHANNEL,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    esp_err_t ret = adc_cali_create_scheme_curve_fitting(&cali_config, &adc_cali_handle);
    if (ret != ESP_OK) {
        printf("Calibrazione ADC curve-fitting non supportata o fallita\n");
    }
}

uint32_t battery_get_voltage_mv(void) {
    int raw_val = 0;
    int pin_mv = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, BATTERY_ADC_CHANNEL, &raw_val));

    if (adc_cali_handle != NULL) {
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc_cali_handle, raw_val, &pin_mv));
    } else {
        pin_mv = (raw_val * 3100) / 4095;
    }

    return (uint32_t)(pin_mv * BATTERY_VOLTAGE_DIVIDER_RATIO);
}