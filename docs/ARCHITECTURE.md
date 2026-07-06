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
- `PartyMoveState`
- `PokemonDVState`
- `PokemonStatExperienceState`
- `EventSubsetState`

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
- `PokemonStatCalculator`
- `MinimalSaveGenerator`

### Integrity

- `ChecksumAlgorithms`
- `MainChecksumWriter`
- `BankChecksumWriter` (deferred)
- `IntegrityValidator`

### Verification And Reporting

- `SemanticComparator`
- `PartyComparisonRules`
- `GenerationReport`

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
- Milestone 4 still defers:
  - permanent PC storage serialization
  - daycare occupancy reconstruction
  - Hall of Fame reconstruction
  - broader story and battle event coverage
