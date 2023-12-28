#
# @file    run_csim.tcl
# @brief   Automates the simulation process for ssr_multistage_decimator
#
# This script handles test setup, including copying necessary files, running clean simulations,
# and copying results back to the designated directories. It ensures correct project handling
# and execution of different simulation stages such as C simulation and co-simulation.
#
# @author  marco.pausini@ast-science.com
# @date 2023-11-xy
# @version 0.1
#
##

# open project and keep/remove any exisiting data
set resetProject true

set CSIM true
set COSIM false

# choose single or multi test case
set selection "single"

set projectName     prj_ssr_multistage_decimator
set solutionName    solution_0


# ------------------------------------------------------------
# Procedures
# ------------------------------------------------------------

# Set the Testcase variable to a list of test case names
proc setTestcases { selection } {
    switch -exact -- $selection {
        "single" {
            return [list testcase_decim_2_signal_exponential]
        }
        "multi" {
            return [list \
                testcase_decim_1_signal_exponential \
                testcase_decim_2_signal_exponential \
                testcase_decim_4_signal_exponential \
                testcase_decim_8_signal_exponential \
                testcase_decim_16_signal_exponential \
                testcase_decim_32_signal_exponential \
                testcase_decim_64_signal_exponential \
            ]
    }
    default {
        puts "Invalid selection"
        exit
    }
}
}

# Procedure to check if the project is already open
proc openProjectIfNeeded { projectName openCommand } {
    # Replace 'get_project' with proper command or mechanism to get the currently open project
    if {$projectName == [get_project -name]} {
        puts "Project $projectName is already opened"
    } else {
        eval $openCommand
    }
}

#################
# Main workflow
#################
# check if project is already open and - if not, create it from TCL script
#openProjectIfNeeded $projectName "open_tcl_project run.tcl"

if {$resetProject} {
    puts "Resetting project $projectName"
    open_tcl_project run.tcl
}

set topDir [pwd]
# @workDir: directory added to the project for simulation
set workDir "$topDir/data/work"


set Testcases [setTestcases $selection]

foreach testcase $Testcases {

    puts "Processing $testcase"

    set testcaseDir "$topDir/data/$testcase"

    puts "Test case directory is $testcaseDir"
    puts "Work directory is $workDir"

    # ------------------------------------------------------------
    # Delete files OF the previous testcase
    # ------------------------------------------------------------

    # Use glob to match files and then delete them in the work directory
    puts "Deleting files in work directory $workDir"
    foreach file [glob -nocomplain -- $workDir/output_c*] {
        file delete -force -- $file
    }
    foreach file [glob -nocomplain -- $workDir/log*] {
        file delete -force -- $file
    }
    foreach file [glob -nocomplain -- $workDir/*.csv] {
        file delete -force -- $file
    }

    puts "Deleting files in testcase directory $testcaseDir"
    # Use glob to match files and then delete them in the testcase directory
    foreach file [glob -nocomplain -- $testcaseDir/output_c*] {
        file delete -force -- $file
    }
    foreach file [glob -nocomplain -- $testcaseDir/log*] {
        file delete -force -- $file
    }

    # ------------------------------------------------------------
    # Copy files from test case directory to the work directory
    # ------------------------------------------------------------

    # copy files from test case directory to work directory
    puts "copy files from $testcaseDir to work directory $workDir"
    foreach srcFile [glob -directory $testcaseDir *] {
        if {[catch {file copy -force $srcFile $workDir} result]} {
            puts "Failed to copy $srcFile: $result"
        } else {
            puts "Copied $srcFile"
        }
    }


    #######################
    # Perform C Simulation
    #######################
    if {$CSIM} {
        # When C simulation completes, a csim folder is created inside the solution folder. This folder contains the following elements:
        #  * csim/build: The primary location for all files related to the C simulation
        #      ** Any files read by the test bench are copied to this folder.
        #      ** The C executable file csim.exe is created and run in this folder.
        #      ** Any files written by the test bench are created in this folder.
        #  * csim/report: Contains a log file of the C simulation build and run.

        #  @buildDir csim/build/work:
        set buildDir "$topDir/$projectName/$solutionName/csim/build/work"

        # use flag -clean to enable a clean build
        set csimResult [catch {csim_design -clean -profile} csimOutput]
        #csim_design -clean

        puts "copy simulation results from $buildDir to $testcaseDir"
        # Copy simulation results to test case directory
        foreach pattern {log* output_c*} {
            foreach srcFile [glob -directory $buildDir $pattern] {
                if {[catch {file copy -force $srcFile $testcaseDir} result]} {
                    puts "Failed to copy $srcFile: $result"
                }
            }
        }
        puts "C simulation $testcase ended"
    }


    #######################
    # Perform Co-Simulation
    #######################
    if {$COSIM} {
        # When C/RTL Cosimulation completes, the sim folder is created inside the solution folder.
        # This folder contains the following elements:
        #   * The sim/report folder contains the report and log file for each type of RTL simulated

        # the tool re-use the files used for C simulation

        # Run the Co-Simulation
        cosim_design -rtl verilog -trace_level all

        # -----------------------------------------------
        # Copy the co-simulatior report and log files to the testcase directory
        # -----------------------------------------------
        set simReportFile "${projectName}/${solutionName}/sim/report/ssr_multistage_decimator_cosim.rpt"
        set simLogFile "${projectName}/${solutionName}/sim/report/verilog/ssr_multistage_decimator.log"

        #
        set destinationFile [file join $testcaseDir [file tail $simReportFile]]
        file copy -force $simReportFile $destinationFile
        if { [file exists $destinationFile] } {
            puts "File copied successfully to $destinationFile."
        } else {
            puts "Error: File could not be copied."
        }

        #
        set destinationFile [file join $testcaseDir [file tail $simLogFile]]
        file copy -force $simLogFile $destinationFile
        if { [file exists $destinationFile] } {
            puts "File copied successfully to $destinationFile."
        } else {
            puts "Error: File could not be copied."
        }

        puts "C/RTL cosimulation $testcase ended"
    }


}
