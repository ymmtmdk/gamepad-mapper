#pragma once
#include <nlohmann/json.hpp>
#include <windows.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include <span>
#include <utility>

using json = nlohmann::json;

// ゲームパッド設定データ
struct GamepadConfig {
    struct Button {
        int index;
        std::vector<std::string> keys;
        
        // JSON自動変換サポート
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Button, index, keys)
    };
    
    struct Stick {
        std::vector<std::string> left, right, up, down;
        
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Stick, left, right, up, down)
    };
    
    struct DPad {
        std::vector<std::string> up, down, left, right;
        
        NLOHMANN_DEFINE_TYPE_INTRUSIVE(DPad, up, down, left, right)
    };
    
    std::vector<Button> buttons;
    DPad dpad;
    Stick left_stick;
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(GamepadConfig, buttons, dpad, left_stick)
};

// システム設定
struct SystemConfig {
    int stick_threshold = 400;
    std::string log_level = "info";
    
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SystemConfig, stick_threshold, log_level)
};

// メイン設定クラス
class ConfigManager {
public:
    explicit ConfigManager(std::string configPath);
    ~ConfigManager() = default;
    
    // 移動のみ可能
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    ConfigManager(ConfigManager&&) = default;
    ConfigManager& operator=(ConfigManager&&) = default;
    
    // メイン API
    bool load();
    bool save() const;
    
    // クエリAPI（モダンで使いやすい）
    std::vector<WORD> getButtonKeys(int buttonIndex) const;
    std::vector<WORD> getDpadKeys(const std::string& direction) const;
    std::vector<WORD> getStickKeys(const std::string& direction) const;
    
    int getStickThreshold() const { return m_system.stick_threshold; }
    std::string getLogLevel() const { return m_system.log_level; }
    
    // ユーティリティ
    bool isLoaded() const { return m_loaded; }
    std::string getConfigPath() const { return m_configPath; }
    std::wstring getConfigFilePath() const { 
        int size = MultiByteToWideChar(CP_UTF8, 0, m_configPath.c_str(), -1, nullptr, 0);
        if (size > 0) {
            std::wstring wpath(size - 1, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, m_configPath.c_str(), -1, &wpath[0], size);
            return wpath;
        }
        return L"";
    }

    // 設定のセッター
    void setConfig(const GamepadConfig& gamepad, const SystemConfig& system);

    // 静的メソッド
    static std::pair<GamepadConfig, SystemConfig> createDefaultConfig();

private:
    // 内部処理
    void compileKeyMappings();
    std::vector<WORD> compileKeySequence(const std::vector<std::string>& keys) const;
    
    // データ
    GamepadConfig m_gamepad;
    SystemConfig m_system;
    
    // コンパイル済みキャッシュ（パフォーマンス向上）
    std::unordered_map<int, std::vector<WORD>> m_buttonCache;
    std::unordered_map<std::string, std::vector<WORD>> m_dpadCache;
    std::unordered_map<std::string, std::vector<WORD>> m_stickCache;
    
    // 状態
    std::string m_configPath;
    bool m_loaded = false;
};