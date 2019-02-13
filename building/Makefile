

### Compilers and tools
CXX			=	g++
CC			=	gcc

### Flags
ROOTFLAGS	=	$(shell root-config --cflags)
ROOTLIBS	=	$(shell root-config --libs)
CFLAGS		=	-pipe -Wall -W -fPIC -m64 -O3 -march=native
CXXFLAGS	=	-pthread -std=c++11 -m64 -fPIC -Wall -W -O3 -march=native $(ROOTFLAGS)
INCLUDES	=	-I../TDR2tree
LIBS		=	$(ROOTLIBS)

### Files
SRCDIR		=	../TDR2tree
CXXSOURCES 	=	$(notdir $(wildcard ../TDR2tree/*.cpp))
CCSOURCES	=	$(notdir $(wildcard ../TDR2tree/*.c))
CXXOBJECTS	=	$(CXXSOURCES:.cpp=.o)
CCOBJECTS	=	$(CCSOURCES:.c=.o)

TARGET		=	TDR2tree

all: $(TARGET)

$(TARGET): $(CXXOBJECTS) $(CCOBJECTS)
	$(CXX) -o $@ $^ $(LIBS)	

%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f *.o $(TARGET)