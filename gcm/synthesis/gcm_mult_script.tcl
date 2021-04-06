#Set search path and libraries
set search_path ../
#set target_library "NangateOpenCellLibrary_typical_reduced_INV0_XOR0_AND1_NOR2.db"
set target_library "dff_full.lib"
set link_library "$target_library"

#Define design lib
define_design_lib WORK -path "./work"

#Define top level and arch
set TOPCELL_ENT gcm_mult
set TOPCELL_ARCH rtl

##Analyze target design
analyze -library WORK -autoread {./}

#Elaborate design
elaborate $TOPCELL_ENT -architecture $TOPCELL_ARCH -library DEFAULT

#Link design with the library
link

set_dont_touch_network -no_propagate [get_pins *]
set_dont_touch [get_nets {*}]
#Avoid optimization of registers signals

set ungroup_keep_original_design true

compile -exact_map -ungroup_all -only_design_rule

#Verilog netlist
define_name_rules myrules -allowed "A-Za-z0-9_"
change_names -rules myrules -hierarchy
set verilogout_single_bit false
write -format verilog -hierarchy -output gcm_mult.net


