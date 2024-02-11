#include <cassert>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Decimal_String_Trimmer.hxx"


namespace {

//==============
// Helper class
//==============

class Trimmability_Packet {
public:
    Trimmability_Packet(double value_doub, ushort min_run_length_for_trim)
            : min_run_length_for_trim_(min_run_length_for_trim) {
        // Check for negative value.
        value_doub_ = value_doub;
        is_neg_ = (value_doub < 0);

        value_str_ = boost::lexical_cast<std::string>(value_doub);
        abs_value_str_ = is_neg_ ? value_str_.substr(1) : value_str_;

        // Check for scientific notation ("a.bbbbe-cc").
        e_pos_ = abs_value_str_.find("e");
        got_exp_ = (e_pos_ != std::string::npos);

        abs_str_len_ = abs_value_str_.size();
        abs_coeff_str_len_ = got_exp_ ? e_pos_ : abs_str_len_;

        // Prepare to check for runs of 0s or of 9s starting to the right
        // of the decimal point and on or to the right of the first
        // non-zero digit.
        anchor_pos_ = find_anchor_pos(abs_value_str_, abs_coeff_str_len_);

        bool keep_looking =
                is_candidate_for_adjustment(anchor_pos_, abs_coeff_str_len_);
        if (keep_looking) {
            // Set default values of these variables which will then capture
            // the return value of get_rounding_info_for_double().
            adjusted_abs_coeff_str_len_ = abs_coeff_str_len_;
            run_end_pos_ = 0;
            got_run_of_9s_ = false;
            abs_coeff_str_ =
                    got_exp_ ? abs_value_str_.substr(0, e_pos_) : abs_value_str_;

            std::tie(adjusted_abs_coeff_str_len_, run_end_pos_, got_run_of_9s_) =
                    get_rounding_info_for_double(abs_coeff_str_, anchor_pos_,
                                                 adjusted_abs_coeff_str_len_);

            if (adjusted_abs_coeff_str_len_ == abs_coeff_str_len_) {
                // Another early exit: there is no run of 0s or of 9s to be
                // trimmed.
                keep_looking = false;
            }
        }
        is_trimmable_ = keep_looking;
    }

    //================================================

    const std::string get_possibly_trimmed_string() const {
        if (!is_trimmable_) {
            return value_str_;
        }

        // If we get here, abs_value_str contains a run of 0s or of 9s to
        // the right of its anchor_pos.  We now round up or down to
        // eliminate the run and everything beyond it.  If the run is of
        // 0s, we simply truncate at the decimal point preceding the run,
        // the position indicated by adjusted_abs_coeff_str_len.  If the
        // run is of 9s, we round up at that position before truncating.

        // Set return value for run of 0s, in which case we just truncate
        // the original string, and then adjust for run of 9s.

        std::string sign_str = is_neg_ ? "-" : "";

        std::string abs_ret_coeff_str =
                abs_coeff_str_.substr(0, adjusted_abs_coeff_str_len_);
        std::string exp_str_ = got_exp_ ? abs_value_str_.substr(e_pos_) : "";


        if (!got_run_of_9s_) {
            // The run is of 0s; all that's needed is to truncate and, if
            // appropriate, replace the sign.
            return sign_str + abs_ret_coeff_str + exp_str_;
        }

        // If we get here, we found a run of 9s.

        // Modifying the abs_coeff_str directly to reflect rounding up or
        // down would require us to consider possible follow-on effects on
        // digits to the left of the decimal point, if the run begins
        // immediately after the decimal point (e.g. if abs_value_str is
        // 99.99999992). Instead, we modify the value in numeric form by
        // adding a small value (a 1 in the decimal place at the
        // right-hand end of the run) to abs_value_doub.  We then convert
        // the modified value to a string and truncate the string at the
        // indicated position.

        short sign = is_neg_ ? -1 : 1;

        double abs_value_doub = sign * value_doub_;  // JTODO
        double epsilon = pow(0.1, run_end_pos_ - anchor_pos_ - 1);

        double abs_doub_to_adjust =
                (got_exp_) ? boost::lexical_cast<double>(abs_coeff_str_)
                           : abs_value_doub;

        // Add epsilon and truncate.
        std::string adjusted_abs_coeff_str =
                boost::lexical_cast<std::string>(abs_doub_to_adjust + epsilon)
                        .substr(0, adjusted_abs_coeff_str_len_);

        if (got_exp_) {
            // adjusted_abs_coeff_str might now have two digits rather
            // than one to the left of the decimal point (e.g. if
            // coeff_str was 9.9999912, adjusted_abs_coeff_str would be
            // 10.0).  This next call produces a string in proper
            // scientific notation format, replacing e.g. 10.0e-5 with
            // 1.0e-4.
            return sign_str +
                   standardize_scientific_notation(adjusted_abs_coeff_str, exp_str_);
        }
        return sign_str + adjusted_abs_coeff_str;
    }

    //================================================

    size_t get_possibly_trimmed_length() const {
        if (is_trimmable_) {
            static const short MAX_EXP_LEN = 4;  // "e-cd"
            // Don't bother estimating length of possibly adjusted exp_str.
            size_t max_exp_len = got_exp_ ? MAX_EXP_LEN : 0;
            short sign_len = is_neg_ ? 1 : 0;

            return sign_len + adjusted_abs_coeff_str_len_ + max_exp_len;
        }
        return value_str_.size();
    }

