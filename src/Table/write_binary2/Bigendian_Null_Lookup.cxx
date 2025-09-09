#include "Bigendian_Null_Lookup.hxx"

namespace tablator {

const uint8_t *Bigendian_Null_Lookup::get_bigendian_null_ptr(Data_Type data_type) {
    if (first_time_) {
        init();
        first_time_ = false;
    }
    const auto lookup_iter = bigendian_null_lookup_.find(data_type);
    if (lookup_iter == bigendian_null_lookup_.end()) {
        throw std::runtime_error(
                "Unknown data type when writing null in binary2 "
                "format: ");
    }
    return lookup_iter->second;
}

//===============================================

void Bigendian_Null_Lookup::init() {
    uint8_t *bigendian_null_int8_ptr = reinterpret_cast<uint8_t *>(&null_int8_);
    write_binary2_null<int8_t>(bigendian_null_int8_ptr);

    uint8_t *bigendian_null_uint8_ptr = reinterpret_cast<uint8_t *>(&null_uint8_);
    write_binary2_null<uint8_t>(bigendian_null_uint8_ptr);

    uint8_t *bigendian_null_int16_ptr = reinterpret_cast<uint8_t *>(&null_int16_);
    write_binary2_null<int16_t>(bigendian_null_int16_ptr);

    uint8_t *bigendian_null_uint16_ptr = reinterpret_cast<uint8_t *>(&null_uint16_);
    write_binary2_null<uint16_t>(bigendian_null_uint16_ptr);

    uint8_t *bigendian_null_int32_ptr = reinterpret_cast<uint8_t *>(&null_int32_);
    write_binary2_null<int32_t>(bigendian_null_int32_ptr);

    uint8_t *bigendian_null_uint32_ptr = reinterpret_cast<uint8_t *>(&null_uint32_);
    write_binary2_null<uint32_t>(bigendian_null_uint32_ptr);

    uint8_t *bigendian_null_int64_ptr = reinterpret_cast<uint8_t *>(&null_int64_);
    write_binary2_null<int64_t>(bigendian_null_int64_ptr);

    uint8_t *bigendian_null_uint64_ptr = reinterpret_cast<uint8_t *>(&null_uint64_);
    write_binary2_null<uint64_t>(bigendian_null_uint64_ptr);

    uint8_t *bigendian_null_float32_ptr = reinterpret_cast<uint8_t *>(&null_float32_);
    write_binary2_null<float>(bigendian_null_float32_ptr);

    uint8_t *bigendian_null_float64_ptr = reinterpret_cast<uint8_t *>(&null_float64_);
    write_binary2_null<double>(bigendian_null_float64_ptr);

    bigendian_null_lookup_[tablator::Data_Type::INT8_LE] = bigendian_null_int8_ptr;
    bigendian_null_lookup_[tablator::Data_Type::UINT8_LE] = bigendian_null_uint8_ptr;

    bigendian_null_lookup_[tablator::Data_Type::INT16_LE] = bigendian_null_int16_ptr;
    bigendian_null_lookup_[tablator::Data_Type::UINT16_LE] = bigendian_null_uint16_ptr;

    bigendian_null_lookup_[tablator::Data_Type::INT32_LE] = bigendian_null_int32_ptr;
    bigendian_null_lookup_[tablator::Data_Type::UINT32_LE] = bigendian_null_uint32_ptr;

    bigendian_null_lookup_[tablator::Data_Type::INT64_LE] = bigendian_null_int64_ptr;
    bigendian_null_lookup_[tablator::Data_Type::UINT64_LE] = bigendian_null_uint64_ptr;

    bigendian_null_lookup_[tablator::Data_Type::FLOAT32_LE] =
            bigendian_null_float32_ptr;
    bigendian_null_lookup_[tablator::Data_Type::FLOAT64_LE] =
            bigendian_null_float64_ptr;
}

//===============================================

// Initialize static class members.
std::map<tablator::Data_Type, const uint8_t *>
        tablator::Bigendian_Null_Lookup::bigendian_null_lookup_ =
                std::map<tablator::Data_Type, const uint8_t *>();

int8_t tablator::Bigendian_Null_Lookup::null_int8_ = int8_t(0);
uint8_t tablator::Bigendian_Null_Lookup::null_uint8_ = uint8_t(0);
int16_t tablator::Bigendian_Null_Lookup::null_int16_ = int16_t(0);
uint16_t tablator::Bigendian_Null_Lookup::null_uint16_ = uint16_t(0);
int32_t tablator::Bigendian_Null_Lookup::null_int32_ = int32_t(0);
uint32_t tablator::Bigendian_Null_Lookup::null_uint32_ = uint32_t(0);
int64_t tablator::Bigendian_Null_Lookup::null_int64_ = uint64_t(0);
uint64_t tablator::Bigendian_Null_Lookup::null_uint64_ = uint64_t(0);
float tablator::Bigendian_Null_Lookup::null_float32_ = float(0.0);
double tablator::Bigendian_Null_Lookup::null_float64_ = double(0.0);

bool tablator::Bigendian_Null_Lookup::first_time_ = true;


}  // namespace tablator
