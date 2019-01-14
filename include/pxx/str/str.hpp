//
// Created by garrick on 19-1-11.
//

#ifndef PXX11_STRING_HPP
#define PXX11_STRING_HPP

#include "str_impl.hpp"

namespace pxx {
    namespace str {
        std::vector<std::string> split(const std::string s, char delim) {
            return fmt::split(s, delim);
        }
        std::vector<std::string> split(const std::string& s, const std::string& delim="") {
            if (delim.empty())
                return fmt::split_whitespace(s);
            else if (delim.length() == 1)
                return fmt::split(s, delim);
            else
                return fmt::split_multi_delims(s, delim);
        }
        std::string join(const std::vector<std::string>& strs, char delim) {
            return fmt::join(strs, delim);
        }
        std::string join(const std::vector<std::string>& strs, const std::string& delim="") {
            return fmt::join(strs, delim);
        }
        std::string replace(const std::string& str, char oldc, char newc) {
            std::string ns(str);
            return fmt::replace_all(ns, oldc, newc);
        }
        std::string replace(const std::string& str, const std::string& olds, const std::string& news) {
            std::string ns(str);
            return fmt::replace_all(ns, olds, news);
        }
        std::string lstrip(std::string str, std::string what="") {
            std::string ns(str);
            if (what.empty()) {
                what = " \t\n\r\f\v";
            }
            ns.erase(0, ns.find_first_not_of(what.c_str()));
            return ns;
        }
        std::string rstrip(std::string str, std::string what="") {
            std::string ns(str);
            if (what.empty()) {
                what = " \t\n\r\f\v";
            }
            ns.erase(ns.find_last_not_of(what.c_str()) + 1);
            return ns;
        }
        std::string strip(std::string str, std::string what="") {
            return lstrip(rstrip(str, what), what);
        }
    } // namespace str
} // namespace pxx

#endif //PXX11_STRING_HPP
