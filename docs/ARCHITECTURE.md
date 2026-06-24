# Architecture

## Milestone 0 Status

The repository currently preserves the existing Xcode skeleton:

- `Pkmn Red Save Generator/main.cpp`
- `Pkmn Red Save Generator.xcodeproj`

No source reorganization was performed during Milestone 0. This was intentional so repository setup, audit work, and path stability could happen before build-system changes.

## Safe Reorganization Assessment

The current layout is acceptable for initialization, but it is not a good long-term architecture for a semantic generator. A future safe reorganization should move the codebase toward a source layout such as:

```text
src/
  cli/
  input/
  model/
  encoding/
  template/
  generation/
  integrity/
  comparison/
  reporting/
tests/
fixtures/
docs/
```

Recommended Milestone 1 reorganization:

1. Keep the existing Xcode project.
2. Introduce `src/` and `tests/` under the repository root.
3. Move placeholder `main.cpp` into `src/cli/` only after the Xcode project is updated safely.
4. Leave `Dummy Save/` untouched as a fixed audit resource directory.
5. Leave `rescource/` untouched during early bring-up; reassess later whether to move it into a documented `references/` area.

## Proposed Runtime Components

### Input

- `RedJsonReader`
- `RedJsonValidator`
- `InputFieldContract`
- `TargetProfileResolver`

### Semantic Model

- `RedSemanticState`
- `TrainerState`
- `PokedexState`
- `InventoryState`
- `PartyState`
- `StorageState`
- `DaycareState`
- `HallOfFameState`
- `EventState`

### Encoding And Primitive Rules

- `Gen1TextEncoder`
- `BcdEncoder`
- `BitfieldWriter`
- `PrimitiveWriter`

### Template Layer

- `DummySaveLoader`
- `TemplateProfile`
- `TemplateValidator`
- `WorkingSaveBuffer`

### Generation

- `TrainerSerializer`
- `InventorySerializer`
- `PokedexSerializer`
- `PartySerializer`
- `BoxSerializer`
- `DaycareSerializer`
- `HallOfFameSerializer`
- `EventSerializer`
- `CacheSynchronizer`
- `RedSaveGenerator`

### Integrity

- `MainChecksum`
- `BankChecksum`
- `BoxChecksum`
- `IntegrityValidator`

### Verification And Reporting

- `SemanticComparator`
- `EquivalenceReport`
- `GenerationReport`

## Data Flow

```text
target .red.json
-> schema/profile validation
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
