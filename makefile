SRCDIR := src
BINDIR := bin

all: evaluador 

VPATH := src bin include

LDFLAGS:= -lpthread -lrt
CXXFLAGS := -Wall -Wextra -std=c++11 -I include

$(SRCDIR)/stop.o: stop.cpp elementos.h init.h stop.h 
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(SRCDIR)/reg.o: reg.cpp elementos.h reg.h 
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(SRCDIR)/rep.o: rep.cpp elementos.h rep.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(SRCDIR)/control.o: control.cpp control.h init.h elementos.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(SRCDIR)/init.o: init.cpp elementos.h init.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(SRCDIR)/evaluador.o: evaluador.cpp reg.h init.h rep.h
	$(CXX) -o $@ $(CXXFLAGS) -c $<

$(BINDIR)/evaluador: evaluador.o reg.o init.o stop.o rep.o control.o 
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(SCRDIR)/*.cpp~ makefile~ $(SRCDIR)/*.o $(BINDIR)/evaluador
.PHONY: dirs
dirs:
	@test -d bin || mkdir bin
