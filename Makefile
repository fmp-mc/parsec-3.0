#
# Makefile for a workspace of EV3 Platform.
#

swaptions: clean
	parsecmgmt -a uninstall -p swaptions
	parsecmgmt -a build -p swaptions

swaptions-linux-cfs: clean
	parsecmgmt -a uninstall -p swaptions -c tile-gcc
	parsecmgmt -a build -p swaptions -c tile-gcc

swaptions-linux-fifo: clean
	parsecmgmt -a uninstall -p swaptions -c tile-gcc-fifo
	parsecmgmt -a build -p swaptions -c tile-gcc-fifo

dedup: clean
	parsecmgmt -a uninstall -p dedup
	parsecmgmt -a build -p dedup

dedup-linux-cfs: clean
	parsecmgmt -a uninstall -p dedup -c tile-gcc
	parsecmgmt -a build -p dedup -c tile-gcc

dedup-linux-fifo: clean
	parsecmgmt -a uninstall -p dedup -c tile-gcc-fifo
	parsecmgmt -a build -p dedup -c tile-gcc-fifo

streamcluster:
	cp pkgs/kernels/streamcluster/src/parsec_barrier.cpp.ORIG pkgs/kernels/streamcluster/src/parsec_barrier.cpp
	cp pkgs/kernels/streamcluster/src/parsec_barrier.hpp.ORIG pkgs/kernels/streamcluster/src/parsec_barrier.hpp
	parsecmgmt -a uninstall -p streamcluster
	parsecmgmt -a build -p streamcluster

streamcluster-spin-barrier:
	cp pkgs/kernels/streamcluster/src/parsec_barrier.cpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.cpp
	cp pkgs/kernels/streamcluster/src/parsec_barrier.hpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.hpp
	parsecmgmt -a uninstall -p streamcluster
	export PORTABILITY_FLAGS="-DDISABLE_AUTOMATIC_DROPIN"; \
	parsecmgmt -a build -p streamcluster

streamcluster-linux-cfs:
	parsecmgmt -a uninstall -p streamcluster -c tile-gcc
	parsecmgmt -a build -p streamcluster -c tile-gcc

streamcluster-linux-fifo:
	parsecmgmt -a uninstall -p streamcluster -c tile-gcc-fifo
	parsecmgmt -a build -p streamcluster -c tile-gcc-fifo

streamcluster-linux-cfs-spin-barrier:
	cp pkgs/kernels/streamcluster/src/parsec_barrier.cpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.cpp
	cp pkgs/kernels/streamcluster/src/parsec_barrier.hpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.hpp
	parsecmgmt -a uninstall -p streamcluster -c tile-gcc
	export PORTABILITY_FLAGS="-DDISABLE_AUTOMATIC_DROPIN"; \
	parsecmgmt -a build -p streamcluster -c tile-gcc

streamcluster-linux-fifo-spin-barrier:
	cp pkgs/kernels/streamcluster/src/parsec_barrier.cpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.cpp
	cp pkgs/kernels/streamcluster/src/parsec_barrier.hpp.SPIN_BARRIER pkgs/kernels/streamcluster/src/parsec_barrier.hpp
	parsecmgmt -a uninstall -p streamcluster -c tile-gcc-fifo
	export PORTABILITY_FLAGS="-DDISABLE_AUTOMATIC_DROPIN"; \
	parsecmgmt -a build -p streamcluster -c tile-gcc-fifo

fluidanimate: clean
	parsecmgmt -a build -p fluidanimate

blackscholes: clean
	parsecmgmt -a uninstall -p blackscholes
	parsecmgmt -a build -p blackscholes

blackscholes-linux-cfs: clean
	parsecmgmt -a uninstall -p blackscholes -c tile-gcc
	parsecmgmt -a build -p blackscholes -c tile-gcc

blackscholes-linux-fifo: clean
	parsecmgmt -a uninstall -p blackscholes -c tile-gcc-fifo
	parsecmgmt -a build -p blackscholes -c tile-gcc-fifo

usage:
	@echo make app="<folder>"
	@echo make mod="<folder>"

clean:

realclean: clean
	parsecmgmt -a fulluninstall -p all
	parsecmgmt -a fullclean -p all
	#rm -rf $(notdir $(OBJFILENAME)) uImage app $(LIBKERNELDIR)/libkernel.a


.PHONY: clean realclean app appmod

