#!/usr/bin/env bash
set -euo pipefail

URL="${1:-http://localhost:8080/health}"
N="${2:-500}"
C="${3:-25}"

echo "Benchmarking $URL"
echo "Requests: $N"
echo "Concurrency: $C"
echo

# Simple parallel load using curl + xargs (works on macOS)
seq 1 "$N" | xargs -n1 -P "$C" -I{} curl -s -o /dev/null -w "%{http_code}\n" "$URL" \
  | awk '{count[$1]++} END {for (c in count) print c, count[c]}'