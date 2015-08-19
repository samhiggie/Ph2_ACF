ROOTFLAGS=`root-config --cflags`
ROOTLIBDIR=`root-config --libdir`
ROOTLIBFLAGS=`root-config --libs`
ROOTLIBFLAGS+=-lHistPainter
INCLUDEFLAGS=-I$(BOOST_INCLUDE)

BOOSTLIBFLAGS=-L$(BOOST_LIB)
BOOSTLIBFLAGS+=-L$(BOOST_LIB) -lboost_system$(BOOST_SUFFIX) -lboost_filesystem$(BOOST_SUFFIX) -lboost_program_options$(BOOST_SUFFIX)
GLIBFLAGS=`root-config --glibs`
INCLUDEFLAGS+=-Iinclude/
SRCDIR=src
INCDIR=include
LIBDIR=lib
# BINDIR=bin
# TESTDIR=test
COMPILERFLAGS+=-std=c++11 
COMPILERFLAGS+=-g -fPIC
COMPILERFLAGS+=-fpermissive
LINKERFLAGS+=-Wl,--copy-dt-needed-entries

CXX := g++

COMP=$(CXX) $(COMPILERFLAGS) $(INCLUDEFLAGS)

LINK=$(CXX) $(LINKERFLAGS)

all: lib

# ROOT-related stuff
$(LIBDIR)/rootweb.o: $(SRCDIR)/rootweb.cpp $(INCDIR)/rootweb.hh
	$(COMP) $(ROOTFLAGS) -c -o $(LIBDIR)/rootweb.o $(SRCDIR)/rootweb.cpp

$(LIBDIR)/publisher.o: $(SRCDIR)/publisher.cpp
	$(COMP) $(ROOTFLAGS) -c $(SRCDIR)/publisher.cpp -o $(LIBDIR)/publisher.o

# Binary
lib: $(LIBDIR)/rootweb.o $(LIBDIR)/publisher.o 
	$(COMP) -shared -o $(LIBDIR)/libRootWeb.so $(LIBDIR)/rootweb.o $(LIBDIR)/publisher.o -pthread $(BOOSTLIBFLAGS)

#CLEANUP
cleanall:
	@rm -rf $(LIBDIR)/*

clean: cleanall

