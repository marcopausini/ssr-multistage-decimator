export VIVADO_PATH=/tools/Xilinx/Vivado/2023.1
export VITIS_PATH=/tools/Xilinx/Vitis/2023.1
export VITIS_HLS_PATH=/tools/Xilinx/Vitis_HLS/2023.1
export PETALINUX_PATH=/tools/Xilinx/PetaLinux/2023.1
export PETALINUX_SDK=/technology/fpga/Xilinx/Petalinux/2023.1
export DSPLIB_ROOT=/technology/fpga/Xilinx/Vitis_2023.1_Libraries/dsp

echo "source $VIVADO_PATH/settings64.sh"
source $VIVADO_PATH/settings64.sh
echo "source $VITIS_PATH/settings64.sh"
source $VITIS_PATH/settings64.sh
echo "source $VITIS_HLS_PATH/settings64.sh"
source $VITIS_HLS_PATH/settings64.sh
# Verify Installation
echo "Vitis Installation:"
which vitis
which vitis_hls
which vivado

