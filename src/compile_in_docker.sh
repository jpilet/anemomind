#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

# The -i (interactive) option allows user to hit CTRL-C to stop the build.
"${DIR}/docker_run.sh" -i ../src/compile_and_test.sh
