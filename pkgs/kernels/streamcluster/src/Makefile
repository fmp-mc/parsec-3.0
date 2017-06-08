# Makefile for streamcluster

PREFIX=${PARSECDIR}/pkgs/kernels/streamcluster/inst/${PARSECPLAT}

TARGET=streamcluster
OBJS=streamcluster.o

ifdef version
  ifeq "$(version)" "pthreads"
    CXXFLAGS :=	$(CXXFLAGS) -DENABLE_THREADS -pthread
    OBJS += parsec_barrier.o
  endif
  ifeq "$(version)" "tbb"
    CXXFLAGS := $(CXXFLAGS) -DTBB_VERSION
    LIBS := $(LIBS) -ltbb
  endif
endif

ifndef BUILD_PARSEC_FOR_LINUX
all: clean $(OBJS)
	#$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
	#
	# Generate a static library instead of executable.
	# 							-- ertl-liyixiao
	#
	$(AR) -rcs $(TARGET).a $(OBJS)
else
all: clean $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) $(LIBS) -o $(TARGET)
endif

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f *.o $(TARGET)

install:
	#mkdir -p $(PREFIX)/bin
	#cp -f $(TARGET) $(PREFIX)/bin/$(TARGET)

