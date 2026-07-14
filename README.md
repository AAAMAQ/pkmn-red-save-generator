# Pkmn Red Save Generator

Status: v1.0.0 is the published historical baseline. A completed-playthrough manual proof subsequently exposed text, current-working-box, boxed-HP, and Hall of Fame defects. Corrective code and automated validation pass, and a focused second emulator retest confirmed all four corrections. Final semantic-sufficiency acceptance still awaits a normal save/shutdown/reload cycle and post-emulator battery-save analysis.

This project generates a new Pokemon Red `.sav` from semantic `.red.json` data produced by the completed Pokemon Red Save Genie. It is a semantic save generator, not an archival byte-for-byte reconstructor.

The target `.red.json` `physicalImage` is deliberately ignored as generation authority. The generator starts from the committed canonical dummy `.sav`, overwrites supported semantic state, regenerates integrity values, writes provenance, and validates through parser and emulator evidence.

## Project Boundary

- Writable project: this repository.
- Independent prerequisite: sibling `Pkmn Red Save Genie` repository. Parser-model corrections are maintained and reviewed in that repository rather than copied into the generator.
- Template resource: `Dummy Save/`.

The Save Genie is the independent parser and research oracle. The generator is separate:

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

Earlier milestone validation confirmed normal Continue/load behavior, movement, menus, trainer/party/Pokedex/Bag/inventory display, broad travel through doors, stairs, warps, scripts, map transitions, PC interaction, and normal save-again. The later completed-playthrough proof showed that parser acceptance and checksums were insufficient: operational defects remained in a valid punctuation glyph, divergent current-box state, boxed current HP, and Hall of Fame members. The corrected model is documented in `docs/FINAL_MANUAL_VALIDATION_FAILURE_ANALYSIS.md`.

## Safety Rules

- The target `.red.json` supplies semantic state only.
- The target `physicalImage` is never used to generate output.
- The standalone dummy `.sav` under `Dummy Save/` is permitted only as a canonical initialized template.
- The dummy template is never modified in place.
- Outputs are gameplay-equivalent and structurally valid, not byte-identical to the source save.
- Unsupported non-empty state must fail or be documented; it must not be silently discarded.
- Non-baseline source locations are canonicalized to the emulator-proven Red's house second-floor start location unless their full runtime state is later implemented and emulator-proven.

## Safe Location Policy

Currently supported generated start location:

- Red's house, second floor.

Viridian City Pokemon Center remains a regression case. A previous generated save using map ID and coordinates without a complete runtime-state contract corrupted immediately after selecting Continue. The generator now canonicalizes unsupported raw source locations to Red's house second floor and reports the canonicalization, rather than guessing unsafe runtime state.

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

Validate an emulator-modified save with a current working box that differs from permanent storage:

```sh
build/pkmn-red-save-generator validate-save --input-save post-save.sav
```

The two box representations are validated independently. Legitimate divergence is reported but does not require an override flag.

Validate operational structures and inspect storage:

```sh
build/pkmn-red-save-generator validate-text --text 'Lt<DOT>Ash'
build/pkmn-red-save-generator validate-boxes --input input.red.json
build/pkmn-red-save-generator validate-pokemon-operability --input input.red.json
build/pkmn-red-save-generator validate-hall-of-fame --input input.red.json
build/pkmn-red-save-generator inspect-box --input input.red.json --box 12 --current-cache
build/pkmn-red-save-generator inspect-pokemon --input input.red.json --box 1 --slot 2
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
  --reparsed-json generated.red.json \
  --report semantic-comparison.json \
  --markdown semantic-comparison.md
```

Compare physical images or analyze an emulator save cycle:

```sh
build/pkmn-red-save-generator compare-physical \
  --original original.sav \
  --generated generated.sav \
  --report physical-comparison.json \
  --markdown physical-comparison.md

build/pkmn-red-save-generator analyze-post-emulator \
  --before generated.sav \
  --after generated-post-emulator.sav \
  --report post-emulator-comparison.json \
  --markdown post-emulator-comparison.md
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

Confirm unsupported-location canonicalization:

```sh
build/pkmn-red-save-generator inspect samples/unsupported-viridian-pokemon-center.red.json
```

That command is expected to pass with a warning that the generated output will start in Red's house second floor.

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
- Save Genie reparse plus independent binary structure checks
- field-aware comparison of complete party, storage/cache, and Hall of Fame contents
- lossless text-token validation without fallback substitution
- boxed-Pokemon withdrawal viability and exact Gen I stat derivation
- emulator load and save-again validation for Milestones 2-6
- manual emulator validation, including a recorded false-positive proof that now remains a permanent regression case
- public sample generation, checksum validation, deterministic output, physical-image isolation, and unsupported-location canonicalization in CI

The game displays trainer IDs as five digits. The final private validation save uses numeric trainer ID `257` (`0x0101`), displayed in-game as `00257`.

## Known Non-Guarantees

- The generator does not claim byte-identical reconstruction.
- Unsupported locations are canonicalized to Red's house second floor with an explicit warning rather than guessed.
- Broader map-runtime serialization is deferred.
- The public CI does not run an emulator or the private Save Genie oracle workflow.
- Public samples are synthetic validation inputs; private saves and emulator evidence are intentionally not committed.
- The corrected completed-playthrough candidate passed the focused emulator checks for punctuation, current Box 12 state, boxed-Pokemon withdrawal HP, and Hall of Fame contents. A post-emulator save was not produced during that focused check, so save-again durability and post-save reparse remain pending.

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
- `docs/FINAL_MANUAL_VALIDATION_FAILURE_ANALYSIS.md`
- `docs/FINAL_SEMANTIC_SUFFICIENCY_PROOF.md`
- `docs/SEMANTIC_EQUIVALENCE_CONTRACT.md`
- `docs/CANONICALIZATION_POLICY.md`
- `docs/KNOWN_LIMITATIONS.md`
- `samples/README.md`

## License

Original project code and project-authored documentation are available under
the MIT License. Copyright (c) 2026 MAQ / BiG MAQ Studios. See `LICENSE`.

The license includes a non-binding stewardship note asking users to keep this
work oriented toward education, research, archival preservation, and
retro-development, and not merely repackage it as software for sale. This is a
personal request, not an additional restriction on the MIT permissions.

Vendored and third-party materials retain their own licenses and attribution.

This is an independent educational, research, archival, and preservation project. Pokemon, Nintendo, Game Freak, Creatures, and related names, trademarks, and copyrighted materials belong to their respective owners. This project does not distribute ROMs, emulator binaries, or copyrighted game assets.
