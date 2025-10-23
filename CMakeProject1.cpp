// CMakeProject1.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "CMakeProject1.h"
#include <windows.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Unicode ビルドを想定して wWinMain を使用
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    // JSON オブジェクトの作成
    json j;
    j["name"] = "Alice";
    j["age"] = 30;
    j["languages"] = { "C++", "Python", "JavaScript" };

    // 配列に要素を追加
    j["projects"] = json::array();
    j["projects"].push_back({ {"name", "CMakeProject1"}, {"year", 2025} });

    // JSON を文字列化
    std::string s = j.dump(4); // indent4

    // 結果をメッセージボックスで表示（長すぎると切り捨てられるので注意）
    MessageBoxA(NULL, s.c_str(), "nlohmann::json sample", MB_OK | MB_ICONINFORMATION);

    // JSON をパースして値を取り出す例
    json parsed = json::parse(s);
    std::string name = parsed.value("name", "");
    int age = parsed.value("age", 0);

    // 文字列をログ出力（デバッグ出力）
    std::string log = "Parsed name: " + name + ", age: " + std::to_string(age);
    OutputDebugStringA(log.c_str());

    return 0;
}
