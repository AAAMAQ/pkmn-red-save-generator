# Milestone 2 Minimal-State Contract

Milestone 2 generates the smallest structurally valid and semantically controlled Pokemon Red save currently supported by this repository.

This contract is intentionally narrow. It exists to prove the generation pipeline, deterministic semantics, checksum repair, independent reparse, and template-safety rules before broader gameplay-state serialization begins.

## Authority Rules

- Target semantic authority comes from the input `.red.json` semantic sections only.
- The target `.red.json` `physicalImage` is ignored completely and must not affect generated output.
- The committed dummy `.sav` is used only as a canonical initialized template copied into an in-memory working buffer.
- The committed dummy `.sav` is never modified in place.

## Owned Milestone 2 Fields

These fields are written from semantic input:

- player name
- rival name
- trainer ID
- options byte
- letter delay byte
- contrast byte
- money
- coins
- playtime
- safe map and coordinate block

These fields are written as verified canonical minimal defaults:

- no badges
- empty Pokédex
- empty bag inventory
- empty PC item inventory
- empty party
- no daycare occupant
- empty Hall of Fame

## Safe Location Rule

Milestone 2 currently supports one verified baseline location only:

- map `38`
- `x = 3`
- `y = 6`
- `yBlock = 0`
- `xBlock = 1`
- `previousMap = 0`

Generation fails if the target semantic input requests a different location during Milestone 2.

## Template Inheritance Policy

Milestone 2 uses Policy A for the suspicious permanent box banks:

- preserve the committed dummy permanent box payload unchanged
- preserve the committed dummy bank 2 all-box checksum byte unchanged
- preserve the committed dummy bank 3 all-box checksum byte unchanged
- preserve the current box cache unchanged

This policy is limited to Milestone 2 while permanent storage remains unsupported. It is justified by:

- Save Genie structural research
- `pret/pokered` save-flow research
- the fact that the dummy already loads and reparses as an early-game save
- the decision not to silently guess storage initialization before storage serialization is implemented

## Exact Milestone 2 Range Policy

Overwritten from target semantic input:

- `0x2598-0x25A2` player name
- `0x25F3-0x25F5` money
- `0x25F6-0x2600` rival name
- `0x2601` options
- `0x2604` letter delay
- `0x2605-0x2606` trainer ID
- `0x2609` contrast
- `0x260A-0x2610` safe location and coordinates
- `0x2850-0x2851` coins
- `0x2CED-0x2CF1` playtime

Cleared to canonical minimal defaults:

- `0x0598-0x1857` Hall of Fame block
- `0x25A3-0x25B5` Pokédex owned bitfield
- `0x25B6-0x25C8` Pokédex seen bitfield
- `0x25C9-0x25F2` bag inventory
- `0x2602` badges
- `0x27E6-0x284B` PC item inventory
- `0x284E` Hall of Fame entry count
- `0x29D6` badge mirror
- `0x2CF4-0x2D2B` daycare block
- `0x2F2C-0x30BF` party block

Regenerated integrity data:

- `0x3523` main checksum

Inherited unchanged from the committed dummy under Policy A:

- `0x0000-0x0597`
- `0x1858-0x2597`
- `0x2603`
- `0x2607-0x2608`
- `0x2611-0x27E5`
- `0x284C-0x284D`
- `0x284F`
- `0x2852-0x29D5`
- `0x29D7-0x2CEC`
- `0x2CF2-0x2CF3`
- `0x30C0-0x3522`
- `0x3524-0x7FFF`

## Acceptance Evidence Required

Milestone 2 is not complete until all of the following are true:

- output size is correct
- generated main checksum validates
- Save Genie reparses the generated save successfully
- owned semantic fields compare successfully
- generation is deterministic
- different target `physicalImage` values with identical semantics produce identical outputs
- emulator shows `Continue`
- emulator loads the generated save safely
- the game can save again
- the emulator-saved result reparses successfully afterward

## Current Validation State

Completed evidence:

- output size is correct
- generated main checksum validates
- Save Genie reparses the generated save successfully
- owned semantic fields compare successfully
- generation is deterministic
- different target `physicalImage` values with identical semantics produce identical outputs
- emulator shows `Continue`
- emulator loads the generated save safely
- trainer `RED`, money `3000`, and the expected house-second-floor location were confirmed in emulator
- no immediate corruption warning appeared on load

Still pending:

- in-game save-again confirmation
- reparse of the emulator-saved output
- stronger validation of Policy A behavior for permanent box banks after an in-game save cycle

## Known Milestone 2 Limits

- permanent PC storage is inherited, not semantically serialized
- current-box cache is inherited, not synchronized from generator-owned storage data
- event and progression coverage is intentionally minimal
- this milestone does not claim byte-identical reconstruction
