#pragma once

namespace tablator {

struct Command_Line_Options {
    Command_Line_Options() : write_null_strings_(false), skip_comments_(false) {}
    Command_Line_Options(bool wns, bool sk) : write_null_strings_(wns), skip_comments_(sk) {}

    bool write_null_strings_;
    bool skip_comments_;
};

static const Command_Line_Options default_options = Command_Line_Options();

}  // namespace tablator
