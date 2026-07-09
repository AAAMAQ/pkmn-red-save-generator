# Pkmn Red Save Generator

Status: v1.0.0 release validation complete for the supported Pokemon Red profile. Milestones 0-6 and the combined Final Release Milestone, merging the original Milestones 7-9, are complete.

This project generates a new Pokemon Red `.sav` from semantic `.red.json` data produced by the completed Pokemon Red Save Genie. It is a semantic save generator, not an archival byte-for-byte reconstructor.

The target `.red.json` `physicalImage` is deliberately ignored as generation authority. The generator starts from the committed canonical dummy `.sav`, overwrites supported semantic state, regenerates integrity values, writes provenance, and validates through parser and emulator evidence.

## Project Boundary

- Writable project: this repository.
- Read-only prerequisite: sibling `Pkmn Red Save Genie` repository.
- Template resource: `Dummy Save/`.

The Save Genie is the trusted read-only parser and research oracle. The generator is separate:

- Save Genie: decode, analyze, and archival reconstruction using preserved raw bytes.
- Save Generator: deterministic semantic synthesis without using the target `physicalImage`.

## Current Coverage

Implemented and emulator-validated through Milestone 6:

- trainer and rival names
- trainer ID
- options and playtime
- money and coins
- badges plus badge mirror
- Pokedex owned and seen bitfields
- bag and PC item inventories
- active party Pokemon
- all 12 permanent PC boxes
- selected box and current-box cache
- per-box checksums
- Bank 2 and Bank 3 all-box checksums
- Daycare occupancy and deposited Pokemon structure
- Hall of Fame records
- named event flags
- trainer battle flags
- static battle flags
- story progress flags
- missable objects
- hidden items and hidden coins
- visited towns and Fly destination bitfields
- persistent script bytes exposed by Save Genie
- Red's-house safe runtime/world subset

Final release emulator validation confirmed normal Continue/load behavior, movement, menus, trainer/party/Pokedex/Bag/inventory display, broad travel through doors, stairs, warps, scripts, and map transitions, normal save-again, emulator shutdown, valid post-save checksums, and successful Save Genie reparse. Earlier milestone evidence covers PC storage interaction and Hall of Fame viewing; final validation did not directly inspect every subsystem again.

## Safety Rules

- The target `.red.json` supplies semantic state only.
- The target `physicalImage` is never used to generate output.
- The standalone dummy `.sav` under `Dummy Save/` is permitted only as a canonical initialized template.
- The dummy template is never modified in place.
- Outputs are gameplay-equivalent and structurally valid, not byte-identical to the source save.
- Unsupported non-empty state must fail or be documented; it must not be silently discarded.
- Non-baseline locations fail closed unless their full runtime state is emulator-proven.

## Safe Location Policy

Currently supported generated start location:

- Red's house, second floor.

Viridian City Pokemon Center remains a regression case. A previous generated save using map ID and coordinates without a complete runtime-state contract corrupted immediately after selecting Continue. The generator now rejects that raw source location until the full map-runtime cluster is owned and emulator-validated.

## Build

CMake:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Xcode:

```sh
xcodebuild -project "Pkmn Red Save Generator.xcodeproj" \
  -scheme "Pkmn Red Save Generator" \
  -configuration Debug \
  -derivedDataPath .xcode-derived \
  CODE_SIGNING_ALLOWED=NO \
  CODE_SIGNING_REQUIRED=NO \
  CODE_SIGN_IDENTITY= \
  build
```

## CLI Usage

Show help:

```sh
build/pkmn-red-save-generator --help
build/pkmn-red-save-generator --version
```

Validate semantic input without writing a save:

```sh
build/pkmn-red-save-generator validate input.red.json
build/pkmn-red-save-generator inspect input.red.json
```

Generate with repository defaults:

```sh
build/pkmn-red-save-generator generate input.red.json output.sav \
  --report output.sav.generation-report.json
```

Generate with explicit resources:

```sh
build/pkmn-red-save-generator generate \
  --input input.red.json \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json \
  --output output.sav \
  --report output.generation-report.json
```

Dry-run generation, summary, and range provenance:

```sh
build/pkmn-red-save-generator generate input.red.json dry-run.sav \
  --dry-run \
  --summary \
  --show-ranges \
  --report dry-run.report.json
```

Validate a generated save:

```sh
build/pkmn-red-save-generator validate-save --input-save output.sav
```

Validate an emulator-modified save whose selected box cache is intentionally dirty:

```sh
build/pkmn-red-save-generator validate-save --input-save post-save.sav --allow-dirty-current-box
```

Check deterministic output:

```sh
build/pkmn-red-save-generator check-determinism \
  --input input.red.json \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json \
  --work-dir local-validation/determinism
```

Check physical-image isolation:

```sh
build/pkmn-red-save-generator check-physical-image-isolation \
  --input input.red.json \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json \
  --work-dir local-validation/physical-image
```

Compare target semantics with a Save Genie reparse:

```sh
build/pkmn-red-save-generator compare-semantics \
  --target-json input.red.json \
  --reparsed-json generated.red.json
```

Generate from public samples:

```sh
build/pkmn-red-save-generator generate samples/minimal.red.json /tmp/minimal.sav \
  --report /tmp/minimal.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/minimal.sav

build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --report /tmp/representative.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/representative.sav
```

Confirm unsafe-location rejection:

```sh
build/pkmn-red-save-generator inspect samples/unsupported-viridian-pokemon-center.red.json
```

That command is expected to fail because non-baseline locations are rejected unless their full runtime state is implemented and emulator-proven.

## Validation Guarantees

For currently supported fields, validation covers:

- input schema and semantic model construction
- physical-image isolation
- template size/hash checks
- deterministic output
- write provenance and overlap detection
- main checksum
- per-box checksums
- Bank 2 and Bank 3 all-box checksums
- Save Genie reparse
- field-aware semantic comparison
- emulator load and save-again validation for Milestones 2-6
- final release emulator validation with public representative and private full-state candidates
- public sample generation, checksum validation, deterministic output, physical-image isolation, and unsafe-location rejection in CI

The game displays trainer IDs as five digits. The final private validation save uses numeric trainer ID `257` (`0x0101`), displayed in-game as `00257`.

## Known Non-Guarantees

- The generator does not claim byte-identical reconstruction.
- Unsupported locations are rejected rather than guessed.
- Broader map-runtime serialization is deferred.
- The public CI does not run an emulator or the private Save Genie oracle workflow.
- Public samples are synthetic validation inputs; private saves and emulator evidence are intentionally not committed.

## Documentation

Important project documents:

- `docs/PROJECT_ROADMAP.md`
- `docs/VALIDATION_PLAN.md`
- `docs/CLI_REFERENCE.md`
- `docs/FINAL_RELEASE_VALIDATION_MATRIX.md`
- `docs/RELEASE_CHECKLIST.md`
- `docs/MILESTONE_5_STORAGE_CONTRACT.md`
- `docs/MILESTONE_6_EXTENDED_STATE_CONTRACT.md`
- `docs/MILESTONE_5_6_LOAD_CORRUPTION_INCIDENT.md`
- `docs/SEMANTIC_EQUIVALENCE_CONTRACT.md`
- `docs/CANONICALIZATION_POLICY.md`
- `docs/KNOWN_LIMITATIONS.md`
- `samples/README.md`
