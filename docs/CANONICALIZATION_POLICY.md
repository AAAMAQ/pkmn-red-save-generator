# Canonicalization Policy

This generator must turn semantic input into a deterministic, valid Pokemon Red save without consulting the target `.red.json` raw image.

## Precedence Order

When generating a byte value, the intended precedence is:

1. target semantic field from `decoded`
2. deterministic value derived from target semantic input
3. fixed format constant required by the save layout
4. audited canonical template byte from the standalone dummy `.sav`
5. explicit failure if the byte cannot yet be assigned safely

## Physical-Image Rule

Forbidden:

- target `.red.json` `physicalImage`
- any raw byte range copied from the target `.red.json`
- any hidden fallback that reconstructs output from target raw bytes

Allowed:

- the standalone dummy template `.sav`, after explicit template validation

## Byte Categories

Every output range must eventually fall into one of these categories:

- overwritten from target semantic input
- derived from target semantic input
- synchronized duplicate/cache data
- regenerated checksum
- retained canonical dummy default
- intentionally cleared
- unsupported
- unknown but inherited
- runtime/scratch inherited
- fixed format constant

## Milestone 0 Default Position

Until a range is implemented safely:

- do not guess
- do not zero blindly
- do not silently inherit semantically meaningful dummy state in covered regions
- prefer explicit `unsupported` or `unknown but inherited` documentation

## Template Policy

If the dummy `.sav` is used:

- load it from a repository-relative path or explicit CLI/config override
- validate path, size, hash, profile, and non-collision status
- copy it into an in-memory working buffer
- never modify the original template file
- record which ranges remained inherited

## Unknown And Runtime Data

Initial policy by class:

| Class | Current policy |
|---|---|
| Fixed constants | Write deterministic required values. |
| Derived fields | Recompute from semantic state rather than inheriting stale bytes. |
| Duplicated caches | Preserve each game-visible semantic representation when the game permits divergence; derive a mirror only when the format contract proves it is a true mirror. |
| Checksums | Always regenerate. |
| Unsupported semantic regions | Do not silently claim correctness; fail or warn according to milestone policy. |
| Runtime-heavy regions | Prefer inherited template bytes until the game-proven behavior is understood. |
| Unknown tails | Keep inherited only if the profile is fixed and the range is explicitly documented as inherited. |

## Current Dummy Template Caveat

The audited dummy template currently shows:

- valid main checksum
- invalid bank 2 all-box checksum
- invalid bank 3 all-box checksum
- permanent boxes 1-12 decoding as fully occupied with invalid-looking data

Therefore:

- inheriting permanent box ranges is not acceptable for outputs that claim storage support
- inheriting bank 2/3 checksum bytes is not acceptable
- current generator builds fully overwrite permanent storage and regenerate all storage checksums
- the dummy now remains inherited only for explicitly documented runtime and unknown-tail ranges

## Current Milestone 5-6 Canonical Rules

- permanent boxes are written from `decoded.pcStorage`
- the Bank 1 current-box working copy is written independently from `decoded.currentBoxCache.cache`; it is not replaced by the selected permanent box when the selected-box dirty bit is set
- the selected-box number and dirty bit are preserved semantically, while the permanent selected box and current working box are each validated for internal structure and operational viability
- current-box/permanent-box divergence is reported explicitly; it is valid only when the selected-box dirty state and semantic input support that divergence
- boxed storage current HP, level, status, types, catch-rate byte, moves, trainer ID, experience, Stat Experience, DVs, and PP are preserved from corrected semantic decoding rather than inferred from the wrong record offsets
- Daycare writes the boxed substructure level from species growth plus experience and writes the Daycare-specific trailing level byte from semantic input
- named event-state is merged from `events`, `trainerBattles`, `staticBattles`, and `storyProgress`; conflicting flag claims fail generation
- named event, missable, and script ranges are cleared before semantic values are written so stale template bits cannot leak into owned Milestone 6 ranges
- unnamed event bits are canonicalized to clear until a named semantic authority exists
- named story-evidence/world bits are written from `decoded.worldState.storyEvidence`
- unsupported source locations are fail-closed by canonicalizing generated output to the emulator-validated Red's-house baseline with a warning
- Viridian City Pokemon Center and other non-baseline maps are disabled until full map-runtime bytes have semantic authority, synchronization rules, and emulator proof
- generation reports include byte provenance for declared write ranges and generation fails on undeclared non-template overlaps
- dry-run and validation CLI workflows use the same generator pipeline so canonicalization, checksum, range, determinism, and physical-image-isolation behavior can be inspected without writing a final `.sav`
- public sample generation uses synthetic semantic examples derived from the committed dummy reference after removing `physicalImage`; these samples are validation aids, not private playthrough fixtures
- release tagging requires emulator evidence that canonicalized outputs load, interact, save again, and reparse without unexpected semantic drift

## Output-Length Policy

Milestone 0 proposal:

- first supported output size: standard `0x8000`
- trailing bytes from source `.red.json`: ignored for generation, but reported as unsupported provenance
