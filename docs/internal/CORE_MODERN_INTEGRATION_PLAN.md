# Core/Modern実装統合 実装計画書

**計画作成日**: 2024年12月19日  
**実装期間**: 4-6週間  
**目標**: GamepadMapperを業界最先端のC++23プロジェクトに進化

## 🎯 統合概要

### 現状 → 目標
```cpp
// 現状: レガシーC++17スタイル
bool Initialize() {
    if (FAILED(DirectInput8Create(...))) {
        LOG_ERROR("Failed");
        return false;
    }
    return true;
}

// 目標: Modern C++23スタイル
auto Initialize() -> VoidResult {
    return CreateDirectInput()
        .and_then([=](auto) { return ConfigureDevice(); })
        .and_then([=](auto) { return LoadConfiguration(); })
        .transform([this](auto) { 
            m_initialized = true;
            LOG_INFO("Initialization complete");
        });
}
```

### 期待効果
- **コード削減**: 945行 → 450行（52%削減）
- **パフォーマンス**: 10-20%向上
- **保守性**: 大幅向上（型安全性、エラーハンドリング）
- **技術レベル**: 業界最先端C++23準拠

## 📋 Phase 1: Core基盤導入（Week 1-2）

### Week 1: Error Handling Modernization

#### Day 1-2: Core.h統合
```cpp
// 1. Application.h/cpp の更新
#include "core/Core.h"
using namespace gm::core;

class Application {
public:
    auto Initialize() -> VoidResult;  // bool → VoidResult
    auto InitializeLogger() -> VoidResult;
    auto InitializeWindow() -> VoidResult;
    auto InitializeGamepadManager() -> VoidResult;
};

// 2. 既存のbool戻り値を段階的にResult<>に変更
auto Application::Initialize() -> VoidResult {
    if (m_initialized) {
        return {};  // 成功
    }
    
    return InitializeLogger()
        .and_then([this](auto) { return InitializeWindow(); })
        .and_then([this](auto) { return InitializeGamepadManager(); })
        .transform([this](auto) {
            m_initialized = true;
            LOG_INFO("Application initialization completed");
        });
}
```

#### Day 3-4: GamepadManager現代化
```cpp
// GamepadManager.h/cpp の更新
#include "core/Core.h"

class GamepadManager {
public:
    auto Initialize(HINSTANCE hInst, HWND hWnd) -> VoidResult;
    auto CreateDirectInput(HINSTANCE hInst) -> VoidResult;
    auto ScanForDevices() -> Result<std::size_t>;
    
private:
    auto CleanupDisconnectedDevices() -> VoidResult;
    auto ProcessAllDevices() -> VoidResult;
};

// HRESULT → Result<> 変換の活用
auto GamepadManager::CreateDirectInput(HINSTANCE hInst) -> VoidResult {
    ComPtr<IDirectInput8> directInput;
    HRESULT hr = DirectInput8Create(hInst, DIRECTINPUT_VERSION, 
                                  IID_IDirectInput8, &directInput, nullptr);
    
    return from_hresult(hr, "DirectInput8Create")
        .transform([this, directInput](auto) {
            m_directInput = std::move(directInput);
            LOG_INFO("DirectInput8 created successfully");
        });
}
```

#### Day 5: GamepadDevice現代化
```cpp
// GamepadDevice.h/cpp のエラーハンドリング更新
class GamepadDevice {
public:
    auto Initialize(IDirectInput8* pDirectInput, 
                   const DIDEVICEINSTANCE* deviceInstance, 
                   HWND hWnd) -> VoidResult;
    auto ConfigureDevice(HWND hWnd) -> VoidResult;
    auto LoadConfiguration() -> VoidResult;
    auto AcquireDevice() -> VoidResult;
};

// モナディック操作チェーン
auto GamepadDevice::Initialize(...) -> VoidResult {
    return StoreDeviceInfo(*deviceInstance)
        .and_then([=](auto) { return CreateDevice(pDirectInput); })
        .and_then([=](auto) { return ConfigureDevice(hWnd); })
        .and_then([=](auto) { return LoadConfiguration(); })
        .and_then([=](auto) { return InitializeInputProcessor(); })
        .transform([this](auto) {
            m_initialized = true;
            LOG_INFO_W(L"Device initialized: {}", m_deviceName);
        });
}
```

### Week 2: RAII++とリソース管理

