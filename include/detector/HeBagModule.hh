//
// HeBagModule.hh
// DRad helium bag between the two GEM stations.
//

#ifndef HeBagModule_h
#define HeBagModule_h 1

#include "detector/DetectorModule.hh"

class G4LogicalVolume;

class HeBagModule : public DetectorModule
{
public:
    HeBagModule(double gemCenter0, double gemCenter1);

    void BuildVolumes(G4LogicalVolume *world) override;

private:
    double fGEMCenter0, fGEMCenter1;
};

#endif
