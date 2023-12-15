/**
 * @file ssr_multistage_decimator.h
 * @brief Header file for the super-sample rate multistage decimator.
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#ifndef SSR_MULTISTAGE_DECIMATOR_H_
#define SSR_MULTISTAGE_DECIMATOR_H_


#include <hls_stream.h>
#include "ap_fixed.h"
#include "ap_int.h"

#include "hls_fir.h"

using namespace std;

// decimation factor data type:
typedef ap_uint<8> dec_factor_t;

// input/output data types: complex s16.15
constexpr int num_bits = 16;
constexpr int frac_bits = 15;
typedef ap_fixed<num_bits, num_bits - frac_bits> fixed_point_t;
// define template for array of complex data type
template <size_t N>
struct cdata_t {
    fixed_point_t re[N]; // Array of N elements for the real part
    fixed_point_t im[N]; // Array of N elements for the imaginary part
};

// Super-Sample Rate => Parallelism Factor - or Hardware Oversampling Rate
const size_t ssr = 8;

// top level function
void ssr_multistage_decimator(hls::stream<cdata_t<ssr>> &in, hls::stream<cdata_t<ssr>> &out, dec_factor_t dec_factor);

// from 1280_to_640 (decimation factor = 2)
//void ssr_dec2(hls::stream<data_vec8_t> &in, hls::stream<data_vec4_t> &out);
void hbf_ssr8(hls::stream<cdata_t<ssr>> &in, hls::stream<cdata_t<ssr/2>> &out);
// from 640_to_320 (decimation factor = 4)
void hbf_ssr4(hls::stream<cdata_t<ssr/2>> &in, hls::stream<cdata_t<ssr/4>> &out);
// from 320_to_160 (decimation factor = 8)
void hbf_ssr2(hls::stream<cdata_t<ssr/4>> &in, hls::stream<cdata_t<ssr/8>> &out);
// from 160_to_80 + from 80_to_40 + from 40_to_20 (decimation factor = 16,32,64)
void hbf_ssr1(hls::stream<cdata_t<1>> &in, hls::stream<cdata_t<1>> &out);

#endif // SSR_MULTISTAGE_DECIMATOR
