#include <iostream>
#include "iter_range.hpp"

void test_range(void) {
    std::cout << "range(15):";
    for (int i : range(15)) {
        std::cout << " " << i;
    }
    std::cout << std::endl;

    std::cout << "range('a', 'z'):";
    for (auto i : range('a', 'z')) {
        std::cout << " " << i;
    }
    std::cout << std::endl; 

    std::cout << "range('a', static_cast<char>('z'+1)):";
    for (auto i : range('a', static_cast<char>('z'+1))) {
        std::cout << " " << i;
    }
    std::cout << std::endl;    

    std::cout << "range(20, 10, -1):";
    for (auto i : range(20, 10, -1)) {
        std::cout << " " << i;
    }
    std::cout << std::endl;    
}

int main(void) {
    test_range();
    return 0;
}