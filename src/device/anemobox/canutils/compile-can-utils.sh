#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

CAN_DIR="${DIR}/can-utils"
BIN=bin
TARGETS="cansend candump jspy jacd jsr"
[ -d "${CAN_DIR}" ] || git clone "git@github.com:jpilet/can-utils.git" "${CAN_DIR}"

git -C "${CAN_DIR}" checkout j1939-v6
git -C "${CAN_DIR}" pull
make -C "${CAN_DIR}" -j2 ${TARGETS}

[ -d "${BIN}" ] || mkdir "${BIN}"

for i in $TARGETS; do
  cp "${CAN_DIR}/$i" "${BIN}"
done
