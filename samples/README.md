# Public Samples

These `.red.json` files are synthetic public examples derived from the committed dummy reference JSON. They do not include a `physicalImage`, private save data, ROM data, screenshots, or emulator output.

## Files

- `minimal.red.json`: minimal safe Red's-house baseline with empty party, empty storage, empty Daycare, empty Hall of Fame, empty bag, and empty Pokedex.
- `representative.red.json`: small representative supported state with one party Pidgey, one boxed Pidgey, occupied Daycare, one Hall of Fame record, inventory, badges, and simple owned/seen Pokedex data.
- `unsupported-viridian-pokemon-center.red.json`: schema-valid negative fixture that intentionally requests a paused unsafe location. The CLI should reject it until the full map-runtime cluster for that location is implemented and emulator-proven.

## Example

```sh
build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --report /tmp/representative.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/representative.sav
```

The samples exercise public generation workflows only. They are not a replacement for private Save Genie oracle validation or emulator load/save-again validation.
