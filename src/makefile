#SRCDIR := src
#BINDIR := bin
all: evaluador delMem

#VPATH := src include bin

LDFLAGS= -pthread #-lrt

#$(SCRDIR)/reg.o: reg.cpp elementos.h reg.h 
stop.o: stop.cpp stop.h
#$(SCRDIR)/reg.o: reg.cpp elementos.h reg.h 
reg.o: reg.cpp elementos.h reg.h externas.h
	$(CXX) -o $@ $(LDFLAGS) -c $<

rep.o: rep.cpp elementos.h rep.h
	$(CXX) -o $@ $(LDFLAGS) -c $<

#$(SCRDIR)/init.o: init.cpp elementos.h init.h
init.o: init.cpp elementos.h init.h
	$(CXX) -o $@ $(LDFLAGS) -c $<

#$(SCRDIR)/evaluador.o: evaluador.cpp reg.h
evaluador.o: evaluador.cpp reg.h init.h rep.h
	$(CXX) -o $@ $(LDFLAGS) -c $<

#$(BINDIR)/evaluador: evaluador.o reg.o init.o #delMem.o 
evaluador: evaluador.o reg.o init.o stop.o rep.o#delMem.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

#.PHONY: clean

clean:
	rm -f *.o
	rm -f *.*~ *~
	rm -f evaluador delMem
