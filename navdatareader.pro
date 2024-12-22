#*****************************************************************************
# Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#****************************************************************************

# =============================================================================
# Set these environment variables for configuration - do not change this .pro file
# =============================================================================
#
# ATOOLS_INC_PATH
# Optional. Path to atools include. Default is "../atools/src" if not set.
# Also reads *.qm translation files from "$ATOOLS_INC_PATH/..".
#
# ATOOLS_LIB_PATH
# Optional. Path to atools static library. Default is "../build-atools-$${CONF_TYPE}"
# ("../build-atools-$${CONF_TYPE}/$${CONF_TYPE}" on Windows) if not set.
#
# ATOOLS_GIT_PATH
# Optional. Path to GIT executable. Revision will be set to "UNKNOWN" if not set.
# Uses "git" on macOS and Linux as default if not set.
# Example: "C:\Git\bin\git"
#
# DEPLOY_BASE
# Optional. Target folder for "make deploy". Default is "../deploy" plus project name ($$TARGET_NAME).
#
# ATOOLS_QUIET
# Optional. Set this to "true" to avoid qmake messages.
#
# =============================================================================
# End of configuration documentation
# =============================================================================

# Define program version here VERSION_NUMBER_TODO
VERSION_NUMBER=1.2.2.beta1

QT += sql core

CONFIG += build_all c++14 console
CONFIG -= app_bundle gui debug_and_release debug_and_release_target

TARGET = navdatareader
TEMPLATE = app

TARGET_NAME=Navdatareader

# =======================================================================
# Copy environment variables into qmake variables

ATOOLS_INC_PATH=$$(ATOOLS_INC_PATH)
ATOOLS_LIB_PATH=$$(ATOOLS_LIB_PATH)
ATOOLS_NO_CRASHHANDLER=$$(ATOOLS_NO_CRASHHANDLER)

GIT_PATH=$$(ATOOLS_GIT_PATH)
DEPLOY_BASE=$$(DEPLOY_BASE)
QUIET=$$(ATOOLS_QUIET)


# =======================================================================
# Fill defaults for unset

CONFIG(debug, debug|release) : CONF_TYPE=debug
CONFIG(release, debug|release) : CONF_TYPE=release

isEmpty(DEPLOY_BASE) : DEPLOY_BASE=$$PWD/../deploy

isEmpty(ATOOLS_INC_PATH) : ATOOLS_INC_PATH=$$PWD/../atools/src
isEmpty(ATOOLS_LIB_PATH) : ATOOLS_LIB_PATH=$$PWD/../build-atools-$$CONF_TYPE

# =======================================================================
# Set compiler flags and paths

unix:!macx {
  isEmpty(GIT_PATH) : GIT_PATH=git

  QMAKE_LFLAGS += -no-pie

  # Makes the shell script and setting LD_LIBRARY_PATH redundant
  QMAKE_RPATHDIR=.
  QMAKE_RPATHDIR+=./lib
}

win32 {
  WINDEPLOY_FLAGS = --compiler-runtime
  CONFIG(debug, debug|release) : WINDEPLOY_FLAGS += --debug
#  CONFIG(release, debug|release) : WINDEPLOY_FLAGS += --release
}

macx {
  isEmpty(GIT_PATH) : GIT_PATH=git

  # Compatibility down to OS X Sierra 10.12 inclusive
  QMAKE_MACOSX_DEPLOYMENT_TARGET = 10.12
}

isEmpty(GIT_PATH) {
  GIT_REVISION=UNKNOWN
  GIT_REVISION_FULL=UNKNOWN
} else {
  GIT_REVISION=$$system('$$GIT_PATH' rev-parse --short HEAD)
  GIT_REVISION_FULL=$$system('$$GIT_PATH' rev-parse HEAD)
}

LIBS += -L$$ATOOLS_LIB_PATH -latools

# Cpptrace ==========================
!isEqual(ATOOLS_NO_CRASHHANDLER, "true") {
  DEFINES += CPPTRACE_STATIC_DEFINE
  win32 : LIBS += -L$$PWD/../cpptrace-$$CONF_TYPE-win64/lib -lcpptrace -ldbghelp -ldwarf -lz -lzstd
  unix:!macx : LIBS += -L$$PWD/../cpptrace-$$CONF_TYPE/lib -lcpptrace -ldwarf -lz -lzstd
  CONFIG += force_debug_info
} else {
  DEFINES += DISABLE_CRASHHANDLER
}

