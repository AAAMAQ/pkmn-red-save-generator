# Final Release Validation Matrix

This matrix tracks the combined Final Release Milestone that merges the original Milestones 7, 8, and 9.

## Automated Matrix

| Area | Evidence | Status |
| --- | --- | --- |
| CMake configure/build | `cmake -S . -B build`, `cmake --build build` | passed locally 2026-07-09 |
| Unit and integration tests | `ctest --test-dir build --output-on-failure` | passed locally 2026-07-09 |
| Xcode build | signing disabled macOS CLI build | passed locally 2026-07-09 |
| CLI help | `pkmn-red-save-generator --help` | passed locally 2026-07-09 |
| CLI version | `pkmn-red-save-generator --version` reports `1.0.0` | passed locally 2026-07-09 |
| Public minimal sample | generate, report, validate-save; SHA-256 `ef381989cc99238b1f29060e1f5eb4b8b4d623bd0c834e57275832f05177601b` | passed locally 2026-07-09 |
| Public representative sample | generate, report, validate-save; SHA-256 `135aa4a72e4eb8ff8111e3ea04ada01eab37913af9fb98041b15fd9188df9588` | passed locally 2026-07-09 |
| Unsafe location rejection | Viridian Pokemon Center negative sample rejected | passed locally 2026-07-09 |
| Determinism | representative sample repeated bytes match | passed locally 2026-07-09 |
| Physical-image isolation | representative sample output unchanged after physicalImage mutation | passed locally 2026-07-09 |
| Write provenance | generation reports contain no undeclared overlaps | passed locally 2026-07-09 |
| Private-path scan | tracked files contain no private absolute paths or private fixture names | passed locally 2026-07-09 |
| Dummy immutability | committed dummy SHA-256 remains `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7` | passed locally 2026-07-09 |
| Fresh-copy reproducibility | clean temporary copy, CMake build/test, CLI version, sample generation, sample validation | passed locally 2026-07-09 |

## Private Oracle Matrix

These checks require the read-only sibling Pokemon Red Save Genie repository and private validation fixtures. They are intentionally not part of public CI.

| Area | Required Evidence | Status |
| --- | --- | --- |
| Save Genie reparse | final public representative and private full post-save SRAM outputs decode successfully | passed locally 2026-07-09 |
| Field-aware comparison | supported semantic fields compare without unexpected mismatches; gameplay movement/save changes are classified as expected drift | passed locally 2026-07-09 |
| Unsupported non-empty state | surfaced or rejected rather than silently discarded | passed locally 2026-07-09 |
| Save-again comparison | emulator-saved SRAM reparses and differs only by expected gameplay drift | passed locally 2026-07-09 |

Private Red's-house full-state candidate generated locally on 2026-07-09:

- output size: `32768`
- SHA-256: `72bc7b91e4665858b6f7ad4f8f9b96f68b61862739e2d3877607f2b6c154bb55`
- checksums: main, Bank 2, Bank 3, and all 12 per-box checksums valid
- determinism: passed
- physical-image isolation: passed
- final post-save SHA-256: `f680c889852febd2101cd0a13d8a93295fefc543e524f2861d0ce5c9fc997c3e`
- final post-save status: Save Genie reparse passed; main, Bank 2, Bank 3, and all per-box checksums valid; selected-box cache coherent

Public representative final post-save candidate:

- generated SHA-256: `135aa4a72e4eb8ff8111e3ea04ada01eab37913af9fb98041b15fd9188df9588`
- post-save SHA-256: `4f34d11612f31874b60d9720144621fc7648e657cf1f63cacb124dd0c2a3a0c5`
- final post-save status: Save Genie reparse passed; main, Bank 2, Bank 3, and all per-box checksums valid; current-box cache coherent

## Emulator Matrix

| Candidate | Required Interaction | Status |
| --- | --- | --- |
| A: Public representative core load/save | Continue, clean load, movement, menus, trainer/party/Pokedex/Bag/inventory/visible progression checks, normal gameplay, normal save, shutdown, reparse | passed 2026-07-09 |
| B: Private full Red's-house runtime/world | Continue, clean load, movement, menus, broad travel, doors/stairs/warps/scripts/map transitions, normal save, shutdown, reparse | passed 2026-07-09 |
| C: PC storage | Bill's PC, view occupied boxes, deposit, switch box, game-triggered save, withdraw, normal save, shutdown, storage/cache comparison | passed in Milestone 5 controlled validation |
| D: Hall of Fame | 18 records viewed in Milestone 6 validation; final post-save Save Genie reparse preserved 18 records | passed with direct prior emulator evidence and final oracle preservation |
| E: Daycare | generated occupied/unoccupied structures and post-save oracle preservation; direct final UI interaction not repeated | passed structurally; direct UI interaction remains deferred |
| F: Unsupported locations | unsafe or incomplete location clusters rejected by CLI before generation | passed via automated negative sample |

## Release Gate

Release readiness requires all automated checks, private oracle checks, and emulator candidates to pass. A green parser/comparator result alone is not sufficient because the Milestone 5-6 incident proved emulator load behavior is authoritative.

The final release evidence preserves one corrected interpretation note: the private validation trainer ID is numeric `257` (`0x0101`), displayed by Pokemon Red as `00257`. An earlier `275`/`00275` visual reading was discarded after screenshot review and Save Genie reparse.
