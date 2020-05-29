
DIR_src = src_sc/
DIR_build = build/

exe: main.exe

main.exe: main.o testbench.o CNN.o CNN_ctor.o layer.o padding_in.o synapse.o padding_out.o act.o PE_array_CK.o
	g++ -L$(SYSTEMC_HOME)/lib-linux64 $(DIR_build)main.o $(DIR_build)testbench.o $(DIR_build)CNN.o $(DIR_build)CNN_ctor.o $(DIR_build)layer.o $(DIR_build)padding_in.o $(DIR_build)synapse.o $(DIR_build)padding_out.o $(DIR_build)act.o $(DIR_build)PE_array_CK.o -lsystemc -o $(DIR_build)main.exe

main.o: $(DIR_src)main.cpp $(DIR_src)top.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)main.cpp
	mv main.o $(DIR_build)

testbench.o: $(DIR_src)testbench.cpp $(DIR_src)testbench.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)testbench.cpp
	mv testbench.o $(DIR_build)

CNN.o: $(DIR_src)CNN.cpp $(DIR_src)CNN.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)CNN.cpp
	mv CNN.o $(DIR_build)

CNN_ctor.o: $(DIR_src)CNN_ctor.cpp $(DIR_src)CNN.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)CNN_ctor.cpp
	mv CNN_ctor.o $(DIR_build)

layer.o: $(DIR_src)layer.cpp $(DIR_src)layer.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)layer.cpp
	mv layer.o $(DIR_build)

padding_in.o: $(DIR_src)padding_in.cpp $(DIR_src)padding_in.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)padding_in.cpp
	mv padding_in.o $(DIR_build)

synapse.o: $(DIR_src)synapse.cpp $(DIR_src)synapse.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)synapse.cpp
	mv synapse.o $(DIR_build)

padding_out.o: $(DIR_src)padding_out.cpp $(DIR_src)padding_out.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)padding_out.cpp
	mv padding_out.o $(DIR_build)

act.o: $(DIR_src)act.cpp $(DIR_src)act.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)act.cpp
	mv act.o $(DIR_build)

PE_array_CK.o: $(DIR_src)PE_array_CK.cpp $(DIR_src)PE_array_CK.h $(DIR_src)config.h
	g++ -c -I$(SYSTEMC_HOME)/include $(DIR_src)PE_array_CK.cpp
	mv PE_array_CK.o $(DIR_build)

runexe:
	./$(DIR_build)main.exe

clean:
	rm -r $(DIR_build)

all:
	make clean
	make exe
	make runexe

run:
	rm log.txt
	touch log.txt
	python3 src_py/run.py

rm0:
	rm -r txt_CIFAR10
