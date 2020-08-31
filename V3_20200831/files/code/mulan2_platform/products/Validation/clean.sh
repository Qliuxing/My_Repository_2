#!/bin/sh
# Clean validation tests

process_example()
{
    echo -n "Product: $product, profile: $profile, example: $example .."
    cd ./$example
    gmake --silent clean PRODUCT=$product PROFILE=$profile 1> /dev/null
    cd ..
    echo " [OK]"
}

. $1

