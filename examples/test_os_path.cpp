//
// Created by garrick on 19-1-11.
//

#include <iostream>
#include "pxx/os/os.hpp"

void print_exist(const std::string& path) {
    std::cout << "Path " << path << " exist: " << pxx::os::path::exists(path) << std::endl;
}

void print_abspath(const std::string& path) {
    std::cout << "Path " << path << " Abspath: " << pxx::os::path::abspath(path) << std::endl;
}

void dump_input_output(const std::string& input, const std::string& output) {
    std::cout << "Input " << input << " Output " << output << std::endl;
}

void dump_input_output(const std::string& input, bool output) {
    std::cout << "Input " << input << " Output " << output << std::endl;
}

void dump_input_output(const std::string& input, const std::vector<std::string>& output) {
    std::cout << "Input " << input << " Output ";
    for(const auto& o : output) {
        std::cout << o << " ";
    }
    std::cout << std::endl;
}

void test_os_path_exist() {
    print_exist("/home");
    print_exist("/home/garrick/.bashrc");
    print_exist("/home/garrick/nothing");
    print_exist("test_os_path");
}

void test_os_path_abspath() {
    print_abspath("test_os_path");
    print_abspath("nothing");
}

void test_os_path_basename() {
    dump_input_output("/home/none", pxx::os::path::basename("/home/none"));
    dump_input_output("/home", pxx::os::path::basename("/home"));
    dump_input_output("a.jpg", pxx::os::path::basename("a.jpg"));
    dump_input_output("~/a.jpg", pxx::os::path::basename("~/a.jpg"));
//    dump_input_output("", pxx::os::path::basename(""));
}

void test_os_path_dirname() {
    dump_input_output("/home/none", pxx::os::path::dirname("/home/none"));
    dump_input_output("/home", pxx::os::path::dirname("/home"));
    dump_input_output("/", pxx::os::path::dirname("/"));
    dump_input_output("a.jpg", pxx::os::path::dirname("a.jpg"));
    dump_input_output("~/a.jpg", pxx::os::path::dirname("~/a.jpg"));
}

void test_os_path_isfile() {
    dump_input_output("a.jpg", pxx::os::path::isfile("a.jpg"));
    dump_input_output("test_os_path", pxx::os::path::isfile("test_os_path"));
}

void test_os_path_isdir() {
    dump_input_output("/home", pxx::os::path::isdir("/home"));
    dump_input_output("/home/ee", pxx::os::path::isdir("/home/ee"));
    dump_input_output("test_os_path", pxx::os::path::isdir("test_os_path"));
}

void test_os_path_split() {
    dump_input_output("/home/ee", pxx::os::path::split("/home/ee"));
    dump_input_output("/home", pxx::os::path::split("/home"));
    dump_input_output("/home/ee/cc/dd/aa/bb", pxx::os::path::split("/home/ee/cc/dd/aa/bb"));
}

void test_os_path_join() {
    dump_input_output("None", pxx::os::path::join({"/home/", "bb"}));
    dump_input_output("None", pxx::os::path::join({"/home", "bb"}));
    dump_input_output("None", pxx::os::path::join({"home", "bb", "aa"}));
}

int main() {
    test_os_path_exist();
    test_os_path_abspath();
    test_os_path_basename();
    test_os_path_dirname();
    test_os_path_isfile();
    test_os_path_isdir();
    test_os_path_split();
    test_os_path_join();
}