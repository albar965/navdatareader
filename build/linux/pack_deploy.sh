#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

export TARGET_NAME=Navdatareader

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# Override by envrionment variable for another target
export SSH_DEPLOY_TARGET=${SSH_DEPLOY_TARGET:-"sol:/data/alex/Public/Releases"}

if [ -f "/etc/lsb-release" ]; then
  source /etc/lsb-release
  export FILENAME=$DISTRIB_RELEASE-$(head -n1 ${APROJECTS}/deploy/"$TARGET_NAME"/version.txt)
else
  export FILENAME=$(head -n1 ${APROJECTS}/deploy/"$TARGET_NAME"/version.txt)
fi

export FILENAME=$DISTRIB_RELEASE-$(head -n1 ${APROJECTS}/deploy/"$TARGET_NAME"/version.txt)

(
  cd ${APROJECTS}/deploy
  tar cfvz "$TARGET_NAME.tar.gz" "$TARGET_NAME"
)

scp "${APROJECTS}/deploy/$TARGET_NAME.tar.gz" "${SSH_DEPLOY_TARGET}/$TARGET_NAME-linux-${FILENAME}.tar.gz"


