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
// CosmicsGenerator.hh
// Developer : Chao Gu
// History:
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef CosmicsGenerator_h
#define CosmicsGenerator_h 1

#include "TFoamIntegrand.h"

#include "G4VPrimaryGenerator.hh"

#include <memory>

class G4Event;

class TTree;
class TFoam;
class TRandom2;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class CosmicsGenerator;

class CosmicsIntegrand : public TFoamIntegrand
{
public:
    CosmicsIntegrand(CosmicsGenerator *gen, double e0, double eps, double rd, double nn);

    double Density(int nDim, double *arg);

    double E0, epsilon, Rd, n;

    double fEMin, fEMax;
    double fZenithMin, fZenithMax;
};

class CosmicsGenerator : public G4VPrimaryGenerator
{
    friend class CosmicsIntegrand;

public:
    static constexpr int kMaxN = 30;


    CosmicsGenerator();
    virtual ~CosmicsGenerator();

    virtual void GeneratePrimaryVertex(G4Event *);

protected:
    void Register(TTree *);

    void Print() const;
    void Clear();

    bool fRegistered;

    int fN;
    int fPID[kMaxN];
    double fX[kMaxN], fY[kMaxN], fZ[kMaxN];
    double fE[kMaxN], fMomentum[kMaxN];
    double fTheta[kMaxN], fPhi[kMaxN];

    double fEMin, fEMax;
    double fZenithMin, fZenithMax;

    std::unique_ptr<CosmicsIntegrand> fFoamI;
    std::unique_ptr<TRandom2> fPseRan;
    std::unique_ptr<TFoam> fETGenerator;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
