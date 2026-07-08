# Milestone 5-6 Load Corruption Incident

Date: 2026-07-07

Status: root cause identified and corrected for Milestone 5; Milestone 5 storage validation is complete, and Milestone 6 remains incomplete.

## Incident Summary

Milestone 5-6 generated a 32768-byte save that passed automated validation, checksum validation, Save Genie reparse, semantic comparison, physical-image independence checks, and deterministic-output checks.

The emulator title screen rendered normally and offered `Continue`. Severe corruption appeared immediately after selecting `Continue`, before any movement, menu access, Bill's PC interaction, deposit, withdraw, box switching, game-triggered save, or normal in-game save.

Confirmed failing artifacts are preserved under an ignored local incident-evidence directory.

Key preserved evidence:

- failing save: private ignored incident evidence
- failing save SHA-256: `77ba8ffd186e69e67a3119f58dba41f60a1a205e734e6bf49c014916d0cce7a5`
- failing save size: `32768`
- generation report: private ignored incident evidence
- private source fixture copy: private ignored incident evidence
- generated Save Genie reparse copy: private ignored incident evidence
- exact Save Genie diagnostic `.red.json` copy: private ignored incident evidence
- exact Save Genie summary copy: private ignored incident evidence
- exact Save Genie party summary copy: private ignored incident evidence
- exact Save Genie box summary copy: private ignored incident evidence
- last working Milestone 4 save: private ignored incident evidence
- corruption screenshot copy: private ignored incident evidence
- normal title/Continue screenshot copy: private ignored incident evidence

Emulator name, emulator version, ROM hash, and exact emulator configuration were not recorded in the repository at the time of this incident. The supported generator profile remains `pokemon-red-usa-europe-v1`.

No post-failure in-game save was performed.

## Visible Symptoms

Confirmed observations from emulator screenshots:

- title screen and Continue menu appeared normal before loading
- malformed text immediately after Continue
- nonsensical numbers
- corrupted tiles
- graphics rendered over text regions
- broken screen layout
- unusable game screen
- inability to continue normal gameplay

These observations identify a load-time corruption defect. They do not, by themselves, prove a specific byte-level root cause.

## Validation-Model Failure

The automated model missed the defect because:

- Save Genie decoded structures that the generator also tried to write, so shared assumptions could pass both sides.
- The comparator validated semantic self-consistency, not runtime game execution.
- Valid checksums prove byte sums only, not coherent map/runtime state.
- A save can be parseable and still unsafe for the game's load routines.
- The existing report ledger allowed broad and overlapping declared ranges, which made generation-order risk harder to see.
- Emulator load is a mandatory safety gate and cannot be replaced by parser acceptance.

## Evidence Preservation

The ignored evidence directory contains:

- exact failing generated save
- exact generation report
- copied private source fixture
- physical-image-removed and physical-image-replaced local projections
- generated Save Genie reparse
- Save Genie-generated diagnostic `.red.json`, save summary, Pokemon summary, and box summary for the exact failing save
- copied screenshots
- last working Milestone 4 generated save
- binary comparison summaries
- Save Genie diagnostic comparison notes
- diagnostic generated saves and reports
- Save Genie oracle working directories for diagnostic saves

Private fixtures, screenshots, generated saves, ROM files, PDFs, and emulator outputs remain untracked.

## Additional Save Genie Diagnostic Evidence

After the incident, a Save Genie-side `full-fixture.generated.sav` was found with SHA-256 `f4304c9da0843536788e894a738d3ac644668d38a4b41bb0fa07e42ae38cc23f`, which did not match the preserved failing save. To avoid mixing artifacts, the preserved exact failing save was reparsed through Save Genie inside the ignored incident directory. The exact reparse produced four diagnostic outputs preserved under `savegenie-exact-failing/`:

- `Pokemon - Red Version (USA, Europe) (SGB Enhanced).red.json`
- `SaveGenieSummary.txt`
- `PokemonSummary.json`
- `PokemonBoxes.json`

The exact Save Genie `.red.json` embeds source SHA-256 `77BA8FFD186E69E67A3119F58DBA41F60A1A205E734E6BF49C014916D0CCE7A5`, matching the preserved failing save.

Those outputs are evidence, not emulator-safety authority. They show that the failing save remained semantically readable to Save Genie while still crashing the actual game load path.

Save Genie decoded:

