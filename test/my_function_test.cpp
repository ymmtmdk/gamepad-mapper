// #include "gtest/gtest.h"
#include <windows.h>
#include <stdio.h>
// テスト対象の関数を宣言またはインクルード
// この例では、同じソースファイル内で定義されていると仮定
/*
int add(int a, int b);

// TESTマクロを使ってテストケースを定義する
// TEST(テストスイート名, テスト名)
TEST(AdditionTest, PositiveNumbers) {
    // ASSERT_EQは、期待値と実際の値が等しいことをアサートするマクロ
    // 失敗した場合はテストがそこで終了する（致命的）
    ASSERT_EQ(add(2, 3), 5);
}

TEST(AdditionTest, NegativeNumbers) {
    // EXPECT_EQは、失敗してもテストは続行する（非致命的）
    EXPECT_EQ(add(-1, -1), -2);
}
*/
// テスト実行のためのメイン関数（gtest_mainライブラリを使う場合は不要）
// vcpkgを使用している場合は通常gtest_mainが自動的にリンクされる
int main(int argc, char** argv) {
    //::testing::InitGoogleTest(&argc, argv);
    //return RUN_ALL_TESTS();
	printf("Hello, World!\n");
	return 0;

}

