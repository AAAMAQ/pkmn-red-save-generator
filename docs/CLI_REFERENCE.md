# CLI Reference

The executable is `pkmn-red-save-generator`. The CMake build places it at `build/pkmn-red-save-generator`.

## General

```sh
build/pkmn-red-save-generator --help
build/pkmn-red-save-generator --version
```

`--version` currently reports `pkmn-red-save-generator 1.0.0`.

## Generate

```sh
build/pkmn-red-save-generator generate <input.red.json> <output.sav> \
  --report <output.report.json> \
  --summary \
  --show-ranges
```

Equivalent explicit form:

```sh
build/pkmn-red-save-generator generate \
  --input <input.red.json> \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json \
  --output <output.sav> \
  --report <output.report.json>
```

Safety behavior:

- output save paths must not collide with the input JSON or template;
- report paths must not collide with the output save or template;
- existing output and report files are not overwritten;
- target `physicalImage` is ignored;
- unsupported locations are canonicalized to Red's house second floor with an explicit warning;
- contradictory required mirrors are rejected, while valid current-working-box divergence is preserved.

## Dry Run

```sh
build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --dry-run \
  --summary \
  --show-ranges
```

Dry-run generation builds and validates bytes but does not write the `.sav`.

## Validate

```sh
build/pkmn-red-save-generator validate samples/minimal.red.json
build/pkmn-red-save-generator inspect samples/representative.red.json
build/pkmn-red-save-generator validate-template \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json
build/pkmn-red-save-generator validate-save --input-save /tmp/generated.sav
```

`validate-save` validates the permanent boxes and current working box independently. A working box that differs from the selected permanent copy is reported as valid Gen I state and does not require an override.

Operational validators and inspectors:

```sh
build/pkmn-red-save-generator validate-text --text 'Lt<DOT>Ash'
build/pkmn-red-save-generator validate-boxes --input <input.red.json>
build/pkmn-red-save-generator validate-pokemon-operability --input <input.red.json>
build/pkmn-red-save-generator validate-hall-of-fame --input <input.red.json>
build/pkmn-red-save-generator inspect-box --input <input.red.json> --box 12
build/pkmn-red-save-generator inspect-box --input <input.red.json> --box 12 --current-cache
build/pkmn-red-save-generator inspect-pokemon --input <input.red.json> --box 1 --slot 2
```

`--current-cache` selects the Bank 1 working representation and requires `--box` to equal the selected box number.

## Determinism And Physical-Image Isolation

```sh
build/pkmn-red-save-generator check-determinism \
  --input samples/representative.red.json \
  --work-dir /tmp/savegen-determinism

build/pkmn-red-save-generator check-physical-image-isolation \
  --input samples/representative.red.json \
  --work-dir /tmp/savegen-physical-image
```

Expected invariant:

```text
same semantic input + same template + same profile => identical generated .sav bytes
```

Changing, removing, or corrupting the target JSON `physicalImage` must not alter output bytes.

## Semantic Comparison

```sh
build/pkmn-red-save-generator compare-semantics \
  --target-json <input.red.json> \
  --reparsed-json <generated.red.json> \
  --report <semantic-comparison.json> \
  --markdown <semantic-comparison.md>
```

The `reparsed-json` should come from the independent Pokemon Red Save Genie oracle. Public CI does not require the sibling Save Genie repository.

## Physical And Post-Emulator Comparison

```sh
build/pkmn-red-save-generator compare-physical \
  --original <original.sav> \
  --generated <generated.sav> \
  --report <physical-comparison.json> \
  --markdown <physical-comparison.md>

build/pkmn-red-save-generator analyze-post-emulator \
  --before <generated.sav> \
  --after <generated-post-emulator.sav> \
  --report <post-emulator-comparison.json> \
  --markdown <post-emulator-comparison.md>
```

Reports use zero-based inclusive offsets. Post-emulator analysis validates checksums, all permanent box structures, and the current working box in both files. Gameplay drift still requires field-aware classification after Save Genie reparse.

## Public Samples

```sh
build/pkmn-red-save-generator generate samples/minimal.red.json /tmp/minimal.sav \
  --report /tmp/minimal.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/minimal.sav

build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --report /tmp/representative.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/representative.sav
```

The unsupported-location sample is expected to pass inspection with a warning that generation will use the safe Red's-house location:

```sh
build/pkmn-red-save-generator inspect samples/unsupported-viridian-pokemon-center.red.json
```

## Exit Codes

- `0`: command succeeded, or help/version was displayed.
- nonzero: invalid arguments, missing file, invalid JSON, unsupported semantic input, unsafe location, checksum failure, write-overlap failure, output collision, or semantic mismatch.
