#!/bin/bash
# run_regression.sh — fixed-seed regression run for one config.
#
#   tests/regression/run_regression.sh <config> <tag> [nevents] [seed]
#
# Builds nothing; runs ./build/prad2sim with config/<config>.json, a fixed
# seed, and dumps tree stats to tests/regression/out/<config>_<tag>.txt.
# Compare tags with diff, e.g.:
#   diff out/prad_baseline.txt out/prad_refactor.txt

set -e
CONF=${1:?usage: run_regression.sh <config> <tag> [nevents] [seed]}
TAG=${2:?usage: run_regression.sh <config> <tag> [nevents] [seed]}
NEV=${3:-300}
SEED=${4:-12345}

cd "$(dirname "$0")/../.."   # repo root

export ROOTSYS=$HOME/Apps/root
source ~/Apps/geant4/bin/geant4.sh
export ROOTSYS=$HOME/Apps/root
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

MACRO=$(mktemp /tmp/regress_XXXX.mac)
printf '/run/initialize\n/run/beamOn %d\n' "$NEV" > "$MACRO"

# run number bookkeeping: read next run number that prad2sim will use
RUNNO=$(awk '!/^#/{print $2; exit}' output/file.output 2>/dev/null || echo 1)

./build/prad2sim -c "config/${CONF}.json" -s "$SEED" "$MACRO" > /tmp/regress_run.log 2>&1
RC=$?
rm -f "$MACRO"
[ $RC -ne 0 ] && { echo "RUN FAILED rc=$RC (see /tmp/regress_run.log)"; exit $RC; }

OUTFILE=$(ls -t output/*.root | head -1)
mkdir -p tests/regression/out
root -b -q "tests/regression/tree_stats.C(\"$OUTFILE\")" 2>/dev/null \
    | grep -vE 'Welcome|ROOT Team|tags/v|With c\+\+|^---|^\s*\|' \
    > "tests/regression/out/${CONF}_${TAG}.txt"

echo "stats: tests/regression/out/${CONF}_${TAG}.txt  (from $OUTFILE, run#$RUNNO, seed $SEED, $NEV evts)"
