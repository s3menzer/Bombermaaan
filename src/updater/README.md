# Updater library

`updater.cpp` contains the platform-independent update logic used by
Bombermaaan. The embedding application supplies its identity and paths through
`updater::Config`.

On Switch, Bombermaaan checks:

```text
sdmc:/switch/bombermaaan-update.json
sdmc:/switch/bombermaaan-update.yaml
sdmc:/switch/bombermaaan-update.yml
```

Each file may contain `manifest_url: ...` (JSON or simple YAML). The manifest
contains `platform`, `version`, `build`, `download`, and optionally `sha256`.
The numeric `build` is monotonically increasing and is checked independently
of `version`, so rebuilding the same application version still produces an
update.
Downloads are written atomically when possible, or staged as
`Bombermaaan.nro.pending` for the next launch.

Embedders can set `Config::progressCallback` to receive human-readable
progress messages such as network initialization, manifest parsing, download,
verification, and installation. `progressUserData` is passed through to the
callback.
