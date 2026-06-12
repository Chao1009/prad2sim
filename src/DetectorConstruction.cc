//
// DetectorConstruction.cc
// Developer : Chao Peng, Chao Gu
// History:
//   Aug 2012, C. Peng, Original version.
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//   Mar 2017, C. Gu, Add DRad configuration.
//   Jun 2026, Modular rework: geometry split into DetectorModule subsystems,
//             prad2 / x17 configurations added.
//

#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"
#include "SimConfig.hh"

#include "detector/BeamlineModule.hh"
#include "detector/CadInsertsModule.hh"
#include "detector/DetectorModule.hh"
#include "detector/GEMModule.hh"
#include "detector/HeBagModule.hh"
#include "detector/HyCalModule.hh"
#include "detector/MaterialBuilder.hh"
#include "detector/ScintillatorModule.hh"
#include "detector/TargetModule.hh"
#include "detector/VacuumSystemModule.hh"
#include "detector/VirtualDetModule.hh"

#include "json.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

#include "G4PhysicalConstants.hh"
#include "G4String.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(G4String conf, const SimConfig &config) : G4VUserDetectorConstruction(), fConfig(conf)
{
    if (fConfig != "prad" && fConfig != "drad" && fConfig != "prad2" && fConfig != "x17" && fConfig != "test")
        fConfig = "prad";

    bool isPRad2Like = (fConfig == "prad2" || fConfig == "x17");

    fVisAtts.clear();

    // World geometry
    fWorldSizeXY = config.GetDouble("world", "size_xy", 150.0) * cm;
    fWorldSizeZ = config.GetDouble("world", "size_z", 600.0) * cm;

    // Target
    fTargetCenter = config.GetDouble("target", "center", -291.1) * cm;
    fTargetR = config.GetDouble("target", "radius", 14.5) * cm;
    fTargetHalfL = config.GetDouble("target", "half_length", 2.75) * cm;
    fTargetMat = config.GetString("target", "material", "D2Gas");
    fTargetDensityRatio = config.GetDouble("target", "density_ratio", 1.0);
    fTargetCellAperture = config.GetDouble("target", "cell_aperture_mm", isPRad2Like ? 1.0 : 2.0) * mm;

    // Recoil detector
    fRecoilDetNSeg = config.GetInt("recoil_detector", "n_segments", 20);
    fRecoilDetCenter = fTargetCenter;
    fRecoilDetR = config.GetDouble("recoil_detector", "radius", 13.5) * cm;
    fRecoilDetHalfL = config.GetDouble("recoil_detector", "half_length", 2.6) * cm;
    fRecoilDetL1Thickness = config.GetDouble("recoil_detector", "l1_thickness_um", 200) * um;
    fRecoilDetL2Thickness = config.GetDouble("recoil_detector", "l2_thickness_um", 300) * um;

    // GEM detectors (center_ref "target" places them relative to the target)
    std::vector<double> gemCenters = config.GetDoubleArray("gem", "center", {217.5, 257.5});
    double gemBase = (config.GetString("gem", "center_ref", "absolute") == "target") ? fTargetCenter : 0.0;

    for (size_t i = 0; i < 10; i++)
        fGEMCenter[i] = gemBase;

    for (size_t i = 0; i < gemCenters.size() && i < 10; i++)
        fGEMCenter[i] = gemCenters[i] * cm + gemBase;

    // Scintillator plane (DRad single plane)
    fSciPlaneCenter = config.GetDouble("scintillator_plane", "center", 262.5) * cm;

    // HyCal (surface_ref "target" places it relative to the target).
    // prad2/x17 build the calorimeter from a table generated from the prad2
    // reconstruction map (hycal_map.json) so cluster centers resolve — see
    // database/make_hycal_table_from_map.py.
    double hycalBase = (config.GetString("hycal", "surface_ref", "absolute") == "target") ? fTargetCenter : 0.0;
    fCrystalSurf = config.GetDouble("hycal", "crystal_surface", 295.0) * cm + hycalBase;
    fHyCalModuleFile = config.GetString("hycal", "module_file",
                                        isPRad2Like ? "database/hycal_module_prad2.dat"
                                                    : "database/hycal_module_shuffled.dat");

    fExtDensityRatio = config.GetDouble("ext_density_ratio", 1.0);

    // Virtual detector plane. PRad: full disk at target + 6 cm (scattering
    // check). PRad-II / X17: annulus just behind the last GEM station — the
    // "VD" plane read by the prad2 sim2replay tool.
    fVDZFromTarget = config.GetDouble("virtual_det", "z_from_target", isPRad2Like ? 590.7 : 6.0) * cm;
    fVDInnerR = config.GetDouble("virtual_det", "inner_r", isPRad2Like ? 2.0 : 0.0) * cm;
    fVDOuterR = config.GetDouble("virtual_det", "outer_r", isPRad2Like ? 100.0 : 50.0) * cm;

    // PRad-II / X17 vacuum options
    fUseHeBag = config.GetBool("vacuum", "use_he_bag", false);
    fUseShielding = config.GetBool("vacuum", "shielding", false);

    // CAD inserts
    fCadModelDir = config.GetString("cad", "model_dir", "database/CADmodel");
    fCadInserts.clear();

    if (const nlohmann::json *inserts = config.GetNode("cad", "inserts")) {
        if (inserts->is_array()) {
            for (const auto &item : *inserts) {
                CadInsert ins;
                ins.stl = item.value("stl", "");
                ins.material = item.value("material", "Aluminum");
                ins.name = item.value("name", ins.stl);
                ins.zOffset = item.value("z_offset_mm", 0.0) * mm;

                if (!ins.stl.empty())
                    fCadInserts.push_back(ins);
            }
        }
    }

    // Sensitive detector flags
    fTargetSDOn = config.GetBool("sensitive_detectors", "target", false);
    fRecoilDetSDOn = config.GetBool("sensitive_detectors", "recoil", false);
    fGEMSDOn = config.GetBool("sensitive_detectors", "gem", true);
    fSciPlaneSDOn = config.GetBool("sensitive_detectors", "scintillator_plane", false);
    fSciVirtualSDOn = config.GetBool("sensitive_detectors", "sci_virtual", false);
    fHyCalSDOn = config.GetBool("sensitive_detectors", "hycal", true);
    fVirtualSDOn = config.GetBool("sensitive_detectors", "virtual", false);

    // HyCal optical properties
    fAttenuationLG = config.GetDouble("hycal", "attenuation_lg", 0.0);
    fReflectanceLG = config.GetDouble("hycal", "reflectance_lg", 1.0);

    detectorMessenger = std::make_unique<DetectorMessenger>(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    // Define materials (density ratios may have been changed by macro commands)
    MaterialBuilder::Build(fTargetDensityRatio, fExtDensityRatio, fVisAtts);

    // World
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4VSolid *solidWorld = new G4Box("WorldS", fWorldSizeXY, fWorldSizeXY, fWorldSizeZ);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, DefaultM, "WorldLV");
    G4VPhysicalVolume *physiWorld = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicWorld, "World", 0, false, 0);

    // Assemble and build the per-configuration module list
    BuildModules();

    for (auto &module : fModules)
        module->BuildVolumes(logicWorld);

    // Apply visualization attributes by material name
    G4LogicalVolumeStore *pLogicalVolume = G4LogicalVolumeStore::GetInstance();

    for (unsigned long i = 0; i < pLogicalVolume->size(); i++)
        (*pLogicalVolume)[i]->SetVisAttributes(fVisAtts[(*pLogicalVolume)[i]->GetMaterial()->GetName()]);

    // Always return the physical World
    return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
    for (auto &module : fModules)
        module->BuildSDs();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::BuildModules()
{
    fModules.clear();

    if (fConfig == "drad") {
        fModules.push_back(std::make_unique<DRadTargetModule>(
            fTargetCenter, fTargetR, fTargetHalfL, fTargetMat,
            fRecoilDetCenter, fRecoilDetNSeg, fRecoilDetR, fRecoilDetHalfL,
            fRecoilDetL1Thickness, fRecoilDetL2Thickness, fRecoilDetSDOn));

        fModules.push_back(std::make_unique<VacuumSystemModule>(
            VacuumSystemModule::Style::kPRad1, fTargetCenter, fWorldSizeZ));

        fModules.push_back(std::make_unique<GEMModule>(
            GEMModule::Style::kPRad1,
            std::vector<GEMModule::Station>{{fGEMCenter[0], true}, {fGEMCenter[1], false}},
            fGEMSDOn));

        fModules.push_back(std::make_unique<HeBagModule>(fGEMCenter[0], fGEMCenter[1]));

        fModules.push_back(std::make_unique<ScintillatorModule>(
            ScintillatorModule::Style::kSinglePlane, fSciPlaneCenter, fSciPlaneSDOn));

        fModules.push_back(std::make_unique<HyCalModule>(fCrystalSurf, fAttenuationLG, fHyCalSDOn, fHyCalModuleFile));
    } else if (fConfig == "test") {
        // Simple test setup: target at the origin + virtual detector,
        // both always sensitive (the target records every step)
        fModules.push_back(std::make_unique<PRadTargetModule>(
            0.0, 25.0 * mm, 20.0 * mm, "H2Gas", 2.0 * mm,
            PRadTargetModule::SdStyle::kStepRecord, true));

        G4double VirtualDetZ = 0.1 * mm;
        G4double VirtualDetL = 99.0 * cm;
        G4double VirtualDetIR = (VirtualDetL - 20.0 * mm) * tan(0.5 / 180.0 * pi);
        G4double VirtualDetOR = (VirtualDetL + 20.0 * mm) * tan(10.0 / 180.0 * pi);
        fModules.push_back(std::make_unique<VirtualDetModule>(
            VirtualDetL, VirtualDetIR, VirtualDetOR, VirtualDetZ, true));
    } else if (fConfig == "prad2" || fConfig == "x17") {
        // PRad-II / X17: PRad-style target (configurable cell), PRad-II GEM
        // stations with 3 mm drift gas, four-paddle scintillator, HyCal at
        // the PRad-II position, and the sim2replay "VD" plane behind the
        // last GEM station. X17 differs through the configuration only
        // (vacuum shielding, CAD window set, target options).
        G4String targetMat = fTargetMat;

        if (targetMat == "hydrogen") targetMat = "H2Gas";

        fModules.push_back(std::make_unique<PRadTargetModule>(
            fTargetCenter, fTargetR, fTargetHalfL, targetMat, fTargetCellAperture,
            PRadTargetModule::SdStyle::kCheckScattering, fTargetSDOn));

        fModules.push_back(std::make_unique<BeamlineModule>(fTargetCenter));

        fModules.push_back(std::make_unique<VacuumSystemModule>(
            fConfig == "x17" ? VacuumSystemModule::Style::kX17 : VacuumSystemModule::Style::kPRad2,
            fTargetCenter, fWorldSizeZ,
            fTargetCenter + 308.38 * mm, fUseHeBag, fUseShielding,
            fGEMCenter[0], fGEMCenter[1]));

        fModules.push_back(std::make_unique<CadInsertsModule>(
            fTargetCenter, fCadModelDir, fCadInserts));

        fModules.push_back(std::make_unique<GEMModule>(
            GEMModule::Style::kPRad2,
            std::vector<GEMModule::Station>{{fGEMCenter[0], false}, {fGEMCenter[1], false}},
            fGEMSDOn));

        // the four-paddle scintillator + housing is a PRad-II addition;
        // the X17 setup runs without it
        if (fConfig == "prad2")
            fModules.push_back(std::make_unique<ScintillatorModule>(
                ScintillatorModule::Style::kFourPlane, fTargetCenter, fSciPlaneSDOn, fSciVirtualSDOn));

        fModules.push_back(std::make_unique<HyCalModule>(fCrystalSurf, fAttenuationLG, fHyCalSDOn, fHyCalModuleFile));

        fModules.push_back(std::make_unique<VirtualDetModule>(
            fTargetCenter + fVDZFromTarget, fVDInnerR, fVDOuterR, 0.1 * mm / 2.0, fVirtualSDOn));
    } else {
        // prad. The PRad-I target cell is fixed 25 mm radius x 40 mm length
        // (independent of the DRad target.radius/half_length parameters).
        fModules.push_back(std::make_unique<PRadTargetModule>(
            fTargetCenter, 25.0 * mm, 20.0 * mm, "H2Gas", fTargetCellAperture,
            PRadTargetModule::SdStyle::kCheckScattering, fTargetSDOn));

        fModules.push_back(std::make_unique<BeamlineModule>(fTargetCenter));

        fModules.push_back(std::make_unique<VacuumSystemModule>(
            VacuumSystemModule::Style::kPRad1, fTargetCenter, fWorldSizeZ));

        fModules.push_back(std::make_unique<GEMModule>(
            GEMModule::Style::kPRad1,
            std::vector<GEMModule::Station>{{fGEMCenter[0], false}},
            fGEMSDOn));

        fModules.push_back(std::make_unique<HyCalModule>(fCrystalSurf, fAttenuationLG, fHyCalSDOn, fHyCalModuleFile));

        fModules.push_back(std::make_unique<VirtualDetModule>(
            fTargetCenter + fVDZFromTarget, fVDInnerR, fVDOuterR, 0.1 * mm / 2.0, fVirtualSDOn));
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