- valid main checksum
- valid Bank 2 all-box checksum
- valid Bank 3 all-box checksum
- trainer `GOON`
- rival `KILLUA`
- six-party Pokemon
- all 12 PC boxes, with counts `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`
- location `map=41`, Viridian Pokemon Center, `x=3`, `y=6`
- block coordinates `xBlock=1`, `yBlock=0`
- movement mode `Walking`
- event/story/script counts matching the private source fixture surface

The same diagnostics strengthen the map/runtime root-cause finding. Compared with the original working source save, the failing save decoded the Viridian location but retained Red's-house-compatible values for runtime map structure:

- source `0x2613` tileset `0x06`, failing `0x04`
- source `0x2615` map width `0x07`, failing `0x04`
- source `0x2616` map data pointer `0x40DF`, failing `0x4010`
- source `0x2618` map text pointer `0x4263`, failing `0x40CF`
- source `0x261A` map script pointer `0x425D`, failing `0x40B0`
- source `0x265A` warp count `0x02`, failing `0x01`
- source `0x278D` sprite count `0x04`, failing `0x00`
- source `0x27D1` view width `0x0E`, failing `0x08`
- source `0x27D2` map view pointer `0x9BC0`, failing `0x9800`
- source `0x29E8` warped-from map Viridian City, failing Pallet Town
- source `0x2CDC` trainer-header pointer `0x7D58`, failing `0x0000`

This comparison explains why parser acceptance, checksum validity, and semantic comparison all passed while the emulator corrupted immediately after Continue.

## Hypotheses Investigated

Storage-layout hypotheses remain possible but are not the leading finding after the added Save Genie diagnostics:

- boxed Pokemon record stride could be wrong
- box species/count/terminator handling could be wrong
- current-box cache could differ from the permanent selected box in a game-sensitive way
- box padding could be invalid
- bank checksums could be insufficient evidence
- Save Genie and the generator could still share a wrong storage assumption, so storage remains experimental until PC interaction is emulator-proven

Extended-state and location hypotheses became the leading finding:

- map ID and location bytes were changed to Viridian Pokemon Center
- the generator only wrote a small location/runtime subset
- map-runtime bytes such as map pointers, map dimensions, warp counts, object data, and trainer-header pointers remained inherited from the Red's-house template or were otherwise inconsistent with Viridian Pokemon Center
- Save Genie could still parse the save because these runtime bytes are readable, but the game needed coherent values at load time

Shared-system hypotheses still under watch:

- serializer and parser can share mistaken layout assumptions
- duplicate fields may require synchronization not yet modeled
- checksums can hide invalid overlapping writes by validating the final byte sum

## Binary Comparison Findings

Local private binary reports are preserved in the ignored incident-evidence directory.

Important findings:

- last working Milestone 4 save vs failing Milestone 5-6 save: `15329` changed bytes across `191` changed ranges
- original private source save vs failing generated save: `13819` changed bytes across `721` changed ranges
- storage-only Red's-house diagnostic vs failing save: `1447` changed bytes, concentrated in bank 0 runtime/core/event areas rather than storage
- storage-only Red's-house diagnostic vs Milestone 4 save: storage and checksum changes dominate, with Red's-house runtime retained

Map/runtime bytes that differed between the known-good private source save and the failing generated save include:

- `0x2612` source `0x14`, failing `0x00`
- `0x2613` source `0x06`, failing `0x04`
- `0x2615` source `0x07`, failing `0x04`
- `0x2616-0x261B` source Viridian map pointers, failing Red's-house pointer bytes
- `0x261E-0x2627` source non-zero map data, failing zero bytes
- `0x265A` source `0x02`, failing `0x01`
- `0x27D1-0x27D3` source Viridian view dimensions/pointer, failing Red's-house-compatible bytes
- `0x29C6` source `0x01`, failing `0x00`
- `0x2CDC-0x2CDD` source `0x58 0x7D`, failing `0x00 0x00`

These differences strongly indicate an incoherent loaded-map runtime cluster.

## Overlapping-Write Findings

The preserved failing report contained seven overlapping declared ranges, including:

- Hall of Fame clear overlapped Hall of Fame write
- Daycare clear overlapped Daycare write
- broad `runtimeState subset` range overlapped visited towns, badge mirror, event flags, playtime, and scripts

The code now tightens report ranges and fails generation if undeclared non-template ranges overlap.

## Root Cause

Root cause: unsafe non-baseline location admission without a complete map-runtime serialization contract.

The generator admitted a Viridian Pokemon Center semantic location while only writing the small `map/x/y/block/previousMap` location subset and a few runtime flags. The output therefore combined Viridian location bytes with Red's-house template map-runtime pointers, dimensions, object/warp metadata, warped-from state, and adjacent runtime values. Save Genie could parse the save and report coherent high-level semantics, but the game corrupted immediately after Continue when it loaded the incoherent map/runtime state.

