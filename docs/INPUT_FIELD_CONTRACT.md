# Input Field Contract

This document defines which parts of a target `.red.json` are allowed to influence generation.

## Authority Rules

### Authoritative semantic input

The generator reads semantic state from the target `.red.json`, primarily from `decoded`.

### Forbidden input

The generator must not use:

- `physicalImage.standardSramHex`
- `physicalImage.trailingDataHex`
- any raw byte image derived from the target save

### Secondary reference only

The generator may inspect selected metadata for compatibility, validation, or reporting, but not as a hidden byte source:

- `schema`
- `source`
- `integrity`
- `coverage`
- `diagnostics`
- selected `conversionModel` notes

## Compatibility Gate

Milestone 0 proposed compatibility requirements:

| Field | Status | Use |
|---|---|---|
| `schema.format` | required | Must equal `pkmn-red-master-save`. |
| `schema.schemaVersion` | required | Initial support target is `0.1.0`. |
| `schema.game` | required | Must identify Pokemon Red. |
| `schema.regionAssumption` | required | Initial support target is `USA-Europe`. |
| `source.fileSize.decimal` | required | Used for profile sanity checks and provenance only. |
| `source.hashes.wholeFileSha256` | optional | Reporting and fixture traceability only. |

## Primary Semantic Sections

These sections are the intended source of gameplay state:

| Section | Status | Intended generator role |
|---|---|---|
| `decoded.trainer` | required when supported | Player name and trainer ID. |
| `decoded.rival` | required when supported | Rival name. |
| `decoded.moneyAndCoins` | required when supported | Money and coin serialization. |
| `decoded.options` | required when supported | User-facing options and related bytes. |
| `decoded.playtime` | required when supported | Playtime fields. |
| `decoded.location` | required when supported | Current map and safe coordinate policy. |
| `decoded.badges` | required when supported | Badge bitfield and mirrored state. |
| `decoded.pokedex` | required when supported | Seen and owned data plus derived counts. |
| `decoded.inventory` | required when supported | Bag and PC item storage. |
| `decoded.party` | required when supported | Party count, species, records, OT names, nicknames. |
| `decoded.pcStorage` | required when supported | Permanent boxes 1-12. |
| `decoded.currentBoxCache` | required when supported | Current-box synchronization target. |
| `decoded.daycare` | required when supported | Daycare occupancy and stored mon. |
| `decoded.hallOfFame` | required when supported | Hall of Fame records or intentional clearing. |
| `decoded.events` | required when supported | Named event-bit state. |
| `decoded.trainerBattles` | required when supported | Persistent trainer battle state. |
| `decoded.staticBattles` | required when supported | Static encounter state. |
| `decoded.storyProgress` | required when supported | Story milestone state. |
| `decoded.scripts` | required when supported | Persistent script-related values only. |
| `decoded.missableObjects` | required when supported | Persistent missable object state. |
| `decoded.hiddenItems` | required when supported | Hidden item persistence. |
| `decoded.hiddenCoins` | required when supported | Hidden coin persistence. |
| `decoded.visitedTowns` | required when supported | Fly/visited-town persistence. |
| `decoded.worldState` | required when supported | Supported persistent world-state values. |
| `decoded.runtimeState` | deferred / restricted | Only consume fields proven required for safe load/save behavior. |

## Conversion Model Policy

`conversionModel` is not a blanket authority for save generation.

Milestone 0 proposal:

- prefer `decoded` when a field already has direct Red save semantics
- use `conversionModel` only for well-documented normalization or policy hints
- document every use explicitly before implementation

## Reference-Only Sections

These sections are useful for validation or provenance but are not intended direct byte authorities:

- `integrity`
- `coverage`
- `unknownData`
- `reconstruction`
- `diagnostics`

## First Deferred Areas

Even after generation starts, some fields may remain deferred until they are proven safe:

- runtime-only map buffers
- post-checksum Bank 1 tail `0x3524..0x3FFF`
- unknown Bank 2 and Bank 3 tails
- ambiguous script/cache values
- impossible event combinations without a safety policy
