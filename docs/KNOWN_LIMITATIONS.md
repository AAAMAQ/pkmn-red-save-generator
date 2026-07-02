# Known Limitations

## Repository State

- The project is implemented through Milestone 3 core owned-field generation.
- The CLI now supports save generation and owned-field semantic comparison.
- Full-save reconstruction is still incomplete.

## Dummy Template Limitations

- The audited dummy save is not semantically blank.
- Bank 2 and bank 3 all-box checksums are invalid.
- Permanent boxes 1-12 decode as fully occupied with invalid-looking data.
- The dummy may be useful only as a tightly controlled template base, not as an unquestioned canonical save state.

## Input Model Limitations

- Current support planning targets `.red.json` schema `0.1.0` only.
- `runtimeState` remains only partially suitable for write-back.
- Unknown Bank 1 and Bank 2/3 tail bytes still need stronger policy evidence.
- Milestone 3 currently accepts only the proven Red's house second-floor baseline location.
- Milestone 3 rejects non-empty party, occupied daycare, and non-empty Hall of Fame input states.

## Validation Limitations

- Save Genie reparses have been completed for Milestone 2 and Milestone 3 owned-field outputs.
- Emulator load validation has been completed for a Milestone 2 generated save.
- Milestone 3 post-save `.sav` reparse after an emulator save-again cycle has now been completed successfully for the approved local validation path.
- Milestone 3 contamination proof is strongest for owned fields and still narrower for preserved storage banks under Policy A because storage interaction itself remains deferred.
- Direct in-game Pokedex UI verification has not yet been performed; current Pokedex proof comes from Save Genie reparse and semantic comparison.

## Licensing Limitations

- No final license has been selected for this repository.
- The prerequisite repository also lacks a visible top-level license file.

## Research Limitations

- Revision identity for the supported Pokemon Red target profile is still conservative rather than proven.
- Safe handling for contradictory event combinations remains to be formalized during implementation.
- Unsafe map/location combinations still need conservative acceptance rules.
- Policy A is now justified for the validated load and save-again flow that does not touch PC storage, but it remains provisional for future scenarios that interact with permanent boxes or rely on their semantic meaning.
