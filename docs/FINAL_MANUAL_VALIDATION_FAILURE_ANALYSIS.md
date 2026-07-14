# Final Manual Validation Failure Analysis

Status: corrected automated phase and focused second emulator retest passed; save-again durability and post-emulator analysis remain required.

## Executive Summary

The first completed-playthrough proof produced a save that booted, loaded, and supported broad gameplay, but manual emulator testing found four gameplay-semantic defects:

1. a valid Gen I dot glyph in an original-trainer name became `?`;
2. selected Box 12 appeared empty even though its Bank 1 working copy contained 20 Pokemon;
3. a Pokemon withdrawn from Box 1 had `0` current HP instead of its stored full HP;
4. each Hall of Fame record lost Charizard and could display an invalid `?? BIRD` entry.

The failures were real. Valid checksums, deterministic output, physical-image isolation, Save Genie reparse, and the old semantic comparator did not establish operational equivalence. The parser and generator shared incomplete assumptions, allowing the first automated proof to pass incorrectly.

The underlying text, box-record, working-box, Hall of Fame, validator, comparator, and reporting models have been corrected. A new private candidate passes the corrected automated gates. The focused second emulator test also confirmed the four corrected behaviors. Because that test did not include a normal save, full shutdown, battery-save reload, or returned post-emulator save, final acceptance remains blocked only on the save-again durability and post-save analysis gate.

## Evidence Method

The complete first-pass evidence was preserved before source edits. The private ignored evidence workspace retains:

- source and first generated saves before emulator interaction;
- original and generated saves after equivalent manual actions;
- source and reparsed `.red.json` documents;
- Save Genie summaries;
- generation and comparison reports;
- all manual screenshots and PDF contact sheets;
- Git state and patches from the start of the investigation;
- file sizes and SHA-256 identities.

No ROM, save, screenshot, or private semantic report is tracked by Git. Public documentation uses logical evidence names and cryptographic identities only.

| Logical artifact | Size | SHA-256 |
|---|---:|---|
| Source before emulator | 32768 | `a09b29f6c4c928c0e38d44d95c07076399a552aa5e3ef1d77680a1dd5d3ce9cc` |
| First generated candidate | 32768 | `2f57392a4f08983f397c5a257d7b0341557aff76a588a1cc26e0ce6938fa2c62` |
| Source after equivalent actions | 32768 | `15736170815f46814a71c2f3a602bc40b9c0a1aa820528881396185d6db1b157` |
| First generated post-emulator save | 32768 | `58ee7fb9aaf3c63b1bdd8a71af674e42d1d27e0f2d08380c9f411aa843b4bbf9` |
| Corrected generated candidate | 32768 | `e8d7a72121ae2599f56dce95ab045803c54916e686105d204fb77eeb7c640a36` |

The emulator used for the first manual pass was mGBA 0.10.5. Screenshot observations were checked directly against the associated binaries and corrected Save Genie reparses; OCR was not used as acceptance authority.

## Functionality That Passed

The first candidate successfully demonstrated:

- normal boot, Continue, load, movement, menus, and graphics;
- safe Red's-house start and broad map traversal;
- doors, transitions, Fly, wild battles, Pokemon Centers, and healing;
- trainer/core state, five-member party count, Pokedex, Bag, and PC item inventory;
- Bill's PC access, box switching, and withdrawal mechanics;
- Daycare occupancy with Dragonite;
- a normal in-game save without immediate whole-save corruption.

These results remain valid evidence, but they do not override the subsystem defects below.

## Defect A: Lossy Gen I Text

### Symptom

Blastoise's original trainer displayed as `Lt?Ash` in the first generated save instead of `Lt.Ash`.

### Raw Evidence

The source field contained:

```text
8B B3 F2 80 B2 A7 50 ...
 L  t  .  A  s  h END
```

The first parser decoded byte `0xF2` through its unknown-character fallback, producing `?`. The first generator then encoded that fallback as byte `0xE6`, permanently replacing a valid source glyph.

### Root Cause

The Save Genie and generator text codecs covered only a partial character table. The decoder conflated unknown bytes with the visible question-mark glyph, and the encoder previously allowed lossy fallback behavior. Unicode display text alone could not distinguish the two visually similar period glyphs `0xE8` and `0xF2`.

### Correction

- Both codecs now expose a display value and a lossless value.
- `0xF2` round-trips as `<DOT>` while displaying as `.`.
- `0xE8` round-trips separately as `<PERIOD>`.
- supported punctuation, contraction glyphs, gender symbols, and special tokens have explicit mappings;
- unknown bytes decode as `<0xHH>` and can round-trip explicitly;
- unsupported Unicode throws instead of silently becoming `?` or space;
- a terminator token inside a value is rejected.

The corrected field reparses as display `Lt.Ash`, lossless `Lt<DOT>Ash`, with raw byte `0xF2` restored.

### Regression Coverage

