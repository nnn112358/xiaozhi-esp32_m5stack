/**
 * @file audio_processor.h
 * @brief 音声処理プロセッサの抽象インターフェース
 * 
 * 音声信号のAEC（エコーキャンセレーション）、VAD（音声活動検出）、
 * ノイズリダクションなどの処理を抽象化したインターフェースです。
 */
#ifndef AUDIO_PROCESSOR_H
#define AUDIO_PROCESSOR_H

#include <string>
#include <vector>
#include <functional>

#include "audio_codec.h"

/**
 * @class AudioProcessor
 * @brief 音声処理プロセッサの抽象インターフェース
 * 
 * 音声データのリアルタイム処理を行い、エコー除去やノイズ低減、
 * 音声活動検出などの機能を提供します。
 */
class AudioProcessor {
public:
    virtual ~AudioProcessor() = default;
    
    /** オーディオコーデックで初期化 */
    virtual void Initialize(AudioCodec* codec) = 0;
    
    /** 音声データをプロセッサに供給 */
    virtual void Feed(const std::vector<int16_t>& data) = 0;
    
    /** 音声処理を開始 */
    virtual void Start() = 0;
    
    /** 音声処理を停止 */
    virtual void Stop() = 0;
    
    /** 処理中かどうかを確認 */
    virtual bool IsRunning() = 0;
    
    /** 処理済み音声データのコールバック設定 */
    virtual void OnOutput(std::function<void(std::vector<int16_t>&& data)> callback) = 0;
    
    /** VAD（音声活動検出）状態変化コールバック設定 */
    virtual void OnVadStateChange(std::function<void(bool speaking)> callback) = 0;
    
    /** 1回のフィードで必要なサンプル数を取得 */
    virtual size_t GetFeedSize() = 0;
};

#endif
