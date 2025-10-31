#include "ConfigManager.h"
#include "KeyResolver.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

using json = nlohmann::json;

ConfigManager::ConfigManager(std::string configPath) 
    : m_configPath(std::move(configPath)), m_loaded(false) {
}

bool ConfigManager::load() {
    std::ifstream configFile(m_configPath);
    if (!configFile.is_open()) {
        // ログに詳細な情報を出力
        std::string msg = "Config file cannot be opened: " + m_configPath;
        OutputDebugStringA(msg.c_str());
        return false; // File doesn't exist or cannot be opened
    }

    try {
        json j;
        configFile >> j;

        // JSON構造をログに出力
        std::string jsonStr = j.dump();
        std::string logMsg = "JSON loaded: " + jsonStr.substr(0, 200) + "...";
        OutputDebugStringA(logMsg.c_str());

        m_gamepad = j.at("gamepad").get<GamepadConfig>();
        m_system = j.at("config").get<SystemConfig>();

        // 読み込んだ設定をログに出力
        std::string configMsg = "Config loaded - Buttons: " + std::to_string(m_gamepad.buttons.size()) + 
                               ", Threshold: " + std::to_string(m_system.stick_threshold);
        OutputDebugStringA(configMsg.c_str());

        compileKeyMappings();
        m_loaded = true;
    } catch (const json::exception& e) {
        // Failed to parse JSON - 詳細なエラー情報をログに出力
        std::string errorMsg = "JSON parse error: " + std::string(e.what()) + " (file: " + m_configPath + ")";
        OutputDebugStringA(errorMsg.c_str());
        return false;
    }

    return true;
}

bool ConfigManager::save() const {
    std::ofstream configFile(m_configPath);
    if (!configFile.is_open()) {
        return false;
    }

    json j;
    j["gamepad"] = m_gamepad;
    j["config"] = m_system;

    configFile << j.dump(2); // Indent with 2 spaces
    return true;
}

void ConfigManager::setConfig(const GamepadConfig& gamepad, const SystemConfig& system) {
    m_gamepad = gamepad;
    m_system = system;
    compileKeyMappings();
    m_loaded = true;
}

std::pair<GamepadConfig, SystemConfig> ConfigManager::createDefaultConfig() {
    GamepadConfig gamepad;
    SystemConfig system;

    // Default buttons
    gamepad.buttons = {
        {0, {"z"}},
        {1, {"x"}},
        {2, {"c"}},
        {3, {"v"}},
        {5, {"win"}},
        {7, {"alt", "tab"}},
    };

    // Default DPad
    gamepad.dpad = {{"up"}, {"down"}, {"left"}, {"right"}};

    // Default Left Stick
    gamepad.left_stick = {{"a"}, {"d"}, {"w"}, {"s"}};

    // Default System Config
    system.stick_threshold = 400;
    system.log_level = "info";

    return {gamepad, system};
}

void ConfigManager::compileKeyMappings() {
    m_buttonCache.clear();
    m_dpadCache.clear();
    m_stickCache.clear();

    // Compile buttons
    for (const auto& button : m_gamepad.buttons) {
        m_buttonCache[button.index] = compileKeySequence(button.keys);
    }

    // Compile DPad
    m_dpadCache["up"] = compileKeySequence(m_gamepad.dpad.up);
    m_dpadCache["down"] = compileKeySequence(m_gamepad.dpad.down);
    m_dpadCache["left"] = compileKeySequence(m_gamepad.dpad.left);
    m_dpadCache["right"] = compileKeySequence(m_gamepad.dpad.right);

    // Compile Left Stick
    m_stickCache["up"] = compileKeySequence(m_gamepad.left_stick.up);
    m_stickCache["down"] = compileKeySequence(m_gamepad.left_stick.down);
    m_stickCache["left"] = compileKeySequence(m_gamepad.left_stick.left);
    m_stickCache["right"] = compileKeySequence(m_gamepad.left_stick.right);
}

std::vector<WORD> ConfigManager::compileKeySequence(const std::vector<std::string>& keys) const {
    return KeyResolver::resolveSequence(keys);
}

std::vector<WORD> ConfigManager::getButtonKeys(int buttonIndex) const {
    auto it = m_buttonCache.find(buttonIndex);
    if (it != m_buttonCache.end()) {
        return it->second;
    }
    return {}; // Return empty vector if not found
}

std::vector<WORD> ConfigManager::getDpadKeys(const std::string& direction) const {
    auto it = m_dpadCache.find(direction);
    if (it != m_dpadCache.end()) {
        return it->second;
    }
    return {};
}

std::vector<WORD> ConfigManager::getStickKeys(const std::string& direction) const {
    auto it = m_stickCache.find(direction);
    if (it != m_stickCache.end()) {
        return it->second;
    }
    return {};
}