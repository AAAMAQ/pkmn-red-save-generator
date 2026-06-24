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
| Duplicated caches | Synchronize from the chosen canonical semantic structure. |
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

- inheriting permanent box ranges is not acceptable for any output that claims storage support
- inheriting bank 2/3 checksum bytes is not acceptable
- the dummy may only be used safely where the generator fully overwrites or intentionally excludes the affected regions

## Output-Length Policy

Milestone 0 proposal:

- first supported output size: standard `0x8000`
- trailing bytes from source `.red.json`: ignored for generation, but reported as unsupported provenance
