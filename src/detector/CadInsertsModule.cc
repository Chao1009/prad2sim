//
// CadInsertsModule.cc
// STL import through the vendored CADMesh header (include/external/).
// CADMesh's built-in STL reader needs only Geant4 — the assimp/tetgen
// readers stay disabled.
//

#include "detector/CadInsertsModule.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"

#include "G4ios.hh"

#ifdef PRAD2SIM_USE_CADMESH
#include "external/CADMesh.hh"
#endif

#include <fstream>

CadInsertsModule::CadInsertsModule(double targetCenter, const std::string &modelDir,
                                   std::vector<CadInsert> inserts)
    : DetectorModule("CadInserts"), fTargetCenter(targetCenter),
      fModelDir(modelDir), fInserts(std::move(inserts))
{
}

void CadInsertsModule::BuildVolumes(G4LogicalVolume *world)
{
    if (fInserts.empty()) return;

#ifndef PRAD2SIM_USE_CADMESH
    (void)world;
    G4cout << "CadInsertsModule: built without CADMesh support — skipping "
           << fInserts.size() << " CAD insert(s). Reconfigure with "
           << "-DPRAD2SIM_USE_CADMESH=ON for the full geometry." << G4endl;
#else

    for (const auto &insert : fInserts) {
        std::string path = fModelDir + "/" + insert.stl;

        std::ifstream test(path);

        if (!test.good()) {
            G4cout << "CadInsertsModule: WARNING — missing STL " << path
                   << " ; skipping insert '" << insert.name << "'. "
                   << "Copy the CAD models into " << fModelDir
                   << " (see database/CADmodel/README.md)." << G4endl;
            continue;
        }

        G4Material *material = G4Material::GetMaterial(insert.material);

        if (!material) {
            G4cout << "CadInsertsModule: WARNING — unknown material '"
                   << insert.material << "' for insert '" << insert.name
                   << "'; skipping." << G4endl;
            continue;
        }

        auto mesh = CADMesh::TessellatedMesh::FromSTL(path);
        mesh->SetScale(1);
        mesh->SetOffset(0, 0, fTargetCenter + insert.zOffset);

        G4LogicalVolume *logical = new G4LogicalVolume(mesh->GetSolid(), material, (insert.name + "LV").c_str());
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logical, insert.name.c_str(), world, false, 0);
    }

#endif
}
