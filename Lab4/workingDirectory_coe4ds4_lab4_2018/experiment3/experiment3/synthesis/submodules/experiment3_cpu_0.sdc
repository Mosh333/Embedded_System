# Legal Notice: (C)2018 Altera Corporation. All rights reserved.  Your
# use of Altera Corporation's design tools, logic functions and other
# software and tools, and its AMPP partner logic functions, and any
# output files any of the foregoing (including device programming or
# simulation files), and any associated documentation or information are
# expressly subject to the terms and conditions of the Altera Program
# License Subscription Agreement or other applicable license agreement,
# including, without limitation, that your use is for the sole purpose
# of programming logic devices manufactured by Altera and sold by Altera
# or its authorized distributors.  Please refer to the applicable
# agreement for further details.

#**************************************************************
# Timequest JTAG clock definition
#   Uncommenting the following lines will define the JTAG
#   clock in TimeQuest Timing Analyzer
#**************************************************************

#create_clock -period 10MHz {altera_internal_jtag|tckutap}
#set_clock_groups -asynchronous -group {altera_internal_jtag|tckutap}

#**************************************************************
# Set TCL Path Variables 
#**************************************************************

set 	experiment3_cpu_0 	experiment3_cpu_0:*
set 	experiment3_cpu_0_oci 	experiment3_cpu_0_nios2_oci:the_experiment3_cpu_0_nios2_oci
set 	experiment3_cpu_0_oci_break 	experiment3_cpu_0_nios2_oci_break:the_experiment3_cpu_0_nios2_oci_break
set 	experiment3_cpu_0_ocimem 	experiment3_cpu_0_nios2_ocimem:the_experiment3_cpu_0_nios2_ocimem
set 	experiment3_cpu_0_oci_debug 	experiment3_cpu_0_nios2_oci_debug:the_experiment3_cpu_0_nios2_oci_debug
set 	experiment3_cpu_0_wrapper 	experiment3_cpu_0_jtag_debug_module_wrapper:the_experiment3_cpu_0_jtag_debug_module_wrapper
set 	experiment3_cpu_0_jtag_tck 	experiment3_cpu_0_jtag_debug_module_tck:the_experiment3_cpu_0_jtag_debug_module_tck
set 	experiment3_cpu_0_jtag_sysclk 	experiment3_cpu_0_jtag_debug_module_sysclk:the_experiment3_cpu_0_jtag_debug_module_sysclk
set 	experiment3_cpu_0_oci_path 	 [format "%s|%s" $experiment3_cpu_0 $experiment3_cpu_0_oci]
set 	experiment3_cpu_0_oci_break_path 	 [format "%s|%s" $experiment3_cpu_0_oci_path $experiment3_cpu_0_oci_break]
set 	experiment3_cpu_0_ocimem_path 	 [format "%s|%s" $experiment3_cpu_0_oci_path $experiment3_cpu_0_ocimem]
set 	experiment3_cpu_0_oci_debug_path 	 [format "%s|%s" $experiment3_cpu_0_oci_path $experiment3_cpu_0_oci_debug]
set 	experiment3_cpu_0_jtag_tck_path 	 [format "%s|%s|%s" $experiment3_cpu_0_oci_path $experiment3_cpu_0_wrapper $experiment3_cpu_0_jtag_tck]
set 	experiment3_cpu_0_jtag_sysclk_path 	 [format "%s|%s|%s" $experiment3_cpu_0_oci_path $experiment3_cpu_0_wrapper $experiment3_cpu_0_jtag_sysclk]
set 	experiment3_cpu_0_jtag_sr 	 [format "%s|*sr" $experiment3_cpu_0_jtag_tck_path]

#**************************************************************
# Set False Paths
#**************************************************************

set_false_path -from [get_keepers *$experiment3_cpu_0_oci_break_path|break_readreg*] -to [get_keepers *$experiment3_cpu_0_jtag_sr*]
set_false_path -from [get_keepers *$experiment3_cpu_0_oci_debug_path|*resetlatch]     -to [get_keepers *$experiment3_cpu_0_jtag_sr[33]]
set_false_path -from [get_keepers *$experiment3_cpu_0_oci_debug_path|monitor_ready]  -to [get_keepers *$experiment3_cpu_0_jtag_sr[0]]
set_false_path -from [get_keepers *$experiment3_cpu_0_oci_debug_path|monitor_error]  -to [get_keepers *$experiment3_cpu_0_jtag_sr[34]]
set_false_path -from [get_keepers *$experiment3_cpu_0_ocimem_path|*MonDReg*] -to [get_keepers *$experiment3_cpu_0_jtag_sr*]
set_false_path -from *$experiment3_cpu_0_jtag_sr*    -to *$experiment3_cpu_0_jtag_sysclk_path|*jdo*
set_false_path -from sld_hub:*|irf_reg* -to *$experiment3_cpu_0_jtag_sysclk_path|ir*
set_false_path -from sld_hub:*|sld_shadow_jsm:shadow_jsm|state[1] -to *$experiment3_cpu_0_oci_debug_path|monitor_go
