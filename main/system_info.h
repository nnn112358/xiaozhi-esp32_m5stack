/**
 * @file system_info.h
 * @brief システム情報取得ユーティリティクラス
 * 
 * ESP32システムの各種情報（メモリ使用量、フラッシュサイズ、
 * MACアドレス、チップ情報など）を取得するためのユーティリティクラスです。
 * デバッグやシステムモニタリングに使用されます。
 */
#ifndef _SYSTEM_INFO_H_
#define _SYSTEM_INFO_H_

#include <string>

#include <esp_err.h>
#include <freertos/FreeRTOS.h>

/**
 * @class SystemInfo
 * @brief ESP32システム情報取得用ユーティリティクラス
 * 
 * ESP32のハードウェア情報、メモリ使用状況、タスク情報などを
 * 取得するための静的メソッド群を提供します。
 * デバッグやシステム監視に役立ちます。
 */
class SystemInfo {
public:
    /** フラッシュメモリの総サイズを取得 */
    static size_t GetFlashSize();
    
    /** 起動後の最小空きヒープサイズを取得 */
    static size_t GetMinimumFreeHeapSize();
    
    /** 現在の空きヒープサイズを取得 */
    static size_t GetFreeHeapSize();
    
    /** デバイスのMACアドレスを取得 */
    static std::string GetMacAddress();
    
    /** チップモデル名を取得 */
    static std::string GetChipModelName();
    
    /** タスクCPU使用率を出力 */
    static esp_err_t PrintTaskCpuUsage(TickType_t xTicksToWait);
    
    /** 実行中タスク一覧を出力 */
    static void PrintTaskList();
    
    /** ヒープメモリ統計を出力 */
    static void PrintHeapStats();
};

#endif // _SYSTEM_INFO_H_
