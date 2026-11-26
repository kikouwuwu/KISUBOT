// KISUBOT - Cabeza para XiaoZhi
// Basada en bread-compact-wifi-s3cam, con estos cambios:
// - Sin pantalla (los pines 19/20 quedan para el USB nativo).
// - Audio en modo duplex con los pines del plan de KISUBOT.
// - LED común en GPIO 2 (GpioLed), en el canal PWM 1 para no chocar con la cámara.
// - La cámara se enciende solo para sacar una foto y después la OV5640 queda en
//   reposo (registro 0x3008 = 0x42). Sin esto se calienta aunque no se use,
//   porque la placa no tiene pin de apagado (ver fase 5 en docs/plan-cabeza.md).

#include "wifi_board.h"
#include "codecs/no_audio_codec.h"
#include "application.h"
#include "button.h"
#include "config.h"
#include "led/gpio_led.h"
#include "esp32_camera.h"

#include <esp_log.h>
#include <memory>

#define TAG "KisubotCabeza"

// Cámara que se enciende solo para cada foto y después queda en reposo.
class CamaraBajoDemanda : public Camera {
private:
    camera_config_t config_;
    std::unique_ptr<Esp32Camera> camara_;
    std::string url_;
    std::string token_;
    // Orientación de la imagen. build.py no ofrece camera_vflip para esta placa
    // (busca "new Esp32Camera" y aquí se crea con make_unique), así que va aquí.
    // Volteada, igual que VOLTEAR_IMAGEN = 1 en la fase 5. -1 = no cambiar.
    int hmirror_ = 0;
    int vflip_ = 1;

    bool Encender() {
        if (camara_) {
            return true;
        }
        camara_ = std::make_unique<Esp32Camera>(config_);
        sensor_t* s = esp_camera_sensor_get();
        if (s == nullptr) {
            ESP_LOGE(TAG, "La camara no inicio");
            camara_.reset();
            return false;
        }
        if (!url_.empty()) {
            camara_->SetExplainUrl(url_, token_);
        }
        if (hmirror_ >= 0) camara_->SetHMirror(hmirror_);
        if (vflip_ >= 0) camara_->SetVFlip(vflip_);
        // Recién encendida, la cámara tarda en ajustar la luz: botamos unas fotos
        for (int i = 0; i < 6; i++) {
            camera_fb_t* fb = esp_camera_fb_get();
            if (fb) esp_camera_fb_return(fb);
        }
        return true;
    }

    void Apagar() {
        if (!camara_) {
            return;
        }
        sensor_t* s = esp_camera_sensor_get();
        if (s && s->id.PID == OV5640_PID) {
            int r = s->set_reg(s, 0x3008, 0xff, 0x42);  // software power down
            ESP_LOGI(TAG, "Camara en reposo: %s", r == 0 ? "OK" : "FALLO");
        }
        camara_.reset();  // el destructor llama a esp_camera_deinit()
    }

public:
    explicit CamaraBajoDemanda(const camera_config_t& config) : config_(config) {
        // Prueba al arrancar y deja el sensor en reposo de inmediato
        if (Encender()) {
            sensor_t* s = esp_camera_sensor_get();
            ESP_LOGI(TAG, "Camara OK, sensor PID 0x%x", s ? s->id.PID : 0);
        }
        Apagar();
    }

    ~CamaraBajoDemanda() override { Apagar(); }

    void SetExplainUrl(const std::string& url, const std::string& token) override {
        url_ = url;
        token_ = token;
        if (camara_) camara_->SetExplainUrl(url, token);
    }

    bool Capture() override {
        if (!Encender()) {
            return false;
        }
        bool ok = camara_->Capture();
        if (!ok) {
            Apagar();
        }
        return ok;
    }

    bool SetHMirror(bool enabled) override {
        hmirror_ = enabled ? 1 : 0;
        return camara_ ? camara_->SetHMirror(enabled) : true;
    }

    bool SetVFlip(bool enabled) override {
        vflip_ = enabled ? 1 : 0;
        return camara_ ? camara_->SetVFlip(enabled) : true;
    }

    std::expected<std::string, std::string> Explain(const std::string& question) override {
        return Explain(question, url_, token_);
    }

    std::expected<std::string, std::string> Explain(const std::string& question,
                                                    const std::string& url,
                                                    const std::string& token) override {
        if (!camara_) {
            return std::unexpected("No camera frame captured");
        }
        auto resultado = camara_->Explain(question, url, token);
        Apagar();  // la foto ya se envió: la cámara vuelve a reposo
        return resultado;
    }
};

class KisubotCabezaBoard : public WifiBoard {
private:
    Button boot_button_;
    CamaraBajoDemanda* camera_ = nullptr;

    void InitializeCamera() {
        camera_config_t config = {};
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = CAMERA_PIN_D0;
        config.pin_d1 = CAMERA_PIN_D1;
        config.pin_d2 = CAMERA_PIN_D2;
        config.pin_d3 = CAMERA_PIN_D3;
        config.pin_d4 = CAMERA_PIN_D4;
        config.pin_d5 = CAMERA_PIN_D5;
        config.pin_d6 = CAMERA_PIN_D6;
        config.pin_d7 = CAMERA_PIN_D7;
        config.pin_xclk = CAMERA_PIN_XCLK;
        config.pin_pclk = CAMERA_PIN_PCLK;
        config.pin_vsync = CAMERA_PIN_VSYNC;
        config.pin_href = CAMERA_PIN_HREF;
        config.pin_sccb_sda = CAMERA_PIN_SIOD;
        config.pin_sccb_scl = CAMERA_PIN_SIOC;
        config.sccb_i2c_port = 0;
        config.pin_pwdn = CAMERA_PIN_PWDN;
        config.pin_reset = CAMERA_PIN_RESET;
        config.xclk_freq_hz = XCLK_FREQ_HZ;
        config.pixel_format = PIXFORMAT_RGB565;
        config.frame_size = FRAMESIZE_VGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;  // con fb_count = 1, igual que s3cam
        camera_ = new CamaraBajoDemanda(config);
    }

    void InitializeButtons() {
        boot_button_.OnClick([this]() {
            auto& app = Application::GetInstance();
            if (app.GetDeviceState() == kDeviceStateStarting) {
                EnterWifiConfigMode();
                return;
            }
            app.ToggleChatState();
        });
    }

public:
    KisubotCabezaBoard() : boot_button_(BOOT_BUTTON_GPIO) {
        InitializeButtons();
        InitializeCamera();
    }

    Led* GetLed() override {
        // Canal PWM 1: el canal 0 lo usa el reloj de la cámara (XCLK)
        static GpioLed led(BUILTIN_LED_GPIO, 0, LEDC_TIMER_1, LEDC_CHANNEL_1);
        return &led;
    }

    AudioCodec* GetAudioCodec() override {
        static NoAudioCodecDuplex audio_codec(AUDIO_INPUT_SAMPLE_RATE, AUDIO_OUTPUT_SAMPLE_RATE,
            AUDIO_I2S_GPIO_BCLK, AUDIO_I2S_GPIO_WS, AUDIO_I2S_GPIO_DOUT, AUDIO_I2S_GPIO_DIN);
        return &audio_codec;
    }

    Camera* GetCamera() override {
        return camera_;
    }
};

DECLARE_BOARD(KisubotCabezaBoard);
