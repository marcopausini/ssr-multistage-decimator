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


#include "ap_fixed.h"
#include "ap_int.h"

// C++ class (ap_shift_reg) to ensure that the shift register defined in the C code is always implemented using an SRL resource
#include "ap_shift_reg.h"

//#define _DEBUG_ 

// decimation factor data type:
typedef ap_uint<8> dec_factor_t;

// fixed point data type:
//constexpr unsigned int num_coefficients = 31;
constexpr int coef_bits = 18;
constexpr int coef_fractional_bits = 17;
constexpr int coef_integer_bits = coef_bits - coef_fractional_bits;
//
constexpr int datain_bits = 16;
constexpr int datain_fractional_bits = 15;
constexpr int datain_integer_bits = datain_bits - datain_fractional_bits;
//
constexpr int data_bits = 16;
constexpr int data_fractional_bits = 15;
constexpr int data_integer_bits = data_bits - data_fractional_bits;
//
constexpr int dataout_bits = 16;
constexpr int dataout_fractional_bits = 15;
constexpr int dataout_integer_bits = dataout_bits - dataout_fractional_bits;

typedef ap_int<coef_bits> coef_int_t;                                               // integer type to load coefficients from file
typedef ap_fixed<coef_bits, coef_integer_bits> coef_t;                              // coef is s18.17
typedef ap_fixed<datain_bits, datain_integer_bits> datain_t;                        // data is s16.15
typedef ap_fixed<data_bits, data_integer_bits> data_t;                              // data is s16.15
typedef ap_fixed<34, 34 - 32> mult_t;                                   // s18.17 x s16.15 = s34.32
typedef ap_fixed<40, 40 - 32> acc_t;                                    // 
typedef ap_fixed<dataout_bits, dataout_integer_bits, AP_RND_INF, AP_SAT> dataout_t; // s16.15
typedef ap_uint<8> coef_addr_t;

// Define a complex number struct
typedef struct
{
    datain_t re;
    datain_t im;
} cdatain_t;

typedef struct
{
    dataout_t re;
    dataout_t im;
} cdataout_t;

typedef struct 
{
    data_t re;
    data_t im;
} cdata_t;

typedef struct
{
    acc_t re;
    acc_t im;
} cacc_t;

template <std::size_t N>
struct cdatain_vec_t
{
    datain_t re[N];
    datain_t im[N];
};

template <std::size_t N>
struct cdata_vec_t
{
    data_t re[N];
    data_t im[N];
};

template <std::size_t N>
struct cdataout_vec_t
{
    dataout_t re[N];
    dataout_t im[N];
};


// Super-Sample Rate => Parallelism Factor - or Hardware Oversampling Rate
const std::size_t ssr = 8;

// top level function
void ssr_multistage_decimator(dec_factor_t dec_factor, bool tvalid_i, cdatain_vec_t<ssr> tdata_i, bool &tvalid_o, cdataout_vec_t<ssr> &tdata_o);

#endif // SSR_MULTISTAGE_DECIMATOR
