#!/bin/sh -e

rm -rf OUTPUT

mkdir OUTPUT

g++ -g -O0 simple-dump.cpp -o OUTPUT/simple-dump

export SIMPLE_DUMP=OUTPUT/simple-dump

./prepare-timed-data.perl \
  --mcc-main=MCC \
  --distinct=distinct.tab \
  --single=single.tab \
  mcc-wikipedia-*.html \
  --single-output=OUTPUT/single.data \
  --distinct-output=OUTPUT/distinct.data \
  --full-output=OUTPUT/olson.data \
  --country-by-mcc-output=OUTPUT/country-by-mcc.data \
  --zones-by-country-output=OUTPUT/zones-by-country.data

