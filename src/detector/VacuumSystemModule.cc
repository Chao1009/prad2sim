//
// VacuumSystemModule.cc
// kPRad1: faithful port of DetectorCommon.cc AddVaccumBox().
// kPRad2: faithful port of PRadSim_PRad2 AddVaccumBox() (programmatic parts;
//         the CAD window assembly lives in CadInsertsModule), with the
//         optional X17 beam-pipe shielding from PRadSim_X17.
//

#include "detector/VacuumSystemModule.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4Trd.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

VacuumSystemModule::VacuumSystemModule(Style style, double targetCenter, double worldSizeZ,
                                       double sciHouseCenter, bool useHeBag,
                                       bool useShielding, double gemCenter0, double gemCenter1)
    : DetectorModule("VacuumSystem"), fStyle(style), fTargetCenter(targetCenter),
      fWorldSizeZ(worldSizeZ), fSciHouseCenter(sciHouseCenter), fUseHeBag(useHeBag),
      fUseShielding(useShielding), fGEMCenter0(gemCenter0), fGEMCenter1(gemCenter1)
{
}

void VacuumSystemModule::BuildVolumes(G4LogicalVolume *world)
{
    if (fStyle == Style::kX17)
        BuildX17(world);
    else if (fStyle == Style::kPRad2)
        BuildPRad2(world);
    else
        BuildPRad1(world);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void VacuumSystemModule::BuildPRad1(G4LogicalVolume *mother)
{
    G4Material *ChamberM = G4Material::GetMaterial("Aluminum");
    G4Material *ChamberWindowM = G4Material::GetMaterial("Kapton");
    G4Material *VacuumBoxM = G4Material::GetMaterial("Aluminum");
    G4Material *VacuumTubeM = G4Material::GetMaterial("SSteel");

    // Target chamber
    // For now, only built the downstream chamber with window
    // The downstream chamber window should locate at -3000.0 + 89.0 + 74.0  = -2837.0 mm
    // The length of the downstream chamber is 381.7 mm
    // The total length of the downstream chamber and the tube in total is 710.0 mm
    // Here the downstream chamber and the tube are built together to be the new downstream chamber.
    // So the center of this geometry should be at -2837.0 + 710.0 / 2 = -2482.0 mm
    G4double DownChamberCenter = fTargetCenter + 74.0 * mm + 71.0 * cm / 2.0;
    G4double DownChamberHalfL = 71.0 / 2.0 * cm;
    G4double DownChamberUR = 8.00 * cm;

    // Downstream chamber
    G4double rInnerDC[] = {7.56 * cm, 7.56 * cm, 7.56 * cm, 7.56 * cm, 17.30 * cm, 17.30 * cm};
    G4double rOuterDC[] = {8.00 * cm, 8.00 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm};
    G4double zPlaneDC[] = {0, 32.83 * cm, 32.83 * cm, 35.37 * cm, 35.37 * cm, 71.00 * cm};
    G4VSolid *solidDownChamber = new G4Polycone("DownstreamChamberS", 0, twopi, 6, zPlaneDC, rInnerDC, rOuterDC);
    G4LogicalVolume *logicDownChamber = new G4LogicalVolume(solidDownChamber, ChamberM, "DownstreamChamberLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL), logicDownChamber, "Downstream Chamber", mother, false, 0);

    // Downstream chamber window
    G4double DownChamberApertureR = 22.8 * mm;
    G4double DownChamberWinThickness = 7.5 * um;
    G4Tubs *solidDownChamberWin = new G4Tubs("DownstreamChamberWindowS", DownChamberApertureR, DownChamberUR, DownChamberWinThickness / 2.0, 0, twopi);
    G4LogicalVolume *logicDownChamberWin = new G4LogicalVolume(solidDownChamberWin, ChamberWindowM, "DownstreamChamberWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL - DownChamberWinThickness / 2.0), logicDownChamberWin, "Downstream Chamber Window", mother, false, 0);

    // Vacuum box
    // The length of the vacuum box is 4251.7 mm
    // So the center of this geometry should be at -3000.0 + 89.0 + 74.0 + 710.0 + 2125.85 = -1.15 mm
    G4double VacBoxCenter = fTargetCenter + 74.0 * mm + 71.0 * cm + 425.17 * cm / 2.0;
    G4double VacBoxHalfL = 425.17 * cm / 2.0;
    G4double VacBoxMaxR = 78.11 * cm;
    G4double rInner2[] = {17.30 * cm, 17.30 * cm, 50.17 * cm, 50.17 * cm, 78.11 * cm, 78.11 * cm};
    G4double rOuter2[] = {17.78 * cm, 17.78 * cm, 50.80 * cm, 50.80 * cm, 78.74 * cm, 78.74 * cm};
    G4double zPlane2[] = {0, 6.8 * cm, 17.6 * cm, 215.3 * cm, 229.5 * cm, 425.17 * cm};
    G4VSolid *solidVacBox = new G4Polycone("VacuumBoxS", 0, twopi, 6, zPlane2, rInner2, rOuter2);
    G4LogicalVolume *logicVacBox = new G4LogicalVolume(solidVacBox, VacuumBoxM, "VacuumBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter - VacBoxHalfL), logicVacBox, "Vacuum Box", mother, false, 0);

    // Vacuum box window
    G4double VacBoxWinFlangeOffset = 3.81 * cm;
    G4double ArcDistance = 5.59 * cm;
    G4double ArcEndR = (ArcDistance * ArcDistance + VacBoxMaxR * VacBoxMaxR) / (2 * ArcDistance);
    G4double ArcEndThickness = 1.6 * mm;
    G4double VacBoxWinApertureR = 3.0 * cm;
    G4VSolid *solidVacBoxWin = new G4Sphere("VacuumBoxWindowS", ArcEndR - ArcEndThickness, ArcEndR, 0, twopi, pi - asin(VacBoxMaxR / ArcEndR), asin(VacBoxMaxR / ArcEndR) - asin((VacBoxWinApertureR + 0.1 * mm) / ArcEndR));
    G4LogicalVolume *logicVacBoxWin = new G4LogicalVolume(solidVacBoxWin, VacuumBoxM, "VacuumBoxWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter + VacBoxHalfL + ArcEndR - ArcDistance - VacBoxWinFlangeOffset), logicVacBoxWin, "Vacuum Box Window", mother, false, 0);

    // Vacuum Tube
    G4double VacTubeOR = 1.9 * cm;
    G4double VacTubeIR = VacTubeOR - 0.12446 * cm; // 0.049 in = 0.12446 cm from Eugene
    G4double VacTubeL = fWorldSizeZ - 10.0 * cm - VacBoxCenter - VacBoxHalfL + ArcDistance;
    G4VSolid *solidVacTube = new G4Tubs("VacuumTubeS", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
    G4LogicalVolume *logicVacTube = new G4LogicalVolume(solidVacTube, VacuumTubeM, "VacuumTubeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fWorldSizeZ - 10.0 * cm - VacTubeL / 2.0), logicVacTube, "Vacuum Tube", mother, false, 0);

    // Flange on vacuum tube
    G4double FlangeOR = VacBoxWinApertureR;
    G4double FlangeIR = VacTubeOR;
    G4double FlangeHalfL = 0.5 * cm;
    G4VSolid *solidFlange = new G4Tubs("FlangeS", FlangeIR, FlangeOR, FlangeHalfL, 0, twopi);
    G4LogicalVolume *logicFlange = new G4LogicalVolume(solidFlange, VacuumTubeM, "FlangeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter + VacBoxHalfL - ArcDistance + FlangeHalfL), logicFlange, "Flange", mother, false, 0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void VacuumSystemModule::BuildPRad2(G4LogicalVolume *mother)
{
    G4Material *ChamberM = G4Material::GetMaterial("Aluminum");
    G4Material *ChamberWindowM = G4Material::GetMaterial("Kapton");
    G4Material *VacuumBoxM = G4Material::GetMaterial("Aluminum");
    G4Material *VacuumTubeM = G4Material::GetMaterial("Aluminum");
    G4Material *HeTubeM = G4Material::GetMaterial("Kapton");
    G4Material *TubeGasM = G4Material::GetMaterial("HeGas");

    // Downstream chamber, extended by 89.3 mm with a cutout for the
    // scintillator housing (ported from PRadSim_PRad2)
    G4double DownChamberCenter = fTargetCenter + 74.0 * mm + 71.0 * cm / 2.0 + 89.3 * mm / 2.;
    G4double DownChamberHalfL = 71.0 / 2.0 * cm + 89.3 * mm / 2.;
    G4double DownChamberUR = 8.00 * cm;

    G4double rInnerDC[] = {7.56 * cm, 7.56 * cm, 7.56 * cm, 7.56 * cm, 17.30 * cm, 17.30 * cm};
    G4double rOuterDC[] = {8.00 * cm, 8.00 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm};
    G4double zPlaneDC[] = {0, 32.83 * cm, 32.83 * cm, 35.37 * cm, 35.37 * cm, 71.00 * cm + 89.3 * mm};
    G4VSolid *solidDownChamber = new G4Polycone("DownstreamChamberS", 0, twopi, 6, zPlaneDC, rInnerDC, rOuterDC);
    G4Tubs *scintillatorTube = new G4Tubs("sTube", 0, 30. * cm, 92.2 * mm * 0.5 + 0.1 * mm, 0, twopi);
    G4SubtractionSolid *solidDownChamber2 = new G4SubtractionSolid("DownstreamChamberS1", solidDownChamber, scintillatorTube, 0, G4ThreeVector(0, 0, fSciHouseCenter - DownChamberCenter + DownChamberHalfL - 0.35 * mm));
    G4LogicalVolume *logicDownChamber = new G4LogicalVolume(solidDownChamber2, ChamberM, "DownstreamChamberLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL), logicDownChamber, "Downstream Chamber", mother, false, 0);

    // Downstream chamber window
    G4double DownChamberApertureR = 22.8 * mm;
    G4double DownChamberWinThickness = 7.5 * um;
    G4Tubs *solidDownChamberWin = new G4Tubs("DownstreamChamberWindowS", DownChamberApertureR, DownChamberUR, DownChamberWinThickness / 2.0, 0, twopi);
    G4LogicalVolume *logicDownChamberWin = new G4LogicalVolume(solidDownChamberWin, ChamberWindowM, "DownstreamChamberWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL - DownChamberWinThickness / 2.0), logicDownChamberWin, "Downstream Chamber Window", mother, false, 0);

    // Vacuum box (PRad-II position)
    G4double VacBoxCenter = fTargetCenter + 5125. * mm - 425.17 * cm / 2.0;
    G4double VacBoxHalfL = 425.17 * cm / 2.0;
    G4double rInner2[] = {17.30 * cm, 17.30 * cm, 50.17 * cm, 50.17 * cm, 78.11 * cm, 78.11 * cm};
    G4double rOuter2[] = {17.78 * cm, 17.78 * cm, 50.80 * cm, 50.80 * cm, 78.74 * cm, 78.74 * cm};
    G4double zPlane2[] = {0, 6.8 * cm, 17.6 * cm, 215.3 * cm, 229.5 * cm, 425.17 * cm};
    G4VSolid *solidVacBox = new G4Polycone("VacuumBoxS", 0, twopi, 6, zPlane2, rInner2, rOuter2);
    G4LogicalVolume *logicVacBox = new G4LogicalVolume(solidVacBox, VacuumBoxM, "VacuumBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter - VacBoxHalfL), logicVacBox, "Vacuum Box", mother, false, 0);

    if (!fUseHeBag) {
        // Small aluminum vacuum tube to HyCal (dimensions from Bob and Eugene).
        // The CAD window assembly that couples it to the vacuum box is added
        // by CadInsertsModule from the configuration.
        G4double VacTubeOR = 1.375 * 2.54 * cm * 0.5;
        G4double VacTubeIR = VacTubeOR - 0.035 * 2.54 * cm;
        G4double VacTubeL = 2387.6 * mm;
        G4VSolid *solidVacTube = new G4Tubs("VacuumTubeS", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
        G4LogicalVolume *logicVacTube = new G4LogicalVolume(solidVacTube, VacuumTubeM, "VacuumTubeLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 6469.06 * mm), logicVacTube, "Vacuum Tube", mother, false, 0);
    } else {
        // He bag: kapton tube filled with He gas + thick window with hole
        G4double move = -1280. * mm;
        G4double VacTubeOR = 1.375 * 2.54 * cm * 0.5;
        G4double VacTubeIR = VacTubeOR - 0.075 * mm;
        G4double VacTubeL = 2745. * mm;
        G4VSolid *solidHeTube = new G4Tubs("VacuumTubeS", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
        G4LogicalVolume *logicHeTube = new G4LogicalVolume(solidHeTube, HeTubeM, "HeTubeLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 7736.93 * mm + move), logicHeTube, "He Tube", mother, false, 0);
        G4VSolid *solidHeGas = new G4Tubs("HeGasTubeS", 0, VacTubeIR, VacTubeL / 2.0, 0, twopi);
        G4LogicalVolume *logicHeGas = new G4LogicalVolume(solidHeGas, TubeGasM, "HeGasLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 7736.93 * mm + move), logicHeGas, "He Gas", mother, false, 0);

        G4VSolid *solidThickWindow = new G4Tubs("ThickWindowS", 7.5 * mm, 22. * mm, 0.55 * mm / 2.0, 0, twopi);
        G4LogicalVolume *logicThickWindow = new G4LogicalVolume(solidThickWindow, VacuumBoxM, "Vacuum Window with hole");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 6361.11 * mm + move), logicThickWindow, "Vacuum Window with hole", mother, false, 0);

        G4VSolid *solidHoleWindow = new G4Tubs("HoleWindowS", 0, 17.5 * mm, 0.03 * mm / 2.0, 0, twopi);
        G4LogicalVolume *logicHoleWindow = new G4LogicalVolume(solidHoleWindow, VacuumBoxM, "Vacuum Window Hole Window");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 6361.11 * mm + 0.9 * mm + move), logicHoleWindow, "Vacuum Hole Window", mother, false, 0);
    }

    BuildShielding(mother);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void VacuumSystemModule::BuildX17(G4LogicalVolume *mother)
{
    // Faithful port of PRadSim_X17 AddVaccumBox(): straight downstream
    // chamber, vacuum box ending at target + 6405.82 mm, He-bag tube with
    // thick/hole exit windows. The CAD window adapter (Solid008/009/010 from
    // the X17 model set, z offset 0) comes from the configuration via
    // CadInsertsModule.
    G4Material *ChamberM = G4Material::GetMaterial("Aluminum");
    G4Material *VacuumBoxM = G4Material::GetMaterial("Aluminum");
    G4Material *VacuumTubeM = G4Material::GetMaterial("Kapton");
    G4Material *TubeGasM = G4Material::GetMaterial("HeGas");

    // Downstream chamber (straight, 163.2255 cm)
    G4double DownChamberCenter = fTargetCenter + 521.865 * mm + 163.2255 * cm / 2.0;
    G4double DownChamberHalfL = 163.2255 / 2.0 * cm;

    G4double rInnerDC[] = {17.30 * cm, 17.30 * cm, 17.30 * cm, 17.30 * cm, 17.30 * cm, 17.30 * cm};
    G4double rOuterDC[] = {17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm};
    G4double zPlaneDC[] = {0, 32.83 * cm, 32.83 * cm, 35.37 * cm, 35.37 * cm, 163.2255 * cm};
    G4VSolid *solidDownChamber = new G4Polycone("DownstreamChamberS", 0, twopi, 6, zPlaneDC, rInnerDC, rOuterDC);
    G4LogicalVolume *logicDownChamber = new G4LogicalVolume(solidDownChamber, ChamberM, "DownstreamChamberLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, DownChamberCenter - DownChamberHalfL), logicDownChamber, "Downstream Chamber", mother, false, 0);

    // Vacuum box (X17 position)
    G4double VacBoxCenter = fTargetCenter + 6405.82 * mm - 425.17 * cm / 2.0;
    G4double VacBoxHalfL = 425.17 * cm / 2.0;
    G4double rInner2[] = {17.30 * cm, 17.30 * cm, 50.17 * cm, 50.17 * cm, 78.11 * cm, 78.11 * cm};
    G4double rOuter2[] = {17.78 * cm, 17.78 * cm, 50.80 * cm, 50.80 * cm, 78.74 * cm, 78.74 * cm};
    G4double zPlane2[] = {0, 6.8 * cm, 17.6 * cm, 215.3 * cm, 229.5 * cm, 425.17 * cm};
    G4VSolid *solidVacBox = new G4Polycone("VacuumBoxS", 0, twopi, 6, zPlane2, rInner2, rOuter2);
    G4LogicalVolume *logicVacBox = new G4LogicalVolume(solidVacBox, VacuumBoxM, "VacuumBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VacBoxCenter - VacBoxHalfL), logicVacBox, "Vacuum Box", mother, false, 0);

    // He Bag (dimension from Bob or Chris)
    G4double VacTubeOR = 1.375 * 2.54 * cm * 0.5;
    G4double VacTubeIR = VacTubeOR - 0.075 * mm;
    G4double VacTubeL = 2745. * mm;
    G4VSolid *solidHeTube = new G4Tubs("VacuumTubeS", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
    G4LogicalVolume *logicHeTube = new G4LogicalVolume(solidHeTube, VacuumTubeM, "HeTubeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 7736.93 * mm), logicHeTube, "He Tube", mother, false, 0);
    G4VSolid *solidHeGas = new G4Tubs("HeGasTubeS", 0, VacTubeIR, VacTubeL / 2.0, 0, twopi);
    G4LogicalVolume *logicHeGas = new G4LogicalVolume(solidHeGas, TubeGasM, "HeGasLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 7736.93 * mm), logicHeGas, "He Gas", mother, false, 0);

    G4VSolid *solidThickWindow = new G4Tubs("ThickWindowS", 7.5 * mm, 22. * mm, 0.55 * mm / 2.0, 0, twopi);
    G4LogicalVolume *logicThickWindow = new G4LogicalVolume(solidThickWindow, VacuumBoxM, "Vacuum Window with hole");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 6361.11 * mm), logicThickWindow, "Vacuum Window with hole", mother, false, 0);

    G4VSolid *solidHoleWindow = new G4Tubs("HoleWindowS", 0, 17.5 * mm, 0.03 * mm / 2.0, 0, twopi);
    G4LogicalVolume *logicHoleWindow = new G4LogicalVolume(solidHoleWindow, VacuumBoxM, "Vacuum Window Hole Window");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 6361.11 * mm + 0.9 * mm), logicHoleWindow, "Vacuum Hole Window", mother, false, 0);

    BuildShielding(mother);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void VacuumSystemModule::BuildShielding(G4LogicalVolume *mother)
{
    // Rectangular shielding boxes around the beam pipe (ported from PRadSim_X17)
    if (fUseShielding) {
        G4Material *ShieldingM = G4Material::GetMaterial("Aluminum");

        G4double ShieldTubeL1 = 22.0 * cm;
        G4double ShieldTubeL2 = 26.0 * cm;
        G4double ShieldCenter1 = fGEMCenter0 - 70.0 * mm - ShieldTubeL1 / 2.0;
        G4double ShieldCenter2 = (fGEMCenter0 + fGEMCenter1) / 2.0;

        G4double ShieldBoxThickness = 1.0 * mm;
        G4double ShieldBoxHalfXY0 = (ShieldCenter1 - fTargetCenter - ShieldTubeL1 / 2.0) * 35.0 / (fGEMCenter0 - fTargetCenter);
        G4double ShieldBoxHalfXY1 = (ShieldCenter1 - fTargetCenter + ShieldTubeL1 / 2.0) * 35.0 / (fGEMCenter0 - fTargetCenter);
        G4double ShieldBoxHalfXY2 = (ShieldCenter2 - fTargetCenter - ShieldTubeL2 / 2.0) * 35.0 / (fGEMCenter1 - fTargetCenter);
        G4double ShieldBoxHalfXY3 = (ShieldCenter2 - fTargetCenter + ShieldTubeL2 / 2.0) * 35.0 / (fGEMCenter1 - fTargetCenter);

        G4VSolid *solidShieldBox1_out = new G4Trd("ShieldBox1S_out", ShieldBoxHalfXY0, ShieldBoxHalfXY1,
                                                  ShieldBoxHalfXY0, ShieldBoxHalfXY1, ShieldTubeL1 / 2.0);
        G4VSolid *solidShieldBox1_in = new G4Trd("ShieldBox1S_in", ShieldBoxHalfXY0 - ShieldBoxThickness, ShieldBoxHalfXY1 - ShieldBoxThickness,
                                                 ShieldBoxHalfXY0 - ShieldBoxThickness, ShieldBoxHalfXY1 - ShieldBoxThickness, ShieldTubeL1 / 2.0 + 1.0 * mm);
        G4SubtractionSolid *solidShieldBox1 = new G4SubtractionSolid("ShieldBox1S", solidShieldBox1_out, solidShieldBox1_in);
        G4LogicalVolume *logicShieldBox1 = new G4LogicalVolume(solidShieldBox1, ShieldingM, "ShieldBox1LV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, ShieldCenter1), logicShieldBox1, "Shielding Box", mother, false, 0);

        G4VSolid *solidShieldBox2_out = new G4Trd("ShieldBox2S_out", ShieldBoxHalfXY2, ShieldBoxHalfXY3,
                                                  ShieldBoxHalfXY2, ShieldBoxHalfXY3, ShieldTubeL2 / 2.0);
        G4VSolid *solidShieldBox2_in = new G4Trd("ShieldBox2S_in", ShieldBoxHalfXY2 - ShieldBoxThickness, ShieldBoxHalfXY3 - ShieldBoxThickness,
                                                 ShieldBoxHalfXY2 - ShieldBoxThickness, ShieldBoxHalfXY3 - ShieldBoxThickness, ShieldTubeL2 / 2.0 + 1.0 * mm);
        G4SubtractionSolid *solidShieldBox2 = new G4SubtractionSolid("ShieldBox2S", solidShieldBox2_out, solidShieldBox2_in);
        G4LogicalVolume *logicShieldBox2 = new G4LogicalVolume(solidShieldBox2, ShieldingM, "ShieldBox2LV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, ShieldCenter2), logicShieldBox2, "Shielding Box", mother, false, 0);
    }
}
