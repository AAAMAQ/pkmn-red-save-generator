# Final Semantic Sufficiency Proof

Status: corrected automated phase and focused second emulator retest passed; final acceptance awaits save-again durability and post-emulator save analysis.

## Research Question

Does the supported Pokemon Red Save Genie semantic model contain enough gameplay information to independently generate a new Pokemon Red save without using the source save's physical image as generation authority?

The target is gameplay-semantic equivalence, not byte-identical reconstruction. Physical bytes may differ because the generator writes supported structures into a canonical template, preserves or derives documented semantic state, regenerates checksums, and canonicalizes unsupported runtime locations.

## Source Fixture

The private completed-playthrough fixture is referred to as `originalRed2026.sav`.

- Size: 32768 bytes
- SHA-256: `a09b29f6c4c928c0e38d44d95c07076399a552aa5e3ef1d77680a1dd5d3ce9cc`
- Source status: private validation fixture, not committed
- ROM status: not copied, committed, or redistributed

## Method

1. Preserve and hash the source and all prior evidence.
2. Decode the source through Pokemon Red Save Genie.
3. Generate only from supported semantic fields.
4. Remove and replace `physicalImage`, then prove identical outputs.
5. repeat generation three times and prove byte identity;
6. validate every checksum, box structure, text field, stored Pokemon, Hall of Fame record, and declared write range;
7. reparse with Save Genie;
8. compare every supported semantic structure field-by-field;
9. compare physical bytes as evidence rather than acceptance authority;
10. validate behavior in an emulator and reparse the resulting battery save.

## First Automated Pass

The first candidate had SHA-256 `2f57392a4f08983f397c5a257d7b0341557aff76a588a1cc26e0ce6938fa2c62`. It passed checksums, determinism, physical-image isolation, Save Genie reparse, and the old comparator.

That automated success was a false positive. Manual emulator testing proved:

- valid punctuation changed to `?`;
- selected Box 12 lost its 20-member player-visible working state;
- Box 1 Dugtrio withdrew as `0/201` instead of `201/201`;
- Hall of Fame records lost Charizard and displayed invalid `?? BIRD` entries.

The successful portions of the first emulator pass remain useful evidence: boot/load, movement, maps, Fly, Pokedex, party menus, battles, Pokemon Centers, healing, inventory, Bill's PC access, box switching, Daycare inspection, and save-again all worked.

## Corrective Investigation

The exact causes were:

- incomplete text codecs conflated byte `0xF2` with an unknown fallback;
- permanent selected storage was incorrectly treated as authority over the Bank 1 current working box;
- the box parser read level one byte past the record and forced stored HP/status to zero;
- the Hall of Fame parser treated internal species IDs as if they were National Dex numbers, rejecting Charizard internal ID `0xB4`.

The detailed raw-byte and emulator analysis is in `docs/FINAL_MANUAL_VALIDATION_FAILURE_ANALYSIS.md`.

## Corrected Automated Result

- Corrected filename for retest: `generatedRed2026.sav`
- Size: 32768 bytes
- SHA-256: `e8d7a72121ae2599f56dce95ab045803c54916e686105d204fb77eeb7c640a36`
- Three-run determinism: passed
- Physical-image present/removed/replaced isolation: passed
- Main checksum: valid
- Bank 2 all-box checksum: valid
- Bank 3 all-box checksum: valid
- All 12 per-box checksums: valid
- All 12 permanent box structures: valid
- Current working-box structure: valid
- Write overlaps: none
- Save Genie corrected reparse: passed
- Strengthened supported semantic comparison: passed

Corrected subsystem evidence:

- selected box is 12 with raw byte `0x8B`;
- permanent Box 12 remains empty;
- Bank 1 current Box 12 working state contains 20 Pokemon;
- all 215 permanent Pokemon plus 20 working-copy Pokemon and the Daycare Pokemon pass stored-record viability checks;
- Box 1 Dugtrio stores current HP 201 and derives maximum HP 201;
- Blastoise OT displays `Lt.Ash` and losslessly encodes as `Lt<DOT>Ash` with byte `0xF2`;
- Hall of Fame contains 9 records, 54 populated slots, and 9 Charizard entries.

## Physical Comparison

The source and corrected generated saves are not byte-identical.

- equal bytes: 24287 / 32768 (74.1180%);
- differing bytes: 8481 / 32768 (25.8820%);
- first differing offset: `0x004A`;
- last differing offset: `0x7A52`.

This physical inequality is expected. It includes canonical template bytes, unused or padding bytes, safe-location canonicalization, generated storage, working-cache state, Hall of Fame, text, mirrors, and regenerated integrity values. Acceptance depends on classified semantic equivalence and game behavior, not an aggregate similarity percentage.

## Canonicalization Decisions

- The source location is outside the emulator-validated safe-location profile, so output starts at Red's house second floor with an explicit warning.
- The current working box is no longer canonicalized from permanent storage. Source divergence is preserved because it is player-visible Gen I state.
- Current HP above a derived boxed maximum is reported and preserved rather than silently clamped. One source Golbat has such an externally edited value.
- Text padding after terminators may be canonicalized without changing displayed text; valid pre-terminator bytes are preserved through lossless tokens.

## Second Manual Emulator Gate

The focused second emulator retest confirmed that the corrected save:

- boots and loads normally;
- displays `Lt.Ash` correctly;
- opens selected Box 12 with 20 Pokemon;
- withdraws Box 1 Dugtrio at `201/201` HP;
- shows all nine six-member Hall of Fame records;
- shows all nine expected Charizard entries and no `?? BIRD`;
- passed the four defect-focused visual and interaction checks without recurrence.

The user did not perform a normal in-game save during this focused retest, so no post-emulator battery save was produced. The proof remains incomplete until the corrected save:

- saves normally;
- survives full emulator shutdown and battery-save reload;
- reparses with valid checksums and no unexplained semantic loss.

## Current Conclusion

The first generated save preserved much of the playthrough but did not prove semantic sufficiency. The corrected automated phase and focused second emulator retest now support the hypothesis for the documented fixture and support boundary. Final acceptance remains pending the save-again durability cycle and returned post-emulator battery save.