#### Day 6-7: RAII Chain導入
```cpp
// core/RAII.h の活用
auto GamepadDevice::ConfigureDevice(HWND hWnd) -> VoidResult {
    return RaiiChain{}
        .add([this] { 
            return from_hresult(
                m_device->SetDataFormat(&c_dfDIJoystick2),
                "SetDataFormat"
            );
        })
        .add([this, hWnd] {
            return from_hresult(
                m_device->SetCooperativeLevel(hWnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE),
                "SetCooperativeLevel"
            );
        })
        .add([this] { return SetAxisRanges(); })
        .finalize();
}
```

#### Day 8-10: パフォーマンス最適化マクロ
```cpp
// 既存コードにパフォーマンスヒント追加
auto GamepadDevice::PollAndGetState() -> bool {
    GM_FUNCTION_TIMER();  // 関数実行時間測定
    
    if (GM_UNLIKELY(!m_connected || !m_device)) {
        return false;
    }
    
    HRESULT hr = m_device->Poll();
    if (GM_LIKELY(SUCCEEDED(hr) || hr == DI_NOEFFECT)) {
        hr = m_device->GetDeviceState(sizeof(DIJOYSTATE2), &m_currentState);
        return GM_LIKELY(SUCCEEDED(hr));
    }
    
    return false;
}
```

## 📋 Phase 2: Ranges/Algorithms導入（Week 3-5）

### Week 3: デバイス処理の関数型化

#### Day 11-13: GamepadManager ranges化
```cpp
// 従来のループ → ranges pipeline
void GamepadManager::ProcessAllDevices() {
    // Before: 手動ループ
    for (size_t i = 0; i < m_devices.size(); ++i) {
        auto& device = m_devices[i];
        if (device && device->IsConnected()) {
            device->ProcessInput();
        }
    }
    
    // After: ranges pipeline
    auto connected_devices = m_devices 
        | std::views::filter([](const auto& device) { 
            return device && device->IsConnected(); 
        });
    
    for (auto& device : connected_devices) {
        if (auto result = device->ProcessInput(); !result) {
            LOG_WARN("Device processing failed: {}", result.error().what());
        }
    }
}

// デバイス削除もerase_ifで
auto GamepadManager::CleanupDisconnectedDevices() -> VoidResult {
    auto removed_count = std::erase_if(m_devices, 
        [](const auto& device) { 
            return !device || !device->IsConnected(); 
        });
    
    if (removed_count > 0) {
        LOG_INFO("Removed {} disconnected devices", removed_count);
    }
    
    return {};
}
```

#### Day 14-15: 入力処理パイプライン
```cpp
// InputProcessor の関数型化
class InputProcessor {
    auto ProcessGamepadInput(const DIJOYSTATE2& state) -> VoidResult {
        return ProcessButtons(state)
            .and_then([&](auto) { return ProcessDPad(state); })
            .and_then([&](auto) { return ProcessAnalogSticks(state); });
    }
    
private:
    auto ProcessButtons(const DIJOYSTATE2& state) -> VoidResult {
        // ボタン状態をrangesで処理
        auto button_states = std::views::iota(0, 32)
            | std::views::transform([&](int i) {
                return std::pair{i, (state.rgbButtons[i] & 0x80) != 0};
            })
            | std::views::filter([](const auto& pair) {
                return pair.second;  // 押されているボタンのみ
            });
        
        for (const auto& [index, pressed] : button_states) {
            if (auto result = ProcessSingleButton(index, pressed); !result) {
                LOG_WARN("Button {} processing failed: {}", index, result.error().what());
            }
        }
        
        return {};
    }
};
```

### Week 4-5: 設定・文字列処理の最適化

#### Day 16-18: JsonConfigManager ranges化
```cpp
// 設定ファイル処理の関数型化
class JsonConfigManager {
    auto GetButtonKeys(int buttonIndex) -> Result<std::vector<std::string>> {
        // JSON配列 → std::vector の変換をrangesで
        if (auto button_array = FindButtonArray(buttonIndex)) {
            return button_array->get<std::vector<std::string>>()
                | std::views::filter([](const std::string& key) {
                    return !key.empty() && IsValidKeyName(key);
                })
                | std::ranges::to<std::vector>();
        }
        
        return std::unexpected(config_error("Button not found"));
    }
    
private:
    auto LoadConfigurationFile(const std::string& path) -> VoidResult {
        return ReadFileToString(path)
            .and_then([](const std::string& content) {
                return ParseJsonContent(content);
            })
            .and_then([this](const nlohmann::json& json) {
                return ValidateConfigStructure(json);
            })
            .transform([this](const nlohmann::json& json) {
                m_config = json;
                LOG_INFO("Configuration loaded successfully");
            });
    }
};
```

