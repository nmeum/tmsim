#!/bin/sh

TMSIM="${TMSIM:-$(pwd)/../../tmsim}"
if [ ! -x "${TMSIM}" ]; then
	echo "Couldn't find tmsim executable: '${TMSIM}'" 1>&2
	exit 1
fi

outfile=$(mktemp ${TMPDIR:-/tmp}/tmsimXXXXXX)
trap "rm -f '${outfile}'" INT EXIT

exitstatus=0
exresfile=

for test in *; do
	[ -d "${test}" ] || continue

	name=${test##*/}
	printf "Running test case '%s': " "${name}"

	exitstatus=0
	read -r exitstatus < "${test}/exit"

	(cd "${name}" && "${TMSIM}" input 2>"${outfile}")
	ret=$?

	if ! cmp -s "${outfile}" "${test}/output"; then
		printf "FAIL: Output didn't match.\n\n" 2>&1
		diff -u "${outfile}" "${test}/output"
		exit 1
	fi

	if [ ${ret} -ne ${exitstatus} ]; then
		printf "FAIL: Expected '%d', got '%d'.\n" \
			"${exitstatus}" "${ret}"
		exit
	fi

	printf "OK.\n"
done
