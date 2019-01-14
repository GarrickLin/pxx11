//
// Created by garrick on 19-1-11.
//

#ifndef PXX11_PATH_IMPL_HPP
#define PXX11_PATH_IMPL_HPP

#include "pxx/config/platform.h"
#include "pxx/str/str_impl.hpp"

namespace path_impl {
    bool path_exists(std::string path, bool considerFile)
    {
#if PXX_OS_WINDOWS
        DWORD fileType = GetFileAttributesW(utf8_to_wstring(path).c_str());
			if (fileType == INVALID_FILE_ATTRIBUTES) {
				return false;
			}
			return considerFile ? true : ((fileType & FILE_ATTRIBUTE_DIRECTORY) == 0 ? false : true);
#elif PXX_OS_UNIX
        struct stat st;
			int ret = stat(path.c_str(), &st);
			return considerFile ? (ret == 0) : S_ISDIR(st.st_mode);
#endif
        return false;
    }

    std::string os_path_join(std::vector<std::string> elems, char delim)
    {
        std::string str;
        elems = fmt::erase_empty(elems);
        if (elems.empty()) return str;
        str = elems[0];
        for (std::size_t i = 1; i < elems.size(); ++i)
        {
            if (elems[i].empty()) continue;
            if (str[str.length()-1] == delim)
                str += elems[i];
            else
                str += delim + elems[i];
        }
        return str;
    }

    std::string path_join(std::vector<std::string> elems)
    {
#if PXX_OS_WINDOWS
        return os_path_join(elems, '\\');
#else
        return os_path_join(elems, '/');
#endif
    }

    std::string path_join(const std::string& a, const std::string& b) {
        path_join({a, b});
    }

    std::vector<std::string> path_split(std::string path)
    {
#if PXX_OS_WINDOWS
        std::replace(path.begin(), path.end(), '\\', '/');
        return fmt::split(path, '/');
#else
        return fmt::split(path, '/');
#endif
    }

    std::string current_working_directory()
    {
#if PXX_OS_WINDOWS
        wchar_t *buffer = nullptr;
			if ((buffer = _wgetcwd(nullptr, 0)) == nullptr)
			{
				// failed
				log::detail::zupply_internal_warn("Unable to get current working directory!");
				return std::string(".");
			}
			else
			{
				std::wstring ret(buffer);
				free(buffer);
				return wstring_to_utf8(ret);
			}
#elif _GNU_SOURCE
        char *buffer = realpath(".", nullptr);
        if (buffer == nullptr)
        {
            // failed
            std::cerr << "Unable to get current working directory!\n";
            return std::string(".");
        }
        else
        {
            // success
            std::string ret(buffer);
            free(buffer);
            return ret;
        }
#else
        char *buffer = getcwd(nullptr, 0);
			if (buffer == nullptr)
			{
				// failed
				std::cerr << "Unable to get current working directory!\n";
				return std::string(".");
			}
			else
			{
				// success
				std::string ret(buffer);
				free(buffer);
				return ret;
			}
#endif
    }

    std::string absolute_path(std::string reletivePath)
    {
#if PXX_OS_WINDOWS
        wchar_t *buffer = nullptr;
			std::wstring widePath = utf8_to_wstring(reletivePath);
			buffer = _wfullpath(buffer, widePath.c_str(), _MAX_PATH);
			if (buffer == nullptr)
			{
				// failed
				std::cerr << "Unable to get absolute path for: " + reletivePath + "! Return original.\n";
				return reletivePath;
			}
			else
			{
				std::wstring ret(buffer);
				free(buffer);
				return wstring_to_utf8(ret);
			}
#elif PXX_OS_UNIX
        char *buffer = realpath(reletivePath.c_str(), nullptr);
			if (buffer == nullptr)
			{
				// failed
				if (ENOENT == errno)
				{
					// try recover manually
					std::string dirtyPath;
					if (fmt::starts_with(reletivePath, "/"))
					{
						// already an absolute path
						dirtyPath = reletivePath;
					}
					else
					{
						dirtyPath = path_join({ current_working_directory(), reletivePath });
					}
					std::vector<std::string> parts = path_split(dirtyPath);
					std::vector<std::string> ret;
					for (auto i = parts.begin(); i != parts.end(); ++i)
					{
						if (*i == ".") continue;
						if (*i == "..")
						{
							if (ret.size() < 1) throw std::runtime_error("Invalid path: " + dirtyPath);
							ret.pop_back();
						}
						else
						{
							ret.push_back(*i);
						}
					}
					std::string tmp = path_join(ret);
					if (!fmt::starts_with(tmp, "/")) tmp = "/" + tmp;
					return tmp;
				}
				//still failed
				std::cerr << "Unable to get absolute path for: " + reletivePath + "! Return original.";
				return reletivePath;
			}
			else
			{
				std::string ret(buffer);
				free(buffer);
				return ret;
			}
#endif
        return std::string();
    }

    std::string path_split_filename(std::string path)
    {
#if PXX_OS_WINDOWS
        std::string::size_type pos = fmt::trim(path).find_last_of("/\\");
#else
        std::string::size_type pos = fmt::trim(path).find_last_of("/");
#endif
        if (pos == std::string::npos) return path;
        if (pos != path.length())
        {
            return path.substr(pos + 1);
        }
        return std::string();
    }

    std::string path_split_directory(std::string path)
    {
#if PXX_OS_WINDOWS
        std::string::size_type pos = fmt::trim(path).find_last_of("/\\");
#else
        std::string::size_type pos = fmt::trim(path).find_last_of("/");
#endif
        if (pos != std::string::npos)
        {
            if (pos==0 && path[0]=='/')
                return "/";
            return path.substr(0, pos);
        }
        return std::string();
    }

    std::vector<std::string> path_split_directory_filename(std::string path)
    {
#if PXX_OS_WINDOWS
        std::string::size_type pos = fmt::trim(path).find_last_of("/\\");
#else
        std::string::size_type pos = fmt::trim(path).find_last_of("/");
#endif
        if (pos != std::string::npos)
        {
            if (pos==0 && path[0]=='/')
                return {"/", path.substr(pos+1)};
            return {path.substr(0, pos), path.substr(pos+1)};
        }
        return {};
    }

    bool is_file(std::string path)
    {
#if PXX_OS_WINDOWS
        DWORD fileType = GetFileAttributesW(utf8_to_wstring(path).c_str());
			if (fileType == INVALID_FILE_ATTRIBUTES || ((fileType & FILE_ATTRIBUTE_DIRECTORY) != 0))
			{
				return false;
			}
			return true;
#elif PXX_OS_UNIX
        struct stat st;
			if (stat(path.c_str(), &st) != 0) return false;
			if (S_ISDIR(st.st_mode)) return false;
			return true;
#endif
        return false;
    }

    bool is_directory(std::string path)
    {
#if PXX_OS_WINDOWS
        DWORD fileType = GetFileAttributesW(utf8_to_wstring(path).c_str());
			if (fileType == INVALID_FILE_ATTRIBUTES) return false;
			if ((fileType & FILE_ATTRIBUTE_DIRECTORY) != 0) return true;
			return false;
#elif PXX_OS_UNIX
        struct stat st;
			if (stat(path.c_str(), &st) != 0) return false;
			if (S_ISDIR(st.st_mode)) return true;
			return false;
#endif
        return false;
    }
} // namespace path_impl

#endif //PXX11_PATH_IMPL_HPP
