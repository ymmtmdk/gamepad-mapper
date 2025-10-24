#include "KeyResolver.h"
#include <windows.h>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <algorithm> // For std::transform
#include <cctype>    // For std::tolower
#include <stdexcept> // For std::invalid_argument, std::out_of_range

// Initialize static KEY_MAP by calling createKeyMap()
const std::unordered_map<std::string, WORD> KeyResolver::KEY_MAP = KeyResolver::createKeyMap();

std::optional<WORD> KeyResolver::resolve(const std::string& keyName) {
    std::string lowerKeyName = keyName;
    std::transform(lowerKeyName.begin(), lowerKeyName.end(), lowerKeyName.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    auto it = KEY_MAP.find(lowerKeyName);
    if (it != KEY_MAP.end()) {
        return it->second;
    }
    return parseNumericKey(keyName);
}

std::vector<WORD> KeyResolver::resolveSequence(const std::vector<std::string>& keys) {
    std::vector<WORD> sequence;
    for (const auto& key : keys) {
        if (auto resolvedKey = resolve(key)) {
            sequence.push_back(resolvedKey.value());
        }
    }
    return sequence;
}

std::optional<WORD> KeyResolver::parseNumericKey(const std::string& keyName) {
    try {
        size_t pos;
        long long_val;
        if (keyName.size() > 2 && (keyName.substr(0, 2) == "0x" || keyName.substr(0, 2) == "0X")) {
            long_val = static_cast<long>(std::stoll(keyName, &pos, 16));
        } else {
            long_val = static_cast<long>(std::stoll(keyName, &pos, 10));
        }

        if (pos == keyName.size() && long_val >= 0 && long_val <= 0xFFFF) {
            return static_cast<WORD>(long_val);
        }
    } catch (const std::invalid_argument&) {
        // Not a number, which is fine
    } catch (const std::out_of_range&) {
        // Number is out of range for long, which is also fine
    }
    return std::nullopt;
}


std::unordered_map<std::string, WORD> KeyResolver::createKeyMap() {
    std::unordered_map<std::string, WORD> map;

    // Alphanumeric keys
    for (char c = 'a'; c <= 'z'; ++c) {
        map[std::string(1, c)] = toupper(c);
    }
    for (char c = '0'; c <= '9'; ++c) {
        map[std::string(1, c)] = c;
    }

    // Special keys
    map["space"] = VK_SPACE;
    map["enter"] = VK_RETURN;
    map["escape"] = VK_ESCAPE;
    map["tab"] = VK_TAB;
    map["backspace"] = VK_BACK;
    map["delete"] = VK_DELETE;

    // Modifier keys
    map["ctrl"] = VK_CONTROL;
    map["alt"] = VK_MENU;
    map["shift"] = VK_SHIFT;
    map["win"] = VK_LWIN;
    map["lwin"] = VK_LWIN;
    map["rwin"] = VK_RWIN;

    // Function keys
    for (int i = 1; i <= 12; ++i) {
        map["f" + std::to_string(i)] = VK_F1 + (i - 1);
    }

    // Arrow keys
    map["up"] = VK_UP;
    map["down"] = VK_DOWN;
    map["left"] = VK_LEFT;
    map["right"] = VK_RIGHT;

    // PrintScreen
    map["printscreen"] = VK_SNAPSHOT;
    map["prtsc"] = VK_SNAPSHOT;

    return map;
}
