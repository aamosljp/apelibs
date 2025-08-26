#!/usr/bin/env bash

cd "$(dirname "$0")/.." || exit

for d in src/*; do
    echo "Building $(basename "$d")"
    ./tools/generate_from_source.sh "$(basename "$d")"
done
