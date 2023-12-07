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

using namespace std;

// filter coefficients data type: s18.18
typedef ap_int<18> coef_t; 

// decimation factor data type: 
typedef ap_int<8> dec_factor_t;

// input/output data type: complex s16.15
typedef struct
{
    ap_int<16> re;
    ap_int<16> im;
} cdata_t;

// Parallelism Factor - or Hardware Oversampling Rate
const int8_t P = 8;
typedef hls::vector<cdata_t, P> cdata_vec_t;

void ssr_multistage_decimator(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec_t> &out, dec_factor_t dec_factor);

#endif // SSR_MULTISTAGE_DECIMATOR