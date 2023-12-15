/**
 * @file hb_filters.cpp
 *
 * @brief Half-band filters implementing the SSR multi-stage decimator
 *
 * - hbf_ssr8: 1280 -> 640 (decimation factor = 2, SSR = 8)
 * - hbf_ssr4: 640 -> 320 (decimation factor = 4, SSR = 4)
 * - hbf_ssr2: 320 -> 160 (decimation factor = 8, SSR = 2)
 * - hbf       160 -> 80 (decimation factor = 16, SSR = 1)
 * - hbf       80 -> 40 (decimation factor = 32, SSR = 1)
 * - hbf       40 -> 20 (decimation factor = 64, SSR = 1)
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"

// FIR filter parameters
struct hbf_ssr8_config : hls::ip_fir::params_t
{
    static const unsigned input_width = 16;
    static const unsigned input_fractional_bits = 15;
    static const unsigned output_width = 16;
    static const unsigned output_fractional_bits = 15;
    static const unsigned coeff_width = 18;
    static const unsigned coeff_fractional_bits = 17;

    static const unsigned num_coeffs = 31;
    static const unsigned coeff_sets = 1;
    static const unsigned input_length = 8;  // num_coeffs;
    static const unsigned output_length = 4; // num_coeffs;
    static const unsigned num_channels = 1;

    static const unsigned total_num_coeff = num_coeffs * coeff_sets;
    static const double coeff_vec[total_num_coeff]; //[total_num_coeff];

    static const bool reloadable = false;
    static const unsigned filter_type = hls::ip_fir::decimation;
    static const unsigned rate_change = hls::ip_fir::integer;
    static const unsigned interp_rate = 1;
    static const unsigned decim_rate = 2;
    static const unsigned zero_pack_factor = 1;
    static const unsigned chan_seq = hls::ip_fir::basic;
    static const unsigned rate_specification = hls::ip_fir::input_period;
    static constexpr double sample_period = 0.125;
    static constexpr double sample_frequency = 1280;

    static const unsigned quantization = hls::ip_fir::quantize_only;
    static const bool best_precision = true;
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

const double hbf_ssr8_config::coeff_vec[hbf_ssr8_config::total_num_coeff] = {
    -0.001503, 0.000000, 0.003822, 0.000000, -0.008293, 0.000000,
    0.015862, 0.000000, -0.028404, 0.000000, 0.050323, 0.000000,
    -0.097603, 0.000000, 0.315392, 0.500000, 0.315392, 0.000000,
    -0.097603, 0.000000, 0.050323, 0.000000, -0.028404, 0.000000,
    0.015862, 0.000000, -0.008293, 0.000000, 0.003822, 0.000000,
    -0.001503};


template <typename T, size_t P>
void unpackInputDataStructure(const T& input, typename T::value_type (&xi)[P], typename T::value_type (&xq)[P]) {
    // Unpack the input data structure into real and imag vectors for the filter input
    for (size_t i = 0; i < P; ++i) {
        // Unroll this loop to copy all elements in parallel
        #pragma HLS unroll
        xi[i] = input.re[i];
        xq[i] = input.im[i];
    }
}

template <size_t ssr>
void read_input_stream(hls::stream<cdata_t<ssr>> &in, fixed_point_t (&xi)[ssr], fixed_point_t (&xq)[ssr])
{
    // Read input data
    cdata_t<ssr> input = in.read();

    // Unpack the input data structure into real and imag vectors for the filter input
    for (int i = 0; i < ssr; i++)
    {
    // Unroll this loop to copy all elements in parallel
    #pragma HLS unroll
        xi[i] = input.re[i];
        xq[i] = input.im[i];
    }
}

template <size_t ssr>
void write_output_stream(hls::stream<cdata_t<ssr>> &out, fixed_point_t (&yi)[ssr], fixed_point_t (&yq)[ssr])
{
    // output data
    cdata_t<ssr> output;

    // pack the output data structure from real and imag vectors
    for (int i = 0; i < ssr; i++)
    // Unroll this loop to copy all elements in parallel
    #pragma HLS unroll
    {
        output.re[i] = yi[i];
        output.im[i] = yq[i];
    }

    // write the output data structure to the output stream
    out.write(output);
}

void hbf_ssr8(hls::stream<cdata_t<ssr>> &in, hls::stream<cdata_t<ssr / 2>> &out)
{

    #pragma HLS dataflow

    // real and imag input vectors
    fixed_point_t xi[ssr];
    fixed_point_t xq[ssr];

    // real and imag output vectors
    fixed_point_t yi[ssr / 2];
    fixed_point_t yq[ssr / 2];

    // FIR SSR Halfband filter instantiation
    static hls::FIR<hbf_ssr8_config> filter;

    //==================================================
    // Dataflow process
    //==================================================
    // unpack the input data structure into real and imag vectors for the filter input
    read_input_stream<ssr>(in, xi, xq);
    // run the filter
    filter.run(xi, yi);
    filter.run(xq, yq);
    // pack the output data structure from real and imag vectors
    write_output_stream<ssr / 2>(out, yi, yq);
    //==================================================
    
}