Tests now cover every supported text token, the `0xF2` versus `0xE8` distinction, gender glyphs, explicit raw-byte tokens, unsupported Unicode rejection, embedded terminator rejection, and decode/encode round trips.

## Defect B: Selected Box 12 Appeared Empty

### Symptom

The source opened Bill's PC with 20 Pokemon in selected Box 12. The first generated save opened the same selected box as empty.

### Raw Evidence

- selected-box byte: `0x8B`;
- selected box: 12 (`0x0B + 1`);
- high bit: box history flag set;
- permanent Box 12 count: 0;
- Bank 1 current working-box count: 20;
- source cache/permanent comparison: different.

The 12 permanent counts are:

```text
20, 20, 20, 20, 19, 16, 20, 20, 20, 20, 20, 0
```

The source therefore contains 215 permanent records plus 20 player-visible records in the current Bank 1 working box.

### Root Cause

The first generator treated permanent storage as the only semantic authority and rewrote the Bank 1 cache from permanent Box 12. That canonicalization erased valid player-visible state. The high bit was also described too loosely as a dirty-cache permission flag; game routines use it as box-history metadata, while the cache remains an independent working copy that is committed during box switching.

### Correct Model

```text
selected-box byte
        |
        +--> selected number (low seven bits, zero-based on disk)
        +--> has-changed-boxes-before flag (high bit)

permanent selected box (Bank 2 or 3)     Bank 1 current working box
                 |                                  |
                 +----- may legitimately differ ---+
                                                    |
                                      player-visible authority until
                                      the game commits it on box change
```

### Correction

- Permanent boxes and the current working box are separate semantic values.
- The generator serializes all 12 permanent boxes from `decoded.pcStorage`.
- It serializes the Bank 1 working box from `decoded.currentBoxCache.cache`.
- Divergence is validated and reported, not repaired silently.
- Both representations must be internally coherent.
- The CLI reports selected permanent count, working-cache count, and whether they match.
- `validate-save` no longer rejects valid divergence or requires an opt-in flag.

The corrected candidate preserves permanent Box 12 count 0 and current working-box count 20. The selected byte remains `0x8B`.

### Regression Coverage

Tests generate selected Box 12 with an empty permanent copy and a non-empty working copy, then verify that both survive independently and that all checksums and structures remain valid.

## Defect C: Zero-HP Withdrawal

### Symptom

After withdrawing Dugtrio from Box 1, the source produced a healthy `201/201` party Pokemon while the first generated save produced `0/201` and fainted status.

### Raw Evidence

The source Box 1 Dugtrio record begins:

```text
76 00 C9 64 00 04 04 AE ...
|  |----| |  |  |  |
|  HP=201|  status/types/catch rate
species  level=100
```

The original post-emulator party record reparses as current/max HP `201/201`. The failed post-emulator party record reparses as `0/201`. The game correctly calculated the maximum and preserved the zero current HP it received from the generated box record.

### Root Cause

The first Save Genie boxed-record decoder:

- read level from offset `+0x21`, one byte past the 0x21-byte record;
- forced current HP and status to zero;
- omitted stored types and catch-rate byte.

The generator consumed that incomplete semantic JSON and faithfully serialized zero HP. Checksums and reparsing passed because the writer and parser shared the same mistaken model.

### Correction

The boxed record is now decoded and generated as:

| Relative offset | Field |
|---:|---|
| `+0x00` | internal species ID |
| `+0x01..+0x02` | current HP, big-endian |
| `+0x03` | stored box level |
| `+0x04` | status |
| `+0x05..+0x06` | stored types |
| `+0x07` | catch-rate byte |
| `+0x08..+0x20` | moves, identity, experience, stat experience, DVs, and PP |

The independent stat routine now uses the game's `ceil(sqrt(stat experience)) / 4` rule. The prior floor-square-root implementation was one point low for values such as Slowbro's HP stat experience of 50.

Every stored Pokemon must have a valid species, level, growth-rate/experience relation, DVs, moves, PP, names, record bounds, and a nonzero derived maximum HP. Source current HP is preserved. One source Golbat stores HP 361 while independently deriving maximum HP 272; this externally edited source irregularity is reported explicitly rather than silently clamped.

### Regression Coverage

Tests now assert boxed HP and level byte positions, derive Slowbro's expected 322 maximum HP using the exact Gen I rule, reject malformed records, and validate all stored Pokemon in the private fixture without committing it.

## Defect D: Hall of Fame Charizard and `?? BIRD`

### Symptom

The first generated Hall of Fame lost Pokemon, omitted every expected Charizard, and displayed invalid `?? BIRD` entries.

### Raw Evidence

- Hall of Fame base: `0x0598`;
- active record stride: `0x60` bytes;
- six slots per record;
- slot stride: `0x10` bytes;
- active records: 9;
- expected populated slots: 54;
- Charizard internal species ID: `0xB4` (decimal 180, National Dex 6);
- expected Charizard occurrences: 9.

