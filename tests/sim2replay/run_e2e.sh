#!/bin/bash
# run_e2e.sh — end-to-end data-interface validation:
#   prad2sim (prad2 config) -> truth tree T -> prad2ana_sim2replay -> recon tree
#
# Needs: built ./build/prad2sim, the prad2 toolkit in PATH or ~/Apps/prad2,
# and the CAD models staged (database/CADmodel/README.md) — missing STLs only
# reduce material fidelity, the chain still runs.
#
#   tests/sim2replay/run_e2e.sh [nevents]
#
# Validated 2026-06-12 (400+400 events, 3.5 GeV): 673 recon entries,
# all cl_center resolved against hycal_map.json, 316 HyCal-GEM matched pairs.
# Matching is radius-limited (~r<200 mm) by a sim2replay approximation: it
# labels cluster positions with cl_z=6225 mm while the sim VD plane sits at
# z=5907 mm, so projected positions leave the 10 mm window at large radius.
# Upstream 1-line fix in prad2evviewer/analysis/tools/sim2replay.cpp:
# use sim->VD_z[j] for cl_z instead of the hycal_z constant.

set -e
NEV=${1:-400}
cd "$(dirname "$0")/../.."   # repo root

export ROOTSYS=$HOME/Apps/root
source ~/Apps/geant4/bin/geant4.sh
export ROOTSYS=$HOME/Apps/root
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH

SIM2REPLAY=$(command -v prad2ana_sim2replay || echo "$HOME/Apps/prad2/bin/prad2ana_sim2replay")
WORK=$(mktemp -d /tmp/p2s_e2e_XXXX)
mkdir -p "$WORK/ep" "$WORK/ee"

echo ">>> ep (elastic) sample: $NEV events"
printf '/run/initialize\n/run/beamOn %d\n' "$NEV" > "$WORK/ep.mac"
./build/prad2sim -c config/prad2.json -s 1111 "$WORK/ep.mac" > "$WORK/ep.log" 2>&1
cp "$(ls -t output/*.root | head -1)" "$WORK/ep/ep.root"

echo ">>> ee (moller) sample: $NEV events"
printf '/prad2sim/gun/evtype moller\n/run/initialize\n/run/beamOn %d\n' "$NEV" > "$WORK/ee.mac"
./build/prad2sim -c config/prad2.json -s 2222 "$WORK/ee.mac" > "$WORK/ee.log" 2>&1
cp "$(ls -t output/*.root | head -1)" "$WORK/ee/ee.root"

echo ">>> sim2replay"
(cd "$WORK" && "$SIM2REPLAY" ep ee 1.0 1.0 -o prad2_sim_recon.root)

echo ">>> validation"
root -b -q -e "
TFile f(\"$WORK/prad2_sim_recon.root\"); TTree*t=(TTree*)f.Get(\"recon\");
if(!t){printf(\"FAIL: no recon tree\n\"); exit(1);}
printf(\"recon entries=%lld\n\", t->GetEntries());
t->Draw(\"cl_center>>hc(10,-2.5,7.5)\",\"cl_center<0\",\"goff\");
printf(\"unresolved cl_center: %.0f (want 0)\n\", ((TH1*)gDirectory->Get(\"hc\"))->GetEntries());
t->Draw(\"match_num>>hm(8,-0.5,7.5)\",\"\",\"goff\");
printf(\"matched pairs total: %.0f\n\", ((TH1*)gDirectory->Get(\"hm\"))->GetMean()*t->GetEntries());
" 2>/dev/null | grep -E 'recon|unresolved|matched'

echo "recon file: $WORK/prad2_sim_recon.root  (open with prad2evviewer / analysis)"
