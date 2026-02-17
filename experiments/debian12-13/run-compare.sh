#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
cd "$ROOT_DIR"

RUN_REMEDIATION="${RUN_REMEDIATION:-1}"
FAIL_ON_ANY="${FAIL_ON_ANY:-0}"

declare -a NAMES
declare -a IMAGES
declare -a DOCKERFILES
declare -a OUTDIRS
declare -a GCC_LINES
declare -a LD_LINES
declare -a EXIT_CODES
declare -a RESULTS

add_case() {
    NAMES+=("$1")
    IMAGES+=("$2")
    DOCKERFILES+=("$3")
    OUTDIRS+=("$4")
}

add_case \
    "debian12" \
    "${IMAGE12:-airbreak-exp:debian12}" \
    "$SCRIPT_DIR/Dockerfile.debian12" \
    "experiments/debian12-13/out/build-debian12"

add_case \
    "debian13" \
    "${IMAGE13:-airbreak-exp:debian13}" \
    "$SCRIPT_DIR/Dockerfile.debian13" \
    "experiments/debian12-13/out/build-debian13"

if [[ "$RUN_REMEDIATION" == "1" ]]; then
    add_case \
        "debian13-remediated" \
        "${IMAGE13_REMEDIATED:-airbreak-exp:debian13-remediated}" \
        "$SCRIPT_DIR/Dockerfile.debian13-remediated" \
        "experiments/debian12-13/out/build-debian13-remediated"
fi

run_in_image() {
    local image="$1"
    shift
    docker run --rm \
        --mount "type=bind,source=$ROOT_DIR,target=/workspace" \
        -w /workspace \
        "$image" \
        bash -lc "$*"
}

echo "[0/4] Clean experiment output directory"
rm -rf "$SCRIPT_DIR/out"
mkdir -p "$SCRIPT_DIR/out"

echo "[1/4] Build images"
for i in "${!NAMES[@]}"; do
    echo " - ${NAMES[$i]} (${IMAGES[$i]})"
    docker build -f "${DOCKERFILES[$i]}" -t "${IMAGES[$i]}" .
done

echo "[2/4] Capture toolchain versions"
for i in "${!NAMES[@]}"; do
    gcc_line="$(run_in_image "${IMAGES[$i]}" "arm-none-eabi-gcc --version | head -n 1")"
    ld_line="$(run_in_image "${IMAGES[$i]}" "arm-none-eabi-ld --version | head -n 1")"
    GCC_LINES+=("$gcc_line")
    LD_LINES+=("$ld_line")
    echo " - ${NAMES[$i]} gcc: $gcc_line"
    echo " - ${NAMES[$i]} ld : $ld_line"
done

echo "[3/4] Run same build workflow for each image (fair comparison)"
for i in "${!NAMES[@]}"; do
    outdir="${OUTDIRS[$i]}"
    set +e
    run_in_image "${IMAGES[$i]}" "rm -rf $outdir; mkdir -p $outdir; make binaries BUILD=$outdir"
    code=$?
    set -e
    EXIT_CODES+=("$code")
    if [[ "$code" -eq 0 ]]; then
        RESULTS+=("PASS")
        echo " - ${NAMES[$i]}: PASS"
    else
        RESULTS+=("FAIL")
        echo " - ${NAMES[$i]}: FAIL (exit=$code)"
    fi
done

echo "[4/4] Result matrix"
for i in "${!NAMES[@]}"; do
    echo " - ${NAMES[$i]} | ${RESULTS[$i]} | exit=${EXIT_CODES[$i]}"
    echo "   gcc: ${GCC_LINES[$i]}"
    echo "   ld : ${LD_LINES[$i]}"
done

if [[ "$FAIL_ON_ANY" == "1" ]]; then
    for code in "${EXIT_CODES[@]}"; do
        if [[ "$code" -ne 0 ]]; then
            exit 1
        fi
    done
fi

exit 0
