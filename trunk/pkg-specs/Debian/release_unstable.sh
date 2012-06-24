#!/bin/bash

IFS='
'
if [ $# -ne 1 ]
then
  echo "usage: $0 ubuntu_version_name"
else
  SVN_REV=`svnversion 2>&1 | sed -r -e "s/[MS]//g" -e "s/^[[:digit:]]*://"`
  MAJOR_VERSION=0
  MINOR_VERSION=36
  UBUNTU_VERSION_NAME=$1
  PACKAGE_REVISION=1
  CURRENT_DATE=`date -R`
  
  PPA_VERSION=+ppa2~${UBUNTU_VERSION_NAME}${PACKAGE_REVISION}+${MAJOR_VERSION}.$(( $MINOR_VERSION + 1 ))~svn${SVN_REV}
  QMC2_VERSION=${MAJOR_VERSION}.${MINOR_VERSION}${PPA_VERSION}
  
  # go to repository root
  cd ../../../
  mkdir qmc2_${QMC2_VERSION}
  cp -a trunk/* qmc2_${QMC2_VERSION}/
    
  cd qmc2_${QMC2_VERSION}
  #remove the .svn files
  for i in `find . -name .svn`
  do
    rm -rf $i
  done
  cd ..

   #create orig file
  mkdir qmc2_${QMC2_VERSION}.orig
  cp -a qmc2_${QMC2_VERSION}/* qmc2_${QMC2_VERSION}.orig/
  tar cvjf qmc2_${QMC2_VERSION}.orig.tar.bz2 qmc2_${QMC2_VERSION}.orig
  rm -rf qmc2_${QMC2_VERSION}.orig

  #copy content on pkg-spec/Debian folder to debian folder inside release dir
  mkdir qmc2_${QMC2_VERSION}/debian
  cp -a qmc2_${QMC2_VERSION}/pkg-specs/Debian/* qmc2_${QMC2_VERSION}/debian/
  rm qmc2_${QMC2_VERSION}/debian/*.sh

  #add the changelog
  sed -i "1 i\\
qmc2 (${QMC2_VERSION}) ${UBUNTU_VERSION_NAME}; urgency=low\\
\\
  * ${UBUNTU_VERSION_NAME} port\\
\\
\\
 -- Marcelo Marzola Bossoni <mmbossoni@gmail.com>  ${CURRENT_DATE}\\
\\
" qmc2_${QMC2_VERSION}/debian/changelog

  #build the package
  cd qmc2_${QMC2_VERSION}
  debuild -S -sa
  cd ..
  dput unstable-ppa qmc2_${QMC2_VERSION}_source.changes

fi