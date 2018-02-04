#!/bin/sh

TMSIM="${TMSIM:-$(pwd)/../../../tmsim}"
if [ ! -x "${TMSIM}" ]; then
	echo "Couldn't find tmsim executable: '${TMSIM}'" 1>&2
	exit 1
fi

exitstatus=0

for test in *.csv; do
	tmsimfile="${test%%.csv}.tm"

	printf "\n"

	while read -r line; do
		input="$(echo "${line}" | cut -d ',' -f1)"
		output="$(echo "${line}" | cut -d ',' -f2)"

		echo "Testing '${test##*/}' with input '${input}':"
		result=$(${TMSIM} -r "${tmsimfile}" "${input}" | tr -d \$)

		if [ "${result}" = "${output}" ]; then
			printf "\tOK.\n"
		else
			exitstatus=1
			printf "\tFAIL: Expected '${output}', got '${result}'.\n"
		fi
	done < "${test}"
done

exit ${exitstatus}
