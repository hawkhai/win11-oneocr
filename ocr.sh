#!/bin/sh
set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
case "$(uname -s):$(uname -m)" in
  Linux:x86_64) binary="$root/artifacts/linux-x64/bin/ocr" ;;
  *) printf '%s\n' 'No verified prebuilt runtime for this platform. See docs/portable.md to build from source.' >&2; exit 1 ;;
esac
exec "$binary" --model-dir "$root/models" "$@"
