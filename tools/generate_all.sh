#!/usr/bin/env bash

cd $(dirname $0)/..

for d in src/*; do
    echo "Building $(basename $d)"
    ./tools/generate_from_source.sh "$(basename $d)"
done
