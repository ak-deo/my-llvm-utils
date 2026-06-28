#!/usr/bin/env bash
# Run an out-of-tree LLVM pass plugin over an IR file with opt.
#
# The plugins in this repo register *named* passes (e.g. "my-dce") via the New
# PM pipeline-parsing callback, so opt invokes them by name with -passes. The
# plugin .so is loaded with -load-pass-plugin. The .so base name and the
# registered pass name can differ (e.g. libMyDCE.so registers "my-dce"), so
# both are passed explicitly.
#
# Usage:
#   scripts/run-pass.sh <plugin-name> <pass-name> [input.ll|input.bc] [extra opt args...]
#
# Examples:
#   scripts/run-pass.sh MyDCE my-dce test/hello.ll
#   scripts/run-pass.sh MyDCE my-dce test/hello.ll -o out.ll
set -euo pipefail

PLUGIN_NAME="${1:?usage: run-pass.sh <plugin-name> <pass-name> [input.ll] [extra opt args...]}"
PASS="${2:?missing pass name (the name registered with LLVM, e.g. my-dce)}"
INPUT="${3:-test/hello.ll}"
shift 2 || true
shift || true

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PLUGIN="$ROOT/build/lib/lib${PLUGIN_NAME}.so"
OPT="${OPT:-opt-19}"

if [[ ! -f "$PLUGIN" ]]; then
  echo "error: plugin not found: $PLUGIN (build it first: cmake --build build)" >&2
  exit 1
fi

set -x
"$OPT" \
  -load-pass-plugin="$PLUGIN" \
  -passes="$PASS" \
  -S \
  "$INPUT" \
  "$@"
