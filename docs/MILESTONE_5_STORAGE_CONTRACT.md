# Milestone 5 Storage Contract

Status: the Milestone 5 controlled storage workflow remains complete for its validated fixtures. A later completed-playthrough proof exposed an additional current-working-box authority bug and boxed-field parser bug. Those models are corrected automatically; the completed-playthrough candidate awaits a focused emulator retest.

## Authority Rule

Storage generation preserves two related semantic representations:

1. `decoded.pcStorage.boxes`
2. `decoded.currentBoxCache.selectedBoxNumber`
3. `decoded.currentBoxCache.cache`

The 12 permanent boxes are authoritative for permanent Bank 2/3 state. The Bank 1 cache is authoritative for the currently selected player-visible working box. They may legitimately differ until the game commits the working box during a box switch. Generation validates and serializes both; it never discards a divergent working box by copying the permanent selected box over it.

## Owned Structures

- all 12 permanent boxes
- selected box number and has-changed-boxes-before flag
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
- boxed level is written from the stored semantic field and must agree with species growth rate plus experience
- party-only calculated stats are not written into boxed records

## Current-Box Cache Rule

- the selected number identifies both representations
- the current working-box cache is serialized from its own semantic model
- equality is reported when present but is not required
- divergent source state remains divergent in generated output
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
- malformed current-box cache structure, such as impossible counts or an invalid selected-box reference
- stored records that cannot derive a nonzero withdrawal maximum HP
- level/experience inconsistency
- missing cache semantics for the selected working box

Stored current HP is preserved independently. If a source has an externally edited value above the currently derived maximum, the CLI reports it instead of silently clamping it. The completed-playthrough fixture contains one such Golbat. This warning is distinct from the fixed zero-HP data-loss bug.

## Acceptance Evidence Required

Milestone 5 is not complete until:

- a corrected generated save passes the base-load emulator gate with no immediate corruption after Continue
- generated storage reparses correctly in Save Genie
- current-box cache is structurally valid and its equality or divergence from permanent storage is classified correctly
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

This observation originally led to a too-narrow generator rule. The completed-playthrough source itself contains an empty permanent Box 12 and a 20-Pokemon current working Box 12. The corrected authority rule therefore preserves divergence at generation time, not only after emulator modification. The raw high bit is documented as `hasChangedBoxesBefore`; it is not permission to discard or ignore the working copy.

## Completed-Playthrough Corrective Evidence

The first completed-playthrough candidate replaced its 20-Pokemon Bank 1 Box 12 with empty permanent Box 12. Manual testing exposed the loss immediately on entering Bill's PC. The corrected candidate preserves:

- raw selected-box byte `0x8B`;
- selected box 12;
- permanent Box 12 count 0;
- current working Box 12 count 20;
- all 12 valid permanent structures and checksums;
- valid, independently checked current working-box structure.

The same investigation corrected boxed-record decoding: current HP is at `+0x01..+0x02`, stored level at `+0x03`, status at `+0x04`, types at `+0x05..+0x06`, and catch rate at `+0x07`. The original Box 1 Dugtrio stores HP 201; the failed candidate wrote zero; the corrected candidate writes 201.
