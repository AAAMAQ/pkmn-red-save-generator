# Architecture

## Current Status

The repository now uses a shared source layout while preserving the existing Xcode CLI target:

- `Pkmn Red Save Generator/main.cpp`
- `Pkmn Red Save Generator.xcodeproj`
- `src/`
- `tests/`
- `profiles/`
- `third_party/`

The entrypoint remains inside the original Xcode folder, but real logic now lives under `src/` so both Xcode and CMake build the same implementation.

## Active Source Layout

```text
src/
  cli/
  comparison/
  encoding/
  generation/
  input/
  integrity/
  model/
  reporting/
  template/
tests/
profiles/
third_party/
```

## Build Strategy

- Primary language standard: C++20
- Build paths:
  - existing Xcode CLI target
  - top-level CMake build
- Dependencies:
  - vendored `nlohmann/json`
  - vendored `doctest`
  - `CommonCrypto` for SHA-256 on Apple platforms

## Safe Reorganization Assessment

The Milestone 1 layout is acceptable for foundation work and keeps Xcode compatibility intact. Further reorganization should remain incremental rather than replacing the project structure wholesale.

Deferred cleanup:

- decide later whether the Xcode entrypoint should move into `src/cli/`
- decide later whether `rescource/` should be renamed or archived
- add dedicated fixture directories only when Milestone 2+ local test inputs are approved

## Proposed Runtime Components

### Input

- `RedJsonReader`
- `RedJsonValidator`
- `PhysicalImageIsolationGuard`

### Semantic Model

- `RedSemanticState`
- `IdentityState`
- `CoreState`
- `PokedexState`
- `InventoryState`
- `PartyState`
- `PartyPokemonState`
- `PokemonMoveState`
- `StoredPokemonState`
- `StorageState`
- `StorageBoxState`
- `DaycareState`
- `HallOfFameState`
- `NamedFlagState`
- `ScriptState`
- `MissableObjectState`
- `HiddenObjectState`
- `VisitedTownState`
- `PokemonDVState`
- `PokemonStatExperienceState`

### Encoding And Primitive Rules

- `Gen1TextEncoder`
- `BcdEncoder`
- `BitfieldWriter`
- `PrimitiveWriter`

### Template Layer

- `CanonicalTemplateLoader`
- `TemplateProfile`
- `TemplateValidator`
- `TemplateBaselineAnalyzer`
- `WorkingSaveBuffer`

### Generation

- `RedSaveInitializer`
- `MinimalStateContract`
- `CoreStateSerializer`
- `PartySerializer`
- `PartyValidator`
- `StorageSerializer`
- `StorageValidator`
- `DaycareSerializer`
- `HallOfFameSerializer`
- `ExtendedWorldSerializer`
- `PokemonStatCalculator`
- `MinimalSaveGenerator`

### Integrity

- `ChecksumAlgorithms`
- `BoxChecksumWriter`
- `IntegrityValidator`

### Verification And Reporting

- `SemanticComparator`
- `PartyComparisonRules`
- `GenerationReport`
- CLI validation workflows for input inspection, dry-run generation, checksum validation, determinism, physical-image isolation, and write-range provenance

## Data Flow

```text
target .red.json
-> schema/profile validation
-> physicalImage isolation
-> semantic-only model extraction
-> canonical dummy template validation
-> in-memory working buffer copy
-> semantic overwrite and derivation
-> duplicate/cache synchronization
-> checksum regeneration
-> structural validation
-> Save Genie reparse
-> semantic comparison
-> generation report
```

## Architecture Rules

- The target `.red.json` `physicalImage` is never read as generation authority.
- The standalone dummy `.sav` may be loaded as a template resource only after profile and immutability checks.
- Unknown and runtime-heavy ranges must have explicit policy.
- Coverage and provenance are first-class outputs, not optional notes.
- Milestone 1 is intentionally foundation-only; no production serializer beyond deterministic template copying is considered complete.
- Milestone 2 extends that foundation with a narrow minimal generator:
  - trainer and core identity writes
  - canonical empty party, Pokédex, bag, PC items, daycare, and Hall of Fame clears
  - regenerated main checksum
  - explicit Policy A preservation of untouched permanent storage banks until storage serialization is implemented
- Milestone 3 expands the active generator to own:
  - badges and synchronized badge mirror
  - Pokédex owned and seen bitfields
  - bag inventory and PC item inventory
  - the conservative event subset of visited towns, hidden items, and hidden coins
  - field-aware semantic comparison categories for exact, derived, and permitted deferred differences
- Milestone 4 extends ownership to:
  - full active-party serialization in the main save area
  - party count, species list, terminator, OT names, nicknames, and all six party records
  - field-aware party comparison down to indexed move, PP, DV, Stat Experience, and text paths
  - deterministic party generation that remains independent of target `physicalImage`
- Milestone 5 and Milestone 6 own:
  - all 12 permanent PC boxes
  - selected-box byte and current-box cache synchronization
  - per-box and bank-level storage checksums
  - daycare occupancy and stored Pokemon
  - Daycare-specific deposited level byte
  - Hall of Fame record count and entries
  - named event, trainer-battle, static-battle, and story-progress flags
  - named story-evidence/world bits
  - persistent script bytes, missable objects, hidden items, hidden coins, and visited towns
- Milestone 6 extended-state generation clears owned event, missable, and script ranges before writing target semantics to prevent template-state leakage.
- Milestone 6 has passed emulator validation for the Red's-house safe-location profile, including load, travel, Hall of Fame viewing, Box 11 deposit, Rattata capture, save-again, Save Genie reparse, and expected-drift analysis.
- Generated saves must synchronize the selected permanent box and current-box cache. Emulator-modified saves may legitimately have the selected box dirty flag set and a current-box cache that differs from the permanent selected box until the game flushes the active box during box switching.
- The largest remaining architecture gap is location safety:
  - the validator now accepts only the emulator-validated Red's-house baseline
  - Viridian City Pokemon Center was disabled after immediate post-Continue corruption
  - broader map coverage requires a complete map-runtime cluster serializer, duplicate synchronization, and emulator evidence before re-enablement
- Generation reports now include byte provenance for declared ranges and reject undeclared non-template overlaps before writing output reports.
