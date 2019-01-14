//
// Created by garrick on 19-1-11.
//

#ifndef PXX11_UTILS_HPP
#define PXX11_UTILS_HPP

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


#endif //PXX11_UTILS_HPP
