/**
 * @file audio_codec.cc
 * @brief オーディオコーデック基底クラス実装
 * 
 * すべてのオーディオコーデック（ES8311、ES8374、ES8388等）の
 * 基底クラスの実装です。音声入出力の共通インターフェースを提供します。
 */

#include "audio_codec.h"
#include "board.h"
#include "settings.h"

#include <esp_log.h>
#include <cstring>
#include <driver/i2s_common.h>

#define TAG "AudioCodec"

/**
 * @brief AudioCodecコンストラクタ
 * 
 * オーディオコーデックの基底クラスを初期化します。
 */
AudioCodec::AudioCodec() {
}

/**
 * @brief AudioCodecデストラクタ
 * 
 * オーディオコーデックリソースをクリーンアップします。
 */
AudioCodec::~AudioCodec() {
}

/**
 * @brief 音声データを出力
 * @param data 出力する音声データバッファ
 * 
 * 16ビット音声データをコーデックに出力します。
 */
void AudioCodec::OutputData(std::vector<int16_t>& data) {
    Write(data.data(), data.size());
}

/**
 * @brief 音声データを入力
 * @param data 入力音声データを格納するバッファ
 * @return データが取得できた場合true
 * 
 * コーデックから16ビット音声データを読み取ります。
 */
bool AudioCodec::InputData(std::vector<int16_t>& data) {
    int samples = Read(data.data(), data.size());
    if (samples > 0) {
        return true;
    }
    return false;
}

void AudioCodec::Start() {
    Settings settings("audio", false);
    output_volume_ = settings.GetInt("output_volume", output_volume_);
    if (output_volume_ <= 0) {
        ESP_LOGW(TAG, "Output volume value (%d) is too small, setting to default (10)", output_volume_);
        output_volume_ = 10;
    }

    ESP_ERROR_CHECK(i2s_channel_enable(tx_handle_));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_handle_));

    EnableInput(true);
    EnableOutput(true);
    ESP_LOGI(TAG, "Audio codec started");
}

void AudioCodec::SetOutputVolume(int volume) {
    output_volume_ = volume;
    ESP_LOGI(TAG, "Set output volume to %d", output_volume_);
    
    Settings settings("audio", true);
    settings.SetInt("output_volume", output_volume_);
}

void AudioCodec::EnableInput(bool enable) {
    if (enable == input_enabled_) {
        return;
    }
    input_enabled_ = enable;
    ESP_LOGI(TAG, "Set input enable to %s", enable ? "true" : "false");
}

void AudioCodec::EnableOutput(bool enable) {
    if (enable == output_enabled_) {
        return;
    }
    output_enabled_ = enable;
    ESP_LOGI(TAG, "Set output enable to %s", enable ? "true" : "false");
}
