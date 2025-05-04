#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# Override by envrionment variable for another target
export SSH_DEPLOY_TARGET=${SSH_DEPLOY_TARGET:-"sol:/data/alex/Public/Releases"}

# Navdatareader-linux-ubuntu-22.04-2.8.2.beta.tar.xz
if [ -f "/etc/os-release" ]; then
  source /etc/os-release
  export FILENAME_LNM=$ID-$VERSION_ID-$(head -n1 "${APROJECTS}/deploy/Navdatareader/version.txt")
else
  export FILENAME_LNM=$(head -n1 "${APROJECTS}/deploy/Navdatareader/version.txt")
fi

(
  cd ${APROJECTS}/deploy

  rm -rfv "Navdatareader-linux-${FILENAME_LNM}"
  rm -fv Navdatareader-linux-${FILENAME_LNM}.tar.gz Navdatareader-linux-${FILENAME_LNM}.tar.xz

  cp -av "Navdatareader" "Navdatareader-linux-${FILENAME_LNM}"

  tar cv "Navdatareader-linux-${FILENAME_LNM}" | xz -9 > Navdatareader-linux-${FILENAME_LNM}.tar.xz
)

scp ${APROJECTS}/deploy/Navdatareader-linux-${FILENAME_LNM}.tar.xz ${SSH_DEPLOY_TARGET}/


