//
// RunAction.hh
// Developer : (MT support addition)
// History:
//   Added for per-thread ROOT file lifecycle management in MT mode.
//

#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"

class G4Run;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class RunAction : public G4UserRunAction
{
public:
    RunAction() {}
    virtual ~RunAction() {}

    virtual void EndOfRunAction(const G4Run *);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
