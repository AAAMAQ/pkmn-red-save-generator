# Milestones 1–3 Execution Plan

This document records the approved execution runbook for Milestones 1, 2, and 3 and serves as the implementation gate for progress reporting.

## Execution Contract

- Work only in this repository.
- Treat `Pkmn Red Save Genie` as read-only.
- Complete one milestone at a time.
- Run milestone builds and tests before advancing.
- Stop after each milestone and report acceptance evidence.
- Do not modify the committed dummy save.
- Do not claim byte-identical reconstruction.
- Do not commit or push without explicit approval.

## Milestone 1

Required outcome:

- project layout established
- Xcode preserved
- CMake added
- vendored JSON and test single-header dependencies
- JSON reader
- supported-schema validation
- physical-image isolation
- semantic model for Milestone 3-owned fields
- template profile, loader, validator, and baseline analysis
- primitive encoders and bounded writers
- deterministic working-buffer initialization foundation
- `validate-input`, `validate-template`, and `show-profile` CLI commands

Acceptance evidence:

- files added or changed
- Xcode build result
- CMake build result
- test result
- semantic model coverage
- physical-image isolation proof
- template validation result
- Git status

## Milestone 2

Required outcome:

- minimal valid generated save
- exact write, inherit, and normalization range accounting
- required checksums regenerated
- Save Genie reparse success
- emulator load and save-again success

Special dummy-template rule:

- the committed dummy remains unchanged
- suspicious permanent-box and bank-checksum state must be proven harmless or normalized only in the working buffer

Acceptance evidence:

- minimal-state contract
- exact ranges written
- exact ranges inherited
- exact ranges normalized
- checksum results
- Save Genie reparse result
- semantic comparison result
- emulator load result
- save-again result
- remaining limitations

## Milestone 3

Required outcome:

- trainer/core serialization
- Pokédex serialization
- bag and PC inventory serialization
- conservative event subset serialization
- contamination tests
- determinism tests
- field-aware semantic comparison
- emulator validation records

Acceptance evidence:

- core fields implemented
- Pokédex coverage
- inventory coverage
- event subset coverage
- contamination-test results
- determinism-test results
- semantic-comparison results
- emulator save-again validation
- unresolved deferred fields

## Milestone 1 Module Plan

- `src/cli/`
  - command dispatch and user-facing CLI output
- `src/input/`
  - JSON reader, validator, and physical-image isolation
- `src/model/`
  - generator-owned semantic state
- `src/template/`
  - template profile, loading, validation, and baseline analysis
- `src/encoding/`
  - text, BCD, bitfield, and primitive writers
- `src/generation/`
  - deterministic working-buffer initialization
- `src/integrity/`
  - checksum algorithms for validation and later write phases
- `src/comparison/`
  - semantic-state comparison foundation
- `src/reporting/`
  - generation-report skeleton

## Profile Contract

Milestone 1 introduces a machine-readable profile with:

- `profileId`
- `targetGame`
- `regionAssumption`
- `revisionAssumption`
- `saveSize`
- `templateRelativePath`
- `templateSha256`
- `supportedSchemaVersions`
- `mainChecksumExpectation`
- `bank2ChecksumExpectation`
- `bank3ChecksumExpectation`
- `outputLengthPolicy`
- `trailingDataPolicy`
- `knownLimitations`

## Physical-Image Rule

The generator may parse the target `.red.json`, but generation must not depend on `physicalImage`.

Milestone 1 proof requires:

1. normal input
2. missing `physicalImage`
3. invalid `physicalImage`
4. unrelated `physicalImage`
5. different raw image with identical semantics

Expected result:

semantic mapping remains identical for identical semantic data.