The old parser accepted only numeric species IDs 1 through 151, incorrectly treating internal species IDs as National Dex numbers. It rejected `0xB4`, removed that slot from the semantic vector, and compressed later members. The serializer then wrote the shortened vector sequentially and zero-filled the remaining slot. This explains both missing Charizard and the invalid trailing Hall of Fame display.

### Correction

- One shared internal-ID-to-National-Dex mapping validates species.
- Active leading records are selected from the game record-count byte.
- Slot positions are preserved instead of compressed.
- Invalid internal IDs and levels now fail parsing/validation.
- Serializer validation requires sequential entry and party positions, at most six members, valid internal species IDs, valid levels, and encodable names.
- The semantic comparator checks every entry, slot, species, level, nickname, and slot order rather than only record count.

The corrected source and generated reparses each contain 9 records, 54 Pokemon, and 9 Charizard entries. No record is shortened.

### Regression Coverage

Tests cover internal species ID `0xB4`, a six-member record with Charizard in slot 6, slot preservation, entry boundaries, invalid internal IDs, and full structural comparison.

## Why the First Automated Proof Passed

The first proof established physical integrity but not operational semantics:

- checksums proved byte sums, not field meaning;
- Save Genie and the generator shared the same boxed-record and Hall of Fame assumptions;
- the comparator compared values produced by that same parser;
- Hall of Fame entry count was checked without full slot contents;
- the current working box was classified as disposable cache state;
- text comparison used a lossy display value without byte-aware identity;
- boxed-Pokemon withdrawal behavior was not represented by an invariant.

The corrected validation stack adds independent binary structure checks and game-behavior invariants. A parser reparse is still required, but is no longer sufficient by itself.

## Corrected Automated Result

The corrected candidate is 32768 bytes with SHA-256:

```text
e8d7a72121ae2599f56dce95ab045803c54916e686105d204fb77eeb7c640a36
```

Automated evidence:

- three independent generation runs are byte-identical;
- present, removed, and replaced `physicalImage` inputs are byte-identical;
- main, Bank 2, Bank 3, and all 12 per-box checksums are valid;
- all 12 permanent box structures and the current working box are valid;
- selected Box 12 contains 20 Pokemon in the working copy and 0 in the permanent copy, as in the source;
- all 236 stored records examined across permanent boxes, current cache, and Daycare derive nonzero maximum HP;
- Box 1 Dugtrio stores 201 current HP;
- `Lt<DOT>Ash` encodes with byte `0xF2`;
- all 54 Hall of Fame slots match, including all 9 Charizard entries;
- write provenance reports 47 declared ranges and no overlap;
- corrected Save Genie reparse succeeds;
- strengthened semantic comparison reports no unexpected mismatch.

The source and corrected physical images are intentionally different:

- equal bytes: 24287 of 32768 (74.1180%);
- differing bytes: 8481 of 32768 (25.8820%);
- first difference: `0x004A`;
- last difference: `0x7A52`.

The first and corrected generated images differ in 1502 bytes. Those changes are concentrated in the corrected Hall of Fame, box records, current working box, text fields, regenerated checksums, and associated canonical output.

## Focused Second Emulator Retest

The user confirmed the corrected candidate loaded and that the four defect-focused checks passed:

- the valid punctuation rendered correctly rather than as `?`;
- the selected Box 12 working state appeared with the expected Pokemon;
- the representative Box 1 withdrawal produced valid nonzero HP;
- the Hall of Fame retained the expected members, including Charizard, without `?? BIRD` entries.

This was a focused observation test. No normal in-game save was made because the user did not perform substantial gameplay changes, so no post-emulator battery save exists for checksum, reparse, or drift analysis. The result validates the visible corrections but does not satisfy the separate save-again durability gate.

## Remaining Limitations

- Final operational acceptance requires a normal save, full emulator shutdown, battery-save reload, and returned post-emulator save analysis.
- Unsupported source locations still canonicalize to the verified Red's-house second-floor runtime cluster.
- The source Golbat HP irregularity is preserved and reported; it is not silently repaired.
- The public test suite uses synthetic fixtures; the completed-playthrough saves remain private.
- The proof applies to the documented support boundary and validated fixture, not every possible Pokemon Red save.

## Remaining Save-Again Plan

The focused defect checks in items 1-8 have passed. The remaining durability check must verify:

1. a normal in-game save completes;
2. a full emulator shutdown flushes the battery save;
3. reloading from the battery save succeeds without corruption;
4. the returned post-emulator save passes corrected reparse, checksums, and drift-aware comparison.

## Conclusion Status

The first automated semantic-success claim was invalidated by manual emulator findings. The implementation and automated model have been corrected, and the focused second emulator retest confirmed the four fixes. Semantic sufficiency is not yet concluded because no post-emulator save was produced; acceptance awaits the save-again durability cycle and post-save analysis.
