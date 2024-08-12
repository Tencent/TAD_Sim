#!/bin/bash

BASEDIR=$(dirname "$0")

echo "=== executing upgrade-db ... ==="
${BASEDIR}/upgrade-db $1 $2

# LD_LIBRARY_PATH=${BASEDIR}/../simdeps ${BASEDIR}/updator-scenario $1 $2

echo "=== executing upgrade_pblog.sh ... ==="
bash ${BASEDIR}/upgrade_pblog.sh $1 $2

echo "=== executing upgrade_mil.sh ... ==="
bash ${BASEDIR}/upgrade_mil.sh $1 $2

echo "=== executing upgrade_sensor ... ==="
${BASEDIR}/upgrade_sensor $1 $2

echo "=== executing upgrade_grading_report ... ==="
${BASEDIR}/upgrade_grading_report $1 $2
