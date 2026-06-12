//
// MaterialBuilder.hh
// Central definition of all materials and their visualization attributes.
//
// Build() defines every material used by any configuration (the Geant4
// material table is global, so unused materials are harmless) and fills the
// supplied map with per-material vis attributes, keyed by material name.
// Density-ratio knobs scale the target gas and external material densities,
// matching the original DefineMaterials() behaviour.
//

#ifndef MaterialBuilder_h
#define MaterialBuilder_h 1

#include "G4String.hh"

#include <map>

class G4VisAttributes;

namespace MaterialBuilder
{
// targetDensityRatio scales the H2/D2 target gas density (and LH2),
// extDensityRatio scales structural material densities.
void Build(double targetDensityRatio, double extDensityRatio,
           std::map<G4String, G4VisAttributes *> &visAtts);
}

#endif
