#include "memory/unique_ptr.h"
#include <gtest/gtest.h>
#include <string>
#include <type_traits>

using namespace svr;

struct Dummy {
    int x;
    Dummy(int v = 0) : x(v) {}
    ~Dummy() { x = -1; }
};

struct CustomDeleter {
    int* flag;
    CustomDeleter(int* f = nullptr) : flag(f) {}
    void operator()(Dummy* p) const { if (flag) *flag = 1; delete p; }
    void operator()(Dummy* p) { if (flag) *flag = 2; delete p; }
};

TEST(UniquePtrTest, DefaultConstruct) {
    unique_ptr<int> p;
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_FALSE(p);
}

TEST(UniquePtrTest, NullptrConstruct) {
    unique_ptr<int> p(nullptr);
    EXPECT_EQ(p.get(), nullptr);
}

TEST(UniquePtrTest, RawPointerConstruct) {
    unique_ptr<int> p(new int(42));
    EXPECT_TRUE(p);
    EXPECT_EQ(*p, 42);
}

TEST(UniquePtrTest, MoveConstruct) {
    unique_ptr<int> p1(new int(5));
    unique_ptr<int> p2(std::move(p1));
    EXPECT_FALSE(p1);
    EXPECT_TRUE(p2);
    EXPECT_EQ(*p2, 5);
}

TEST(UniquePtrTest, MoveAssign) {
    unique_ptr<int> p1(new int(7));
    unique_ptr<int> p2;
    p2 = std::move(p1);
    EXPECT_FALSE(p1);
    EXPECT_TRUE(p2);
    EXPECT_EQ(*p2, 7);
}

TEST(UniquePtrTest, Reset) {
    unique_ptr<int> p(new int(10));
    p.reset(new int(20));
    EXPECT_EQ(*p, 20);
    p.reset(nullptr);
    EXPECT_FALSE(p);
}

TEST(UniquePtrTest, Release) {
    unique_ptr<int> p(new int(30));
    int* raw = p.release();
    EXPECT_EQ(p.get(), nullptr);
    EXPECT_EQ(*raw, 30);
    delete raw;
}

TEST(UniquePtrTest, Swap) {
    unique_ptr<int> p1(new int(1));
    unique_ptr<int> p2(new int(2));
    p1.swap(p2);
    EXPECT_EQ(*p1, 2);
    EXPECT_EQ(*p2, 1);
}

TEST(UniquePtrTest, BoolConversion) {
    unique_ptr<int> p(new int(1));
    EXPECT_TRUE(p);
    p.reset();
    EXPECT_FALSE(p);
}

TEST(UniquePtrTest, CustomDeleter) {
    int flag = 0;
    {
        unique_ptr<Dummy, CustomDeleter> p(new Dummy(123));
        p.get_deleter() = CustomDeleter(&flag);
    }
    EXPECT_NE(flag, 0);
}

TEST(UniquePtrTest, GetDeleterConst) {
    unique_ptr<Dummy, CustomDeleter> p(new Dummy(1));
    const auto& d = static_cast<const decltype(p)&>(p).get_deleter();
    (void)d;
}

TEST(UniquePtrTest, ArrayBasic) {
    unique_ptr<int[]> arr(new int[3]{1,2,3});
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 3);
}

TEST(UniquePtrTest, ArrayMove) {
    unique_ptr<int[]> arr1(new int[2]{4,5});
    unique_ptr<int[]> arr2(std::move(arr1));
    EXPECT_FALSE(arr1);
    EXPECT_EQ(arr2[0], 4);
    EXPECT_EQ(arr2[1], 5);
}

TEST(UniquePtrTest, ArrayMoveAssign) {
    unique_ptr<int[]> arr1(new int[2]{6,7});
    unique_ptr<int[]> arr2;
    arr2 = std::move(arr1);
    EXPECT_FALSE(arr1);
    EXPECT_EQ(arr2[0], 6);
    EXPECT_EQ(arr2[1], 7);
}

TEST(UniquePtrTest, ArrayResetRelease) {
    unique_ptr<int[]> arr(new int[2]{8,9});
    int* raw = arr.release();
    EXPECT_EQ(arr.get(), nullptr);
    EXPECT_EQ(raw[0], 8);
    EXPECT_EQ(raw[1], 9);
    delete[] raw;
}

TEST(UniquePtrTest, ArraySwap) {
    unique_ptr<int[]> arr1(new int[2]{10,11});
    unique_ptr<int[]> arr2(new int[2]{12,13});
    arr1.swap(arr2);
    EXPECT_EQ(arr1[0], 12);
    EXPECT_EQ(arr2[1], 11);
}

TEST(UniquePtrTest, ArrayBoolConversion) {
    unique_ptr<int[]> arr(new int[1]{42});
    EXPECT_TRUE(arr);
    arr.reset();
    EXPECT_FALSE(arr);
}

TEST(UniquePtrTest, MakeUniqueSingle) {
    auto p = make_unique<int>(123);
    EXPECT_EQ(*p, 123);
}

TEST(UniquePtrTest, MakeUniqueArray) {
    auto arr = make_unique<int[]>(3);
    arr[0] = 1; arr[1] = 2; arr[2] = 3;
    EXPECT_EQ(arr[0], 1);
    EXPECT_EQ(arr[1], 2);
    EXPECT_EQ(arr[2], 3);
}

TEST(UniquePtrTest, UniquePtrIsNoexceptMoveConstructible) {
    EXPECT_TRUE((std::is_nothrow_move_constructible<unique_ptr<int>>::value));
    EXPECT_TRUE((std::is_nothrow_move_assignable<unique_ptr<int>>::value));
}

TEST(UniquePtrTest, UniquePtrArrayIsNoexceptMoveConstructible) {
    EXPECT_TRUE((std::is_nothrow_move_constructible<unique_ptr<int[]>>::value));
    EXPECT_TRUE((std::is_nothrow_move_assignable<unique_ptr<int[]>>::value));
}
