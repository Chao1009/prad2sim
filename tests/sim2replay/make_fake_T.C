// make_fake_T.C — synthesize a prad2sim-style truth tree "T" carrying exactly
// the branches prad2's `sim2replay` reads, to validate the sim -> recon data
// interface WITHOUT needing a Geant4 build.
//
// Contract (from prad2evviewer/analysis/tools/sim2replay.cpp):
//   tree name : "T"
//   VD.N/I, VD.X/Y/Z/P [VD.N]/D   -- virtual plane at HyCal front; P used as energy (MeV)
//   GEM.N/I, GEM.DID[GEM.N]/I,
//   GEM.X/Y/Z, GEM.Xout/Yout/Zout, GEM.Edep [GEM.N]/D
//
// Positions are in the HyCal/GEM-centered lab frame, mm (sim2replay overrides z
// with its own hycal_z=6225 / gem_z constants, so only x,y and det id matter).
// sim2replay hardcodes beamE = 3500 MeV with cuts VD.P > beamE/300 and
// total > 0.5*beamE, so VD.P must be ~O(GeV) for clusters to survive.
//
// Usage:
//   root -b -q 'make_fake_T.C("ep/ep.root",1)'   // 1 cluster/event (elastic ep)
//   root -b -q 'make_fake_T.C("ee/ee.root",2)'   // 2 clusters/event (Moller ee)
//   prad2ana_sim2replay ep ee 1e6 1e6 -o sim_recon.root
//
// Validated 2026-06-12: sim2replay read this tree and produced a recon tree
// with correct cl_energy / cl_center (real PrimEx module IDs) / n_gem_hits.

void make_fake_T(const char *fname, int nclus, int nevt = 200, double E = 2000.)
{
    TFile f(fname, "RECREATE");
    TTree t("T", "fake sim truth");

    const int MAXN = 500;
    int    VD_N;
    double VD_X[MAXN], VD_Y[MAXN], VD_Z[MAXN], VD_P[MAXN];
    int    GEM_N, GEM_DID[MAXN];
    double GEM_X[MAXN], GEM_Y[MAXN], GEM_Z[MAXN];
    double GEM_Xo[MAXN], GEM_Yo[MAXN], GEM_Zo[MAXN], GEM_E[MAXN];

    t.Branch("VD.N", &VD_N, "VD.N/I");
    t.Branch("VD.X", VD_X, "VD.X[VD.N]/D");
    t.Branch("VD.Y", VD_Y, "VD.Y[VD.N]/D");
    t.Branch("VD.Z", VD_Z, "VD.Z[VD.N]/D");
    t.Branch("VD.P", VD_P, "VD.P[VD.N]/D");
    t.Branch("GEM.N", &GEM_N, "GEM.N/I");
    t.Branch("GEM.DID", GEM_DID, "GEM.DID[GEM.N]/I");
    t.Branch("GEM.X", GEM_X, "GEM.X[GEM.N]/D");
    t.Branch("GEM.Y", GEM_Y, "GEM.Y[GEM.N]/D");
    t.Branch("GEM.Z", GEM_Z, "GEM.Z[GEM.N]/D");
    t.Branch("GEM.Xout", GEM_Xo, "GEM.Xout[GEM.N]/D");
    t.Branch("GEM.Yout", GEM_Yo, "GEM.Yout[GEM.N]/D");
    t.Branch("GEM.Zout", GEM_Zo, "GEM.Zout[GEM.N]/D");
    t.Branch("GEM.Edep", GEM_E, "GEM.Edep[GEM.N]/D");

    const double hycal_z = 6225.;
    const double gz[4] = {5407 + 19.855, 5407 - 19.855, 5807 + 19.855, 5807 - 19.855};
    TRandom3 rng(42);

    for (int e = 0; e < nevt; e++) {
        VD_N = nclus;
        GEM_N = 0;
        for (int c = 0; c < nclus; c++) {
            double x = rng.Uniform(-200, 200), y = rng.Uniform(-200, 200);
            VD_X[c] = x; VD_Y[c] = y; VD_Z[c] = hycal_z; VD_P[c] = E;
            for (int d = 0; d < 4; d++) {          // one GEM hit per plane on the ray from origin
                double s = gz[d] / hycal_z;
                GEM_DID[GEM_N] = d;
                GEM_X[GEM_N] = x * s; GEM_Y[GEM_N] = y * s; GEM_Z[GEM_N] = gz[d];
                GEM_Xo[GEM_N] = x * s; GEM_Yo[GEM_N] = y * s; GEM_Zo[GEM_N] = gz[d];
                GEM_E[GEM_N] = 1e-3;
                GEM_N++;
            }
        }
        t.Fill();
    }
    t.Write();
    f.Close();
    printf("wrote %s : %d events, %d clusters/event\n", fname, nevt, nclus);
}
