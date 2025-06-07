/**
 * @file background_task.h
 * @brief バックグラウンドタスク管理クラス
 * 
 * FreeRTOSタスクを使用してバックグラウンド処理を管理するクラスです。
 * メインスレッドをブロックすることなく、非同期でタスクを実行し、
 * 完了を待機する機能を提供します。
 */
#ifndef BACKGROUND_TASK_H
#define BACKGROUND_TASK_H

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mutex>
#include <list>
#include <condition_variable>
#include <atomic>

/**
 * @class BackgroundTask
 * @brief バックグラウンドタスク管理クラス
 * 
 * FreeRTOSタスクを使用して、メインスレッドをブロックせずに
 * バックグラウンドでタスクを実行するためのクラスです。
 * タスクキューとワーカースレッドを管理し、非同期処理を実現します。
 */
class BackgroundTask {
public:
    /**
     * @brief バックグラウンドタスクコンストラクタ
     * @param stack_size タスクスタックサイズ（デフォルト: 8KB）
     */
    BackgroundTask(uint32_t stack_size = 4096 * 2);
    ~BackgroundTask();

    /**
     * @brief タスクをスケジュール（キューに追加）
     * @param callback 実行するタスクのコールバック関数
     */
    void Schedule(std::function<void()> callback);
    
    /** すべてのタスクの完了を待機 */
    void WaitForCompletion();

private:
    std::mutex mutex_;                                  /**< タスクキュー保護用ミューテックス */
    std::list<std::function<void()>> main_tasks_;      /**< 実行待ちタスクのキュー */
    std::condition_variable condition_variable_;        /**< タスク完了通知用条件変数 */
    TaskHandle_t background_task_handle_ = nullptr;     /**< バックグラウンドタスクハンドル */
    std::atomic<size_t> active_tasks_{0};               /**< アクティブタスク数（アトミック） */

    /** バックグラウンドタスクのメインループ */
    void BackgroundTaskLoop();
};

#endif
