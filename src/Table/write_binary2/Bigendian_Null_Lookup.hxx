#pragma once

#include <map>
#include <type_traits>

#include "../../Data_Type.hxx"
#include "../../Utils/Endian_Utils.hxx"

namespace tablator {

class Bigendian_Null_Lookup {
public:
    static const uint8_t *get_bigendian_null_ptr(Data_Type data_type);
    static void init();

private:
    template <typename T>
    static inline typename std::enable_if<std::is_integral<T>::value, T>::type
    get_binary2_null() {
        if (std::is_same<T, char>::value) {
            return '\0';
        }
        return T(0);
    }

    template <typename T>
    static inline typename std::enable_if<std::is_floating_point<T>::value, T>::type
    get_binary2_null() {
        return std::numeric_limits<T>::quiet_NaN();
    }

    //===============================================

    template <typename T>
    static void write_binary2_null(uint8_t *&big_endian_null_ptr) {
        T little_endian_null = get_binary2_null<T>();
        uint8_t *little_endian_null_ptr =
                reinterpret_cast<uint8_t *>(&little_endian_null);

        tablator::copy_swapped_bytes(big_endian_null_ptr, little_endian_null_ptr,
                                     sizeof(T));
    }

    //===============================================

    static std::map<tablator::Data_Type, const uint8_t *> bigendian_null_lookup_;

    static int8_t null_int8_;
    static uint8_t null_uint8_;
    static int16_t null_int16_;
    static uint16_t null_uint16_;
    static int32_t null_int32_;
    static uint32_t null_uint32_;
    static int64_t null_int64_;
    static uint64_t null_uint64_;
    static float null_float32_;
    static double null_float64_;

    static bool first_time_;
};


}  // namespace tablator
