#!/bin/sh -e

test -d "$INSTALL_ROOT"

echo "$INSTALL_ROOT is a directory"

TZ_DIR="$INSTALL_ROOT/usr/share/zoneinfo"
TIMED="$INSTALL_ROOT/usr/share/tzdata-timed"

mkdir -p "$TZ_DIR/Mobile" "$TIMED"

g++ -O2 simple-dump.cpp -o ./simple-dump
export SIMPLE_DUMP=./simple-dump

./prepare-timed-data.perl \
  --mcc-main=MCC \
  --distinct=distinct.tab \
  --single=single.tab \
  mcc-wikipedia-*.html \
  --single-output=$TIMED/single.data \
  --distinct-output=$TIMED/distinct.data \
  --full-output=$TIMED/olson.data \
  --country-by-mcc-output=$TIMED/country-by-mcc.data \
  --zones-by-country-output=$TIMED/zones-by-country.data

for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 ; do
  ln -s ../Etc/GMT-$i $TZ_DIR/Mobile/UTC+$i
  ln -s ../Etc/GMT+$i $TZ_DIR/Mobile/UTC-$i
done
ln -s ../Etc/GMT $TZ_DIR/Mobile/UTC
ln -s ../Asia/Kolkata "$TZ_DIR/Mobile/UTC+5:30"
