// tree_stats.C — dump per-leaf statistics of the simulation truth tree "T"
// to stdout in a stable, diffable text format.
//
// Used for refactor regression checks: run a fixed-seed job before and after
// a change, dump stats of both files, and diff the text.
//
//   root -b -q 'tests/regression/tree_stats.C("output/simrun_1.root")' > before.txt
//   root -b -q 'tests/regression/tree_stats.C("output/simrun_2.root")' > after.txt
//   diff before.txt after.txt
//
// For identical geometry/physics/seed the dumps must match exactly.

#include <TFile.h>
#include <TTree.h>
#include <TH1D.h>
#include <TLeaf.h>
#include <TObjArray.h>
#include <cstdio>

void tree_stats(const char *fname)
{
    TFile f(fname);
    TTree *t = (TTree *)f.Get("T");

    if (!t) {
        printf("ERROR: no tree T in %s\n", fname);
        return;
    }

    printf("entries %lld\n", t->GetEntries());

    TObjArray *leaves = t->GetListOfLeaves();

    for (int i = 0; i < leaves->GetSize(); i++) {
        TLeaf *leaf = (TLeaf *)leaves->At(i);

        if (!leaf) continue;

        const char *name = leaf->GetName();

        // Project the leaf (flattens arrays); cap values into a wide histogram
        t->Draw(Form("%s>>hstats(1000)", name), "", "goff");
        TH1D *h = (TH1D *)gDirectory->Get("hstats");

        if (!h) {
            printf("%-24s no-histo\n", name);
            continue;
        }

        printf("%-24s n=%-10.0f mean=%-15.8g rms=%-15.8g\n",
               name, h->GetEntries(), h->GetMean(), h->GetRMS());
        delete h;
    }
}
