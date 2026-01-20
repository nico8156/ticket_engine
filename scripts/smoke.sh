#!/usr/bin/env bash
set -euo pipefail

BIN="${1:-./build/ticketverify}"

run() {
  local name="$1"
  local input="$2"
  local expect="$3"

  echo "=== $name ==="
  out="$(printf "%b" "$input" | "$BIN")"

  # check JSON contains expected snippet
  if echo "$out" | grep -q "$expect"; then
    echo "OK"
  else
    echo "FAIL: expected to find: $expect"
    echo "OUTPUT: $out"
    exit 1
  fi
}

# 1) Basic TOTAL with comma + euro sign
run "total_basic_comma" \
"CAFE DE LA PLACE\nTOTAL 4,00 €\n" \
"\"value\":4.0"

# 2) TOTAL TTC
run "total_ttc" \
"TOTAL TTC: 12,50 EUR\n" \
"\"value\":12.5"

# 3) NET A PAYER
run "net_a_payer" \
"NET A PAYER 8,20\n" \
"\"value\":8.2"

# 4) A PAYER with spaces
run "a_payer_spaces" \
"A  PAYER   3,40\n" \
"\"value\":3.4"

# 5) Reject if no total keyword
run "reject_no_total" \
"CAFE DE LA PLACE\nESPRESSO 2,20\n" \
"\"status\":\"reject\""

# 6) Empty input => exit 3
echo "=== empty_input_exit_code ==="
set +e
printf "   \n\n" | "$BIN" >/dev/null 2>&1
code=$?
set -e
if [ "$code" -eq 3 ]; then
  echo "OK"
else
  echo "FAIL: expected exit code 3, got $code"
  exit 1
fi

echo "ALL SMOKE TESTS PASSED"

