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

These must match exactly after reparsing when supported:

- trainer name
- rival name
- trainer ID
- money
- coins
- badges
- party count
- party species
- party moves
- party experience, DVs, Stat Experience, PP, OT names, nicknames
- bag inventory
- PC item inventory
- Pokedex seen/owned state
- permanent PC box contents
- current selected box meaning
- current-box cache decoded contents
- daycare occupancy and stored Pokemon
- Hall of Fame records
- supported persistent event, trainer-battle, story, and world-state values

### Required normalized match

The decoded meaning must match even if the binary form is canonicalized:

- name padding and terminator placement
- item-list terminator style where multiple legal encodings decode identically
- normalized count fields when arrays are canonicalized

### Required derived match

These may be recomputed as long as the decoded meaning remains correct:

- duplicated badge mirror bytes
- Pokedex counts derived from bitfields
- current-box cache synchronized from storage semantics
- regenerated checksum bytes

### Canonicalized difference permitted

These may differ from the target source binary and still pass:

- checksum bytes
- runtime scratch bytes
- unknown tails
- padding values
- canonical default bytes
- stale raw cache differences eliminated by synchronization
- dummy-template bytes in explicitly documented inherited ranges

### Unsupported Or Deferred

If a field is not yet safely serializable:

- it must be reported as unsupported
- comparison output must distinguish unsupported from mismatched
- the generator must not silently claim equivalence for it

## Comparison Method

The preferred comparison flow is:

1. generate output save
2. reparse output with Save Genie
3. compare semantic fields by contract category
4. report exact matches, normalized matches, derived matches, canonicalized differences, unsupported fields, and true mismatches

## Acceptance Standard

A generator milestone is not complete merely because the save loads.

Release-grade acceptance requires:

- structural validity
- checksum validity
- Save Genie reparse success
- semantic equivalence for covered fields
- emulator load success
- in-game save-again success
- second reparse preserving semantic equivalence
