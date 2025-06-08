/**
 * @file background_task.cc
 * @brief バックグラウンドタスク管理クラス実装
 * 
 * FreeRTOSタスクを使用してバックグラウンドでタスクを実行する
 * システムの実装です。メインスレッドをブロックすることなく、
 * 非同期でタスクを処理します。
 */

#include "background_task.h"

#include <esp_log.h>
#include <esp_task_wdt.h>

#define TAG "BackgroundTask"

/**
 * @brief BackgroundTaskコンストラクタ
 * @param stack_size タスクスタックサイズ（バイト）
 * 
 * FreeRTOSバックグラウンドタスクを作成し、
 * ワーカーループを開始します。
 */
BackgroundTask::BackgroundTask(uint32_t stack_size) {
    // FreeRTOSタスクを作成し、バックグラウンドループを開始
    xTaskCreate([](void* arg) {
        BackgroundTask* task = (BackgroundTask*)arg;
        task->BackgroundTaskLoop();
    }, "background_task", stack_size, this, 2, &background_task_handle_);
}

/**
 * @brief BackgroundTaskデストラクタ
 * 
 * バックグラウンドタスクを停止・削除します。
 */
BackgroundTask::~BackgroundTask() {
    if (background_task_handle_ != nullptr) {
        vTaskDelete(background_task_handle_);
    }
}

/**
 * @brief タスクをスケジュール（キューに追加）
 * @param callback 実行するタスクのコールバック関数
 * 
 * 指定されたコールバックをタスクキューに追加し、
 * バックグラウンドで実行させます。
 */
void BackgroundTask::Schedule(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // アクティブタスク数とメモリ使用量をチェック
    if (active_tasks_ >= 30) {
        int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
        if (free_sram < 10000) {
            ESP_LOGW(TAG, "active_tasks_ == %u, free_sram == %u", active_tasks_.load(), free_sram);
        }
    }
    
    active_tasks_++;
    // コールバックをラップして、完了時にカウンタをデクリメント
    main_tasks_.emplace_back([this, cb = std::move(callback)]() {
        cb();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            active_tasks_--;
            // すべてのタスクが完了したら通知
            if (main_tasks_.empty() && active_tasks_ == 0) {
                condition_variable_.notify_all();
            }
        }
    });
    condition_variable_.notify_all();
}

/**
 * @brief すべてのタスクの完了を待機
 * 
 * 現在実行中のすべてのタスクが完了するまでブロックします。
 */
void BackgroundTask::WaitForCompletion() {
    std::unique_lock<std::mutex> lock(mutex_);
    // タスクキューが空でアクティブタスクが0になるまで待機
    condition_variable_.wait(lock, [this]() {
        return main_tasks_.empty() && active_tasks_ == 0;
    });
}

/**
 * @brief バックグラウンドタスクのメインループ
 * 
 * タスクキューにタスクが追加されるのを待ち、
 * 順次実行します。
 */
void BackgroundTask::BackgroundTaskLoop() {
    ESP_LOGI(TAG, "background_task started");
    while (true) {
        std::unique_lock<std::mutex> lock(mutex_);
        // タスクキューにタスクが追加されるまで待機
        condition_variable_.wait(lock, [this]() { return !main_tasks_.empty(); });
        
        // タスクキューをローカルに移動（ロックを解放するため）
        std::list<std::function<void()>> tasks = std::move(main_tasks_);
        lock.unlock();

        // すべてのタスクを順次実行
        for (auto& task : tasks) {
            task();
        }
    }
}
