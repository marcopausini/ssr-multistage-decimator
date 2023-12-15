##
# @file run.tcl
# @brief TCL script to configure the project settings for the ssr_multistage_decimator design using HLS tool. 
#        It sets the project, solution, device settings, adds source and testbench files, specifies top module,
#        adjusts clock settings, run synthesis and implementation, and exports the design.
#
# @usage vitis_hls -f run.tcl
#        vitis_hls -i -> interactive mode, then type "source run.tcl"
#        vitis-run --mode hls --tcl run.tcl
#
#
#
# Users should comment certain lines if they wish to exclude testbenches or additional actions such as C simulation,
# co-simulation, and exporting the design.
#
##

set CSYNTH 1
set EXPORT 0

### default setting
set Project     prj_ssr_multistage_decimator
set Solution    solution_0
set Device      "xczu28dr-ffvg1517-2-e"
set Flow        ""
set ClockFreq   160       ;# Set the desired clock frequency in MHz
set Uncertainty 0.3

# Calculate the clock period in nanoseconds from the clock frequency in MHz
set ClockPeriod [expr {1000.0 / $ClockFreq}]

#### main part

# Project settings
open_project $Project -reset

# Set the top directory and use absolute path to fix issue with relative paths
set TopDir [pwd]
set WorkDir "$TopDir/data/work"

# Add the file for synthesis
add_files $TopDir/hw/src/ssr_multistage_decimator.cpp 
add_files $TopDir/hw/src/hb_filters.cpp

# Add testbench files for co-simulation
add_files -tb  $TopDir/hw/tb/tb_ssr_multistage_decimator.cpp

# create the work directory if it does not exist
if {![file exists $WorkDir]} {
    file mkdir $WorkDir
}
# add the folder, as there seems to be a bug in the tool when adding files separately
add_files -tb $WorkDir

# Set top module of the design
set_top ssr_multistage_decimator

# Solution settings
open_solution -reset $Solution

# set Part Number
set_part $Device

# Set the target clock period
create_clock -period $ClockPeriod
set_clock_uncertainty $Uncertainty

###############
## Directives #
##############

# IO interface

# Inform the tool of the variables not inter dependent

# Print scheduling information for debugging
config_schedule -verbose

#################
# C SIMULATION
#################
# source run_csim.tcl

#############
# SYNTHESIS #
#############
if {$CSYNTH == 1} {
    csynth_design
}

#################
# CO-SIMULATION #
#################
# source run_csim.tcl

##################
# IMPLEMENTATION #
##################
if {$EXPORT == 1} {
    export_design -evaluate verilog -format ip_catalog -flow impl
}

#exit
