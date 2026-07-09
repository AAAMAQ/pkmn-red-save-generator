# Known Limitations

## Repository State

- The project is implemented through completed Milestone 6 extended gameplay-state generation for the emulator-validated Red's-house safe-location profile.
- The CLI now supports save generation, dry-run generation, input inspection, save checksum validation, deterministic-output checks, physical-image-isolation checks, range/provenance display, and owned-field semantic comparison.
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
- Generation currently accepts only the emulator-validated Red's house second-floor baseline location.
- Viridian City Pokemon Center generation is disabled after the Milestone 5-6 load-time corruption incident.
- Broader safe-map support remains incomplete.
- Some boxed storage `level` fields still rely on permitted canonical differences because the boxed-level oracle is not yet trustworthy for every source fixture record.
- Daycare has a separate trailing stored-level byte and now writes that byte from semantic input for occupied Daycare.

## Validation Limitations

- Save Genie reparses have been completed for Milestone 2 and Milestone 3 owned-field outputs.
- Save Genie reparses and emulator save-again validation have now also been completed for a Milestone 4 projected-party output.
- Save Genie reparses and generator-side semantic comparison passed for the original full private Milestone 5 and 6 fixture, but emulator load immediately corrupted after Continue because an unsafe non-baseline location was admitted.
- Emulator load validation has been completed for a Milestone 2 generated save.
- Milestone 3 post-save `.sav` reparse after an emulator save-again cycle has now been completed successfully for the approved local validation path.
- Permanent PC storage is no longer preserved under Policy A in generated outputs; it is generator-owned and checksum-regenerated.
- Milestone 5 PC-storage interaction, box switching, game-triggered save, normal save, post-save reparse, cache behavior, and storage comparison have passed.
- Milestone 6 is complete for the Red's-house safe-location profile after emulator base-load, Hall of Fame viewing, gameplay travel, Box 11 deposit, Rattata capture, normal save-again, post-save Save Genie reparse, checksum validation, and expected-drift analysis.
- Direct Daycare deposit/withdraw emulator testing remains deferred to later representative-save validation because the Milestone 6 validation fixture had an empty Daycare.
- The first PC-storage viewing result is provisional because the tester had to progress gameplay before reaching Bill's PC.
- Post-gameplay current-box-cache state may differ from permanent selected-box state until the game writes the active box back during a box switch or save flow.
- The first controlled-interaction post-save artifact set contained one invalid copied save and one valid candidate proving deposit/cache/write-back behavior; a later final artifact completed withdrawal/Box 12 validation.
- Direct in-game Pokedex UI verification was not a primary Milestone 6 acceptance gate; current Pokedex proof comes from Save Genie reparse and semantic comparison.
- The Milestone 6 validation save uses numeric trainer ID `257` (`0x0101`), displayed by the game in five-digit format.

## Licensing Limitations

- No final license has been selected for this repository.
- The prerequisite repository also lacks a visible top-level license file.

## Research Limitations

- Revision identity for the supported Pokemon Red target profile is still conservative rather than proven.
- Safe handling for contradictory event combinations remains to be formalized during implementation.
- Unsafe map/location combinations now fail closed; broader map support requires full map-runtime serialization plus emulator evidence.
- The original committed dummy still has suspicious permanent-box data; generated outputs now avoid inheriting that storage payload, but the template's runtime-heavy and unknown-tail ranges still require conservative policy.
