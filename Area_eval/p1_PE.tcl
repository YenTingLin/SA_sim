read_file -format verilog {config.v p1_PE.v}
create_clock -name "clk" -period 10 -waveform { 0 5  }  { clk  }
set_fix_hold clk
set_dont_touch_network clk
uplevel #0 check_design
compile -exact_map
uplevel #0 { report_area } > area_p1_PE.txt
uplevel #0 { report_power -analysis_effort low }
uplevel #0 { report_timing -path full -delay max -nworst 1 -max_paths 1 -significant_digits 2 -sort_by group }
write -hierarchy -format verilog -output p1_PE_syn.v
write_script > p1_PE.dc
write_sdf -version 2.1 -context verilog p1_PE_syn.sdf
exit
#run: dc_shell -f p1_PE.tcl