#### Day 19-21: 文字列処理最適化
```cpp
// KeyResolver の文字列処理をranges/formatで最適化
class KeyResolver {
    auto FormatKeySequence(const std::vector<WORD>& keys) -> std::string {
        // Before: 手動ループ
        std::string result;
        for (size_t i = 0; i < keys.size(); ++i) {
            if (i > 0) result += "+";
            result += std::format("0x{:02X}", keys[i]);
        }
        
        // After: ranges pipeline
        return keys 
            | std::views::transform([](WORD key) { 
                return std::format("0x{:02X}", key); 
            })
            | std::views::join_with(std::string_view{"+"})
            | std::ranges::to<std::string>();
    }
};
```

## 📋 Phase 3: Modern実装統合（Week 6-9）

### Week 6: ModernGamepadManager統合

#### Day 22-24: エイリアス・Feature Flag導入
```cpp
// Application.h でのエイリアス作成
#ifdef USE_MODERN_GAMEPAD_MANAGER
    #include "modern/ModernGamepadManager.h"
    using GamepadManagerImpl = gm::modern::ModernGamepadManager;
#else
    #include "GamepadManager.h"
    using GamepadManagerImpl = GamepadManager;
#endif

class Application {
private:
    std::unique_ptr<GamepadManagerImpl> m_gamepadManager;
};

// CMakeLists.txt でのフラグ設定
option(USE_MODERN_IMPLEMENTATION "Use modern C++23 implementation" OFF)
if(USE_MODERN_IMPLEMENTATION)
    target_compile_definitions(GamepadMapper PRIVATE USE_MODERN_GAMEPAD_MANAGER)
endif()
```

#### Day 25-26: API互換性確保
```cpp
// ModernGamepadManager のレガシーAPI対応
namespace gm::modern {
    class ModernGamepadManager {
    public:
        // Modern API
        auto Initialize(HINSTANCE hInst, HWND hWnd) -> VoidResult;
        auto ScanForDevices() -> Result<std::size_t>;
        
        // Legacy compatibility
        bool Initialize_Legacy(HINSTANCE hInst, HWND hWnd) {
            auto result = Initialize(hInst, hWnd);
            if (!result) {
                LOG_ERROR("Initialization failed: {}", result.error().what());
            }
            return result.has_value();
        }
    };
}
```

### Week 7: ModernInputProcessor統合

#### Day 27-29: 関数型パイプライン導入
```cpp
// ModernInputProcessor の段階的統合
#ifdef USE_MODERN_INPUT_PROCESSOR
    using InputProcessorImpl = gm::modern::ModernInputProcessor;
#else
    using InputProcessorImpl = InputProcessor;
#endif

// Modern実装の高度な機能
namespace gm::modern {
    class ModernInputProcessor {
        auto ProcessGamepadInput(const DIJOYSTATE2& state) -> VoidResult {
            return ProcessButtonsWithPipeline(state)
                .and_then([&](auto) { return ProcessAxesWithZip(state); })
                .and_then([&](auto) { return ProcessPOVWithRanges(state); });
        }
        
    private:
        auto ProcessButtonsWithPipeline(const DIJOYSTATE2& state) -> VoidResult {
            auto button_events = std::views::iota(0, 32)
                | std::views::transform([&](int i) { 
                    return ButtonEvent{i, IsPressed(state.rgbButtons[i])}; 
                })
                | std::views::filter(&ButtonEvent::is_pressed)
                | std::views::transform([this](const ButtonEvent& event) {
                    return ProcessButtonEvent(event);
                });
            
            // すべてのイベントを処理
            for (const auto& result : button_events) {
                if (!result) {
                    LOG_WARN("Button processing failed: {}", result.error().what());
                }
            }
            
            return {};
        }
    };
}
```

### Week 8-9: ModernGamepadDevice統合

