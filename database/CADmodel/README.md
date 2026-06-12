# CAD models (STL)

The prad2 / x17 configurations import the PRad-II vacuum window assembly and
the scintillator housing from STL files via CADMesh (see the `cad` section of
`config/prad2.json` / `config/x17.json`).

The STL files (~50 MB total) are **not version-controlled**. Copy them from
the PRadSim_PRad2 / PRadSim_X17 repositories:

```bash
cp ../PRadSim_PRad2/database/CADmodel/*.stl database/CADmodel/
```

Sources:
- `PRad_Housing.stl` + `Solid*.stl` — [YuanLiSDU/PRadSim_PRad2](https://github.com/YuanLiSDU/PRadSim_PRad2) `database/CADmodel`
- X17-specific window adapters — [YuanLiSDU/PRadSim_X17](https://github.com/YuanLiSDU/PRadSim_X17) `database/CADmodel`

Missing files are reported at startup and skipped (the simulation still runs,
with reduced material-budget fidelity around the vacuum exit window). Builds
configured with `-DPRAD2SIM_USE_CADMESH=OFF` skip all CAD inserts.
