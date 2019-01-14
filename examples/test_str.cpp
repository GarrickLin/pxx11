//
// Created by garrick on 19-1-11.
//

#include <iostream>
#include "pxx/str/str.hpp"

namespace test {
    void test() {
        fmt::split("1.2.3", ".");
    }
}

template <typename T>
void print_vector(const std::vector<T>& vec) {
    for(const auto& v : vec) {
        std::cout << v << " ";
    }
    std::cout << std::endl;
}

void test_str_split() {
    print_vector(pxx::str::split("1\t2\t3\t5"));
    print_vector(pxx::str::split("123", "2"));
    print_vector(pxx::str::split("123", '2'));
    print_vector(pxx::str::split("1234567890", "2468"));
}

void test_str_join() {
    std::cout << pxx::str::join({"1","2","3"}, "+") << std::endl;
    std::cout << pxx::str::join({"1","2","3"}, '-') << std::endl;
    std::cout << pxx::str::join({"1","2","3"}, "xxx") << std::endl;
    std::cout << pxx::str::join({"1","2","3"}) << std::endl;
}

void test_str_replace() {
    std::cout << pxx::str::replace("12345678", '5', '9') << std::endl;
    std::cout << pxx::str::replace("12345678", "1234", "4321") << std::endl;
}

void test_str_strip() {
    std::cout << pxx::str::strip(" 12345678 \t\n\r") << std::endl;
    std::cout << pxx::str::strip("AABBCCAAA", "A") << std::endl;
    std::cout << pxx::str::rstrip("AABBCCAAA", "A") << std::endl;
    std::cout << pxx::str::lstrip("AABBCCAAA", "A") << std::endl;
}

int main() {
    test_str_split();
    test_str_join();
    test_str_replace();
    test_str_strip();
    return 0;
}