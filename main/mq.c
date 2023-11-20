#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"

#define TAG_MQ "MQ"
#define RLOAD 10.0
#define RZERO 390
/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 116.6020682
#define PARB 2.769034857
#define ATMOCO2 397.13

static esp_adc_cal_characteristics_t adc1_chars;
int status = 0;

static uint32_t adc_MQ;
static float ppm;

void prvCalibrating(void)
{

    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_0, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11));
    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    vTaskDelay(100);
}

float getResistance(uint32_t voltage)
{
    return ((1023. / (float)voltage) * 5. - 1.) * RLOAD;
}

float getRZero(uint32_t voltage)
{
    return getResistance(voltage) * pow((ATMOCO2 / PARA), (1. / PARB));
}

float getPPM(uint32_t voltage)
{
    return PARA * pow((getResistance(voltage) / RZERO), -PARB);
}

float readData(void)
{
    if (status == 0)
    {
        prvCalibrating();
        status = 1;
    }

    adc_MQ = esp_adc_cal_raw_to_voltage(adc1_get_raw(ADC1_CHANNEL_6), &adc1_chars);
    ppm = getPPM(adc_MQ);
    // ESP_LOGI(TAG_MQ, "PPM: %f", ppm);
    return ppm;
}
