/**
 * @file dummy_audio_processor.h
 * @brief ダミー音声処理プロセッサ
 * 
 * 音声処理を行わないパススループロセッサです。
 * テスト目的や、特別な音声処理が不要な状況で使用します。
 * 入力された音声データをそのまま出力します。
 */
#ifndef DUMMY_AUDIO_PROCESSOR_H
#define DUMMY_AUDIO_PROCESSOR_H

#include <vector>
#include <functional>

#include "audio_processor.h"
#include "audio_codec.h"

/**
 * @class DummyAudioProcessor
 * @brief ダミー音声処理プロセッサ
 * 
 * 実際の音声処理を行わず、入力されたデータをそのまま通す
 * パススループロセッサです。テストやデバッグ、特別な音声処理が
 * 不要な場合に使用します。
 */
class DummyAudioProcessor : public AudioProcessor {
public:
    DummyAudioProcessor() = default;
    ~DummyAudioProcessor() = default;

    /** ダミー初期化（何もしない） */
    void Initialize(AudioCodec* codec) override;
    
    /** データをそのままコールバックに渡す */
    void Feed(const std::vector<int16_t>& data) override;
    
    /** ダミー開始（状態を更新するだけ） */
    void Start() override;
    
    /** ダミー停止（状態を更新するだけ） */
    void Stop() override;
    
    /** 動作状態を返す */
    bool IsRunning() override;
    
    /** 出力コールバックを設定 */
    void OnOutput(std::function<void(std::vector<int16_t>&& data)> callback) override;
    
    /** VADコールバックを設定（ダミー） */
    void OnVadStateChange(std::function<void(bool speaking)> callback) override;
    
    /** フィードサイズを返す */
    size_t GetFeedSize() override;

private:
    AudioCodec* codec_ = nullptr;                                               /**< オーディオコーデックインスタンス */
    std::function<void(std::vector<int16_t>&& data)> output_callback_;          /**< 出力データコールバック */
    std::function<void(bool speaking)> vad_state_change_callback_;              /**< VAD状態変化コールバック（未使用） */
    bool is_running_ = false;                                                   /**< 動作状態フラグ */
};

#endif 