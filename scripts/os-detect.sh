#!/bin/sh
# Detect OS name. If it is Linux, detect the distribution as well.

OS="$(uname -s)"
UNAME="$(uname -a)"
DIST="$(uname -r)"

if [ "${OS}" = "Linux" ] ; then
  if [ -f /etc/os-release ] ; then
    VERSION_ID=$(cat /etc/os-release | egrep "\bVERSION_ID\b" | sed 's/VERSION_ID=//g' | sed 's/\"//g')
    VERSION=$(cat /etc/os-release | egrep "\bVERSION\b" | sed 's/VERSION=//g' | sed 's/\"//g')
    NAME=$(cat /etc/os-release | egrep "\bNAME\b" | sed 's/NAME=//g' | sed 's/\"//g' | tr ' ' '_' | tr '/' '_')
    if [ "${NAME}" = "Fedora" ] ; then
    	NAME="Fedora_release"
    	DIST="$(echo ${NAME})_$(echo ${VERSION_ID})"
    elif [ "${NAME}" = "Ubuntu" ] ; then
    	DIST="$(echo ${NAME})_$(echo ${VERSION_ID})"
    elif [ "${NAME}" = "openSUSE" ] ; then
        if [ "${VERSION}" != "" ] ; then
            DIST="$(echo ${NAME})_$(echo ${VERSION})"
        else
            DIST="$(echo ${NAME})_$(echo ${VERSION_ID})"
        fi
    else
    	DIST="$(echo ${NAME})_$(echo ${VERSION_ID})"
    fi
  elif [ -f /etc/mandriva-release ] ; then
    DIST="$(cat /etc/mandriva-release | sed 's/\ for .*//' | sed 's/\ (.*)//')"
  elif [ -f /etc/redhat-release ] ; then
    DIST="$(cat /etc/redhat-release | sed 's/\ (.*)//')"
  elif [ -f /etc/SuSE-release ] ; then
    DIST="$(cat /etc/SuSE-release | tr '\n' ' ' | sed 's/\ VERSION.*//' | sed 's/\ (.*)//')"
  elif [ -f /etc/debian_version ] ; then
    DIST="$(echo Debian) $(cat /etc/debian_version | sed 's/\//-/')"
  fi
fi

OSCFG="arch/$(echo ${OS} | tr '/' '_').cfg"
OSCFG=$(echo ${OSCFG} | tr " " '_')
if [ -f ${OSCFG} ] ; then
  OSCFG="System cfg-file (ok) ........ `echo ${OSCFG}`"
else
  OSCFG="System cfg-file (?) ......... `echo ${OSCFG}`"
fi
DISTCFG="arch/${OS}/$DIST.cfg"
DISTCFG="`echo ${DISTCFG} | tr " " '_'`"
if [ -f ${DISTCFG} ] ; then
  DISTCFG="Distribution cfg-file (ok) .. `echo ${DISTCFG}`"
else
  DISTCFG="Distribution cfg-file (?) ... `echo ${DISTCFG}`"
fi

echo "Operating System ............ ${OS}"
echo "Distribution / OS version ... ${DIST}"
echo "System information .......... ${UNAME}"
echo "${OSCFG}"
echo "${DISTCFG}"