PRE_TARGETDEPS += $$ATOOLS_LIB_PATH/libatools.a
DEPENDPATH += $$ATOOLS_INC_PATH
INCLUDEPATH += $$PWD/src $$ATOOLS_INC_PATH
DEFINES += VERSION_NUMBER_NAVDATAREADER='\\"$$VERSION_NUMBER\\"'
DEFINES += GIT_REVISION_NAVDATAREADER='\\"$$GIT_REVISION\\"'
DEFINES += QT_NO_CAST_FROM_BYTEARRAY
DEFINES += QT_NO_CAST_TO_ASCII

# =======================================================================
# Include build_options.pro with additional variables

exists($$PWD/../build_options.pro) {
   include($$PWD/../build_options.pro)

   !isEqual(QUIET, "true") {
     message($$PWD/../build_options.pro found.)
   }
} else {
   !isEqual(QUIET, "true") {
     message($$PWD/../build_options.pro not found.)
   }
}

# =======================================================================
# Print values when running qmake

!isEqual(QUIET, "true") {
message(-----------------------------------)
message(VERSION_NUMBER: $$VERSION_NUMBER)
message(GIT_REVISION: $$GIT_REVISION)
message(GIT_REVISION_FULL: $$GIT_REVISION_FULL)
message(GIT_PATH: $$GIT_PATH)
message(ATOOLS_INC_PATH: $$ATOOLS_INC_PATH)
message(ATOOLS_LIB_PATH: $$ATOOLS_LIB_PATH)
message(ATOOLS_NO_CRASHHANDLER: $$ATOOLS_NO_CRASHHANDLER)
message(DEPLOY_BASE: $$DEPLOY_BASE)
message(DEFINES: $$DEFINES)
message(INCLUDEPATH: $$INCLUDEPATH)
message(LIBS: $$LIBS)
message(TARGET_NAME: $$TARGET_NAME)
message(QT_INSTALL_PREFIX: $$[QT_INSTALL_PREFIX])
message(QT_INSTALL_LIBS: $$[QT_INSTALL_LIBS])
message(QT_INSTALL_PLUGINS: $$[QT_INSTALL_PLUGINS])
message(QT_INSTALL_TRANSLATIONS: $$[QT_INSTALL_TRANSLATIONS])
message(QT_INSTALL_BINS: $$[QT_INSTALL_BINS])
message(CONFIG: $$CONFIG)
message(QT: $$QT)
message(OUT_PWD: $$OUT_PWD)
message(PWD: $$PWD)
message(-----------------------------------)
}

# =====================================================================
# Files

SOURCES += \
    main.cpp \
    navdatareader.cpp

HEADERS += \
    navdatareader.h

OTHER_FILES += \
  $$files(build/*, true) \
  $$files(help/*, true) \
  $$files(config/*, true) \
  .gitignore \
  BUILD.txt \
  CHANGELOG.txt \
  LICENSE.txt \
  README.txt \
  uncrustify.cfg

RESOURCES += \
    navdatareader.qrc


# Linux - Copy help and Marble plugins and data
unix:!macx {
  copydata.commands += cp -avfu $$PWD/help $$OUT_PWD
}

# Linux specific deploy target
unix:!macx {
  DEPLOY_DIR=\"$$DEPLOY_BASE/$$TARGET_NAME\"
  DEPLOY_DIR_LIB=\"$$DEPLOY_BASE/$$TARGET_NAME/lib\"

  deploy.commands = rm -Rfv $$DEPLOY_DIR &&
  deploy.commands += mkdir -pv $$DEPLOY_DIR_LIB &&
  deploy.commands += mkdir -pv $$DEPLOY_DIR/sqldrivers &&
  deploy.commands += echo $$VERSION_NUMBER > $$DEPLOY_DIR/version.txt &&
  deploy.commands += echo $$GIT_REVISION_FULL > $$DEPLOY_DIR/revision.txt &&
  deploy.commands += cp -Rvf $$OUT_PWD/$${TARGET} $$DEPLOY_DIR &&
  deploy.commands += cp -Rvf $$OUT_PWD/help $$DEPLOY_DIR &&
  deploy.commands += cp -Rvf $$PWD/resources/config $$DEPLOY_DIR/config &&
  deploy.commands += cp -vf $$PWD/CHANGELOG.txt $$DEPLOY_DIR &&
  deploy.commands += cp -vf $$PWD/README.txt $$DEPLOY_DIR &&
  deploy.commands += cp -vf $$PWD/LICENSE.txt $$DEPLOY_DIR &&
  deploy.commands += cp -vfa $$[QT_INSTALL_PLUGINS]/sqldrivers/libqsqlite.so*  $$DEPLOY_DIR/sqldrivers &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicudata.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicui18n.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libicuuc.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Gui.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Core.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += cp -vfa $$[QT_INSTALL_LIBS]/libQt5Sql.so*  $$DEPLOY_DIR_LIB &&
  deploy.commands += rm -fv $$DEPLOY_DIR_LIB/lib*.so.*.debug $$DEPLOY_DIR_LIB/*/lib*.so.*.debug
}

