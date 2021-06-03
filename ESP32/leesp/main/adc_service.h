
#ifndef ADC_SERVICE_H
#define ADC_SERVICE_H

void adc_Init(void);
float get_adc_val(void);
void adc_task(void *pvParameters);
int get_adc_init_sta(void);

#endif/* ADC_SERVICE_H */
