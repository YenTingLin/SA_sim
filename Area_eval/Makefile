VLOG        = ncverilog
#PJ          = p1_PE
#PJ          = p2_fifo
#PJ          = p3_avg
#PJ          = p4_act
PJ          = p5_skmul
SRC         = config.v \
              $(PJ)_t.v \
              $(PJ).v
SYNSRC      = px_h.v \
              config.v \
              $(PJ)_t.v \
              $(PJ)_syn.v \
              -v /theda21_2/CBDK_IC_Contest/cur/Verilog/tsmc13.v
VLOGARG     = +access+r

TMPFILE     = *.log \
              ncverilog.key \
              nWaveLog \
              INCA_libs

DBFILE      = *.fsdb *.vcd *.bak

RM          = -rm -rf

all :: sim

sim :
	$(VLOG) $(SRC) $(VLOGARG)

syn :
	$(VLOG) $(SYNSRC) $(VLOGARG)

clean :
	$(RM) $(TMPFILE)

veryclean :
	$(RM) $(TMPFILE) $(DBFILE)

run :
	python3 run.py
