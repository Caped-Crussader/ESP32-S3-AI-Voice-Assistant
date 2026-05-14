# ESP32-S3 AI Voice Assistant

An AI-powered voice assistant running on the ESP32-S3, using an **INMP441** I2S microphone for audio capture and a **MAX98357** I2S amplifier for audio playback.

<img width="848" height="480" alt="gtcespvid - Trim - Trim" src="https://github.com/user-attachments/assets/c4a73acb-d0f5-408c-9ce4-00548dcc5805" />

<img width="2332" height="1398" alt="espassi" src="https://github.com/user-attachments/assets/be5680ee-337b-443f-82bc-6fd1afcb6055" />

## Schematic

<img width="1480" height="1027" alt="image" src="https://github.com/user-attachments/assets/bf1c3c40-e8fb-4ca7-bece-3b81ea00ac60" />


---

## Features

- Push-to-talk triggered voice capture
- I2S audio input via INMP441 MEMS microphone
- AI-powered response generation (speech-to-text → LLM → text-to-speech)
- Audio playback through MAX98357 Class D amplifier
- Built with PlatformIO on the Arduino/ESP-IDF framework

---

## Hardware

| Component | Description |
|-----------|-------------|
| ESP32-S3 | Main microcontroller with Wi-Fi |
| INMP441 | I2S MEMS microphone (audio input) |
| MAX98357 | I2S Class D amplifier (audio output) |
| Speaker | 4–8 Ω speaker connected to MAX98357 |

### Wiring

#### INMP441 (Microphone)

| INMP441 Pin | ESP32-S3 GPIO |
|-------------|---------------|
| VDD | 3.3V |
| GND | GND |
| SD (Data) | GPIO 8 |
| WS (Word Select) | GPIO 7 |
| SCK (Clock) | GPIO 17 |

#### MAX98357 (Amplifier)

| MAX98357 Pin | ESP32-S3 GPIO |
|--------------|---------------|
| VIN | 5V |
| GND | GND |
| DIN (Data) | GPIO 4 |
| LRC (Word Select) | GPIO 5 |
| BCLK (Bit Clock) | GPIO 9 |

---

## Software Dependencies

Managed via PlatformIO (`platformio.ini`):

- `ESP32 Arduino` framework
- I2S driver (built-in ESP-IDF)
- HTTP client library (for API calls)
- Any audio encoding/decoding library ( `ArduinoJson`, WAV encoder)

---

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) installed (VS Code extension or CLI)
- ESP32-S3 board support package
- Active Wi-Fi network
- API keys for your AI backend (e.g., OpenAI Whisper + GPT + TTS, or similar)

### Setup

1. **Clone the repo**
   ```bash
   git clone https://github.com/Caped-Crussader/ESP32-S3-AI-Voice-Assistant.git
   cd ESP32-S3-AI-Voice-Assistant
   ```

2. **Configure credentials**

   Create a `include/secrets.h` file (not tracked by git):
   ```cpp
   #pragma once

   #define WIFI_SSID     "your_ssid"
   #define WIFI_PASSWORD "your_password"
   #define OPENAI_API_KEY "sk-..."
   ```

3. **Build and flash**
   ```bash
   pio run --target upload
   ```

4. **Monitor serial output**
   ```bash
   pio device monitor
   ```

---

## How It Works

```
[INMP441 mic] --I2S--> [ESP32-S3] --HTTP/HTTPS--> [AI API (STT + LLM + TTS)]
                            |
                        [MAX98357] ---> [Speaker]
```

1. Audio is sampled from the INMP441 over I2S.
2. The captured PCM buffer is encoded and sent to a speech-to-text API.
3. The transcript is forwarded to an LLM to generate a response.
4. The response text is converted to speech via a TTS API.
5. The returned audio is streamed out through the MAX98357 to the speaker.

---

## Project Structure

```
ESP32-S3-AI-Voice-Assistant/
├── src/          # Main application source
├── include/      # Header files and config
├── lib/          # Local libraries
├── test/         # Unit tests
├── .vscode/      # VS Code / PlatformIO settings
└── platformio.ini
```

---

## License

This project is open-source. 

---

## Contributing

Pull requests are welcome. For major changes, open an issue first to discuss what you'd like to change.
