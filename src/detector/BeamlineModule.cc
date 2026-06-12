//
// BeamlineModule.cc
// Faithful port of the upstream collimator and beam pipe from
// DetectorPRad.cc.
//

#include "detector/BeamlineModule.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

BeamlineModule::BeamlineModule(double targetCenter)
    : DetectorModule("Beamline"), fTargetCenter(targetCenter)
{
}

void BeamlineModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *UCollimatorM = G4Material::GetMaterial("Nickel");
    G4Material *VacuumTubeM = G4Material::GetMaterial("SSteel");

    // Upstream collimator
    // Dimension from PRad beam line drawing (search PRad in JLab drawing database)
    G4double UCollimatorHalfL = 11.8 * 2.54 / 2.0 * cm;
    G4double UCollimatorOR = 3.9 * 2.54 / 2.0 * cm;
    G4double UCollimatorIR = 12.7 * mm / 2.0;
    G4double UCollimatorCenter = fTargetCenter - 2.03 * m + UCollimatorHalfL;
    G4VSolid *solidUCollimator = new G4Tubs("UCollimatorS", UCollimatorIR, UCollimatorOR, UCollimatorHalfL, 0, twopi);
    G4LogicalVolume *logicUCollimator = new G4LogicalVolume(solidUCollimator, UCollimatorM, "UCollimatorLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, UCollimatorCenter), logicUCollimator, "Upstream Collimator", world, false, 0);

    // Upstream beam pipe
    // Dimension from PRad target drawings and PRad beam line drawings
    G4double UBeamPipeIR = 1.87 * 2.54 / 2.0 * cm;
    G4double UBeamPipeOR = 2.0 * 2.54 / 2.0 * cm;
    G4double UBeamPipeHalfL = 0.79 * m;
    G4double UBeamPipeOffset = 0.105 * m;
    G4double UBeamPipeCenter = fTargetCenter - UBeamPipeOffset - UBeamPipeHalfL - 4 * cm; // subtract 4cm for target length
    G4VSolid *solidUBeamPipe = new G4Tubs("UBeamPipeS", UBeamPipeIR, UBeamPipeOR, UBeamPipeHalfL, 0, 2 * pi);
    G4LogicalVolume *logicUBeamPipe = new G4LogicalVolume(solidUBeamPipe, VacuumTubeM, "UBeamPipeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, UBeamPipeCenter), logicUBeamPipe, "Upstream Beam Pipe", world, false, 0);
}
