/**
 * @file tb_ssr_multistage_decimator.cpp
 * @brief Testbench for the ssr multistage decimator.
 *
 * Detailed explanation about what this file contains or does.
 *
 * @author marco.pausini@gmail.com
 * @date 2023-11-xy
 * @version 0.1
 *
 */

#include <fstream>
#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "../src/ssr_multistage_decimator.h"

// Function prototype.
void readParameterFile(const std::string &parameterFilePath, dec_factor_t &dec_factor);

int main(void)
{

    printf("\nA Testbench for the ssr multistage decimator\n\n");

    // Path to the parameters file
    std::string parameterFilePath = "work/parameters.csv";

    // Decimation factor
    dec_factor_t dec_factor;

    // Read the parameters file
    readParameterFile(parameterFilePath, dec_factor);

    // Open the input test vector file
    std::ifstream inputFile("work/input_test_vector.txt");

    // Open the file to write the simulation results
    std::ofstream outputFile("work/output_csim.txt");

    // Read the input test vector file line by line
    std::string line;

    // Declare the input and output streams
    hls::stream<cdata_t<ssr>> inputStream;
    hls::stream<cdata_t<ssr>> outputStream;

    // Declare the input samples
    cdata_t<ssr> inputSamples;
    cdata_t<ssr> outputSamples;

    if (!inputFile.is_open())
    {
        std::cerr << "Error: could not open file." << std::endl;
        return 1;
    }

    // loop over the input test vector file
    while (std::getline(inputFile, line))
    {
        // Read a line from the input test vector file
        std::istringstream iss(line);

        // Parse the line
        for (size_t i = 0; i < ssr; ++i)
        {
            if (!(iss >> inputSamples.re[i] >> inputSamples.im[i]))
            {
                std::cerr << "Error: failed to parse input." << std::endl;
                break;
            }
        }
       
        // Write the input sample to the input stream
        inputStream.write(inputSamples);

        // Call the top-level function
        ssr_multistage_decimator(inputStream, outputStream, dec_factor);

        // Read the output sample from the output stream
        outputSamples = outputStream.read();
        
        // scale the output samples and write them to the output file
        for (size_t i = 0; i < ssr; ++i)
        {
            // Convert to integer by multiplying with (1 << frac_bits). and write to file
            int intValueRe = static_cast<int>(outputSamples.re[i] * (1 << frac_bits));
            int intValueIm = static_cast<int>(outputSamples.im[i] * (1 << frac_bits));

            outputFile << std::setw(6) << intValueRe << " ";
            outputFile << std::setw(6) << intValueIm;

            if (i < ssr - 1)
            {
                outputFile << " "; // Add space between numbers except after the last one
            }
        }
        outputFile << std::endl;

    }
}

void readParameterFile(const std::string &parameterFilePath, dec_factor_t &dec_factor)
{
    // Open the file
    std::ifstream parameterFile(parameterFilePath);

    // Check if the file is open
    if (!parameterFile.is_open())
    {
        std::cerr << "Error: could not open the file " << parameterFilePath << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read the file line by line
    std::string line;
    while (std::getline(parameterFile, line))
    {
        // Skip empty lines
        if (line.empty())
            continue;

        // Skip comments
        if (line[0] == '#')
            continue;

        // First value is the decimation factor
        dec_factor = std::stoi(line);

        // Print the read values
        std::cout << "Decimation factor: " << dec_factor << std::endl;
    }

    // Close the file
    parameterFile.close();
}