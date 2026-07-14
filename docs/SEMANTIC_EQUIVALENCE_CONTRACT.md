# Semantic Equivalence Contract

Semantic equivalence is the target success condition for this project. It is not byte identity.

## Core Definition

For covered fields, the reparsed generated save must describe the same gameplay-relevant state as the target semantic input.

```text
decode(generate(decoded(S))) ~= decoded(S)
```

Where `~=` means equivalence under the categories below.

## Categories

### Required exact semantic match

These must match exactly after reparsing when supported.

Currently owned through implemented and emulator-validated Milestone 6 for the Red's-house safe-location profile:

- trainer name
- rival name
- trainer ID
- options byte
- letter delay byte
- contrast
- money
- coins
- badges
- Red's-house baseline map/location coordinates
- bag inventory
- PC item inventory
- Pokedex seen/owned state
- visited towns
- hidden items
- hidden coins
- party count
- party species list and terminator semantics
- party Pokemon slot ordering
- party Pokemon species
- party current HP
- party status bytes
- party moves
- party PP and PP Ups
- party OT names
- party nicknames
- party trainer IDs
- party experience
- party DVs
- party Stat Experience
- party stored level
- party stored live stats
- all 12 permanent PC boxes
- selected box number and has-changed-boxes-before flag
- every current working-box Pokemon and its independent structure
- classified equality or divergence between the current working box and selected permanent box
- boxed current HP, stored level, status, types, catch rate, moves, trainer ID, experience, Stat Experience, DVs, PP, OT, and nickname
- nonzero independently derived withdrawal maximum HP
- daycare occupancy and stored Pokemon
- daycare trailing deposited-level byte
- Hall of Fame entry count, slot ordering, internal species IDs, levels, nicknames, and empty-slot structure
- named event flags
- trainer-battle flags
- static-battle flags
- story-progress flags
- persistent script bytes exposed by Save Genie
- missable objects
- named story-evidence/world bits exposed by Save Genie

Still deferred or temporarily disabled:

- broader safe-location coverage beyond the Red's-house baseline
- Viridian City Pokemon Center until full map-runtime serialization is implemented and emulator-proven
- unsupported runtime-heavy world bytes outside the documented subset
- fields not yet surfaced by a stable named semantic rule

Current working-box divergence is valid in source, freshly generated, and emulator-modified saves. Both representations must be structurally valid. The current working copy is player-visible authority until the game commits it during a box switch; permanent storage remains authority for the other 11 boxes.

### Required normalized match

The decoded meaning must match even if the binary form is canonicalized:

- name padding and terminator placement
- item-list terminator style where multiple legal encodings decode identically
- normalized count fields when arrays are canonicalized

### Required derived match

These may be recomputed as long as the decoded meaning remains correct:

- duplicated badge mirror bytes
- Pokedex counts derived from bitfields
- party-only calculated stats derived with the exact Gen I formula where policy requires derivation
- synchronized badge and other verified mirror bytes
- per-box checksums
- bank 2 and bank 3 all-box checksums
- regenerated checksum bytes

### Canonicalized difference permitted

These may differ from the target source binary and still pass:

- checksum bytes
- runtime scratch bytes
- unknown tails
- padding values
- canonical default bytes
- canonical text padding after the first terminator
- dummy-template bytes in explicitly documented inherited ranges
- Red's-house location projection for diagnostic storage saves derived from non-baseline private fixtures
- expected gameplay drift after emulator validation, such as travel to a Pokemon Center, playtime increase, party/storage changes from deposit or capture, dirty current-box cache state, and runtime map-state changes caused by normal gameplay

### Unsupported Or Deferred

If a field is not yet safely serializable:

- it must be reported as unsupported
- comparison output must distinguish unsupported from mismatched
- the generator must not silently claim equivalence for it
- the generator should reject unsafe non-empty deferred states rather than clearing them silently

## Comparison Method

The preferred comparison flow is:

1. generate output save
2. reparse output with Save Genie
3. compare semantic fields by contract category
4. report exact matches, normalized matches, derived matches, canonicalized differences, unsupported fields, and true mismatches

## Acceptance Standard

A generator milestone is not complete merely because parser-level semantic comparison passes.

Release-grade acceptance requires:

- structural validity
- checksum validity
- Save Genie reparse success
- semantic equivalence for covered fields
- emulator base-load success with no immediate corruption after Continue
- emulator load success
- in-game save-again success
- second reparse preserving semantic equivalence

For Milestone 5 and Milestone 6, Save Genie reparse plus semantic comparison is explicitly insufficient without emulator base-load, feature interaction, save-again, and post-save reparse evidence.

Earlier Milestone 6 fixtures satisfied this standard for the Red's-house safe-location profile. The completed-playthrough proof later exposed parser/comparator blind spots in text, current working storage, boxed HP, and Hall of Fame contents. Corrected automation passes, but proof-level acceptance awaits the focused second emulator retest. Broader safe-location support remains outside the equivalence surface.

## Final Release Equivalence Categories

The combined Final Release Milestone uses these comparison categories:

- exact match
- normalized match
- derived match
- synchronized mirror match
- permitted canonical difference
- expected gameplay drift
- unsupported or deferred
- unexpected mismatch

Unsupported non-empty source state must be surfaced clearly. A comparison must not pass merely because unsupported fields were ignored.

Public CI proves the generator's own deterministic and checksum guarantees with synthetic samples. Private Save Genie oracle validation and emulator save-again validation remain required for a completed-playthrough semantic-sufficiency claim.
