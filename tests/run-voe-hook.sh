#!/bin/sh
# Build and run the VoE application-hook tests.
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

cc=${CC:-cc}
cflags="-std=c99 -Wall -Wextra -Werror -I$root/tests/include -I$root/soes"

"$cc" $cflags -DUSE_VOE=1 \
   "$root/tests/voe_hook.c" "$root/soes/esc_voe.c" \
   -o "$tmp/voe_hook"
"$tmp/voe_hook"
