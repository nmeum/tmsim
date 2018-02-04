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
		status="$(echo "${line}" | cut -d ',' -f2)"

		echo "Testing '${test##*/}' with input '${input}':"
		${TMSIM} "${tmsimfile}" "${input}"

		ret=$?
		if [ ${ret} -eq ${status} ]; then
			printf "\tOK.\n"
		else
			exitstatus=1
			printf "\tFAIL: Expected '${status}', got '${ret}'.\n"
		fi
	done < "${test}"
done

exit ${exitstatus}
