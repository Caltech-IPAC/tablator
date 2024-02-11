#pragma once

namespace tablator {

static constexpr uint MIN_RUN_LENGTH_FOR_TRIMMING = 5;

// Double shouldn't be this long anyway.
static constexpr uint SIGNAL_NO_TRIMMING = 25;

struct Command_Line_Options {
    Command_Line_Options()
            : min_run_length_for_trim_(SIGNAL_NO_TRIMMING),
              write_null_strings_(false),
              skip_comments_(false) {}
    Command_Line_Options(uint mrlft, bool wns, bool sk)
            : min_run_length_for_trim_(mrlft),
              write_null_strings_(wns),
              skip_comments_(sk) {}

    bool is_trim_decimal_runs() const {
        return min_run_length_for_trim_ < SIGNAL_NO_TRIMMING;
    }
    uint min_run_length_for_trim_;
    bool write_null_strings_;
    bool skip_comments_;
};

static const Command_Line_Options default_options = Command_Line_Options();

}  // namespace tablator
