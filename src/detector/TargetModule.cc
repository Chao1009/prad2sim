//
// TargetModule.cc
// Faithful port of the target geometry from DetectorPRad.cc /
// DetectorCommon.cc (test) / DetectorDRad.cc, with the LH2 / Ta target
// options from PRadSim_PRad2.
//

#include "detector/TargetModule.hh"

#include "CheckScatteringSD.hh"
#include "StepRecordSD.hh"
#include "TrackingDetectorSD.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4Polyhedra.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PRadTargetModule::PRadTargetModule(double center, double r, double halfL,
                                   const G4String &material, double apertureR,
                                   SdStyle sdStyle, bool sdOn)
    : DetectorModule("PRadTarget"), fCenter(center), fR(r), fHalfL(halfL),
      fApertureR(apertureR), fMaterial(material), fSdStyle(sdStyle), fSdOn(sdOn)
{
}

void PRadTargetModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");

    G4Material *TargetM = G4Material::GetMaterial("H2Gas");

    if (fMaterial == "LH2")
        TargetM = G4Material::GetMaterial("H2Liquid");
    else if (fMaterial == "Ta")
        TargetM = G4Material::GetMaterial("Tantalum");

    G4double TargetR = fR;
    G4double TargetHalfL = fHalfL;

    // Target Container
    G4VSolid *solidTargetCon = new G4Box("TargetContainerS", 3.5 * cm, 3.5 * cm, 2.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, DefaultM, "TargetContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fCenter), logicTargetCon, "Target Container", world, false, 0);

    // Target material
    G4VSolid *solidTarget = new G4Tubs("TargetS", 0, TargetR, TargetHalfL, 0, twopi);
    fTargetLV = new G4LogicalVolume(solidTarget, TargetM, "TargetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), fTargetLV, "Target Material", logicTargetCon, false, 0);

    if (fMaterial == "LH2") {
        // LH2 cell: thin aluminum tube (ported from PRadSim_PRad2)
        G4Material *TargetCellM = G4Material::GetMaterial("Aluminum");
        G4Tubs *CellTube1 = new G4Tubs("CellTube1", 0, TargetR + 2 * mm, TargetHalfL + 0.125 * mm, 0, twopi);
        G4Tubs *CellTube2 = new G4Tubs("CellTube2", 0, TargetR + 0.0001 * mm, TargetHalfL + 0.0001 * mm, 0, twopi);
        G4SubtractionSolid *solidCell = new G4SubtractionSolid("TargetCellS", CellTube1, CellTube2);
        G4LogicalVolume *logicCell = new G4LogicalVolume(solidCell, TargetCellM, "TargetCellLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicCell, "Target Cell", logicTargetCon, false, 0);
    } else if (fMaterial == "Ta") {
        // bare foil, no cell
    } else {
        // Gas cell: copper frame + kapton windows
        G4Material *TargetCellM = G4Material::GetMaterial("Copper");
        G4Material *TargetWindowM = G4Material::GetMaterial("Kapton");

        // Target cell
        G4double CellXY = 3.5 * cm;
        G4Box *CellBox = new G4Box("CellBox", CellXY, CellXY, TargetHalfL);
        G4Tubs *CellTube = new G4Tubs("CellTube", 0, TargetR, TargetHalfL + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidCell = new G4SubtractionSolid("TargetCellS", CellBox, CellTube);
        G4LogicalVolume *logicCell = new G4LogicalVolume(solidCell, TargetCellM, "TargetCellLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicCell, "Target Cell", logicTargetCon, false, 0);

        // Target cell windows
        G4double CellApertureR = fApertureR;
        G4double CellWinThickness = 7.5 * um;
        G4Box *CellWinBox = new G4Box("CellWinBox", CellXY, CellXY, CellWinThickness / 2.0);
        G4Tubs *CellWinTube = new G4Tubs("CellWinTube", 0, CellApertureR, CellWinThickness + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidCellWin = new G4SubtractionSolid("TargetWindowS", CellWinBox, CellWinTube);
        G4LogicalVolume *logicCellWin = new G4LogicalVolume(solidCellWin, TargetWindowM, "TargetWindowLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, -TargetHalfL - CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 0);
        new G4PVPlacement(0, G4ThreeVector(0, 0, +TargetHalfL + CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 1);
    }
}

void PRadTargetModule::BuildSDs()
{
    if (!fSdOn || fSdStyle == SdStyle::kNone || !fTargetLV) return;

    if (fSdStyle == SdStyle::kCheckScattering) {
        CheckScatteringSD *TargetSD = new CheckScatteringSD("TargetSD", "TG");
        G4SDManager::GetSDMpointer()->AddNewDetector(TargetSD);
        fTargetLV->SetSensitiveDetector(TargetSD);
    } else {
        StepRecordSD *TargetSD = new StepRecordSD("TargetSD", "TG");
        G4SDManager::GetSDMpointer()->AddNewDetector(TargetSD);
        fTargetLV->SetSensitiveDetector(TargetSD);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DRadTargetModule::DRadTargetModule(double center, double r, double halfL, const G4String &material,
                                   double recoilCenter, int recoilNSeg, double recoilR,
                                   double recoilHalfL, double recoilL1Thickness, double recoilL2Thickness,
                                   bool recoilSdOn)
    : DetectorModule("DRadTarget"), fCenter(center), fR(r), fHalfL(halfL), fMaterial(material),
      fRecoilCenter(recoilCenter), fRecoilNSeg(recoilNSeg), fRecoilR(recoilR),
      fRecoilHalfL(recoilHalfL), fRecoilL1Thickness(recoilL1Thickness),
      fRecoilL2Thickness(recoilL2Thickness), fRecoilSdOn(recoilSdOn)
{
}

void DRadTargetModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4Material *TargetM = G4Material::GetMaterial("D2Gas");

    if (fMaterial == "H2Gas" || fMaterial == "hydrogen")
        TargetM = G4Material::GetMaterial("H2Gas");

    G4Material *TargetCellM = G4Material::GetMaterial("Kapton");
    G4Material *TargetWindowM = G4Material::GetMaterial("Kapton");
    G4Material *RecoilDetectorM = G4Material::GetMaterial("Silicon");
    G4Material *RecoilDetCoverM = G4Material::GetMaterial("SiO2");

    // Target
    G4double CellXY = 15.0 * cm;

    // Target Container
    G4VSolid *solidTargetCon = new G4Box("TargetContainerS", CellXY + 0.1 * cm, CellXY + 0.1 * cm, fHalfL + 0.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, DefaultM, "TargetContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fCenter), logicTargetCon, "Target Container", world, false, 0);

    // Target material
    G4VSolid *solidTarget = new G4Tubs("TargetS", 0, fR, fHalfL, 0, twopi);
    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, TargetM, "TargetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTarget, "Target Material", logicTargetCon, false, 0);

    // Target cell
    G4Box *CellBox = new G4Box("CellBox", CellXY, CellXY, fHalfL);
    G4Tubs *CellTube = new G4Tubs("CellTube", 0, fR, fHalfL + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCell = new G4SubtractionSolid("TargetCellS", CellBox, CellTube);
    G4LogicalVolume *logicCell = new G4LogicalVolume(solidCell, TargetCellM, "TargetCellLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicCell, "Target Cell", logicTargetCon, false, 0);

    // Target cell windows
    G4double CellApertureR = 2.0 * mm;
    G4double CellWinThickness = 7.5 * um;
    G4Box *CellWinBox = new G4Box("CellWinBox", CellXY, CellXY, CellWinThickness / 2.0);
    G4Tubs *CellWinTube = new G4Tubs("CellWinTube", 0, CellApertureR, CellWinThickness + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCellWin = new G4SubtractionSolid("TargetWindowS", CellWinBox, CellWinTube);
    G4LogicalVolume *logicCellWin = new G4LogicalVolume(solidCellWin, TargetWindowM, "TargetWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -fHalfL - CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +fHalfL + CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 1);

    // Recoil detector
    G4double RecoilDetCenter = fRecoilCenter - fCenter;
    G4double CoverThickness = 0.5 * um;
    G4double RecoilDetAng = twopi / fRecoilNSeg;
    G4double RecoilDetL2OR = fRecoilR * cos(RecoilDetAng / 2.0);
    G4double RecoilDetL2IRC = RecoilDetL2OR - fRecoilL2Thickness + CoverThickness;
    G4double RecoilDetL2IR = RecoilDetL2OR - fRecoilL2Thickness;
    G4double RecoilDetL1OR = RecoilDetL2IR;
    G4double RecoilDetL1IRC = RecoilDetL1OR - fRecoilL1Thickness + CoverThickness;
    G4double RecoilDetL1IR = RecoilDetL1OR - fRecoilL1Thickness;

    G4double rInnerRDL2[] = {RecoilDetL2IRC, RecoilDetL2IRC};
    G4double rOuterRDL2[] = {RecoilDetL2OR, RecoilDetL2OR};
    G4double zPlaneRDL2[] = {-fRecoilHalfL, fRecoilHalfL};
    G4VSolid *solidRecoilDet2 = new G4Polyhedra("RecoilDet2S", 0, twopi, fRecoilNSeg, 2, zPlaneRDL2, rInnerRDL2, rOuterRDL2);

    G4double rInnerRDL2Cover[] = {RecoilDetL2IR, RecoilDetL2IR};
    G4double rOuterRDL2Cover[] = {RecoilDetL2IRC, RecoilDetL2IRC};
    G4double zPlaneRDL2Cover[] = {-fRecoilHalfL, fRecoilHalfL};
    G4VSolid *solidRecoilDet2Cover = new G4Polyhedra("RecoilDet2CoverS", 0, twopi, fRecoilNSeg, 2, zPlaneRDL2Cover, rInnerRDL2Cover, rOuterRDL2Cover);

    G4double rInnerRDL1[] = {RecoilDetL1IRC, RecoilDetL1IRC};
    G4double rOuterRDL1[] = {RecoilDetL1OR, RecoilDetL1OR};
    G4double zPlaneRDL1[] = {-fRecoilHalfL, fRecoilHalfL};
    G4VSolid *solidRecoilDet1 = new G4Polyhedra("RecoilDet1S", 0, twopi, fRecoilNSeg, 2, zPlaneRDL1, rInnerRDL1, rOuterRDL1);

    G4double rInnerRDL1Cover[] = {RecoilDetL1IR, RecoilDetL1IR};
    G4double rOuterRDL1Cover[] = {RecoilDetL1IRC, RecoilDetL1IRC};
    G4double zPlaneRDL1Cover[] = {-fRecoilHalfL, fRecoilHalfL};
    G4VSolid *solidRecoilDet1Cover = new G4Polyhedra("RecoilDet1CoverS", 0, twopi, fRecoilNSeg, 2, zPlaneRDL1Cover, rInnerRDL1Cover, rOuterRDL1Cover);

    G4LogicalVolume *logicRecoilDet1 = new G4LogicalVolume(solidRecoilDet1, RecoilDetectorM, "RecoilDet1LV");
    G4LogicalVolume *logicRecoilDet1Cover = new G4LogicalVolume(solidRecoilDet1Cover, RecoilDetCoverM, "RecoilDet1CoverLV");
    G4LogicalVolume *logicRecoilDet2 = new G4LogicalVolume(solidRecoilDet2, RecoilDetectorM, "RecoilDet2LV");
    G4LogicalVolume *logicRecoilDet2Cover = new G4LogicalVolume(solidRecoilDet2Cover, RecoilDetCoverM, "RecoilDet2CoverLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet1Cover, "Recoil Detector 1 Cover", logicTarget, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet1, "Recoil Detector 1", logicTarget, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet2Cover, "Recoil Detector 2 Cover", logicTarget, false, 1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet2, "Recoil Detector 2", logicTarget, false, 1);

    fRecoilDet1LV = logicRecoilDet1;
    fRecoilDet2LV = logicRecoilDet2;
}

void DRadTargetModule::BuildSDs()
{
    if (!fRecoilSdOn || !fRecoilDet1LV) return;

    TrackingDetectorSD *RecoilDetSD = new TrackingDetectorSD("RecoilDetectorSD", "RD");
    G4SDManager::GetSDMpointer()->AddNewDetector(RecoilDetSD);
    fRecoilDet1LV->SetSensitiveDetector(RecoilDetSD);
    fRecoilDet2LV->SetSensitiveDetector(RecoilDetSD);
}
