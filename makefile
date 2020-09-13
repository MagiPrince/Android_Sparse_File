GCC :=
param=-Wall -Wextra -fsanitize=address -fsanitize=undefined -fsanitize=leak
UNAME_S := $(shell uname -s)
sparse:= ./tests/image-test/test.simg
image:= ./tests/image-test/test.img

ifeq ($(UNAME_S), Darwin)
	GCC+=gcc-8
else
	GCC+=gcc
endif

all: sparse2img img2sparse reading_tests writing_tests

sparse2img: sparse2img.o functions.o
	$(GCC) $(param) $^ -o $@
img2sparse: img2sparse.o functions.o
	$(GCC) $(param) $^ -o $@
reading_tests: reading_tests.o functions.o
	$(GCC) $(param) $^ -o $@
writing_tests: writing_tests.o functions.o
	$(GCC) $(param) $^ -o $@

# Creation of the .o file
sparse2img.o: sparse2img.c
	$(GCC) $(param) -c $< -o $@
img2sparse.o: img2sparse.c
	$(GCC) $(param) -c $< -o $@
functions.o: ./library/functions.c
	$(GCC) $(param) -c $< -o $@
reading_tests.o: ./tests/reading_tests.c
	$(GCC) $(param) -c $< -o $@
writing_tests.o: ./tests/writing_tests.c
	$(GCC) $(param) -c $< -o $@

test:
	./test.sh

clean:
	rm -f *.o sparse2img img2sparse reading_tests writing_tests sortie.*