#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

IPROUTE2_DIR="${DIR}/iproute2"
BIN=bin
TARGETS="ip"
[ -d "${IPROUTE2_DIR}" ] || git clone "git@github.com:jpilet/iproute2.git" "${IPROUTE2_DIR}"

git -C "${IPROUTE2_DIR}" checkout j1939-v3.6
git -C "${IPROUTE2_DIR}" pull
make -C "${IPROUTE2_DIR}" -j2 SUBDIRS="lib ip"

[ -d "${BIN}" ] || mkdir "${BIN}"

for i in $TARGETS; do
  cp "${IPROUTE2_DIR}/ip/$i" "${BIN}"
done
