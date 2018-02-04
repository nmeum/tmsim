#!/bin/sh
set -e

(cd decidable-sets ; ./run_tests.sh)
(cd recursive-functions ; ./run_tests.sh)
