//
// BeamlineModule.hh
// Upstream beamline: nickel collimator + stainless steel beam pipe
// (PRad / PRad-II / X17). Dimensions from PRad beam line drawings.
//

#ifndef BeamlineModule_h
#define BeamlineModule_h 1

#include "detector/DetectorModule.hh"

class G4LogicalVolume;

class BeamlineModule : public DetectorModule
{
public:
    explicit BeamlineModule(double targetCenter);

    void BuildVolumes(G4LogicalVolume *world) override;

private:
    double fTargetCenter;
};

#endif
