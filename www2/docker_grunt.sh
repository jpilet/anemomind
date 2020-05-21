#!/bin/bash
set -e

ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." >/dev/null 2>&1 && pwd )"

"${ROOT}/run.sh" dev run --rm anemowebapp ./grunt.sh ${*-test:server}