# Windows specific deploy target only for release builds
win32 {
  defineReplace(p){return ($$shell_quote($$shell_path($$1)))}

  deploy.commands = rmdir /S /Q $$p($$DEPLOY_BASE/$$TARGET_NAME) &
  deploy.commands += mkdir $$p($$DEPLOY_BASE/$$TARGET_NAME/sqldrivers) &&
  deploy.commands += echo $$VERSION_NUMBER > $$p($$DEPLOY_BASE/$$TARGET_NAME/version.txt) &&
  deploy.commands += echo $$GIT_REVISION_FULL > $$p($$DEPLOY_BASE/$$TARGET_NAME/revision.txt) &&
  deploy.commands += xcopy /F $$p($$OUT_PWD/$${TARGET}.exe) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  !isEqual(ATOOLS_NO_CRASHHANDLER, "true") {
    isEqual(CONF_TYPE, "release") {
      deploy.commands += xcopy /F $$p($$OUT_PWD/$${TARGET}.debug) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
    }
  }
  deploy.commands += xcopy /F $$p($$PWD/CHANGELOG.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /F $$p($$PWD/README.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /F $$p($$PWD/LICENSE.txt) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /F $$p($$[QT_INSTALL_BINS]/libgcc*.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /F $$p($$[QT_INSTALL_BINS]/libstdc*.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /F $$p($$[QT_INSTALL_BINS]/libwinpthread*.dll) $$p($$DEPLOY_BASE/$$TARGET_NAME) &&
  deploy.commands += xcopy /I /S /E /F /Y $$p($$PWD/help) $$p($$DEPLOY_BASE/$$TARGET_NAME/help) &&
  deploy.commands += xcopy /I /S /E /F /Y $$p($$PWD/resources/config) $$p($$DEPLOY_BASE/$$TARGET_NAME/config) &&
  deploy.commands += $$p($$[QT_INSTALL_BINS]/windeployqt) $$WINDEPLOY_FLAGS $$p($$DEPLOY_BASE/$$TARGET_NAME)
}

# =====================================================================
# Additional targets

# Need to copy data when compiling
all.depends = copydata

# Deploy needs compiling before
deploy.depends = all

QMAKE_EXTRA_TARGETS += deploy copydata all

!isEqual(ATOOLS_NO_CRASHHANDLER, "true") {
  isEqual(CONF_TYPE, "release") {
    # Copy debug executable and strip original
    win32 {
      defineReplace(p){return ($$shell_quote($$shell_path($$1)))}
      QMAKE_POST_LINK = copy $$p($$OUT_PWD/$${TARGET}.exe) $$p($$OUT_PWD/$${TARGET}.debug) && \
                        strip $$p($$OUT_PWD/$${TARGET}.exe)
    } else {
      QMAKE_POST_LINK = cp -avfu $$OUT_PWD/$${TARGET} $$OUT_PWD/$${TARGET}.debug && strip $$OUT_PWD/$${TARGET}
    }
  }
}
