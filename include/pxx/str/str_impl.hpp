//
// Created by garrick on 19-1-11.
//

#ifndef PXX11_STRING_IMPL_HPP
#define PXX11_STRING_IMPL_HPP

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace thirdparty {
    namespace utf8 {
// The typedefs for 8-bit, 16-bit and 32-bit unsigned integers
        // You may need to change them to match your system.
        // These typedefs have the same names as ones from cstdint, or boost/cstdint
        typedef unsigned char   uint8_t;
        typedef unsigned short  uint16_t;
        typedef unsigned int    uint32_t;

        // Helper code - not intended to be directly called by the library users. May be changed at any time
        namespace internal
        {
            // Unicode constants
            // Leading (high) surrogates: 0xd800 - 0xdbff
            // Trailing (low) surrogates: 0xdc00 - 0xdfff
            const uint16_t LEAD_SURROGATE_MIN = 0xd800u;
            const uint16_t LEAD_SURROGATE_MAX = 0xdbffu;
            const uint16_t TRAIL_SURROGATE_MIN = 0xdc00u;
            const uint16_t TRAIL_SURROGATE_MAX = 0xdfffu;
            const uint16_t LEAD_OFFSET = LEAD_SURROGATE_MIN - (0x10000 >> 10);
            const uint32_t SURROGATE_OFFSET = 0x10000u - (LEAD_SURROGATE_MIN << 10) - TRAIL_SURROGATE_MIN;

            // Maximum valid value for a Unicode code point
            const uint32_t CODE_POINT_MAX = 0x0010ffffu;

            template<typename octet_type>
            inline uint8_t mask8(octet_type oc)
            {
                return static_cast<uint8_t>(0xff & oc);
            }
            template<typename u16_type>
            inline uint16_t mask16(u16_type oc)
            {
                return static_cast<uint16_t>(0xffff & oc);
            }
            template<typename octet_type>
            inline bool is_trail(octet_type oc)
            {
                return ((mask8(oc) >> 6) == 0x2);
            }

            template <typename u16>
            inline bool is_surrogate(u16 cp)
            {
                return (cp >= LEAD_SURROGATE_MIN && cp <= TRAIL_SURROGATE_MAX);
            }

            template <typename u32>
            inline bool is_code_point_valid(u32 cp)
            {
                return (cp <= CODE_POINT_MAX && !is_surrogate(cp) && cp != 0xfffe && cp != 0xffff);
            }

            template <typename octet_iterator>
            inline typename std::iterator_traits<octet_iterator>::difference_type
            sequence_length(octet_iterator lead_it)
            {
                uint8_t lead = mask8(*lead_it);
                if (lead < 0x80)
                    return 1;
                else if ((lead >> 5) == 0x6)
                    return 2;
                else if ((lead >> 4) == 0xe)
                    return 3;
                else if ((lead >> 3) == 0x1e)
                    return 4;
                else
                    return 0;
            }

            enum utf_error { OK, NOT_ENOUGH_ROOM, INVALID_LEAD, INCOMPLETE_SEQUENCE, OVERLONG_SEQUENCE, INVALID_CODE_POINT };

            template <typename octet_iterator>
            utf_error validate_next(octet_iterator& it, octet_iterator end, uint32_t* code_point)
            {
                uint32_t cp = mask8(*it);
                // Check the lead octet
                typedef typename std::iterator_traits<octet_iterator>::difference_type octet_difference_type;
                octet_difference_type length = sequence_length(it);

                // "Shortcut" for ASCII characters
                if (length == 1) {
                    if (end - it > 0) {
                        if (code_point)
                            *code_point = cp;
                        ++it;
                        return OK;
                    }
                    else
                        return NOT_ENOUGH_ROOM;
                }

                // Do we have enough memory?
                if (std::distance(it, end) < length)
                    return NOT_ENOUGH_ROOM;

                // Check trail octets and calculate the code point
                switch (length) {
                    case 0:
                        return INVALID_LEAD;
                        break;
                    case 2:
                        if (is_trail(*(++it))) {
                            cp = ((cp << 6) & 0x7ff) + ((*it) & 0x3f);
                        }
                        else {
                            --it;
                            return INCOMPLETE_SEQUENCE;
                        }
                        break;
                    case 3:
                        if (is_trail(*(++it))) {
                            cp = ((cp << 12) & 0xffff) + ((mask8(*it) << 6) & 0xfff);
                            if (is_trail(*(++it))) {
                                cp += (*it) & 0x3f;
                            }
                            else {
                                std::advance(it, -2);
                                return INCOMPLETE_SEQUENCE;
                            }
                        }
                        else {
                            --it;
                            return INCOMPLETE_SEQUENCE;
                        }
                        break;
                    case 4:
                        if (is_trail(*(++it))) {
                            cp = ((cp << 18) & 0x1fffff) + ((mask8(*it) << 12) & 0x3ffff);
                            if (is_trail(*(++it))) {
                                cp += (mask8(*it) << 6) & 0xfff;
                                if (is_trail(*(++it))) {
                                    cp += (*it) & 0x3f;
                                }
                                else {
                                    std::advance(it, -3);
                                    return INCOMPLETE_SEQUENCE;
                                }
                            }
                            else {
                                std::advance(it, -2);
                                return INCOMPLETE_SEQUENCE;
                            }
                        }
                        else {
                            --it;
                            return INCOMPLETE_SEQUENCE;
                        }
                        break;
                }
                // Is the code point valid?
                if (!is_code_point_valid(cp)) {
                    for (octet_difference_type i = 0; i < length - 1; ++i)
                        --it;
                    return INVALID_CODE_POINT;
                }

                if (code_point)
                    *code_point = cp;

                if (cp < 0x80) {
                    if (length != 1) {
                        std::advance(it, -(length - 1));
                        return OVERLONG_SEQUENCE;
                    }
                }
                else if (cp < 0x800) {
                    if (length != 2) {
                        std::advance(it, -(length - 1));
                        return OVERLONG_SEQUENCE;
                    }
                }
                else if (cp < 0x10000) {
                    if (length != 3) {
                        std::advance(it, -(length - 1));
                        return OVERLONG_SEQUENCE;
                    }
                }

                ++it;
                return OK;
            }

            template <typename octet_iterator>
            inline utf_error validate_next(octet_iterator& it, octet_iterator end) {
                return validate_next(it, end, 0);
            }

        } // namespace internal

        /// The library API - functions intended to be called by the users

        // Byte order mark
        const uint8_t bom[] = { 0xef, 0xbb, 0xbf };

        template <typename octet_iterator>
        octet_iterator find_invalid(octet_iterator start, octet_iterator end)
        {
            octet_iterator result = start;
            while (result != end) {
                internal::utf_error err_code = internal::validate_next(result, end);
                if (err_code != internal::OK)
                    return result;
            }
            return result;
        }

        template <typename octet_iterator>
        inline bool is_valid(octet_iterator start, octet_iterator end)
        {
            return (find_invalid(start, end) == end);
        }

        template <typename octet_iterator>
        inline bool is_bom(octet_iterator it)
        {
            return (
                    (internal::mask8(*it++)) == bom[0] &&
                    (internal::mask8(*it++)) == bom[1] &&
                    (internal::mask8(*it)) == bom[2]
            );
        }

        // Exceptions that may be thrown from the library functions.
        class invalid_code_point : public std::exception {
            uint32_t cp;
        public:
            invalid_code_point(uint32_t _cp) : cp(_cp) {}
            virtual const char* what() const throw() { return "Invalid code point"; }
            uint32_t code_point() const { return cp; }
        };

        class invalid_utf8 : public std::exception {
            uint8_t u8;
        public:
            invalid_utf8(uint8_t u) : u8(u) {}
            virtual const char* what() const throw() { return "Invalid UTF-8"; }
            uint8_t utf8_octet() const { return u8; }
        };

        class invalid_utf16 : public std::exception {
            uint16_t u16;
        public:
            invalid_utf16(uint16_t u) : u16(u) {}
            virtual const char* what() const throw() { return "Invalid UTF-16"; }
            uint16_t utf16_word() const { return u16; }
        };

        class not_enough_room : public std::exception {
        public:
            virtual const char* what() const throw() { return "Not enough space"; }
        };

        template <typename octet_iterator>
        uint32_t next(octet_iterator& it, octet_iterator end)
        {
            uint32_t cp = 0;
            internal::utf_error err_code = internal::validate_next(it, end, &cp);
            switch (err_code) {
                case internal::OK:
                    break;
                case internal::NOT_ENOUGH_ROOM:
                    throw not_enough_room();
                case internal::INVALID_LEAD:
                case internal::INCOMPLETE_SEQUENCE:
                case internal::OVERLONG_SEQUENCE:
                    throw invalid_utf8(*it);
                case internal::INVALID_CODE_POINT:
                    throw invalid_code_point(cp);
            }
            return cp;
        }

        template <typename octet_iterator>
        octet_iterator append(uint32_t cp, octet_iterator result)
        {
            if (!internal::is_code_point_valid(cp))
                throw invalid_code_point(cp);

            if (cp < 0x80)                        // one octet
                *(result++) = static_cast<uint8_t>(cp);
            else if (cp < 0x800) {                // two octets
                *(result++) = static_cast<uint8_t>((cp >> 6) | 0xc0);
                *(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
            }
            else if (cp < 0x10000) {              // three octets
                *(result++) = static_cast<uint8_t>((cp >> 12) | 0xe0);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
            }
            else if (cp <= internal::CODE_POINT_MAX) {      // four octets
                *(result++) = static_cast<uint8_t>((cp >> 18) | 0xf0);
                *(result++) = static_cast<uint8_t>(((cp >> 12) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>(((cp >> 6) & 0x3f) | 0x80);
                *(result++) = static_cast<uint8_t>((cp & 0x3f) | 0x80);
            }
            else
                throw invalid_code_point(cp);

            return result;
        }

        template <typename u16bit_iterator, typename octet_iterator>
        u16bit_iterator utf8to16(octet_iterator start, octet_iterator end, u16bit_iterator result)
        {
            while (start != end) {
                uint32_t cp = next(start, end);
                if (cp > 0xffff) { //make a surrogate pair
                    *result++ = static_cast<uint16_t>((cp >> 10) + internal::LEAD_OFFSET);
                    *result++ = static_cast<uint16_t>((cp & 0x3ff) + internal::TRAIL_SURROGATE_MIN);
                }
                else
                    *result++ = static_cast<uint16_t>(cp);
            }
            return result;
        }

        template <typename u16bit_iterator, typename octet_iterator>
        octet_iterator utf16to8(u16bit_iterator start, u16bit_iterator end, octet_iterator result)
        {
            while (start != end) {
                uint32_t cp = internal::mask16(*start++);
                // Take care of surrogate pairs first
                if (internal::is_surrogate(cp)) {
                    if (start != end) {
                        uint32_t trail_surrogate = internal::mask16(*start++);
                        if (trail_surrogate >= internal::TRAIL_SURROGATE_MIN && trail_surrogate <= internal::TRAIL_SURROGATE_MAX)
                            cp = (cp << 10) + trail_surrogate + internal::SURROGATE_OFFSET;
                        else
                            throw invalid_utf16(static_cast<uint16_t>(trail_surrogate));
                    }
                    else
                        throw invalid_utf16(static_cast<uint16_t>(*start));

                }
                result = append(cp, result);
            }
            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        u32bit_iterator utf8to32(octet_iterator start, octet_iterator end, u32bit_iterator result)
        {
            while (start < end)
                (*result++) = next(start, end);

            return result;
        }

        template <typename octet_iterator, typename u32bit_iterator>
        octet_iterator utf32to8(u32bit_iterator start, u32bit_iterator end, octet_iterator result)
        {
            while (start != end)
                result = append(*(start++), result);

            return result;
        }

    } // namespace utf8
} // namespace thirdparty

namespace fmt
{
    namespace consts
    {
        static const char			kFormatSpecifierEscapeChar = '%';
        static const std::string	kZeroPaddingStr = std::string("0");
    }

    bool is_digit(char c)
    {
        return c >= '0' && c <= '9';
    }

    bool wild_card_match(const char* str, const char* pattern)
    {
        while (*pattern)
        {
            switch (*pattern)
            {
                case '?':
                    if (!*str) return false;
                    ++str;
                    ++pattern;
                    break;
                case '*':
                    if (wild_card_match(str, pattern + 1)) return true;
                    if (*str && wild_card_match(str + 1, pattern)) return true;
                    return false;
                    break;
                default:
                    if (*str++ != *pattern++) return false;
                    break;
            }
        }
        return !*str && !*pattern;
    }

    std::string ltrim(std::string str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(&std::isspace))));
        return str;
    }

    std::string rtrim(std::string str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun<int, int>(&std::isspace))).base(), str.end());
        return str;
    }

    std::string trim(std::string str)
    {
        return ltrim(rtrim(str));
    }

    std::string lstrip(std::string str, std::string what)
    {
        auto pos = str.find(what);
        if (0 == pos)
        {
            str.erase(pos, what.length());
        }
        return str;
    }

    std::string rstrip(std::string str, std::string what)
    {
        auto pos = str.rfind(what);
        if (str.length() - what.length() == pos)
        {
            str.erase(pos, what.length());
        }
        return str;
    }

    std::string lskip(std::string str, std::string delim)
    {
        auto pos = str.find(delim);
        if (pos == std::string::npos)
        {
            str = std::string();
        }
        else
        {
            str = str.substr(pos + 1);
        }
        return str;
    }

    std::string rskip(std::string str, std::string delim)
    {
        auto pos = str.rfind(delim);
        if (pos == 0)
        {
            str = std::string();
        }
        else if (std::string::npos != pos)
        {
            str = str.substr(0, pos);
        }
        return str;
    }

    std::string rskip_all(std::string str, std::string delim)
    {
        auto pos = str.find(delim);
        if (pos == std::string::npos)
        {
            return str;
        }
        else
        {
            return str.substr(0, pos);
        }
    }

    std::vector<std::string> split(const std::string s, char delim)
    {
        std::vector<std::string> elems;
        std::istringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim))
        {
            if (!item.empty()) elems.push_back(item);
        }
        return elems;
    }

    std::vector<std::string> split(const std::string s, std::string delim)
    {
        std::vector<std::string> elems;
        std::string ss(s);
        std::string item;
        size_t pos = 0;
        while ((pos = ss.find(delim)) != std::string::npos) {
            item = ss.substr(0, pos);
            if (!item.empty()) elems.push_back(item);
            ss.erase(0, pos + delim.length());
        }
        if (!ss.empty()) elems.push_back(ss);
        return elems;
    }

    std::vector<std::string> split_multi_delims(const std::string s, std::string delims)
    {
        std::vector<std::string> elems;
        std::string ss(s);
        std::string item;
        size_t pos = 0;
        while ((pos = ss.find_first_of(delims)) != std::string::npos) {
            item = ss.substr(0, pos);
            if (!item.empty()) elems.push_back(item);
            ss.erase(0, pos + 1);
        }
        if (!ss.empty()) elems.push_back(ss);
        return elems;
    }

    std::vector<std::string> split_whitespace(const std::string s)
    {
        auto list = split_multi_delims(s, " \t\n");
        std::vector<std::string> ret;
        for (auto elem : list)
        {
            auto rest = fmt::trim(elem);
            if (!rest.empty()) ret.push_back(rest);
        }
        return ret;
    }

    std::pair<std::string, std::string> split_first_occurance(const std::string s, char delim)
    {
        auto pos = s.find_first_of(delim);
        std::string first = s.substr(0, pos);
        std::string second = (pos != std::string::npos ? s.substr(pos + 1) : std::string());
        return std::make_pair(first, second);
    }

    std::vector<std::string>& erase_empty(std::vector<std::string> &vec)
    {
        for (auto it = vec.begin(); it != vec.end();)
        {
            if (it->empty())
            {
                it = vec.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return vec;
    }

    std::string join(std::vector<std::string> elems, char delim)
    {
        std::string str;
        elems = erase_empty(elems);
        if (elems.empty()) return str;
        str = elems[0];
        for (std::size_t i = 1; i < elems.size(); ++i)
        {
            if (elems[i].empty()) continue;
            str += delim + elems[i];
        }
        return str;
    }

    std::string join(std::vector<std::string> elems, std::string delim)
    {
        std::string str;
        elems = erase_empty(elems);
        if (elems.empty()) return str;
        str = elems[0];
        for (std::size_t i = 1; i < elems.size(); ++i)
        {
            if (elems[i].empty()) continue;
            str += delim + elems[i];
        }
        return str;
    }

    bool starts_with(const std::string& str, const std::string& start)
    {
        return (str.length() >= start.length()) && (str.compare(0, start.length(), start) == 0);
    }

    bool ends_with(const std::string& str, const std::string& end)
    {
        return (str.length() >= end.length()) && (str.compare(str.length() - end.length(), end.length(), end) == 0);
    }

    std::string& replace_all(std::string& str, char replaceWhat, char replaceWith)
    {
        std::replace(str.begin(), str.end(), replaceWhat, replaceWith);
        return str;
    }

    std::string& replace_all(std::string& str, const std::string& replaceWhat, const std::string& replaceWith)
    {
        if (replaceWhat == replaceWith) return str;
        std::size_t foundAt = std::string::npos;
        while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos)
        {
            str.replace(foundAt, replaceWhat.length(), replaceWith);
        }
        return str;
    }

    void replace_first_with_escape(std::string &str, const std::string &replaceWhat, const std::string &replaceWith)
    {
        std::size_t foundAt = std::string::npos;
        while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos)
        {
            if (foundAt > 0 && str[foundAt - 1] == consts::kFormatSpecifierEscapeChar)
            {
                str.erase(foundAt > 0 ? foundAt - 1 : 0, 1);
                ++foundAt;
            }
            else
            {
                str.replace(foundAt, replaceWhat.length(), replaceWith);
                return;
            }
        }
    }

    void replace_all_with_escape(std::string &str, const std::string &replaceWhat, const std::string &replaceWith)
    {
        std::size_t foundAt = std::string::npos;
        while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos)
        {
            if (foundAt > 0 && str[foundAt - 1] == consts::kFormatSpecifierEscapeChar)
            {
                str.erase(foundAt > 0 ? foundAt - 1 : 0, 1);
                ++foundAt;
            }
            else
            {
                str.replace(foundAt, replaceWhat.length(), replaceWith);
                foundAt += replaceWith.length();
            }
        }
    }

    void replace_sequential_with_escape(std::string &str, const std::string &replaceWhat, const std::vector<std::string> &replaceWith)
    {
        std::size_t foundAt = std::string::npos;
        std::size_t candidatePos = 0;
        while ((foundAt = str.find(replaceWhat, foundAt + 1)) != std::string::npos && replaceWith.size() > candidatePos)
        {
            if (foundAt > 0 && str[foundAt - 1] == consts::kFormatSpecifierEscapeChar)
            {
                str.erase(foundAt > 0 ? foundAt - 1 : 0, 1);
                ++foundAt;
            }
            else
            {
                str.replace(foundAt, replaceWhat.length(), replaceWith[candidatePos]);
                foundAt += replaceWith[candidatePos].length();
                ++candidatePos;
            }
        }
    }

    bool str_equals(const char* s1, const char* s2)
    {
        if (s1 == nullptr && s2 == nullptr) return true;
        if (s1 == nullptr || s2 == nullptr) return false;
        return std::string(s1) == std::string(s2);	// this is safe, not with strcmp
    }

    //std::string& left_zero_padding(std::string &str, int width)
    //{
    //	int toPad = width - static_cast<int>(str.length());
    //	while (toPad > 0)
    //	{
    //		str = consts::kZeroPaddingStr + str;
    //		--toPad;
    //	}
    //	return str;
    //}

    std::string to_lower_ascii(std::string mixed)
    {
        std::transform(mixed.begin(), mixed.end(), mixed.begin(), ::tolower);
        return mixed;
    }

    std::string to_upper_ascii(std::string mixed)
    {
        std::transform(mixed.begin(), mixed.end(), mixed.begin(), ::toupper);
        return mixed;
    }

    std::u16string utf8_to_utf16(std::string u8str)
    {
        try
        {
            std::u16string ret;
            thirdparty::utf8::utf8to16(u8str.begin(), u8str.end(), std::back_inserter(ret));
            return ret;
        }
        catch (...)
        {
            throw std::runtime_error("Invalid UTF-8 string.");
        }
    }

    std::string utf16_to_utf8(std::u16string u16str)
    {
        try
        {
            std::vector<unsigned char> u8vec;
            thirdparty::utf8::utf16to8(u16str.begin(), u16str.end(), std::back_inserter(u8vec));
            auto ptr = reinterpret_cast<char*>(u8vec.data());
            return std::string(ptr, ptr + u8vec.size());
        }
        catch (...)
        {
            throw std::runtime_error("Invalid UTF-16 string.");
        }
    }

    std::u32string utf8_to_utf32(std::string u8str)
    {
        try
        {
            std::u32string ret;
            thirdparty::utf8::utf8to32(u8str.begin(), u8str.end(), std::back_inserter(ret));
            return ret;
        }
        catch (...)
        {
            throw std::runtime_error("Invalid UTF-8 string.");
        }
    }

    std::string utf32_to_utf8(std::u32string u32str)
    {
        try
        {
            std::vector<unsigned char> u8vec;
            thirdparty::utf8::utf32to8(u32str.begin(), u32str.end(), std::back_inserter(u8vec));
            auto ptr = reinterpret_cast<char*>(u8vec.data());
            return std::string(ptr, ptr + u8vec.size());
        }
        catch (...)
        {
            throw std::runtime_error("Invalid UTF-32 string.");
        }
    }

} // namespace fmt

#endif //PXX11_STRING_IMPL_HPP
