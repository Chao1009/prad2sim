#!/bin/bash
# Merge per-thread prad2sim output files.
# Usage: ./merge.sh [-f] [run=<N|N M ...|all>]
#   -f              force overwrite of existing merged file
#   run=<N>         merge run N (overrides the RUN value below)
#   run=<N M ...>   merge multiple runs (quote the list: run="1 2 3")
#   run=all         auto-discover all runs that have _t files

# ── Configure run number(s) here ──────────────────────────────────────────
RUN=1
# ──────────────────────────────────────────────────────────────────────────

OUTDIR=output
FORCE=0

for arg in "$@"; do
    case "$arg" in
        -f)     FORCE=1 ;;
        run=*)  RUN="${arg#run=}" ;;
    esac
done

# Discover run numbers when RUN=all
if [[ "$RUN" == "all" ]]; then
    discovered=()
    for f in "${OUTDIR}"/simrun_*_t[0-9]*.root; do
        [[ -e "$f" ]] || continue
        n=$(basename "$f" | sed -n 's/^simrun_\([0-9]*\)_t.*/\1/p')
        [[ -n "$n" ]] && discovered+=("$n")
    done
    if [[ ${#discovered[@]} -eq 0 ]]; then
        echo "No thread files found in ${OUTDIR}/"
        exit 1
    fi
    # Deduplicate and sort
    RUN=$(printf '%s\n' "${discovered[@]}" | sort -un | tr '\n' ' ')
    echo "Discovered run numbers: ${RUN}"
fi

merge_run() {
    local run="$1"
    local MERGED="${OUTDIR}/simrun_${run}_merged.root"
    local PARTS=("${OUTDIR}"/simrun_${run}_t[0-9]*.root)

    if [[ ! -e "${PARTS[0]}" ]]; then
        echo "No thread files found for run ${run} in ${OUTDIR}/"
        return 1
    fi

    local NPARTS=${#PARTS[@]}

    if [[ -f "$MERGED" && "$FORCE" -eq 0 ]]; then
        echo "Skipping run ${run}: ${MERGED} already exists (use -f to overwrite)"
        return 0
    fi

    echo "Merging run ${run}: ${NPARTS} thread files -> ${MERGED}"

    local PARTS_ENTRIES
    PARTS_ENTRIES=$(root -b -l -q -e "
TChain ch(\"T\");
$(for f in "${PARTS[@]}"; do echo "ch.Add(\"$f\");"; done)
printf(\"%lld\n\", ch.GetEntries());
" 2>/dev/null | tail -1)

    echo "Thread files total entries: ${PARTS_ENTRIES}"

    local ADD_CMDS=""
    for f in "${PARTS[@]}"; do
        ADD_CMDS+="ch.Add(\"$f\");"
    done

    root -b -l -q -e "
TChain ch(\"T\");
${ADD_CMDS}
ch.Merge(\"${MERGED}\");
" 2>/dev/null

    if [[ ! -f "$MERGED" ]]; then
        echo "ERROR: Merged file was not created for run ${run}. Thread files NOT deleted."
        return 1
    fi

    local MERGED_ENTRIES
    MERGED_ENTRIES=$(root -b -l -q -e "
TFile ff(\"${MERGED}\",\"READ\");
TTree *t=(TTree*)ff.Get(\"T\");
if(t) printf(\"%lld\n\", t->GetEntries());
else printf(\"0\n\");
ff.Close();
" 2>/dev/null | tail -1)

    echo "Merged file entries:        ${MERGED_ENTRIES}"

    local THRESHOLD
    THRESHOLD=$(echo "$PARTS_ENTRIES * 99 / 100" | bc)

    if [[ "$MERGED_ENTRIES" -ge "$THRESHOLD" ]]; then
        echo "Entry check passed (${MERGED_ENTRIES} >= 99% of ${PARTS_ENTRIES}). Deleting thread files."
        rm -f "${PARTS[@]}"
        echo "Done. Merged file: ${MERGED}"
    else
        echo "WARNING: Merged entries (${MERGED_ENTRIES}) < 99% of thread total (${PARTS_ENTRIES})."
        echo "Thread files NOT deleted. Please inspect ${MERGED}."
        return 1
    fi
}

FAILED=0
for run in $RUN; do
    merge_run "$run" || FAILED=1
done
[[ "$FAILED" -eq 0 ]] || exit 1
