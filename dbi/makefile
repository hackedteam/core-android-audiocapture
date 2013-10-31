# LD=~/tools/arm-2008q3/bin/arm-none-linux-gnueabi-ld
# CC=~/tools/arm-2008q3/bin/arm-none-linux-gnueabi-gcc

CC=/Users/alberto/Documents/Devel/Android/android-ndk//toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-gcc
LD=/Users/alberto/Documents/Devel/Android/android-ndk//toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-ld

all:
	(cd hijack_func; ./build.sh)
	$(CC) -g -c libt.c -o libt.o 
	$(CC) -g -c util.c -o util.o
	$(LD) -shared -Llibs -lc -soname,libt.so -o libt.so libt.o util.o hijack_func/*.o

clean:
	rm -f hijack_func/*.o
	rm *.o libt.so
	rm hijacks.h
