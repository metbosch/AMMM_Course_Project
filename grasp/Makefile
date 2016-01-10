all: grasp

grasp: grasp.cpp
	g++ -O3 $? -o $@

grasp-dbg: grasp.cpp
	g++ -g $? -o $@

clean: 
	rm -f grasp grasp-dbg
