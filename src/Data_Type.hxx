#pragma once

/// This enum exists only because checking what type an H5::DataType
/// is requires a dynamic lookup.  This ends up dominating the run
/// time if used for every row.
namespace tablator {
enum class Data_Type {
    INT8_LE,
    UINT8_LE,
    INT16_LE,
    UINT16_LE,
    INT32_LE,
    UINT32_LE,
    INT64_LE,
    UINT64_LE,
    FLOAT32_LE,
    FLOAT64_LE,
    CHAR
};
}
