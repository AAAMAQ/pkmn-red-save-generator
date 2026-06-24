# Supported Target Profile

Status: initial Milestone 0 profile proposal

## Supported Initial Target

- game: `Pokemon Red`
- region: `USA-Europe`
- generation: `1`
- expected standard SRAM size: `32768` bytes (`0x8000`)
- initial output length policy: `32768` bytes (`0x8000`)
- target `.red.json` schema format: `pkmn-red-master-save`
- supported `.red.json` schema version: `0.1.0`

## Template Profile

- template path:
  - `Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav`
- template SHA-256:
  - `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7`
- template size:
  - `32768`

## Revision Assumptions

- revision is not yet proven from save structure alone
- the filename includes `SGB Enhanced`, but the generator should not treat the filename as sufficient identity proof
- until stronger evidence exists, the supported profile should be described conservatively as:
  - Pokemon Red
  - USA-Europe assumption
  - standard `0x8000` SRAM layout verified by Save Genie

## Unsupported Variants

Not supported in the first generator version:

- Pokemon Blue
- Pokemon Yellow
- Japanese Pokemon Red
- other regional or revision variants
- ROM hacks
- randomizers
- emulator-specific extended formats
- nonstandard raw save wrappers as direct generation targets

## Physical-Image Policy

- target `.red.json` `physicalImage`: forbidden as generation input
- standalone dummy `.sav`: permitted template candidate

## Important Template Caveat

The current dummy template does not qualify as a clean semantic baseline:

- valid main checksum
- invalid bank 2 all-box checksum
- invalid bank 3 all-box checksum
- permanent boxes decode as fully occupied with invalid-looking entries

Therefore the supported profile is narrower than "anything that loads with this template." Generator support must be tied to explicit overwrite and checksum coverage.
