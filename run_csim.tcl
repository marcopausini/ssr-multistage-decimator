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

set CSIM true
set COSIM false

# choose single or multi test case
set selection "single"

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


set projectName     prj_ssr_multistage_decimator
set solutionName    solution_0

set Testcases [setTestcases $selection]

# Procedure to check if the project is already open
proc openProjectIfNeeded { projectName openCommand } {
    # Replace 'get_project' with proper command or mechanism to get the currently open project
    if {$projectName == [get_project -name]} {
        puts "Project $projectName is already opened"
    } else {
        eval $openCommand
    }
}


# Procedure to perform C simulation
proc CSimulation { testcaseDir workDir } {

    # copy files from test case directory to work directory
    foreach srcFile [glob -directory $testcaseDir *] {
        if {[catch {file copy -force $srcFile $workDir} result]} {
            puts "Failed to copy $srcFile: $result"
        }
    }

    # use flag -clean to enable a clean build
    set csimResult [catch {csim_design -clean} csimOutput]
    #csim_design -clean

    puts "copy files simulation"
    # Copy simulation results to test case directory
    foreach pattern {output_c*} {
        foreach srcFile [glob -directory $workDir $pattern] {
            if {[catch {file copy -force $srcFile $testcaseDir} result]} {
                puts "Failed to copy $srcFile: $result"
            }
        }
    }
}

# Procedure to perform Co-Simulation
proc COSimulation { testcaseDir projectName solutionName } {

    set reportDir "./$projectName/$solutionName/syn/report/"
    set reportFile "${reportDir}ssr_multistage_decimator_csynth.rpt"

    if { ![file exists $reportFile] } {
        puts "Synthesis not yet completed or report not found. Running csynth_design"
        csynth_design
    } else {
        puts "Synthesis has been completed"
    }

    # Run the Co-Simulation
    cosim_design -rtl verilog -trace_level port

    # Copy the Co-Simulation log to the testcase directory
    set simLogFile "${projectName}/${solutionName}/sim/report/verilog/ssr_multistage_decimator.log"
    copyFiles ./ $testcaseDir $simLogFile

}


#################
# Main workflow
#################
openProjectIfNeeded $projectName "open_tcl_project run.tcl"

set topDir [pwd]
set workDir "$topDir/data/work"

foreach testcase $Testcases {
    puts "Processing $testcase"

    set testcaseDir "$topDir/data/$testcase"

    #######################
    # Perform C Simulation
    #######################
    if {$CSIM} {
        puts $testcaseDir
        puts $workDir
        CSimulation $testcaseDir $workDir
    }

    #######################
    # Perform Co-Simulation
    #######################
    if {$COSIM} {
        COSimulation $testcaseDir $projectName $solutionName
    }
}