Affected source:

- `src/generation/MinimalStateContract.hpp`
- `src/generation/CoreStateSerializer.hpp`
- `src/generation/ExtendedWorldSerializer.hpp`
- `src/generation/MinimalSaveGenerator.hpp`

Correction:

- fail closed to the emulator-validated Red's-house baseline location only
- remove the Viridian Pokemon Center location profile until full map-runtime state is serialized and emulator-proven
- remove duplicate Daycare/Hall of Fame clearing from the core serializer
- split broad runtime ledger ranges into exact written ranges
- add report-side byte provenance and overlap rejection
- add storage/cache boundary tests
- add a regression test that rejects the Viridian Pokemon Center state

Why automated tests passed:

- structural tests checked checksums, output size, parser acceptance, and semantic equality
- comparator did not validate map-runtime pointer coherence
- Save Genie reparse exposed the same decoded surface but did not execute the game's map load path

## Diagnostic Saves

New ignored diagnostic saves generated after the correction:

- `diagnostic-storage-only-red-house.generated.sav`: Red's-house projection with full private PC storage payload and dummy extended state
- `diagnostic-empty-storage-red-house.generated.sav`: Red's-house projection with deterministic empty storage
- `diagnostic-events-only-red-house-empty-storage.generated.sav`: Red's-house projection with full event/script subset and empty storage
- `diagnostic-full-semantics-red-house.generated.sav`: Red's-house projection with full storage and current extended-state payload

Direct generation from the original full private fixture now fails conservatively because it requests Viridian Pokemon Center, which is no longer an enabled safe-location profile.

Save Genie reparsed the newly generated diagnostic saves successfully. Emulator safety remains unproven until manual load testing.

## Updated Gates

Milestone 5 and Milestone 6 must not be considered complete based only on:

- successful compilation
- passing unit tests
- successful generation
- output size
- checksum validity
- Save Genie reparse
- semantic comparison
- physical-image independence
- determinism

Base load gate now required before feature interaction:

1. title screen renders normally
2. Continue appears
3. selecting Continue loads without corruption
4. player appears on the expected map
5. screen tiles and text are normal
6. movement works
7. menu opens
8. trainer page opens
9. party page opens
10. save can be performed normally

Storage gate after base load:

- open Bill's PC
- open Pokemon storage
- inspect current box
- inspect occupied boxes
- deposit
- withdraw
- switch boxes
- accept game-triggered box save
- return to gameplay
- save normally
- close emulator
- reparse post-save output
- compare party and all boxes
- validate current-box cache and bank checksums

Milestone 6 remains paused until Milestone 5 storage passes this gate.

## Corrected Diagnostic Base-Load Result

The corrected Red's-house storage diagnostic passed the emulator base-load gate.

Confirmed emulator observations:

- title screen and Continue appeared normally
- selecting Continue loaded Red's house second floor without graphical or text corruption
- movement worked
- menu access worked
- trainer information looked correct
- money, coins, badges, and the six-member party looked correct

The original corrected diagnostic was generated in the ignored incident-evidence directory.

That file is no longer treated as a pristine baseline because emulator/gameplay workflows have since changed local copies. The generation report recorded the pristine generated SHA-256 as:

- `6daae0f2a65a148ceee4cabe52a6517089cc192361cb730424ba310f077f4049`

The stable preserved post-gameplay evidence is:

- save: private ignored incident evidence
- save SHA-256: `135418036bc59f1507cabe9a1134ccb34743d82a2d8ab18132082c330de190ce`
- screenshot: private ignored incident evidence
- screenshot SHA-256: `d46ad07206722e289e16b2bc4b44daad9581d3145c8cd683d4edf72793930450`

Because the test save did not include enough supporting event progression to access Bill's PC directly, the tester continued normal gameplay before inspecting PC storage. The resulting save must therefore be treated as gameplay-modified evidence, not a byte-identical or fully unchanged semantic comparison against the originally generated output.

## Provisional PC Storage Viewing Result

PC storage visual verification passed provisionally.

The tester manually counted the 12 PC boxes in game as:

- `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`

The observed occupied/empty indicators matched the expected generated layout, and no major graphical, text, or structural corruption was visible in the box-selection interface.

Save Genie reparse of the post-gameplay save reported:

