#include <Arduino.h>
#include <driver/i2s.h>

#define SAMPLE_RATE 8000
#define SAMPLE_BUFFER_SIZE 1024

// Buffers para double buffering
int16_t buffer_1[SAMPLE_BUFFER_SIZE];
int16_t buffer_2[SAMPLE_BUFFER_SIZE];
volatile bool buffer_1_ready = false;
volatile bool buffer_2_ready = false;

// i2s setup
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_I2S,
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 4,
  .dma_buf_len = 512,
  .use_apll = false,
  .tx_desc_auto_clear = true,
  .fixed_mclk = 1
};

i2s_pin_config_t i2s_mic_pins = {
  .bck_io_num = GPIO_NUM_26,
  .ws_io_num = GPIO_NUM_22,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = GPIO_NUM_21
};

// Tarefa de captura de áudio
void captureAudio(void *param) {
  size_t bytes_read;
  while (true) {
    // Captura para o buffer 1 se estiver disponível
    if (!buffer_1_ready) {
      i2s_read(I2S_NUM_0, buffer_1, sizeof(buffer_1), &bytes_read, portMAX_DELAY);
      buffer_1_ready = true;
    }

    // Captura para o buffer 2 se estiver disponível
    if (!buffer_2_ready) {
      i2s_read(I2S_NUM_0, buffer_2, sizeof(buffer_2), &bytes_read, portMAX_DELAY);
      buffer_2_ready = true;
    }

    // Atraso opcional para ajuste fino
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// Tarefa de envio de áudio pela UART
void sendAudio(void *param) {
  while (true) {
    // Enviar dados do buffer 1
    if (buffer_1_ready) {      
      for(int i =0; i < SAMPLE_BUFFER_SIZE; i++){
        Serial.println(buffer_1[i], DEC);
      }
      buffer_1_ready = false;  // Marcar buffer 1 como pronto para capturar novamente
    }

    // Enviar dados do buffer 2
    if (buffer_2_ready) {
      for(int i =0; i < SAMPLE_BUFFER_SIZE; i++){
        Serial.println(buffer_2[i], DEC);
      }
      buffer_2_ready = false;  // Marcar buffer 2 como pronto para capturar novamente
    }

    // Atraso opcional para ajuste fino
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(250000);
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);

  // Criar tarefas RTOS
  xTaskCreate(captureAudio, "Captura de Áudio", 4096, NULL, 1, NULL);
  xTaskCreate(sendAudio, "Envio de Áudio", 4096, NULL, 1, NULL);
}

void loop() {
  // Não há nada no loop principal, pois tudo ocorre nas tarefas RTOS
}
