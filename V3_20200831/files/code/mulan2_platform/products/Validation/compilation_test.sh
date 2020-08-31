#!/bin/sh
# Script to validate compilation of products within SW Platform (as per validation plan)

# Build SW Platform for all projects and profiles

products="12123 12126 12127 12130 14608 14612 14614 80152 80252AA 80252BA 81106_7 81108_9 81150 81300 81310 81315 90294 90363 90365 90809"

cd ../../libsrc

sh build_pltf_all.sh
cd ../products

build_examples()
{
    cd $1/examples
    echo "Building examples for product: $1 .. "
    ./build_examples.bat
    cd ../..
}


for product in $products
    do
        build_examples $product
    done

