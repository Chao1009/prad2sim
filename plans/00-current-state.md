# Current state & gap analysis

## 1. What `prad2sim` already is

`prad2sim` is a partial rewrite of `../PRadSim` (the JeffersonLab/PRadSim
baseline). Compared with the baseline it has already gained:

- **JSON configuration with inheritance** — `include/SimConfig.hh` +
  `src/SimConfig.cc` load a config file, follow its `_base` key, and deep-merge
  (override wins). Bundled `include/json.hh` (nlohmann). Configs in `config/`:
  `defaults.json`, `prad.json`, `drad.json`, `test.json`. Units in the JSON are
  cm / MeV / deg; conversion to Geant4 units happens in the loader/consumers.
- **Detector code split** by config rather than one 1200-line file:
  - `src/DetectorConstruction.cc` — ctor reads config into members, `Construct()`
    dispatches on `fConfig`, and `DefineMaterials()` (a ~225-line monolith) lives
    here.
  - `src/DetectorCommon.cc` (588 lines) — the `test` geometry plus the shared
    builders `AddVaccumBox()`, `AddGEM()`, `AddHyCal()`.
  - `src/DetectorPRad.cc` (192) and `src/DetectorDRad.cc` (228) — per-config
    volume + SD definitions.
- **Variable-size HyCal** — `5318812 rewrite hycal for variable module size`,
  Ilya crystal-attenuation model, pile-up gun model, PRadAnalyzer-consistent
  step limit (recent commits).
- **CLI** — `prad2sim.cc` parses `-c/--conf`, `-p/--physics`, `-s/--seed`;
  physics-list string supports `EM*`, `*_LOCAL`, `*_EXTRA` suffixes; run number
  is tracked in `output/file.output`.
- **Generators** retained under `evgen/` (esepp, moller, newep, newee, aao, eds,
  qesed, …) plus in-tree `PRadPrimaryGenerator`, `DRadPrimaryGenerator`,
  `CosmicsGenerator`, `DeuteronDisintegration`.
- **Digitization** subproject (`digitization/`) builds `PRadDig` + `PRadRec`,
  targeting EVIO for the *old* PRadAnalyzer.

## 2. The three reference forks (what to harvest)

### `../PRadSim` — baseline
Monolithic `DetectorConstruction.cc` (~1215 lines) and `PrimaryGenerator.cc`
(~1240 lines). Hardcoded module counts (1728), max-hits (300), max-tracks
(3000), survey positions, crystal sizes, trigger threshold. Truth tree `T`.
This is the structure prad2sim is moving away from.

### `../PRadSim_PRad2` — PRad-II geometry (Sep 2024 →)
Newer, richer geometry. Folds in:
- **CAD import** of housing / scintillator via STL (`database/CADmodel/`,
  needs `CADMesh`).
- **4 physical EJ204 scintillator planes** + 4 virtual detectors (vs single
  plane), with a corrected `PRad_Housing.stl`.
- **GEM positions referenced to the target** (5407 / 5807 mm from target) and a
  **3 mm drift gas** sensitive layer.
- **HyCal SD enabled** by default.
- **In/out hit positions** (`fXOut/fYOut/fZOut`) on every tracking hit.
- New materials (Aluminized Kapton, Nomex), target cell aperture 2→1 mm.
- **Geant4 11.4 + Qt6** API (`G4VisAttributes::GetInvisible()` etc.).
- Macros split by channel: `prad.mac`, `prad_ep.mac`, `prad_ee.mac`.

### `../PRadSim_X17` — X17 search (Sep 2024 →, latest Mar 2026)
PRad geometry tuned for a low-mass e+e- mediator search:
- **X17 vacuum window**: `Solid010.stl` (small Al window) + the
  `X17_Window_Adapter_No_Holes_{Al,Steel,Viton}.stl` adapters, currently
  **hardcoded** in `AddVaccumBox()`.
