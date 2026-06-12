//
// HyCalModule.cc
// Faithful port of DetectorCommon.cc AddHyCal() and the HyCal SD
// attachment from DetectorPRad.cc / DetectorDRad.cc.
//

#include "detector/HyCalModule.hh"

#include "CalorimeterSD.hh"

#include "TString.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4SDManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4UserLimits.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include <fstream>

HyCalModule::HyCalModule(double crystalSurf, double attenuationLG,
                         bool sdOn, const G4String &moduleFile, const G4String &attenuationFile)
    : DetectorModule("HyCal"), fCrystalSurf(crystalSurf), fAttenuationLG(attenuationLG),
      fSdOn(sdOn), fModuleFile(moduleFile), fAttenuationFile(attenuationFile)
{
}

void HyCalModule::BuildVolumes(G4LogicalVolume *mother)
{
    G4Material *HyCalConM = G4Material::GetMaterial("Air");
    G4Material *HyCalBoxM = G4Material::GetMaterial("Rohacell");
    G4Material *HyCalBoxWinM = G4Material::GetMaterial("Tedlar");
    G4Material *CollimatorM = G4Material::GetMaterial("Tungsten");
    G4Material *PbWO4ModuleM = G4Material::GetMaterial("PbWO4");
    G4Material *PbGlassModuleM = G4Material::GetMaterial("PbGlass");
    G4Material *TedlarTapeM = G4Material::GetMaterial("Tedlar");
    G4Material *ReflectorM = G4Material::GetMaterial("Polyester");
    G4Material *PlateM = G4Material::GetMaterial("Brass");

    std::ifstream dimension_file;
    dimension_file.open(fModuleFile.c_str());
    G4double pwo[1152][4], lg[576][4];

    for (int i = 0; i < 576; i++)
        dimension_file >> lg[i][0] >> lg[i][1] >> lg[i][2] >> lg[i][3];

    for (int i = 0; i < 1152; i++)
        dimension_file >> pwo[i][0] >> pwo[i][1] >> pwo[i][2] >> pwo[i][3];

    dimension_file.close();

    G4double MaxStep = 1.0 * mm;

    // HyCal
    G4double CrystalL = 18.0 * cm;
    G4double PbGlassL = 45.0 * cm;
    G4double CrystalDiffL = 9.73 * cm; // according to last survey (april 2017)
    G4double CrystalCenter = fCrystalSurf + CrystalL / 2.0;
    G4double PbGlassCenter = fCrystalSurf - CrystalDiffL + PbGlassL / 2.0;

    // HyCal container
    G4double HyCalBoxCenter = PbGlassCenter - 9.0 * cm + 30.0 * cm; // Check
    G4Box *HyCalConNoHole = new G4Box("HyCalConNoHole", 80.0 * cm, 80.0 * cm, 65.0 * cm);
    G4Tubs *HyCalConHole = new G4Tubs("HyCalConHole", 0, 31.75 * mm, 66.0 * cm, 0, twopi);
    G4SubtractionSolid *solidHyCalCon = new G4SubtractionSolid("HyCalConS", HyCalConNoHole, HyCalConHole);
    G4LogicalVolume *logicHyCalCon = new G4LogicalVolume(solidHyCalCon, HyCalConM, "HyCalConLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalBoxCenter), logicHyCalCon, "HyCal Container", mother, false, 0);
    logicHyCalCon->SetUserLimits(new G4UserLimits(MaxStep));

    // HyCal box
    G4Box *HyCalBoxOuter = new G4Box("HyCalBoxOuter", 72.54 * cm, 72.54 * cm, 62.54 * cm);
    G4Box *HyCalBoxInner = new G4Box("HyCalBoxInner", 70.0 * cm, 70.0 * cm, 60.0 * cm);
    G4SubtractionSolid *HyCalBoxNoHole = new G4SubtractionSolid("HyCalBoxNoHole", HyCalBoxOuter, HyCalBoxInner);
    G4Tubs *HyCalBoxHole = new G4Tubs("HyCalBoxHole", 0, 3.175 * cm, 65.0 * cm, 0, twopi);
    G4SubtractionSolid *solidHyCalBox = new G4SubtractionSolid("HyCalBoxS", HyCalBoxNoHole, HyCalBoxHole);
    G4LogicalVolume *logicHyCalBox = new G4LogicalVolume(solidHyCalBox, HyCalBoxM, "HyCalBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicHyCalBox, "HyCal Box", logicHyCalCon, false, 0);

    // HyCal box window
    G4VSolid *solidHyCalBoxWin = new G4Tubs("HyCalBoxWinS", 1.90 * cm, 5.08 * cm, 19.0 * um, 0, twopi);
    G4LogicalVolume *logicHyCalBoxWin = new G4LogicalVolume(solidHyCalBoxWin, HyCalBoxWinM, "HyCalBoxWinLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -62.54 * cm - 19.0 * um), logicHyCalBoxWin, "HyCal Box Window", logicHyCalCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -60.00 * cm + 19.0 * um), logicHyCalBoxWin, "HyCal Box Window", logicHyCalCon, false, 1);

    // Lead glass module container
    G4double PlateThickness = 25.4 * 0.08 * mm; // 0.08 in
    G4double PbGlassPlateHoleR = 25.4 * 0.5 * mm; // 0.5 in
    G4double ReflectorT = 25.0 * um;
    G4double StripThickness = 25.4 * 0.001 * mm; // 0.001 in

    G4Box *PbGlassConBox = new G4Box("PbGlassConBox", 58.199 * cm, 58.165 * cm, PbGlassL / 2.0 + PlateThickness);
    G4Box *PbGlassConHole = new G4Box("PbGlassConHole", 35.309 * cm, 35.275 * cm, PbGlassL / 2.0 + PlateThickness + 1.0 * mm);
    G4SubtractionSolid *solidPbGlassCon = new G4SubtractionSolid("PbGlassModuleContainerS", PbGlassConBox, PbGlassConHole);
    G4LogicalVolume *logicPbGlassCon = new G4LogicalVolume(solidPbGlassCon, HyCalConM, "PbGlassModuleContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, PbGlassCenter - HyCalBoxCenter), logicPbGlassCon, "PbGlass Module Container", logicHyCalCon, false, 0);

    for (int i = 0; i < 576; i++) {
        G4double PbGlassX = lg[i][0] - 0.15 * mm;
        G4double PbGlassY = lg[i][1] - 0.15 * mm;

        // Lead glass module
        G4VSolid *solidPbGlassModule = new G4Box(Form("PbGlassModule%04dS", i), lg[i][0] * mm / 2.0, lg[i][1] * mm / 2.0, PbGlassL / 2.0 + PlateThickness);
        G4LogicalVolume *logicPbGlassModule = new G4LogicalVolume(solidPbGlassModule, HyCalConM, Form("PbGlassModule%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(lg[i][2] * mm, lg[i][3] * mm, 0), logicPbGlassModule, "PbGlass Module", logicPbGlassCon, false, i);

        G4VSolid *solidPbGlassReflector = new G4Box(Form("PbGlassReflector%04dS", i), PbGlassX / 2.0 + ReflectorT, PbGlassY / 2.0 + ReflectorT, PbGlassL / 2.0);
        G4LogicalVolume *logicPbGlassReflector = new G4LogicalVolume(solidPbGlassReflector, ReflectorM, Form("PbGlassReflector%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbGlassReflector, "Reflector", logicPbGlassModule, false, 0);

        G4VSolid *solidPbGlassAbsorber = new G4Box(Form("PbGlassAbsorber%04dS", i), PbGlassX / 2.0, PbGlassY / 2.0, PbGlassL / 2.0);
        G4LogicalVolume *logicPbGlassAbsorber = new G4LogicalVolume(solidPbGlassAbsorber, PbGlassModuleM, Form("PbGlassAbsorber%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbGlassAbsorber, "PbGlass Absorber", logicPbGlassReflector, false, 0);

        G4Box *PbGlassPlateBox = new G4Box("PbGlassPlateBox", PbGlassX / 2.0, PbGlassY / 2.0, PlateThickness / 2.0);
        G4Tubs *PbGlassPlateHole = new G4Tubs("PbGlassPlateHole", 0, PbGlassPlateHoleR, PlateThickness / 2.0 + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidPbGlassPlate = new G4SubtractionSolid(Form("PbGlassPlate%04dS", i), PbGlassPlateBox, PbGlassPlateHole);
        G4LogicalVolume *logicPbGlassPlate = new G4LogicalVolume(solidPbGlassPlate, PlateM, Form("PbGlassPlate%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, - PbGlassL / 2.0 - PlateThickness / 2.0), logicPbGlassPlate, "Brass Plate", logicPbGlassModule, false, 0);


        if ((lg[i][2] > -353.09 && lg[i][3] > 352.75) || (lg[i][2] < 353.09 && lg[i][3] < -352.75)) {
            G4double StripWidth = PbGlassX;

            G4VSolid *solidPbGlassStrip = new G4Box(Form("PbGlassStrip%04dS", i), StripWidth / 2.0, StripThickness / 2.0,  PbGlassL / 2.0);
            G4LogicalVolume *logicPbGlassStrip = new G4LogicalVolume(solidPbGlassStrip, PlateM, Form("PbGlassStrip%04dLV", i));
            new G4PVPlacement(0, G4ThreeVector(0, (lg[i][1] * mm - StripThickness) / 2.0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 0);
            new G4PVPlacement(0, G4ThreeVector(0, (-lg[i][1] * mm + StripThickness) / 2.0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 1);
        } else {
            G4double StripWidth = PbGlassY;

            G4VSolid *solidPbGlassStrip = new G4Box(Form("PbGlassStrip%04dS", i), StripThickness / 2.0, StripWidth / 2.0, PbGlassL / 2.0);
            G4LogicalVolume *logicPbGlassStrip = new G4LogicalVolume(solidPbGlassStrip, PlateM, Form("PbGlassStrip%04dLV", i));
            new G4PVPlacement(0, G4ThreeVector((lg[i][0] * mm - StripThickness) / 2.0, 0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 0);
            new G4PVPlacement(0, G4ThreeVector((-lg[i][0] * mm + StripThickness) / 2.0, 0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 1);
        }
    }

    // PbWO4 module container
    G4double CrystalPlateHoleR = 25.4 * 0.25 * mm; // 0.25 in
    G4double TedlarTapeT = 38.1 * um;
    ReflectorT = 63.0 * um;

    G4Box *PbWO4ConBox = new G4Box("PbWO4ConBox", 35.309 * cm, 35.275 * cm, CrystalL / 2.0 + PlateThickness);
    G4Box *PbWO4ConHole = new G4Box("PbWO4ConHole", 2.0 * cm, 2.0 * cm, CrystalL / 2.0 + PlateThickness + 1.0 * mm);
    G4SubtractionSolid *solidPbWO4Con = new G4SubtractionSolid("PbWO4ModuleContainerS", PbWO4ConBox, PbWO4ConHole);
    G4LogicalVolume *logicPbWO4Con = new G4LogicalVolume(solidPbWO4Con, HyCalConM, "PbWO4ModuleContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, CrystalCenter - HyCalBoxCenter), logicPbWO4Con, "PbWO4 Module Container", logicHyCalCon, false, 576);

    for (int i = 0; i < 1152; i++) {
        G4double CrystalX = pwo[i][0] - 0.27 * mm;
        G4double CrystalY = pwo[i][1] - 0.25 * mm;
        G4double StripWidth = 25.4 * 0.5 * mm; // 0.5 in

        // PbWO4 module
        G4VSolid *solidPbWO4Module = new G4Box(Form("PbWO4Module%04dS", i), pwo[i][0] * mm / 2.0, pwo[i][1] * mm / 2.0, CrystalL / 2.0 + PlateThickness);
        G4LogicalVolume *logicPbWO4Module = new G4LogicalVolume(solidPbWO4Module, HyCalConM, Form("PbWO4Module%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(pwo[i][2] * mm, pwo[i][3] * mm, 0), logicPbWO4Module, "PbGlass Module", logicPbWO4Con, false, i);

        G4VSolid *solidPbWO4TedlarTape = new G4Box(Form("PbWO4TedlarTape%04dS", i), CrystalX / 2.0 + TedlarTapeT + ReflectorT, CrystalY / 2.0 + TedlarTapeT + ReflectorT, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4TedlarTape = new G4LogicalVolume(solidPbWO4TedlarTape, TedlarTapeM, Form("PbWO4TedlarTape%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4TedlarTape, "Tedlar Tape", logicPbWO4Module, false, 0);

        G4VSolid *solidPbWO4Reflector = new G4Box(Form("PbWO4Reflector%04dS", i), CrystalX / 2.0 + ReflectorT, CrystalY / 2.0 + ReflectorT, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Reflector = new G4LogicalVolume(solidPbWO4Reflector, ReflectorM, Form("PbWO4Reflector%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4Reflector, "Reflector", logicPbWO4TedlarTape, false, 0);

        G4VSolid *solidPbWO4Absorber = new G4Box(Form("PbWO4Absorber%04dS", i), CrystalX / 2.0, CrystalY / 2.0, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Absorber = new G4LogicalVolume(solidPbWO4Absorber, PbWO4ModuleM, Form("PbWO4Absorber%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4Absorber, "PbWO4 Absorber", logicPbWO4Reflector, false, 0);

        G4Box *PbWO4PlateBox = new G4Box("PbWO4PlateBox", CrystalX / 2.0, CrystalY / 2.0, PlateThickness / 2.0);
        G4Tubs *PbWO4PlateHole = new G4Tubs("PbWO4PlateHole", 0, CrystalPlateHoleR, PlateThickness / 2.0 + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidPbWO4Plate = new G4SubtractionSolid(Form("PbWO4Plate%04dS", i), PbWO4PlateBox, PbWO4PlateHole);
        G4LogicalVolume *logicPbWO4Plate = new G4LogicalVolume(solidPbWO4Plate, PlateM, Form("PbWO4Plate%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, -(CrystalL + PlateThickness) / 2.0), logicPbWO4Plate, "Brass Plate", logicPbWO4Module, false, 0);

        G4VSolid *solidPbWO4Strip = new G4Box(Form("PbWO4Strip%04dS", i), StripThickness / 2.0, StripWidth / 2.0, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Strip = new G4LogicalVolume(solidPbWO4Strip, PlateM, Form("PbWO4Strip%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector((pwo[i][0] * mm - StripThickness) / 2.0, 0, 0), logicPbWO4Strip, "Brass Strip", logicPbWO4Module, false, 0);
        new G4PVPlacement(0, G4ThreeVector((-pwo[i][0] * mm + StripThickness) / 2.0, 0, 0), logicPbWO4Strip, "Brass Strip", logicPbWO4Module, false, 1);
    }

    // Collimator
    G4Box *CollimatorBox = new G4Box("CollimatorBox", 4.07 * cm, 4.07 * cm, 3.02 * cm);
    G4Tubs *CollimatorTube = new G4Tubs("CollimatorTube", 0, 1.95 * cm, 3.1 * cm, 0, twopi);
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("CollimatorS", CollimatorBox, CollimatorTube);
    G4LogicalVolume *logicCollimator = new G4LogicalVolume(solidCollimator, CollimatorM, "CollimatorLV");
    G4RotationMatrix rmColl;
    rmColl.rotateZ(-8.8 * deg);
    new G4PVPlacement(G4Transform3D(rmColl, G4ThreeVector(0, 0, fCrystalSurf - PlateThickness - 3.1 * cm - HyCalBoxCenter)), logicCollimator, "Collimator", logicHyCalCon, false, 0);

    // Collect absorber LVs for the SD pass: PbWO4 first, then PbGlass,
    // matching the original DefinePRadSDs()/DefineDRadSDs() attach order.
    fAbsorberLVs.clear();
    G4LogicalVolumeStore *store = G4LogicalVolumeStore::GetInstance();

    for (int i = 0; i < 1152; i++)
        fAbsorberLVs.push_back(store->GetVolume(Form("PbWO4Absorber%04dLV", i)));

    for (int i = 0; i < 576; i++)
        fAbsorberLVs.push_back(store->GetVolume(Form("PbGlassAbsorber%04dLV", i)));
}

void HyCalModule::BuildSDs()
{
    if (!fSdOn || fAbsorberLVs.empty()) return;

    CalorimeterSD *HyCalSD = new CalorimeterSD("HyCalSD", "HC", fAttenuationFile);
    HyCalSD->SetAttenuationLG(fAttenuationLG);
    G4SDManager::GetSDMpointer()->AddNewDetector(HyCalSD);

    for (auto *lv : fAbsorberLVs)
        if (lv) lv->SetSensitiveDetector(HyCalSD);
}
