//
// GlobalVars.hh
// Developer : Chao Peng, Chao Gu
// History:
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef GlobalVars_h
#define GlobalVars_h 1

#include "G4AutoLock.hh"
#include "G4Threading.hh"

class RootTree;
extern G4ThreadLocal RootTree *gRootTree;

// Serialize all TTree::Branch() calls across threads.
// ROOT's global class registry is not fully re-entrant during branch creation
// even with ROOT::EnableThreadSafety(). Each thread registers on its own tree,
// but internal ROOT state (TClass lookups, etc.) must not run concurrently.
extern G4Mutex gBranchMutex;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
