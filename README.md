# ssr-multistage-decimator
Hardware design and FPGA-based implementation of a super sample rate multistage decimator using AMD Vivado Design Suite

## Directory Structure

- `doc/`: Documentation files and design specifications.
- `hw/`: Hardware design sources including source code, IPs, and constraints.
  - `src/`: VHDL or Verilog source files for the FPGA design.
  - `tb/`: Testbench files
  - `ip/`: Custom and third-party IP cores used in the design.
  - `constraints/`: Constraint files for synthesis and implementation.
    - `synthesis/`: Constraints specific to synthesis.
    - `implementation/`: Constraints specific to implementation.
- `matlab/`: MATLAB models and scripts for signal generation and verification
- `data/`: Input signals and simulation outptuts
- `scripts/`: Automation scripts like TCL scripts, Makefiles, etc.
  - `build/`: Scripts used for compiling and building the project.
  - `utils/`: Utility scripts for various tasks.
- `xdc/`: Xilinx Design Constraints files.
- `proj/`: Vivado project files including `.xpr`, `.runs`, `.srcs`, etc.
- `output/`: Generated output products like bitstreams, reports, logs, etc.

## Signal Processing Details

The input signal is sampled at a rate of 1280 Mega-Samples Per Second (MSPS). The system clock driving this process works at a frequency of 160 MHz. The design supports multiple decimation factors: 1, 2, 4, 8, 16, 32, and 64. Depending on the chosen decimation factor, the output signal sampling rate will vary accordingly. Below are the output rates for each decimation factor:

| Decimation Factor | Output Rate (MSPS) |
|-------------------|--------------------|
| 1                 | 1280               |
| 2                 | 640                |
| 4                 | 320                |
| 8                 | 160                |
| 16                | 80                 |
| 32                | 40                 |
| 64                | 20                 |

### Anti-Aliasing Low-Pass Filter Specifications

The anti-aliasing low-pass filter is a half-band filter (HBF) designed with the following specifications:

- **Passband Frequency (`Fpass`)**: ![equation](https://latex.codecogs.com/gif.latex?\dpi{110}{\pi}\{2.56})
- **Stopband Frequency (`Fstop`)**: ![equation](https://latex.codecogs.com/gif.latex?\dpi{110}{\pi}\{1.641})
- **Passband Ripple**: `0.01 dB`
- **Stopband Attenuation (`att`)**: `60 dB`

### Decimation Factor Selection

Select the appropriate decimation factor based on the input signal bandwidth, according to the following table:

| Bandwidth [MHz] | Dec Factor | Fpass   | Fstop |
|-----------------|------------|---------|-------|
| 1000            | 1          | NA      | NA    |
| 500             | 2          | 250     | 390   |
| 250             | 4          | 125     | 195   |
| 125             | 8          | 62.5    | 97    |
| 62.5            | 16         | 31.25   | 48    |
| 31.25           | 32         | 15.625  | 24    |
| 15.625          | 64         | 7.8125  | 12    |


Feel free to explore the directories and their contents for a deeper understanding of the project's hardware design and structure.

The decimation factor should be selected based on the input signal bandwidth desired output rate for your specific application.
