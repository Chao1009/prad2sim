# Configurations: prad / drad / prad2 / x17

All four come from one codebase. Differences are expressed in JSON configs
(geometry, materials, SD flags, generator) plus a small number of
config-selected detector modules. Each `config/<name>.json` sets `"_base":
"defaults.json"` and `"config": "<name>"` and overrides only what differs.

## 1. Shared baseline (`defaults.json`)
World, target, GEM, scintillator, HyCal, recoil, SD flags, generator — already
present. Extend the schema with the fields the four configs need (below).

## 2. `prad` — hydrogen elastic e-p
- Target: **H2 gas**, no recoil detector.
- GEM 2 planes, HyCal on, scintillator off (or virtual only).
- Generator: elastic ep (`esepp`/`newep`) + Møller background.
- Mostly already encoded in `config/prad.json` (GEM/HyCal z overrides).

## 3. `drad` — deuteron, with recoil
- Target: **D2 gas**; **recoil detector ON** (20-segment, 2-layer Si).
- Scintillator plane ON. Generators add quasi-elastic / disintegration
  (`qesed`, `DeuteronDisintegration`).
- Already encoded in `config/drad.json` (SD flags); add target material + recoil
  geometry overrides explicitly rather than relying on defaults.

## 4. `prad2` — PRad-II (harvest from `../PRadSim_PRad2`)
New config file `config/prad2.json`. Differences to encode:
- **GEM** centers referenced to target (5407 / 5807 mm from target), **3 mm
  drift gas** sensitive layer.
- **Scintillator**: 4 physical EJ204 planes (65×100×3 mm, 40 mm central hole) at
  the two corrected z's + 4 virtual detectors → select `ScintillatorModule`
  variant `"four_plane"`.
- **Housing**: import `PRad_Housing.stl` (CAD) at `target + 308.38 mm`.
- **HyCal SD on** (already default here).
- Materials: Aluminized Kapton, Nomex; target cell aperture 1 mm.
- Macros mirror `prad_ep.mac` / `prad_ee.mac` (file-based ep / Møller).
- **This is the config whose output must match `REPLAYED_DATA.md`** — the module
  map / `module_id` scheme is the PRad-II HyCal (PbWO4 + PbGlass + Veto + LMS).

## 5. `x17` — X17 search (harvest from `../PRadSim_X17`)
New config file `config/x17.json`, based on `prad2` with:
- **Vacuum window**: `Solid010.stl` small Al window + `X17_Window_Adapter_No_
  Holes_{Al,Steel,Viton}.stl`. Drive these from config (paths/scale/offset)
  through `VacuumSystemModule`, not hardcoded in `AddVaccumBox()`.
- **Beam-pipe shielding** boxes (config-toggled).
- GEM 3 mm drift + in/out positions (same as prad2).
- **Signal**: X17 → e+e-. Two options:
  - (A) event-file generator under `evgen/` producing e+e- pairs with the X17
    kinematics (simplest; reuses the file-gun path); or
  - (B) a Geant4 custom boson (m≈17 MeV, narrow) + decay channel in a physics
    list. More faithful, more work. **Recommend (A) first**, (B) later if needed.

## 6. Config schema additions (proposed)

```jsonc
{
  "config": "prad2",
  "_base": "defaults.json",

  "target":   { "material": "H2Gas", "cell_aperture_mm": 1.0 },

  "gem": {
    "center_ref": "target",          // "target" | "absolute"
    "center": [540.7, 580.7],        // cm from ref
    "drift_mm": 3.0
  },

  "scintillator_plane": {
    "type": "four_plane",            // "single" | "four_plane" | "none"
    "z": [30.488, 31.188]            // cm from target
  },

  "cad": {                            // optional STL imports
    "housing":  { "stl": "database/CADmodel/PRad_Housing.stl", "offset_cm": 30.838 }
  },

  "vacuum": {
    "window": "standard"             // "standard" | "x17"
  },

  "sensitive_detectors": { "hycal": true, "scintillator_plane": true, "virtual": true },

  "output": { "truth": true, "events": false, "recon": true }
}
```

`x17.json` would set `"vacuum": { "window": "x17" }`, add the adapter STL list,
`"shielding": true`, and an X17 generator block.

## 7. Validation matrix
For each config, a smoke test should confirm: geometry builds without overlaps,
the expected SDs are attached, a few hundred events run, and the output trees
contain the expected branches. See [04-roadmap.md](04-roadmap.md).
