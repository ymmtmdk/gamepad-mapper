#pragma once

#include <string>
#include <vector>
#include <optional>
#include <windows.h>
#include <unordered_map>

class KeyResolver {
public:
    static std::optional<WORD> resolve(const std::string& keyName);
    static std::vector<WORD> resolveSequence(const std::vector<std::string>& keys);
private:
    static const std::unordered_map<std::string, WORD> KEY_MAP;
    static std::unordered_map<std::string, WORD> createKeyMap();
    static std::optional<WORD> parseNumericKey(const std::string& keyName);
};