    //================================================

private:
    size_t find_anchor_pos(const std::string &abs_value_str, size_t coeff_str_len) {
        size_t point_pos = abs_value_str.find(".");
        if (point_pos == std::string::npos) {
            // An integer, not a float. No need to adjust.
            return std::string::npos;
        }

        size_t digit_pos = abs_value_str.find_first_of("123456789");
        if (digit_pos >= coeff_str_len) {
            // coeff_string must be 0.0... . Assume the precision is
            // intentional.
            return std::string::npos;
        }

        if (digit_pos == 0) {
            // There is a non-0 digit before the decimal point.
            return point_pos;
        }

        // The first non-0 digit comes after the decimal point.
        return digit_pos - 1;
    }

    //==============================================

    bool is_candidate_for_adjustment(size_t anchor_pos, size_t coeff_str_len) {
        return (anchor_pos != std::string::npos &&
                anchor_pos + 1 + min_run_length_for_trim_ < coeff_str_len);
    }

    //==============================================

    // This function assumes that is_candidate_for_adjustment() has
    // already returned true.

    // Returns a triple:

    // strlen of rounded-off value

    // the position of the MIN_RUN_LENGTH-th digit in the first run of
    // either consecutive 0s or consecutive 9s to the right of anchor_pos,
    // if such exists

    // a flag indicating whether the run, if it exists, consists of all 9s

    std::tuple<size_t, size_t, bool> get_rounding_info_for_double(
            const std::string &coeff_str, size_t anchor_pos, size_t coeff_str_len) {
        assert(is_candidate_for_adjustment(anchor_pos, coeff_str_len));

        // Set defaults and adjust as necessary.
        size_t adjusted_str_len = coeff_str_len;
        bool got_run_of_9s = false;

        // Prepare to check for a run of 0s or 9s to the right of anchor_pos.
        size_t run_start_pos = anchor_pos + 1;
        char match_val = coeff_str[run_start_pos];
        bool within_run = (match_val == '0' || match_val == '9');
        size_t run_len = (within_run) ? 1 : 0;

        size_t curr_pos = run_start_pos + 1;
        for (/* */; curr_pos < coeff_str_len; ++curr_pos) {
            char curr_val = coeff_str[curr_pos];
            if (within_run && curr_val == match_val) {
                ++run_len;
            } else if (curr_val == '0' || curr_val == '9') {
                match_val = curr_val;
                run_len = 1;
                run_start_pos = curr_pos;
                within_run = true;
            } else {
                within_run = false;
                run_len = 0;
            }

            if (run_len == min_run_length_for_trim_) {
                adjusted_str_len = run_start_pos;
                if (adjusted_str_len == anchor_pos + 1) {
                    // Retain at least one digit after the decimal point.
                    ++adjusted_str_len;
                }

                got_run_of_9s = (match_val == '9');
                if (got_run_of_9s) {
                    size_t non_nine_pos = coeff_str.find_first_not_of("9.");
                    if (non_nine_pos > adjusted_str_len) {
                        // Rounding up will increase the length of coeff_str
                        // by 1.
                        ++adjusted_str_len;
                    }
                }
                break;
            }
        }
        return std::make_tuple(adjusted_str_len, curr_pos, got_run_of_9s);
    }

    //==============================================

    // This function takes as input a string created by the main
    // algorithm of get_possibly_trimmed_string(). This input string will either
    // be in standard scientific-notation form already or will have a
    // coefficient string that looks like "10." followed by one or more
    // 0s.  The function's output is a string in standard
    // scientific-notation form whose value equals the value of the
    // input string.

    static const std::string standardize_scientific_notation(
            const std::string &abs_coeff_str, const std::string exp_str) {
        if (!boost::starts_with(abs_coeff_str, "10.")) {
            return abs_coeff_str + exp_str;
        }

        std::stringstream new_ss;
        // Adjust the coefficient...
        new_ss << "1.0" << abs_coeff_str.substr(3);

        // ...and adjust the exponent accordingly.
        int exp = boost::lexical_cast<int>(exp_str.substr(1));
        exp += 1;

        new_ss << "e" << exp;
        return new_ss.str();
    }

    //==============================================

    const ushort min_run_length_for_trim_;
    double value_doub_;
    bool is_neg_;

    std::string value_str_;
    std::string abs_value_str_;

    size_t e_pos_;
    bool got_exp_;

    size_t abs_str_len_;
    size_t abs_coeff_str_len_;

    size_t anchor_pos_;
    bool is_trimmable_;

    size_t adjusted_abs_coeff_str_len_;
    size_t run_end_pos_;
    bool got_run_of_9s_;
    std::string abs_coeff_str_;
};  // Trimmability_Packet

}  // namespace

//==============================================
//==============================================

namespace tablator {


// This function checks for a run of 9s or of 0s to the right of the
// decimal point in the decimal representation of doub_value. If it
// finds such a run, it rounds up or down accordingly and truncates
// the string.

const std::string Decimal_String_Trimmer::get_decimal_string(
        double doub_value, ushort min_run_length_for_trim) {
    Trimmability_Packet trim_packet(doub_value, min_run_length_for_trim);
    return trim_packet.get_possibly_trimmed_string();
}

// This function returns the length of the string returned by get_decimal_string().

size_t tablator::Decimal_String_Trimmer::get_decimal_string_length(
        double doub_value, ushort min_run_length_for_trim) {
    Trimmability_Packet trim_packet(doub_value, min_run_length_for_trim);
    return trim_packet.get_possibly_trimmed_length();
}

}  // namespace tablator
