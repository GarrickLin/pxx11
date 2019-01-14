//
// Created by garrick on 19-1-11.
//

#ifndef PXX11_PATH_HPP
#define PXX11_PATH_HPP

#include "pxx/os/path/path_impl.hpp"
#include <sstream>

namespace pxx {
    namespace os {
        namespace path {
            bool exists (const std::string& path) {
                return path_impl::path_exists(path, true);
            }
            std::string abspath(const std::string& path) {
                return path_impl::absolute_path(path);
            }
            std::string basename(const std::string& path) {
                return path_impl::path_split_filename(path);
            }
            std::string dirname(const std::string& path) {
                return path_impl::path_split_directory(path);
            }
            bool isfile(const std::string& path) {
                return path_impl::is_file(path);
            }
            bool isdir(const std::string& path) {
                return path_impl::is_directory(path);
            }
            std::string join(const std::vector<std::string>& paths) {
                return path_impl::path_join(paths);
            }
            std::vector<std::string> split(const std::string& path) {
                return path_impl::path_split_directory_filename(path);
            }
        }
    }
}

#endif //PXX11_PATH_HPP
