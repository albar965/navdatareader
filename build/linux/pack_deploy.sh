#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# Override by envrionment variable for another target
export SSH_DEPLOY_TARGET=${SSH_DEPLOY_TARGET:-"darkon:/data/alex/Public/Releases"}

export FILENAME=`date "+20%y%m%d-%H%M"`

(
cd ${APROJECTS}/deploy

tar cfvz Navdatareader.tar.gz "Navdatareader"
)

scp ${APROJECTS}/deploy/Navdatareader.tar.gz ${SSH_DEPLOY_TARGET}/Navdatareader-linux-${FILENAME}.tar.gz


