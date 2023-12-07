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

The anti-aliasing low-pass filter is a half-band filter (HBF) designed using the DSP Matlab toolbox, and with the following specifications:

- **Passband Frequency (`Fpass`)**: ![equation](https://latex.codecogs.com/gif.latex?\dpi{110}\frac{\pi}{2.56})
- **Stopband Frequency (`Fstop`)**: ![equation](https://latex.codecogs.com/gif.latex?\dpi{110}\frac{\pi}{1.641})
- **Passband Ripple**: `0.01 dB`
- **Stopband Attenuation (`att`)**: `60 dB`

The filter is symmetric with order 30, and only 17 coefficients are different from zero. The decimation filters frequency response are available in the `doc\` folder.

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


To improve readability and apply the correct format for a README.md file, we will structure the information under appropriate headings, include a brief description, usage instructions, pre-requisites if any, and a proper command format. Here's an example of how you could lay out the README content:

## Simulation

This section provides instructions on how to run simulations using the provided MATLAB scripts.

### Create Input and Reference Signals

The MATLAB scripts `tb_run.m` and `tb_run_all.m` are used to generate input and output reference signals for the simulation of our system. `tb_run.m` is responsible for running a single simulation, while `tb_run_all.m` executes all simulations in a batch process.

To generate the reference signals, follow these steps:

1. Open MATLAB and navigate to the directory containing `tb_run.m` and `tb_run_all.m`.
2. Set variable `usecase = 'a'`
3. Run a single simulation by executing the following command in the MATLAB Command Window:

   ```matlab
   tb_run
   ```

4. To run all simulations, execute this command instead:

   ```matlab
   tb_run_all
   ```

These scripts create an instance of the Matlab class `MatlabTestBench.m` and call its methods. Alternatively, the user can create the testbench object directly.

### Run C/RTL Simulation

### Validate Results

---

By structuring it in this way, the README.md file becomes more organized and user-friendly, guiding users through the necessary steps clearly and efficiently.

Feel free to explore the directories and their contents for a deeper understanding of the project's hardware design and structure.

The decimation factor should be selected based on the input signal bandwidth desired output rate for your specific application.
