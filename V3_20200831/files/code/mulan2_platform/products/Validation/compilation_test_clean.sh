#!/bin/sh

products="12123 12126 12127 12130 14608 14612 14614 80152 80252AA 80252BA 81106_7 81108_9 81150 81300 81310 81315 90294 90363 90365 90809"

cd ../../libsrc

sh clean_pltf_all.sh
cd ../products

clean_examples()
{
    cd $1/examples
    echo "Cleaning examples for product: $1 .. "
    ./clean_examples.bat
    cd ../..
}


for product in $products
    do
        clean_examples $product
    done

