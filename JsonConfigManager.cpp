#include "JsonConfigManager.h"
#include "KeyResolver.h"
#include <fstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

JsonConfigManager::JsonConfigManager() : m_loaded(false) {
    // Attempt to load the default config file.
    // If it fails, a default config will be created and loaded.
    load();
}

bool JsonConfigManager::load(const std::string& configPath) {
    m_configPath = configPath;
    std::ifstream configFile(m_configPath);

    if (!configFile.is_open()) {
        // If the file doesn't exist, create a default one.
        createDefaultConfig();
        // Try to open it again.
        configFile.open(m_configPath);
        if (!configFile.is_open()) {
            // If it still fails, something is wrong.
            return false;
        }
    }

    try {
        json j;
        configFile >> j;

        m_gamepad = j.at("gamepad").get<GamepadConfig>();
        m_system = j.at("config").get<SystemConfig>();

        compileKeyMappings();
        m_loaded = true;
    } catch (json::exception& e) {
        // Failed to parse JSON
        // Consider logging the error e.what()
        return false;
    }

    return true;
}

bool JsonConfigManager::save(const std::string& configPath) const {
    std::ofstream configFile(configPath);
    if (!configFile.is_open()) {
        return false;
    }

    json j;
    j["gamepad"] = m_gamepad;
    j["config"] = m_system;

    configFile << j.dump(2); // Indent with 2 spaces
    return true;
}

void JsonConfigManager::createDefaultConfig() {
    // This function is called when the config file is not found.
    // It populates the members with default values and then saves them.

    // Default buttons
    m_gamepad.buttons = {
        {0, {"z"}},
        {1, {"x"}},
        {2, {"c"}},
        {3, {"v"}},
        {4, {"printscreen"}},
        {5, {"win", "x"}},
        {6, {"ctrl", "c"}},
        {7, {"alt", "tab"}},
        {8, {"ctrl", "alt", "delete"}}
    };

    // Default DPad
    m_gamepad.dpad = {{"up"}, {"down"}, {"left"}, {"right"}};

    // Default Left Stick
    m_gamepad.left_stick = {{"a"}, {"d"}, {"w"}, {"s"}};

    // Default System Config
    m_system.stick_threshold = 400;
    m_system.log_level = "info";

    // Save this default configuration.
    save(m_configPath);
}

void JsonConfigManager::compileKeyMappings() {
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

std::vector<WORD> JsonConfigManager::compileKeySequence(const std::vector<std::string>& keys) const {
    return KeyResolver::resolveSequence(keys);
}

std::vector<WORD> JsonConfigManager::getButtonKeys(int buttonIndex) const {
    auto it = m_buttonCache.find(buttonIndex);
    if (it != m_buttonCache.end()) {
        return it->second;
    }
    return {}; // Return empty vector if not found
}

std::vector<WORD> JsonConfigManager::getDpadKeys(const std::string& direction) const {
    auto it = m_dpadCache.find(direction);
    if (it != m_dpadCache.end()) {
        return it->second;
    }
    return {};
}

std::vector<WORD> JsonConfigManager::getStickKeys(const std::string& direction) const {
    auto it = m_stickCache.find(direction);
    if (it != m_stickCache.end()) {
        return it->second;
    }
    return {};
}
