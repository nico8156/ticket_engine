#!/usr/bin/env bash
set -euo pipefail

BIN="${1:-./build/ticketverify}"

pass() { echo "OK"; }
fail() { echo "FAIL: $*"; exit 1; }

# run_json NAME INPUT EXPECT_SNIPPET [ARGS...]
run_json() {
  local name="$1"
  local input="$2"
  local expect="$3"
  shift 3

  echo "=== $name ==="
  rm -f /tmp/tv_out /tmp/tv_err
  set +e
  printf "%b" "$input" | "$BIN" "$@" > /tmp/tv_out 2> /tmp/tv_err
  local code=$?
  set -e

  local out; out="$(cat /tmp/tv_out)"
  if echo "$out" | grep -q "$expect"; then
    pass
  else
    echo "---- STDOUT ----"; cat -A /tmp/tv_out; echo
    echo "---- STDERR ----"; cat -A /tmp/tv_err; echo
    fail "expected stdout to contain: $expect"
  fi
}

# assert_exit NAME INPUT EXPECT_CODE [ARGS...]
assert_exit() {
  local name="$1"
  local input="$2"
  local expect_code="$3"
  shift 3

  echo "=== $name ==="
  rm -f /tmp/tv_out /tmp/tv_err
  set +e
  printf "%b" "$input" | "$BIN" "$@" > /tmp/tv_out 2> /tmp/tv_err
  local code=$?
  set -e

  if [ "$code" -eq "$expect_code" ]; then pass
  else
    echo "---- STDOUT ----"; cat -A /tmp/tv_out; echo
    echo "---- STDERR ----"; cat -A /tmp/tv_err; echo
    fail "expected exit code $expect_code, got $code"
  fi
}

# assert_stdout_empty NAME INPUT [ARGS...]
assert_stdout_empty() {
  local name="$1"
  local input="$2"
  shift 2

  echo "=== $name ==="
  rm -f /tmp/tv_out /tmp/tv_err
  set +e
  printf "%b" "$input" | "$BIN" "$@" > /tmp/tv_out 2> /tmp/tv_err
  local code=$?
  set -e

  # stdout must be empty
  if [ -s /tmp/tv_out ]; then
    echo "---- STDOUT ----"; cat -A /tmp/tv_out; echo
    echo "---- STDERR ----"; cat -A /tmp/tv_err; echo
    fail "expected stdout empty, got content (exit=$code)"
  fi
  pass
}

# assert_stderr_contains NAME INPUT NEEDLE [ARGS...]
assert_stderr_contains() {
  local name="$1"
  local input="$2"
  local needle="$3"
  shift 3

  echo "=== $name ==="
  rm -f /tmp/tv_out /tmp/tv_err
  set +e
  printf "%b" "$input" | "$BIN" "$@" > /tmp/tv_out 2> /tmp/tv_err
  local code=$?
  set -e

  if grep -q "$needle" /tmp/tv_err; then pass
  else
    echo "---- STDOUT ----"; cat -A /tmp/tv_out; echo
    echo "---- STDERR ----"; cat -A /tmp/tv_err; echo
    fail "expected stderr to contain: $needle (exit=$code)"
  fi
}

# assert_stderr_empty NAME INPUT [ARGS...]
assert_stderr_empty() {
  local name="$1"
  local input="$2"
  shift 2

  echo "=== $name ==="
  rm -f /tmp/tv_out /tmp/tv_err
  set +e
  printf "%b" "$input" | "$BIN" "$@" > /tmp/tv_out 2> /tmp/tv_err
  local code=$?
  set -e

  if [ -s /tmp/tv_err ]; then
    echo "---- STDOUT ----"; cat -A /tmp/tv_out; echo
    echo "---- STDERR ----"; cat -A /tmp/tv_err; echo
    fail "expected stderr empty (exit=$code)"
  fi
  pass
}

# ---------- CONTRACT TESTS ----------

# 1) help/version => stderr only, exit 0, stdout empty
assert_exit        "help_exit_0"    "" 0 --help
assert_stdout_empty "help_stdout_empty" "" --help
assert_stderr_contains "help_stderr_has_usage" "" "Usage" --help

assert_exit        "version_exit_0" "" 0 --version
assert_stdout_empty "version_stdout_empty" "" --version
# on check juste que c'est pas vide
echo "=== version_stderr_non_empty ==="
"$BIN" --version > /tmp/tv_out 2> /tmp/tv_err || true
[ -s /tmp/tv_err ] || fail "expected stderr non-empty for --version"
pass

# 2) invalid args => exit 2, stdout JSON error, stderr empty (sans --debug)
assert_exit "args_invalid_exit_2" "" 2 --locale xx
run_json    "args_invalid_json"   "" "\"ok\":false" --locale xx
assert_stderr_empty "args_invalid_stderr_empty" "" --locale xx

# 3) empty stdin => exit 2, stdout JSON error INPUT_EMPTY, stderr empty (sans debug)
assert_exit "stdin_empty_exit_2" "   \n\n" 2
run_json    "stdin_empty_json"   "   \n\n" "\"code\":\"INPUT_EMPTY\""
assert_stderr_empty "stdin_empty_stderr_empty" "   \n\n"

# 4) happy-ish => exit 0, stdout JSON, stderr empty
assert_exit "happy_exit_0" "TOTAL 1,00\n" 0
run_json    "happy_stdout_json" "TOTAL 1,00\n" "\"ok\":"
assert_stderr_empty "happy_stderr_empty" "TOTAL 1,00\n"

# 5) debug: ne doit jamais polluer stdout (on force une troncature pour avoir du stderr)
assert_exit "debug_truncate_exit_0" "$(python3 - <<'PY'
print("x\n"*5000, end="")
PY
)" 0 --max-lines 10 --debug
run_json "debug_truncate_stdout_json" "$(python3 - <<'PY'
print("x\n"*5000, end="")
PY
)" "\"ok\":" --max-lines 10 --debug
assert_stderr_contains "debug_truncate_stderr_has_debug" "$(python3 - <<'PY'
print("x\n"*5000, end="")
PY
)" "debug" --max-lines 10 --debug

echo "ALL CONTRACT SMOKE TESTS PASSED"

