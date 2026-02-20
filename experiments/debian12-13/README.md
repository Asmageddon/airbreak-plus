# Debian 12 vs 13 Experiment

This experiment runs a fair, per-image comparison.

- All images run the same build workflow: `make binaries`
- Output is reported as per-image `PASS/FAIL`
- `debian13-remediated` can be included as an extra case

## Run

```bash
./experiments/debian12-13/run-compare.sh
```

Optional modes:

```bash
# Skip remediation path
RUN_REMEDIATION=0 ./experiments/debian12-13/run-compare.sh

# Return non-zero if any image fails
FAIL_ON_ANY=1 ./experiments/debian12-13/run-compare.sh
```

## What it does

1. Builds images:
   - `experiments/debian12-13/Dockerfile.debian12`
   - `experiments/debian12-13/Dockerfile.debian13`
2. Optionally builds remediation image:
   - `experiments/debian12-13/Dockerfile.debian13-remediated`
3. Captures toolchain versions (`arm-none-eabi-gcc`, `arm-none-eabi-ld`)
4. Runs the same build command for each image:
   - `make binaries BUILD=<case-output-dir>`
5. Uses isolated output dirs under this folder:
   - `experiments/debian12-13/out/build-debian12`
   - `experiments/debian12-13/out/build-debian13`
   - `experiments/debian12-13/out/build-debian13-remediated`
6. Prints a result matrix with:
   - case name
   - `PASS/FAIL`
   - exit code
   - gcc/ld version lines
