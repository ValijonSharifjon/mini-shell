#!/bin/bash

cd "$(dirname "$0")/.."

PASS=0
FAIL=0

check() {
    if [ "$2" = "$3" ]; then
        echo "PASS: $1"
        ((PASS++))
    else
        echo "FAIL: $1 (expected: '$2', got: '$3')"
        ((FAIL++))
    fi
}

run() {
    echo "$1" | ./myshell | grep -v "^exit" | sed 's/^myshell>> //' | grep -v "^$" | tr -d '\n'
}

check "pwd"        "$(pwd)"  "$(run 'pwd')"
check "echo"       "hello"   "$(run 'echo hello')"
check "pipe"       "hello"   "$(run 'echo hello | cat')"
check "cd"         "/tmp"    "$(printf 'cd /tmp\npwd' | ./myshell | grep -v '^exit' | awk 'NR==1{print $NF}')"

echo "echo hello > /tmp/test_out.txt" | ./myshell > /dev/null
check "redirect >" "hello"   "$(cat /tmp/test_out.txt 2>/dev/null)"

rm -f /tmp/test_out.txt
echo ""
echo "Results: $PASS passed, $FAIL failed"