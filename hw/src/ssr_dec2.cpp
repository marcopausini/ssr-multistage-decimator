/**
 * @file ssr_dec2.cpp
 *
 * @brief SSR decimator with 2x decimation factor: 1280 -> 640
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"

// FIR filter parameters
struct ssr_dec2_config : hls::ip_fir::params_t
{
    static const unsigned input_width = 16;
    static const unsigned input_fractional_bits = 0;
    static const unsigned output_width = 16;
    static const unsigned output_fractional_bits = 0;
    static const unsigned coeff_width = 18;
    static const unsigned coeff_fractional_bits = 0;

    static const unsigned num_coeffs = 31;
    static const unsigned coeff_sets = 1;
    static const unsigned input_length = 8;  // num_coeffs;
    static const unsigned output_length = 4; // num_coeffs;
    static const unsigned num_channels = 1;

    static const unsigned total_num_coeff = 31; // = num_coeffs * coeff_sets;
    static const double coeff_vec[total_num_coeff]; //[total_num_coeff];

    static const bool reloadable = false;
    static const unsigned filter_type = hls::ip_fir::decimation; // hls::ip_fir::single_rate; // hls::ip_fir::decimation;
    static const unsigned rate_change = hls::ip_fir::integer;
    static const unsigned interp_rate = 1;
    static const unsigned decim_rate = 2; // 2;
    static const unsigned zero_pack_factor = 1;
    static const unsigned chan_seq = hls::ip_fir::basic;
    static const unsigned rate_specification = hls::ip_fir::input_period;
    static const unsigned sample_period = 0.125;
    static constexpr double sample_frequency = 1280;

    static const unsigned quantization = hls::ip_fir::integer_coefficients;
    static const bool best_precision = false;
    static const unsigned coeff_structure = hls::ip_fir::inferred;
    static const unsigned output_rounding_mode = hls::ip_fir::symmetric_rounding_to_zero;
    static const unsigned filter_arch = hls::ip_fir::systolic_multiply_accumulate;
    static const unsigned optimization_goal = hls::ip_fir::area;
    static const unsigned inter_column_pipe_length = 4;
    static const unsigned column_config = 16;
    static const unsigned config_sync_mode = hls::ip_fir::on_vector;
    static const unsigned config_method = hls::ip_fir::single;
    static const unsigned coeff_padding = 0;

    static const unsigned num_paths = 1;
    static const unsigned data_sign = hls::ip_fir::value_signed;
    static const unsigned coeff_sign = hls::ip_fir::value_signed;
};

const double ssr_dec2_config::coeff_vec[ssr_dec2_config::total_num_coeff] = {-197, 0, 501, 0, -1087, 0, 2079, 0, -3723, 0, 6596, 0, -12793, 0, 41339, 65536, 41339, 0, -12793, 0, 6596, 0, -3723, 0, 2079, 0, -1087, 0, 501, 0, -197};

void ssr_dec2(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec4_t> &out){

static hls::FIR<ssr_dec2_config> dec2_filter;

cdata_vec_t input = in.read();

ap_fixed<16, 16> xi[ssr_dec2_config::input_length] = {input[0].re, input[1].re, input[2].re, input[3].re, input[4].re, input[5].re, input[6].re, input[7].re};
ap_fixed<16, 16> yi[ssr_dec2_config::output_length];
dec2_filter.run(xi, yi);

ap_fixed<16, 16> xq[ssr_dec2_config::input_length] = {input[0].im, input[1].im, input[2].im, input[3].im, input[4].im, input[5].im, input[6].im, input[7].im};
ap_fixed<16, 16> yq[ssr_dec2_config::output_length];
dec2_filter.run(xq, yq);

cdata_vec4_t output;
for (int i = 0; i < ssr_dec2_config::output_length; i++)
{
    output[i].re = yi[i];
    output[i].im = yq[i];
}

out.write(output);

}