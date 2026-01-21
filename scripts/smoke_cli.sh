
#!/usr/bin/env bash
set -euo pipefail

BIN="${BIN:-./build/ticketverify}"

echo "== 1) --help: exit 0, stdout vide, stderr non vide =="
out="$("$BIN" --help > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 0
test ! -s /tmp/tv_out
test -s /tmp/tv_err
grep -q "Usage" /tmp/tv_err

echo "== 2) --version: exit 0, stdout vide, stderr non vide =="
out="$("$BIN" --version > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 0
test ! -s /tmp/tv_out
test -s /tmp/tv_err

echo "== 3) stdin vide: exit 2, stdout JSON erreur, stderr vide =="
out="$(printf "" | "$BIN" > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 2
test -s /tmp/tv_out
test ! -s /tmp/tv_err
grep -q '"ok":false' /tmp/tv_out
grep -q '"code":"INPUT_EMPTY"' /tmp/tv_out

echo "== 4) args invalides: exit 2, stdout JSON erreur =="
out="$("$BIN" --locale xx > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 2
grep -q '"code":"ARGS_INVALID"' /tmp/tv_out

echo "== 5) happy-ish path: exit 0, stdout JSON, stderr vide =="
out="$(printf "TOTAL 12.30\n" | "$BIN" > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 0
test -s /tmp/tv_out
test ! -s /tmp/tv_err
grep -q '"ok":' /tmp/tv_out

echo "== 6) --debug: stdout JSON, stderr non vide (truncation) =="
out="$(python3 - <<'PY'
:wprint("x\n"*5000, end="")
PY
| "$BIN" --max-lines 10 --debug > /tmp/tv_out 2> /tmp/tv_err; echo $?)"
test "$out" -eq 0
test -s /tmp/tv_out
test -s /tmp/tv_err
grep -q '"ok":' /tmp/tv_out

echo "✅ SMOKE TEST OK"