- valid main checksum
- valid Bank 2 all-box checksum
- valid Bank 3 all-box checksum
- trainer `GOON`
- rival `KILLUA`
- selected box `3`
- current box byte `0x82`
- current-box changed flag set
- current-box cache count `17`
- permanent PC box counts `[20, 18, 0, 19, 20, 17, 0, 0, 0, 0, 0, 5]`

A cache-aware comparison found no storage mismatches when the selected box 3 contents were compared through the active current-box cache and all other boxes were compared through permanent storage. This is consistent with normal Gen I current-box-cache behavior after gameplay, but it is not yet the controlled Milestone 5 storage interaction gate.

Milestone 5 remains incomplete until deposit, withdraw, box-switch, game-triggered save, normal save, post-save Save Genie reparse, cache synchronization, and all box/bank checksum comparisons pass.

## Controlled Storage Interaction Follow-Up

The tester reported that controlled Milestone 5 storage interaction succeeded in the emulator:

- save loaded successfully
- Bill's PC opened successfully
- level 3 Pidgey was deposited successfully
- active box count increased correctly
- switching boxes worked correctly
- game-triggered save during box switching completed successfully
- withdrawing a Pokemon worked correctly
- party and box counts updated correctly
- normal in-game save completed successfully
- emulator was fully closed afterward
- no graphical corruption, text corruption, missing Pokemon, invalid names, or obvious storage errors appeared

Post-save artifact analysis found two user-supplied local candidates:

- `controlled-storage-interaction-start copy.sav`: preserved privately, SHA-256 `c1005fd8ad32468e748a1e7a2fda8c1fc36dad708d39e70fb9baaf7852c62ce3`, rejected because Save Genie parsed it as an invalid erased/mostly-`0xFF` SRAM image with invalid main, Bank 2, and Bank 3 checksums
- `controlled-storage-interaction-start.sav`: preserved privately, SHA-256 `b0b86a6581c1ce1199d1dc4860c452ea6efa6959e60e1ba910bcbcb69950db03`, parsed successfully as the likely post-interaction candidate

The valid candidate showed:

- main checksum valid
- Bank 2 all-box checksum valid
- Bank 3 all-box checksum valid
- all 12 per-box checksums valid
- party count `5`
- `PEGGY` / `PIDGEY` removed from party and present in the active current-box cache
- permanent box counts `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`
- selected box `11`, raw current-box byte `0x8A`, changed flag set
- current-box cache count `1`
- previously active Box 3 contents flushed into permanent Box 3

This validates the deposit path, dirty selected-box cache behavior, and permanent Box 3 write-back. It does not yet prove the full controlled gate because the preserved valid candidate does not represent the requested final post-withdraw/post-Box-12 state:

- selected box is `11`, not `12`
- party count remains `5`
- Box 12 count remains `5`
- no withdrawn Box 12 Pokemon appears in the party

This intermediate candidate was superseded by the final post-withdrawal artifact below.

## Final Controlled Storage Interaction Result

The final post-withdrawal save was preserved as private ignored incident evidence:

- size `32768`
- SHA-256 `3da8ec51484f2aa707c32dde36dc70712cb29163ef992bdb672a79c2bcfa89fe`

Save Genie reparse and independent checksum verification confirmed:

- main checksum valid: stored `0x02`, calculated `0x02`
- Bank 2 all-box checksum valid: stored `0x92`, calculated `0x92`
- Bank 3 all-box checksum valid: stored `0x2B`, calculated `0x2B`
- all 12 per-box checksums valid
- selected/current box `12`
- raw current-box byte `0x8B`
- current-box changed flag set
- current-box cache count `4`
- current-box cache contains the remaining Box 12 Pokemon: `SLAYER`, `PARASECT`, `PIDGEOT`, and `RHYHORN`
- permanent Box 11 contains deposited `PEGGY` / `PIDGEY`
- permanent Box 12 count is `0`, consistent with Box 12 being active in the current-box cache
- party count `6`
- party slot 6 contains withdrawn `RED` / `CHARIZARD`
- `RED` is fainted with HP `0/165`
- no Pokemon loss, duplication, name corruption, or checksum corruption was found in the controlled workflow evidence

Milestone 5 storage validation is complete. The original load-time corruption root cause remains resolved by the Red's-house fail-closed location policy, and PC storage generation now has emulator interaction evidence covering load, Bill's PC access, deposit, box switching, game-triggered save, withdrawal, normal save, post-save reparse, cache behavior, and storage checksum validation.

Milestone 6 remains incomplete and must not be claimed complete until Daycare, Hall of Fame, extended event/world state, safe-location handling, and emulator save-again validation pass.
