// ------------------------------------------------------------------------------------------------------------------------------
// ----------------                Using LEGACY I2S driver (driver/i2s.h) for ESP-IDF 4.x compatibility        ----------------
// ----------------                                                                                              ----------------
// ----------------   bool I2S_Record_Init();                // Initialization (once)                            ----------------
// ----------------   bool Record_Start(file);               // appending I2S buffer to file (in loop, ongoing)  ----------------
// ----------------   bool Record_Available(file, &duration) // stop recording (once)                            ----------------
// ------------------------------------------------------------------------------------------------------------------------------

#include "driver/i2s.h"         // Legacy I2S driver for ESP-IDF 4.x
#include <SD.h>
#include <Arduino.h>

// --- defines & macros --------

#ifndef DEBUG
#  define DEBUG true
#  define DebugPrint(x);        if(DEBUG){Serial.print(x);}
#  define DebugPrintln(x);      if(DEBUG){Serial.println(x);}
#endif

// --- PIN assignments ---------

#define I2S_WS      7
#define I2S_SD      8
#define I2S_SCK     17

// --- define your settings ----

#define SAMPLE_RATE             16000
#define BITS_PER_SAMPLE         8
#define GAIN_BOOSTER_I2S        45

bool I2S_Record_Init();
bool Record_Start(String audio_filename);
bool Record_Available(String audio_filename, float* audiolength_sec);

// --- global vars -------------

#define I2S_NUM         I2S_NUM_0

// WAV Header
struct WAV_HEADER {
    char  riff[4] = {'R','I','F','F'};
    long  flength = 0;
    char  wave[4] = {'W','A','V','E'};
    char  fmt[4]  = {'f','m','t',' '};
    long  chunk_size = 16;
    short format_tag = 1;
    short num_chans = 1;
    long  srate = SAMPLE_RATE;
    long  bytes_per_sec = SAMPLE_RATE * (BITS_PER_SAMPLE/8);
    short bytes_per_samp = (BITS_PER_SAMPLE/8);
    short bits_per_samp = BITS_PER_SAMPLE;
    char  dat[4] = {'d','a','t','a'};
    long  dlength = 0;
} myWAV_Header;

bool flg_is_recording = false;
bool flg_I2S_initialized = false;

// ------------------------------------------------------------------------------------------------------------------------------

bool I2S_Record_Init() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,  // I2S uses 16-bit internally
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD
    };

    esp_err_t err = i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("Failed to install I2S driver: %d\n", err);
        return false;
    }

    err = i2s_set_pin(I2S_NUM, &pin_config);
    if (err != ESP_OK) {
        Serial.printf("Failed to set I2S pins: %d\n", err);
        return false;
    }

    flg_I2S_initialized = true;
    return true;
}

// ------------------------------------------------------------------------------------------------------------------------------

bool Record_Start(String audio_filename) {
    if (!flg_I2S_initialized) {
        Serial.println("ERROR in Record_Start() - I2S not initialized");
        return false;
    }

    if (!flg_is_recording) {
        flg_is_recording = true;

        if (SD.exists(audio_filename)) {
            SD.remove(audio_filename);
            DebugPrintln("\n> Existing AUDIO file removed.");
        } else {
            DebugPrintln("\n> No AUDIO file found");
        }

        File audio_file = SD.open(audio_filename, FILE_WRITE);
        audio_file.write((uint8_t *)&myWAV_Header, 44);
        audio_file.close();

        DebugPrintln("> WAV Header generated, Audio Recording started ... ");
        return true;
    }

    if (flg_is_recording) {
        int16_t audio_buffer[1500];
        uint8_t audio_buffer_8bit[1500];

        size_t bytes_read = 0;
        esp_err_t result = i2s_read(I2S_NUM, audio_buffer, sizeof(audio_buffer), &bytes_read, portMAX_DELAY);

        if (result != ESP_OK) {
            Serial.printf("I2S read error: %d\n", result);
            return false;
        }

        // Gain boost
        if (GAIN_BOOSTER_I2S > 1 && GAIN_BOOSTER_I2S <= 64) {
            for (int16_t i = 0; i < (bytes_read / 2); ++i) {
                audio_buffer[i] = audio_buffer[i] * GAIN_BOOSTER_I2S;
            }
        }

        // Convert to 8-bit if needed
        if (BITS_PER_SAMPLE == 8) {
            for (int16_t i = 0; i < (bytes_read / 2); ++i) {
                audio_buffer_8bit[i] = (uint8_t)(((audio_buffer[i] + 32768) >> 8) & 0xFF);
            }
        }

        File audio_file = SD.open(audio_filename, FILE_APPEND);
        if (audio_file) {
            if (BITS_PER_SAMPLE == 16) {
                audio_file.write((uint8_t*)audio_buffer, bytes_read);
            }
            if (BITS_PER_SAMPLE == 8) {
                audio_file.write((uint8_t*)audio_buffer_8bit, bytes_read/2);
            }
            audio_file.close();
            return true;
        }

        if (!audio_file) {
            Serial.println("ERROR in Record_Start() - Failed to open audio file!");
            return false;
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------------------------------------

bool Record_Available(String audio_filename, float* audiolength_sec) {
    if (!flg_is_recording) {
        return false;
    }

    if (!flg_I2S_initialized) {
        return false;
    }

    if (flg_is_recording) {
        File audio_file = SD.open(audio_filename, "r+");
        long filesize = audio_file.size();
        audio_file.seek(0);
        myWAV_Header.flength = filesize;
        myWAV_Header.dlength = (filesize-8);
        audio_file.write((uint8_t *)&myWAV_Header, 44);
        audio_file.close();

        flg_is_recording = false;

        *audiolength_sec = (float)(filesize-44) / (SAMPLE_RATE * BITS_PER_SAMPLE/8);

        DebugPrintln("> ... Done. Audio Recording finished.");
        DebugPrint("> New AUDIO file: '" + audio_filename + "', filesize [bytes]: " + (String)filesize);
        DebugPrintln(", length [sec]: " + (String)*audiolength_sec);

        return true;
    }
    return false;
}
