# Target architecture

Goal: one package, four experiment configs, geometry/physics/output driven by
config, with small composable units that are easy to read, test, and extend.

## 1. Layering

```
 prad2sim.cc        CLI, RunManager, physics-list selection, output file
   │
   ├─ SimConfig                 JSON load + inheritance + unit handling
   │
   ├─ DetectorConstruction      thin: owns a DetectorAssembly built from config
   │     └─ DetectorAssembly    ordered list of DetectorModule's
   │           ├─ TargetModule
   │           ├─ VacuumSystemModule   (chamber, box, windows, X17 adapter)
   │           ├─ GEMModule            (N planes, 3 mm drift)
   │           ├─ ScintillatorModule   (single OR 4-plane)
   │           ├─ HyCalModule          (variable module table)
   │           └─ RecoilModule         (DRad only)
   │
   ├─ MaterialFactory           all G4Material + vis attributes
   ├─ Sensitive detectors       StandardDetectorSD / CalorimeterSD (+ in/out)
   ├─ Generators                PrimaryGeneratorAction + GeneratorFactory
   └─ Output                     TruthTree (T) + DigiWriter (events/recon)
```

## 2. Detector modules (biggest structural change)

Replace the `if (fConfig == ...)` dispatch with a **`DetectorModule`
interface** so each subsystem is independently testable and reusable across
configs:

```cpp
class DetectorModule {
public:
  virtual ~DetectorModule() = default;
  virtual const char* Name() const = 0;
  // build geometry into `world`, given resolved config
  virtual void Build(G4LogicalVolume* world, const SimConfig&) = 0;
  // attach sensitive detectors / set branches
  virtual void ConstructSD(G4SDManager*) {}
};
```

`DetectorConstruction` becomes thin: it reads the config, asks an
**assembly builder** for the module list of the active config, calls `Build()`
on each in `Construct()` and `ConstructSD()` in `ConstructSDandField()`.

Config → module list (one table, not scattered conditionals):

| Module | prad | drad | prad2 | x17 |
|---|---|---|---|---|
| Target | H2 | D2 + recoil | H2 | H2/He |
| VacuumSystem | std | std | std | + X17 window adapter |
| GEM | 2 plane | 2 plane | 2 plane (3 mm) | 2 plane (3 mm) |
| Scintillator | — | plane | 4-plane | 4-plane |
| HyCal | on | on | on | on |
| Recoil | — | yes | — | — |

This keeps shared builders shared (one `GEMModule` used by all four) while
isolating the differences in config + a couple of module subclasses.

### CAD import
PRad2/X17 need STL geometry (housing, scintillator, X17 window/adapter). Add an
optional **`CADMesh`** dependency (guarded in CMake) and a small
`CadImporter` helper so STL paths, scale and offset come from config, not
hardcoded calls inside `AddVaccumBox()`.

## 3. Materials

Move `DefineMaterials()` out of `DetectorConstruction.cc` into a
**`MaterialFactory`** (`Build()` once, look up by name). Add PRad2/X17 materials
(Aluminized Kapton, Nomex, Viton). Keep density-ratio scaling (`fExtDensityRatio`)
as a factory parameter. Vis attributes live next to material creation.

## 4. Sensitive-detector layer

- **Unify `StandardHit`** to carry in *and* out positions + momenta (harvest
  PRad2/X17). One hit class for tracking detectors; `CalorimeterHit` stays for
  per-module energy accumulation.
- **Reduce SD boilerplate**: a base SD that, given a branch-name prefix, registers
  its arrays once and clears per event. Concrete SDs only define how a step maps
  to a hit. Candidate improvement: replace fixed `kMaxNHits=300` C-arrays with
  `std::vector<...>` branches (no silent truncation, smaller files) — verify the
  digitization/readers can consume vector branches first.
- Keep `StepRecordSD` / `CheckScatteringSD` as opt-in debug SDs behind config
  flags.

## 5. Generators

- A **`GeneratorFactory`** keyed by `generator.type` / `event_type` from config,
  replacing the ad-hoc selection in `PrimaryGeneratorAction`.
- Keep external `evgen/` programs as-is (they pre-generate event files); the
  in-sim side just needs clean readers.
- Add an **X17 signal path**: either an X17→e+e- event-file generator under
  `evgen/`, or a Geant4 custom particle + decay. Decide in
  [02-configurations.md](02-configurations.md).

## 6. Output

Two writers, both optional via config (see [03-data-interface.md](03-data-interface.md)):
- **`TruthTree`** — the existing `T` tree (truth hits) for acceptance/efficiency
  studies. Keep, but right-size.
- **`DigiWriter`** — emits `events` and/or `recon` trees matching
  `REPLAYED_DATA.md`, with the correct `module_id` / `module_type` encoding.

Drop the global `gRootTree` singleton in favour of passing the writer through
`ActionInitialization` (removes hidden coupling; the SDs currently reach into
`gRootTree->GetTree()` directly).

## 7. Build & toolchain

- CMake **≥ 3.16**, target-based (`target_link_libraries(... PUBLIC ...)`),
  drop `${Geant4_USE_FILE}` include-style if moving to modern G4.
- Replace `file(GLOB ...)` with an explicit source list (GLOB doesn't
  re-trigger on new files and obscures the module structure).
- Guard Geant4 11.x / Qt6 API (`GetInvisible()`), and make `CADMesh` and the
  EVIO/PRadAnalyzer digitization **optional** components so the core sim builds
  without them.
- Consider splitting a `libprad2sim` core library + thin `main`, so the
  digitization and any unit tests can link the same code.

## 8. Naming / readability conventions

- Pick one member convention (`fCamelCase` is already dominant) and apply it.
- Lift magic numbers (survey positions, crystal sizes, thresholds, array caps)
  into config or named constants in one header.
- Keep the Geant4 license banner, but trim the duplicated `oooOO0OOooo` rulers to
  reduce noise (optional, low priority).
