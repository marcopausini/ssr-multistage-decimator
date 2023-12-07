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

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <stdexcept>

#include <hls_stream.h>
#include "hls_vector.h"
#include "ap_int.h"

#include "hls_fir.h"

using namespace std;

// filter coefficients data type: s18.18
typedef ap_int<18> coef_t;

// decimation factor data type:
typedef ap_int<8> dec_factor_t;

// one sample: input/output data type: complex s16.15
typedef ap_int<16> data_t;
typedef struct
{
    data_t re;
    data_t im;
} cdata_t;


// Parallelism Factor - or Hardware Oversampling Rate
const int8_t P = 8;

// multiple samples: input/output data type:
typedef hls::vector<data_t, P> data_vec_t;
typedef hls::vector<cdata_t, P> cdata_vec_t;
//
typedef hls::vector<data_t, 4> data_vec4_t;
typedef hls::vector<cdata_t, 4> cdata_vec4_t;
//
typedef hls::vector<data_t, 2> data_vec2_t;
typedef hls::vector<cdata_t, 2> cdata_vec2_t;
//
//typedef hls::vector<data_t, 1> data_vec1_t;

// top level function
void ssr_multistage_decimator(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec_t> &out, dec_factor_t dec_factor);
// from 1280_to_640
void ssr_dec2(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec4_t> &out);
// from 640_to_320
void ssr_dec4(hls::stream<cdata_vec4_t> &in, hls::stream<cdata_vec2_t> &out);
// from 320_to_160
void ssr_dec8(hls::stream<cdata_vec2_t> &in, hls::stream<cdata_t> &out);
// from 160_to_80 + from 80_to_40 + from 40_to_20 
void hbf_dec(hls::stream<cdata_t> &in, hls::stream<cdata_t> &out);

#endif // SSR_MULTISTAGE_DECIMATOR