#### Day 30-33: CRTP最適化導入
```cpp
// ModernGamepadDevice の完全統合
namespace gm::modern {
    template<typename Config = DefaultDeviceConfig>
    class ModernGamepadDevice {
        using StateType = DIJOYSTATE2;
        using ConfigManager = ConfiguredJsonManager<Config>;
        using InputProcessor = OptimizedInputProcessor<Config>;
        
    public:
        auto Initialize(IDirectInput8* pDirectInput, 
                       const DIDEVICEINSTANCE* deviceInstance, 
                       HWND hWnd) -> VoidResult {
            GM_FUNCTION_TIMER();
            
            return StoreDeviceInfo(*deviceInstance)
                .and_then([=](auto) { return CreateDevice(pDirectInput, deviceInstance->guidInstance); })
                .and_then([=](auto) { return ConfigureDevice(hWnd); })
                .and_then([=](auto) { return LoadConfiguration(); })
                .and_then([=](auto) { return InitializeInputProcessor(); })
                .transform([this](auto) {
                    m_initialized = true;
                    LOG_INFO_W(L"Device initialized: {}", m_deviceName);
                });
        }
        
    private:
        auto ProcessInputWithOptimizations() -> VoidResult {
            if constexpr (Config::enable_batch_processing) {
                return ProcessInputBatched();
            } else {
                return ProcessInputSequential();
            }
        }
    };
}
```

#### Day 34-36: 最終統合・テスト
```cpp
// 最終的なApplication統合
class Application {
    auto InitializeGamepadManager() -> VoidResult {
        GM_FUNCTION_TIMER();
        
        #ifdef USE_MODERN_IMPLEMENTATION
            LOG_INFO("Using Modern C++23 implementation");
            auto manager = std::make_unique<gm::modern::ModernGamepadManager>();
        #else
            LOG_INFO("Using legacy implementation");
            auto manager = std::make_unique<GamepadManager>();
        #endif
        
        return manager->Initialize(m_hInstance, m_windowManager->GetHwnd())
            .transform([this, manager = std::move(manager)](auto) mutable {
                m_gamepadManager = std::move(manager);
                LOG_INFO("Gamepad manager initialized successfully");
            });
    }
};
```

## 📋 Phase 4: 最終最適化（Week 10）

### Day 37-40: パフォーマンス検証・調整

#### パフォーマンステスト
```cpp
// ベンチマーク実装
class PerformanceBenchmark {
    auto BenchmarkDeviceProcessing() {
        constexpr int iterations = 10000;
        
        auto legacy_time = MeasureProcessingTime<GamepadManager>(iterations);
        auto modern_time = MeasureProcessingTime<gm::modern::ModernGamepadManager>(iterations);
        
        auto improvement = (legacy_time - modern_time) / legacy_time * 100.0;
        
        LOG_INFO("Performance improvement: {:.2f}%", improvement);
        LOG_INFO("Legacy: {:.2f}ms, Modern: {:.2f}ms", 
                legacy_time.count(), modern_time.count());
    }
    
private:
    template<typename ManagerType>
    auto MeasureProcessingTime(int iterations) -> std::chrono::nanoseconds {
        // 実際の処理時間測定実装
    }
};
```

#### メモリ使用量最適化
```cpp
// Memory pooling 活用
namespace gm::core {
    class DeviceMemoryPool {
        static auto AllocateDevice() -> std::unique_ptr<ModernGamepadDevice> {
            return memory::allocate_from_pool<ModernGamepadDevice>();
        }
        
        static void DeallocateDevice(std::unique_ptr<ModernGamepadDevice> device) {
            memory::return_to_pool(std::move(device));
        }
    };
}
```

## 🎯 成功指標・検証方法

### 定量的指標
```cpp
// 自動化されたメトリクス収集
class IntegrationMetrics {
    struct Metrics {
        std::size_t code_lines_before;
        std::size_t code_lines_after;
        std::chrono::nanoseconds processing_time_before;
        std::chrono::nanoseconds processing_time_after;
        std::size_t memory_usage_before;
        std::size_t memory_usage_after;
    };
    
    auto GenerateReport() -> Metrics {
        // 統合前後の詳細比較レポート生成
    }
};
```

### 品質検証
- **単体テスト**: 各Phase完了時
- **統合テスト**: 週次実行
- **パフォーマンステスト**: Phase完了時
- **メモリリークテスト**: 継続実行

## 🏁 統合完了後の状態

### 技術的成果
- **C++23完全準拠**: 最新標準の実戦活用
- **52%コード削減**: 高い保守性・可読性
- **10-20%性能向上**: ゼロコスト抽象化の恩恵
- **型安全性**: Conceptsによる制約

### プロジェクト価値
- **業界最先端**: C++23機能の先進的活用
- **技術的負債ゼロ**: 完全に現代化されたコードベース
- **拡張性**: 将来機能追加が容易
- **教育価値**: 他プロジェクトの模範

---

**この計画により、GamepadMapperは技術的に世界トップクラスのC++プロジェクトに進化します。**