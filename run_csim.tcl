
#
#

set Testcase    testcase_decim_1_signal_exponential

set Project     prj_ssr_multistage_decimator
set Solution    solution_0

# check if the project is already opened
if {$Project == [get_project -name]} {
    puts "Project $Project is already opened"
} else {
    # Create a project by sourcing a Tcl file, but skipping all design
    # commands in the Tcl script: cosim_design, csynth_design, and csim_design.
    open_tcl_project run.tcl
}

set TopDir [pwd]
#set ProjDir [get_project -directory]
set WorkDir "$TopDir/data/work"
set TestcaseDir "$TopDir/data/$Testcase"


# copy files from test case directory to work directory
foreach srcFile [glob -nocomplain -directory $TestcaseDir *] {
    if {[catch {file copy -force $srcFile $WorkDir} result]} {
        puts "Failed to copy $srcFile: $result"
    }
}

# use flag -clean to enable a clean build
csim_design -clean

# Copy simulation results to test case directory
foreach pattern {log* output_c*} {
    foreach srcFile [glob -nocomplain -directory $WorkDir $pattern] {
        if {[catch {file copy -force $srcFile $TestcaseDir} result]} {
            puts "Failed to copy $srcFile: $result"
        }
    }
}
