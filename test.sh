#!/bin/bash

sparse=./tests/image-test/test.simg
image=./tests/image-test/test.img

make clean --silent

test_ok=1

# Reading functions test
echo "-------- Compiling reading_tests..."
make reading_tests --silent
if [ $? -eq 0 ]; then
    echo "-------- Compiling reading_tests Ok !"
    echo "======== Executing reading_tests"
    ./reading_tests $sparse sortie.img
else
    echo "Error while compiling the reading_tests program"
    make clean
    exit 1
fi

# Are the tests ok ?
if [ $? -eq 0 ]; then
    echo "======== End of reading_tests : Tests Ok !"
else
    test_ok=0
    echo ""
    echo "======== Error while testing reading functions"
fi


# Writing functions test
echo ""
echo "-------- Compiling writing_tests..."
make writing_tests --silent
if [ $? -eq 0 ]; then
    echo "-------- Compiling writing_tests Ok !"
    echo "======== Executing writing_tests"
    ./writing_tests $image sortie.simg
else
    echo "Error while compiling the writing_tests program"
fi

# Are the tests ok ?
if [ $? -eq 0 ]; then
    echo "========  End of writing_tests : Tests Ok !"
else
    test_ok=0
    echo ""
    echo "======== Error while testing writing functions"
fi

make clean --silent

if [ $test_ok -eq 0 ]; then
    exit 1
fi

# Test compression
echo ""
echo "-------- Compiling img2sparse..."
make img2sparse --silent

if [ $? -eq 0 ]; then
    echo "-------- Compiling img2sparse Ok !"
else
    echo "Error while compiling the img2sparse program"
fi

echo ""
echo "-------- Compiling sparse2img..."
make sparse2img --silent

if [ $? -eq 0 ]; then
    echo "-------- Compiling sparse2img Ok !"
else
    echo "Error while compiling the sparse2img program"
fi

echo ""
echo "======== Executing img2sparse"
./img2sparse ./tests/image-test/test.img sortie.simg

if [ $? -eq 0 ]; then
    echo "======== End of compression : Compression Ok !"
else
    echo "======== Error during compression"
fi
echo ""
echo "======== Executing sparse2img"
./sparse2img sortie.simg sortie.img

if [ $? -eq 0 ]; then
    echo "======== End of decompression : Decompression Ok !"
else
    echo "======== Error during decompression"
fi

diff sortie.img ./tests/image-test/test.img

#Test of the pipe
echo ""
echo "======== Testing the pipe"

echo "  ==> test.img | ./img2sparse | ./sparse2img > sortie.img"
cat ./tests/image-test/test.img | ./img2sparse | ./sparse2img > sortie.img
diff sortie.img ./tests/image-test/test.img

echo "  ==> test.simg | ./sparse2img | ./img2sparse > sortie.simg"
cat ./tests/image-test/test.simg | ./sparse2img | ./img2sparse > sortie.simg
diff sortie.simg ./tests/image-test/notre-test.simg

echo "======== Pipe test finish"

echo ""
make clean --silent
echo "Cleaning services at work..."

exit 0