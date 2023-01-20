#!/bin/bash -x

if -z which nanopb_generator.py; then
    echo "nanopb_generator.py is not on the $PATH"
    exit 1
fi

for i in *.proto; do
    nanopb_generator.py $i
    mv $(basename $i .proto).pb.h include
done
