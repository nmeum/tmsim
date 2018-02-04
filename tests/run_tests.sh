#!/bin/sh
set -e

for dir in *; do
	[ -d "${dir}" ] || continue
	printf "\n##\n# Running %s tests.\n##\n\n" "${dir##*/}"
	(cd "${dir}" ; ./run_tests.sh)
done
