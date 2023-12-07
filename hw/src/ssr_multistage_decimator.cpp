/**
 * @file ssr_multistage_decimator.cpp
 * 
 * @brief Source file for the super-sample rate multistage decimator.
 *
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include "ssr_multistage_decimator.h"

void ssr_multistage_decimator(hls::stream<cdata_vec_t> &in, hls::stream<cdata_vec_t> &out, dec_factor_t dec_factor){

    cdata_vec_t input = in.read();

    switch(dec_factor){
        case 1:
            out.write(input);
            break;
        case 2:
            std::cout << "Not implemented "<< std::endl;
            break;
        default:
            throw std::invalid_argument("Invalid decimation factor");
    }


}