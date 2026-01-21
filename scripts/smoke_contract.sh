#!/usr/bin/env bash
set -euo pipefail

BIN="${1:-./build/ticketverify}"
run_json() {
  local name="$1"
  local input="$2"
  local expect_ere="$3"
  shift 3

  echo "=== $name ==="

  set +e
  if [ -n "$input" ]; then
    out="$(printf "%b" "$input" | "$BIN" "$@")"
  else
    out="$("$BIN" "$@")"
  fi
  rc=$?
  set -e

  # NOTE: on ne vérifie pas rc ici; c'est le rôle de expect_exit.
  # run_json sert juste à valider le contenu stdout (JSON).

  if echo "$out" | grep -Eq "$expect_ere"; then
    echo "OK"
  else
    echo "FAIL: expected to find (ERE): $expect_ere"
    echo "EXIT: $rc"
    echo "OUTPUT: $out"
    exit 1
  fi
}

expect_exit() {
  local name="$1"
  local input="$2"
  local expect_code="$3"
  shift 3

  echo "=== $name ==="
  set +e
  printf "%b" "$input" | "$BIN" "$@" >/dev/null 2>/dev/null
  code=$?
  set -e

  if [ "$code" -eq "$expect_code" ]; then
    echo "OK"
  else
    echo "FAIL: expected exit code $expect_code, got $code"
    exit 1
  fi
}

expect_stdout_empty() {
  local name="$1"
  shift

  echo "=== $name ==="
  out="$("$BIN" "$@" 2>/dev/null || true)"
  if [ -z "$out" ]; then
    echo "OK"
  else
    echo "FAIL: expected empty stdout, got: $out"
    exit 1
  fi
}

expect_stderr_non_empty() {
  local name="$1"
  shift

  echo "=== $name ==="
  err="$( { "$BIN" "$@" 1>/dev/null; } 2>&1 || true )"

  if [ -n "$err" ]; then
    echo "OK"
  else
    echo "FAIL: expected non-empty stderr"
    exit 1
  fi
}

expect_stderr_empty_with_stdin() {
  local name="$1"
  local input="$2"
  shift 2

  echo "=== $name ==="
  err="$( { printf "%b" "$input" | "$BIN" "$@" 1>/dev/null; } 2>&1 || true )"

  if [ -z "$err" ]; then
    echo "OK"
  else
    echo "FAIL: expected empty stderr, got: $err"
    exit 1
  fi
}

expect_stderr_contains_with_stdin() {
  local name="$1"
  local input="$2"
  local needle_ere="$3"
  shift 3

  echo "=== $name ==="
  err="$( { printf "%b" "$input" | "$BIN" "$@" 1>/dev/null; } 2>&1 || true )"

  if echo "$err" | grep -Eq "$needle_ere"; then
    echo "OK"
  else
    echo "FAIL: expected stderr to contain (ERE): $needle_ere"
    echo "STDERR: $err"
    exit 1
  fi
}

# ----------------- CONTRACT -----------------

# help / version: exit 0, stdout empty, stderr non-empty
expect_exit "help_exit_0" "" 0 --help
expect_stdout_empty "help_stdout_empty" --help
expect_stderr_non_empty "help_stderr_non_empty" --help

expect_exit "version_exit_0" "" 0 --version
expect_stdout_empty "version_stdout_empty" --version
expect_stderr_non_empty "version_stderr_non_empty" --version

# args invalid: exit 2, stdout json error, stderr empty
expect_exit "args_invalid_exit_2" "" 2 --locale xx
run_json "args_invalid_json" "" "\"code\"[[:space:]]*:[[:space:]]*\"ARGS_INVALID\"" --locale xx
expect_stderr_empty_with_stdin "args_invalid_stderr_empty" "" --locale xx

# empty stdin: exit 2, stdout json INPUT_EMPTY, stderr empty
expect_exit "stdin_empty_exit_2" "   \n\n" 2
run_json    "stdin_empty_json"    "   \n\n" "\"code\"[[:space:]]*:[[:space:]]*\"INPUT_EMPTY\""
expect_stderr_empty_with_stdin "stdin_empty_stderr_empty" "   \n\n"

# happy path: exit 0, stdout json, stderr empty
expect_exit "happy_exit_0" "TOTAL 1,00\n" 0
run_json "happy_stdout_json" "TOTAL 1,00\n" "\"schema\"[[:space:]]*:[[:space:]]*\"ticketverify\\.v1\""
run_json "happy_has_result"  "TOTAL 1,00\n" "\"result\"[[:space:]]*:"
expect_stderr_empty_with_stdin "happy_stderr_empty" "TOTAL 1,00\n"

# debug: stdout stays json, stderr contains debug (force truncation)
big_input="$(python3 - <<'PY'
print("x\n"*5000, end="")
PY
)"
expect_exit "debug_truncate_exit_0" "$big_input" 0 --max-lines 10 --debug
run_json "debug_truncate_stdout_json" "$big_input" "\"schema\"[[:space:]]*:[[:space:]]*\"ticketverify\\.v1\"" --max-lines 10 --debug
run_json "debug_truncate_lines_10"    "$big_input" "\"lines\"[[:space:]]*:[[:space:]]*10" --max-lines 10 --debug
expect_stderr_contains_with_stdin "debug_truncate_stderr_has_debug" "$big_input" "debug" --max-lines 10 --debug

echo "ALL CONTRACT SMOKE TESTS PASSED"

