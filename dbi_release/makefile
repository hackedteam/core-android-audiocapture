# WARNING !! to use enviromental variables an export is needed es:
# export CC=<path to gcc>
#wget http://files.chumby.com/toolchain/arm-2008q3-72-arm-none-linux-gnueabi-i686-pc-linux-gnu.tar.bz2
#CC=~/tools/arm-2008q3/bin/arm-none-linux-gnueabi-gcc
#LD=~/tools/arm-2008q3/bin/arm-none-linux-gnueabi-ld
ifndef CC
	error "CC --  undefined valorize it!"
endif
ifndef LD
	error "LD --  undefined valorize it!"
endif
#if test -z "$$CC" then $(error "CC --  undefined valorize it!"); fi;
#if test -z "$$LD" then $(error "LD undefined valorize it!");  fi;

ifdef CFLAGS
ifneq (,$(findstring -DDEBUG ,$(CFLAGS)))
	#$(info " DEBUG ENABLED!!!")
	LIB_OBJ=libt_debug.so
        LOCAL_LDLIBS += -llog
else
	#$(info " DEBUG NOT ENABLED!!!")
	LIB_OBJ=libt.so
endif
else
	LIB_OBJ=libt.so
endif


all:
	@echo going to use CC=$(CC) and LD=$(LD) LIB_OBJ=$(LIB_OBJ)
	(cd hijack_func;  ./build.sh)
	$(CC) $(CFLAGS) -O2 -c libt.c -o libt.o -Ilibnfc-nxp/inc -Ilibnfc-nxp/src
	$(CC) $(CFLAGS) -O2 -c util.c -o util.o
	$(LD) -s -shared -Llibs $(LOCAL_LDLIBS) -lc -soname,$(LIB_OBJ) -o $(LIB_OBJ)  libt.o util.o hijack_func/*.o
ifeq ($(BUILD_HIJACK),YES)
	make build_hijack
endif
clean:
	rm -f hijack_func/*.o
	rm *.o
	@if test -e libt_debug.so; then rm libt_debug.so ; fi;
	@if test -e libt.so;  then rm libt.so ; fi;
	rm hijacks.h


build_hijack:
	cd hijack/jni
	NDK_PROJECT_PATH="$(shell pwd)/hijack/" CFLAGS="$(CFLAGS)" ndk-build clean
	NDK_PROJECT_PATH="$(shell pwd)/hijack/" CFLAGS="$(CFLAGS)" ndk-build
	cd -
