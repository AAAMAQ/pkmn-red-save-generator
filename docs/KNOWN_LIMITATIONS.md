# Known Limitations

## Repository State

- The project is still at Milestone 0.
- No generator implementation exists yet.
- The current `main.cpp` is only a placeholder.

## Dummy Template Limitations

- The audited dummy save is not semantically blank.
- Bank 2 and bank 3 all-box checksums are invalid.
- Permanent boxes 1-12 decode as fully occupied with invalid-looking data.
- The dummy may be useful only as a tightly controlled template base, not as an unquestioned canonical save state.

## Input Model Limitations

- Current support planning targets `.red.json` schema `0.1.0` only.
- `runtimeState` remains only partially suitable for write-back.
- Unknown Bank 1 and Bank 2/3 tail bytes still need stronger policy evidence.

## Validation Limitations

- No semantic generator output exists yet to compare.
- No emulator validation has been run for generated saves.
- No save-again validation has been run.

## Licensing Limitations

- No final license has been selected for this repository.
- The prerequisite repository also lacks a visible top-level license file.

## Research Limitations

- Revision identity for the supported Pokemon Red target profile is still conservative rather than proven.
- Safe handling for contradictory event combinations remains to be formalized during implementation.
- Unsafe map/location combinations still need conservative acceptance rules.
