//
// RunAction.cc
// Developer : (MT support addition)
// History:
//   Added for per-thread ROOT file lifecycle management in MT mode.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "RunAction.hh"

#include "GlobalVars.hh"
#include "RootTree.hh"

#include "G4Run.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run *)
{
    // Each worker thread (and master in ST mode) closes its own ROOT file here.
    // In MT mode gRootTree is thread-local, so this is safe without any locks.
    if (gRootTree) {
        G4cout << "Closing output file for this thread." << G4endl;
        delete gRootTree;
        gRootTree = nullptr;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
