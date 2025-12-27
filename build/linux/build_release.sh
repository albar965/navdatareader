#!/bin/bash

# Echo all commands and exit on failure
set -e
set -x

# Error checking for required variable APROJECTS
if [ -z "$APROJECTS" ] ; then echo APROJECTS environment variable not set ; exit 1 ; fi
if [ ! -d "$APROJECTS" ]; then echo "$APROJECTS" does not exist ; exit 1 ; fi

# =============================================================================
# Set the required environment variable APROJECTS to the base directory for
# atools and navdatareader.

# =============================================================================
# Configuration can be overloaded on the command line by setting the
# variables below before calling this script.
#
# See the *.pro project files for more information.
#
# Example:
# export QMAKE_STATIC=~/Projekte/build-qt-5.12.11-release/bin/qmake
# export MARBLE_LIB_PATH=~/Programme/Marble-debug/lib
# export MARBLE_INC_PATH=~/Programme/Marble-debug/include

export CONF_TYPE=${CONF_TYPE:-"release"}
export ATOOLS_INC_PATH=${ATOOLS_INC_PATH:-"${APROJECTS}/atools/src"}
export ATOOLS_LIB_PATH=${ATOOLS_LIB_PATH:-"${APROJECTS}/build-atools-${CONF_TYPE}"}
export ATOOLS_NO_CRASHHANDLER=${ATOOLS_NO_CRASHHANDLER:-"true"}

# Defines the used Qt for all builds
export QMAKE_SHARED=${QMAKE_SHARED:-"${HOME}/Qt/$QT_VERSION/gcc_64/bin/qmake"}

# Do not change the DEPLOY_BASE since some scripts depend on it
export DEPLOY_BASE="${APROJECTS}/deploy"

export ATOOLS_NO_GRIB=true
export ATOOLS_NO_GUI=true
export ATOOLS_NO_NAVSERVER=true
export ATOOLS_NO_ROUTING=true
export ATOOLS_NO_TRACK=true
export ATOOLS_NO_USERDATA=true
export ATOOLS_NO_WEATHER=true
export ATOOLS_NO_WEB=true

# ===========================================================================
# ========================== navdatareader - shared Qt
# ===========================================================================

# ===========================================================================
# ========================== atools
rm -rf ${APROJECTS}/build-atools-${CONF_TYPE}
mkdir -p ${APROJECTS}/build-atools-${CONF_TYPE}
cd ${APROJECTS}/build-atools-${CONF_TYPE}

${QMAKE_SHARED} ${APROJECTS}/atools/atools.pro -spec linux-g++ CONFIG+=${CONF_TYPE}
make -j4

# ===========================================================================
# ========================== navdatareader
rm -rf ${APROJECTS}/build-navdatareader-${CONF_TYPE}
mkdir -p ${APROJECTS}/build-navdatareader-${CONF_TYPE}
cd ${APROJECTS}/build-navdatareader-${CONF_TYPE}

${QMAKE_SHARED} ${APROJECTS}/navdatareader/navdatareader.pro -spec linux-g++ CONFIG+=${CONF_TYPE}
make -j4

make copydata
make deploy
