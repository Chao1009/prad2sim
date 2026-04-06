//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// PRadPrimaryGenerator.hh
// Developer : Chao Gu, Weizhi Xiong
// History:
//   Apr 2017, W. Xiong, Add target thickness profile.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef PRadPrimaryGenerator_h
#define PRadPrimaryGenerator_h 1

#include "PrimaryGenerator.hh"

#include "ConfigParser.h"

#include "TFoamIntegrand.h"
#include "Math/Interpolator.h"

#include <memory>

class G4Event;

class TFile;
class TFoam;
class TH1D;
class TH2D;
class TRandom2;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class PRadPrimaryGenerator;

class TargetProfileIntegrand : public TFoamIntegrand
{
public:
    TargetProfileIntegrand(PRadPrimaryGenerator *gen);

    double Density(int nDim, double *arg);

    ROOT::Math::Interpolator *fTargetProfile;
    double fZMin, fZMax;
};

class PRadPrimaryGenerator : public PrimaryGenerator
{
    friend class TargetProfileIntegrand;

public:
    PRadPrimaryGenerator(G4String type, G4bool rec, G4String par); // DRadPrimaryGenerator uses this
    PRadPrimaryGenerator(G4String type, G4bool rec, G4String par, G4String path, G4String pile_up_profile, G4String target_profile);
    virtual ~PRadPrimaryGenerator();

    virtual void GeneratePrimaryVertex(G4Event *);

protected:
    void LoadTargetProfile(const std::string &path);

    virtual double GenerateZ();

    G4String fEventType;

    G4bool fRecoilOn;
    G4String fRecoilParticle;

    TFile *fPileUpProfile;
    TH1D *fClusterNumber;
    TH2D *fClusterEvsTheta;

    std::unique_ptr<ROOT::Math::Interpolator> fTargetProfile;
    double fZMin, fZMax;

    std::unique_ptr<TargetProfileIntegrand> fFoamI;
    std::unique_ptr<TRandom2> fPseRan;
    std::unique_ptr<TFoam> fZGenerator;

    ConfigParser fParser;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
