#!/bin/sh
# Build and run the FoE read boundary tests with reads disabled and enabled.
set -eu

root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

cc=${CC:-cc}
cflags="-std=c99 -Wall -Wextra -Werror -I$root/tests/include -I$root/soes"

for enabled in 0 1; do
   "$cc" $cflags -DFOE_READ_SUPPORTED="$enabled" \
      "$root/tests/foe_read.c" "$root/soes/esc_foe.c" \
      -o "$tmp/foe_read_$enabled"
   "$tmp/foe_read_$enabled"
done
