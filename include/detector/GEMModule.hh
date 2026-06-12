//
// GEMModule.hh
// GEM tracker stations. One module instance owns ALL stations so a single
// TrackingDetectorSD (branch prefix "GEM") spans them.
//
// Each station is a pair of side-by-side chambers ("GEM n L" / "GEM n R",
// copy numbers 0/1) separated by 39.71 mm in z; the station container
// carries copy number 2*station. TrackingDetectorSD derives the detector
// ID as the sum of copy numbers along the touchable history, so hits get
//   DID = (L/R = 0/1) + 2*station  ->  0..3 for the two PRad-II stations,
// matching the gem_z[4] convention of the prad2 sim2replay tool.
//
// Internals styles:
//  - kPRad1: PRad-I chamber stack (DetectorCommon.cc AddGEM); the sensitive
//    volume is the 50 um cathode foil ("GEM%dCathodeLV").
//  - kPRad2: PRad-II chamber stack (PRadSim_PRad2 AddGEM): 4-layer
//    Al/Kapton/Nomex/Kapton/Al-Kapton windows and a 3 mm ArCO2 drift gap
//    ("GEM%dDriftGasLV") as the sensitive volume.
//

#ifndef GEMModule_h
#define GEMModule_h 1

#include "detector/DetectorModule.hh"

#include <vector>

class G4LogicalVolume;

class GEMModule : public DetectorModule
{
public:
    enum class Style { kPRad1, kPRad2 };

    struct Station {
        double center;  // world z of the station center
        bool culess;    // build without copper layers (PRad-I GEM0 in DRad)
    };

    GEMModule(Style style, std::vector<Station> stations, bool sdOn);

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    void AddStationPRad1(G4LogicalVolume *mother, int layerid, double center, bool culess);
    void AddStationPRad2(G4LogicalVolume *mother, int layerid, double center, bool culess);

    Style fStyle;
    std::vector<Station> fStations;
    bool fSdOn;

    std::vector<G4LogicalVolume *> fSensitiveLVs;
};

#endif
