# Milestone 5 Storage Contract

Status: complete. The load-time corruption trigger has been corrected for the Red's-house storage diagnostic, the corrected diagnostic passed the base-load emulator gate, PC storage viewing passed, and controlled deposit, withdraw, box switching, game-triggered save, normal save, post-save reparse, cache behavior, and checksum validation passed.

## Authority Rule

Storage generation uses one semantic authority:

1. `decoded.pcStorage.boxes`
2. `decoded.currentBoxCache.selectedBoxNumber`
3. generator-owned synchronization of the current-box cache from the selected permanent box

The input `decoded.currentBoxCache.cache` is validation material, not write authority. If it disagrees with the selected permanent box, generation fails conservatively.

## Owned Structures

- all 12 permanent boxes
- selected box number and changed flag
- current-box cache payload
- per-box checksums
- bank 2 all-box checksum
- bank 3 all-box checksum

## Box Serialization Rules

For every box:

- `count` must match the Pokemon array length
- species list is written in slot order
- the first unused species slot receives `0xFF`
- unused records and name slots are zeroed and then Gen I text-terminated canonically
- OT names and nicknames are written for every occupied slot

For every boxed Pokemon:

- species, current HP, status, types, catch-rate byte, moves, PP, trainer ID, experience, Stat Experience, and DVs are written from semantic input
- boxed level is canonicalized from species growth rate plus experience
- party-only calculated stats are not written into boxed records

## Current-Box Cache Rule

- permanent selected box is authoritative
- current-box cache is rewritten to match it byte-for-byte
- the dummy cache is never inherited as authoritative output state

## Validation Rules

Generation rejects:

- fewer or more than 12 boxes
- more than 20 Pokemon in any box
- selected box outside `1..12`
- count/species-array mismatches
- non-encodable OT names or nicknames
- unsupported species or moves
- invalid DV or PP ranges
- input cache disagreement with the selected permanent box

## Known Canonical Differences

The current Save Genie oracle can emit implausible boxed-level values for a handful of fixture records. The generator therefore treats boxed `level` as a permitted canonical difference and uses `experience` as the authoritative stored progression field.

## Acceptance Evidence Required

Milestone 5 is not complete until:

- a corrected generated save passes the base-load emulator gate with no immediate corruption after Continue
- generated storage reparses correctly in Save Genie
- current-box cache matches the selected permanent box
- all per-box and bank-level checksums validate
- storage survives emulator withdraw, deposit, box switch, and save-again flow

## Incident Response

The first full Milestone 5-6 emulator validation corrupted immediately after Continue, before Bill's PC was opened. Binary analysis points to unsafe non-baseline location/runtime state rather than a proven storage-layout defect.

The corrected Red's-house storage diagnostic passed base load. The tester then progressed gameplay before reaching Bill's PC, so the resulting post-gameplay save is not a clean unchanged-storage baseline. Visual PC storage verification showed the expected box counts `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]` and expected occupied/empty indicators.

Save Genie reparse of the gameplay-modified save showed permanent box 3 empty while selected box 3 was active in the current-box cache with count `17` and the box-changed flag set. A cache-aware comparison found no storage mismatches.

A later controlled-interaction candidate validated the deposit path: Box 3 was flushed to permanent storage, `PEGGY` / `PIDGEY` moved from the party into the active Box 11 cache, all per-box checksums were valid, and Bank 2 and Bank 3 all-box checksums were valid.

The final post-withdrawal artifact closed the gate:

- selected/current box `12`
- current-box cache count `4`
- current-box cache contains the remaining Box 12 Pokemon
- permanent Box 11 contains deposited `PEGGY` / `PIDGEY`
- party count `6`
- party slot 6 contains withdrawn `RED` / `CHARIZARD`
- `RED` is fainted with HP `0/165`
- main, Bank 2, Bank 3, and all per-box checksums validate

This confirms that generated storage survives real emulator load, Bill's PC access, deposit, box switching, game-triggered save, withdrawal, normal save, and Save Genie post-save reparse.

## Milestone 6 Follow-Up Observation

The Milestone 6 save-again validation deposited `PEGGY` / `PIDGEY` into selected Box 11 and then saved normally without switching boxes. Save Genie reparsed the post-save file with:

- selected box `11`
- raw current-box byte `0x8A`
- current-box cache count `1`
- current-box cache containing `PEGGY` / `PIDGEY`
- permanent Box 11 still empty
- dirty flag set
- main, Bank 2, Bank 3, and all 12 per-box checksums valid

This reinforces the storage authority rule: freshly generated saves synchronize the selected permanent box and current-box cache, while emulator-modified saves may keep the active selected box in the dirty cache until the game flushes it through a box-switch flow.
