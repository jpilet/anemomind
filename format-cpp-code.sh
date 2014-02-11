#!/bin/bash

set -e

# REQUIREMENT: Assuming the 'astyle' version 2.04 executable is on the PATH
#   You can check this by typing
#       astyle --version
#     (astyle version 2.03 doesn't
#      seem to contain the --style=google option)
#

# Make sure the current directory is the script parent directory.
cd $(dirname "$0")

# make sure no git changes are pending
if git diff --shortstat | grep -q "changed"; then
  echo "Please commit your changes before formatting."
  exit
fi

astyle --style=google --indent=spaces=2 --recursive --suffix=none \
    src/*.cpp src/*.h