- **Beam-pipe shielding** boxes around the pipe.
- Same 3 mm GEM drift gas + in/out positions as PRad2; G4 11.4 / Qt6.
- No dedicated X17 signal generator in-tree — e+e- comes from the existing
  pair/Møller generators; an X17→e+e- generator/decay is a future addition.

## 3. Target data interface (`REPLAYED_DATA.md`)

The PRad-II replay tools write, per ROOT file, a main per-event tree plus
`scalers` / `epics` / `runinfo` side trees:

- **`events`** (raw): `hycal.*` FADC250 arrays over **all** channels
  (HyCal+Veto+LMS, distinguished by `module_type` 0–4 and a globally-unique
  `module_id`), raw `gem.*` strip data, and raw VTP/TDC banks. Optional soft +
  firmware peak analysis with `-p`.
- **`recon`** (reconstructed, lab frame, mm): HyCal clusters (`cl_x/y/z`,
  `cl_energy`, `cl_center`, …), GEM hits (`gem_x/y/z`, charges, sizes), and
  per-cluster HyCal↔GEM matches, plus Veto/LMS peak summaries.

This is the interface `prad2evviewer` and the rest of the `prad2*` chain read.

**Bridge already exists:** the `prad2` toolkit ships `prad2ana_sim2replay`
(`prad2evviewer/analysis/tools/sim2replay.cpp`), which converts sim truth files
into the `recon` tree. So the headline goal is mostly "make prad2sim emit the
truth `T` format `sim2replay` reads," not "write a new recon writer." The exact
contract is documented and **validated end to end** in
[03-data-interface.md](03-data-interface.md) and `../tests/sim2replay/`.

**Environment caveat (this machine):** ROOT 6.28 and the `prad2` toolkit are
installed; **Geant4 is not** — the data interface is testable here, but the
Geant4-side refactor must be built on a Geant4 11.4 box.

## 4. Gap analysis (what this rework must deliver)

| Area | Now | Target |
|---|---|---|
| **Output format** | Legacy truth tree `T`; EVIO for old PRadAnalyzer | `events` + `recon` trees per `REPLAYED_DATA.md`, consumed by `prad2*` |
| **Configs** | `prad`, `drad`, `test` | + `prad2`, `x17`; one code path, config-driven |
| **Geometry source** | Hardcoded in C++ builders | Config-driven; CAD/STL import for housing, windows, adapters |
| **Hit content** | Entry position only, fixed `kMaxNHits=300` arrays | In/out positions; right-sized / vector branches |
| **GEM sensitive layer** | baseline | 3 mm drift gas (PRad2/X17) |
| **Scintillator** | single plane | 4-plane + virtual detectors (PRad2) |
| **Materials** | ~225-line monolith in `DetectorConstruction.cc` | `MaterialFactory` / table |
| **Detector assembly** | dispatch on `fConfig` string in several files | composable detector-module registry |
| **Generators** | retained, ad-hoc selection | factory; X17→e+e- signal path |
| **Build / toolchain** | CMake ≥2.6, `file(GLOB)`, no G4 11.4/Qt6 guards | CMake ≥3.16, explicit/target-based, G4 11.x + Qt6 |
| **SD boilerplate** | repeated Register/Clear/array logic | shared base, auto-registered branches |

## 5. Decisions (confirmed with user, 2026-06-12)

1. **Data-interface depth** — **Both, Phase A first**: ship a truth-based `recon`
   tree now, then add raw-waveform `events` + real-recon validation. See
   [03-data-interface.md](03-data-interface.md).
2. **Format coupling** — **link `prad2` structs/clusterer** (`../prad2_decoder` /
   `prad2det`) so the branch layout and ID encodings cannot drift from the real
   format.
3. **Toolchain** — target **Geant4 11.4 + Qt6** (matches the newest forks);
   add the `GetInvisible()`-style API guards.
4. **Keep the old `T` truth tree** alongside the new output, behind a config flag
   (`output.truth`) — useful for acceptance/efficiency studies.

Remaining minor question: whether the old-PRadAnalyzer EVIO `digitization/` path
is retired once `events`/`recon` lands (default: keep until Phase B validates).
