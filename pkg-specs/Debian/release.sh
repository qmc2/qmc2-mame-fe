#!/bin/bash

IFS='
'
if [ $# -ne 5 ]
then
  echo "usage: $0 (ubuntu_distro_name|debian) (stable|unstable) major_version minor_version package_revision"
else
  if [ x$1 = 'xdebian' ] && [ x$2 != 'xstable' ]
  then
    echo "unsupported combination: debian + unstable"
    exit 0
  fi
  GIT_REV=`git describe --always`
  UBUNTU_VERSION_NAME=$1
  MAJOR_VERSION=$3
  MINOR_VERSION=$4
  PACKAGE_REVISION=$5
  CURRENT_DATE=`date -R`
  
  if [ x$2 = 'xstable' ]
  then
    if [ x$1 = 'xdebian' ]
    then
      PPA_VERSION=
    else
      PPA_VERSION=+ppa1~${UBUNTU_VERSION_NAME}
    fi
    QMC2_VERSION=${MAJOR_VERSION}.${MINOR_VERSION}${PPA_VERSION}
    PACKAGE_VERSION=${QMC2_VERSION}-${PACKAGE_REVISION}
  else
  PPA_VERSION=+ppa2~${UBUNTU_VERSION_NAME}+${MAJOR_VERSION}.${MINOR_VERSION}~git${GIT_REV}
  QMC2_VERSION=${MAJOR_VERSION}.$(( $MINOR_VERSION - 1 ))${PPA_VERSION}
  PACKAGE_VERSION=${QMC2_VERSION}-${PACKAGE_REVISION}
  fi

  
  
  # go to repository root
  # currently only works when we run from trunk. need better way to detect
  cd ../../../
  if [ ! -d tags ]
  then
    cd ..
  fi
  mkdir qmc2_${PACKAGE_VERSION}

  if [ x$2 = 'xstable' ]
  then
    cp -a tags/${MAJOR_VERSION}.${MINOR_VERSION}/* qmc2_${PACKAGE_VERSION}/
  else
    cp -a trunk/* qmc2_${PACKAGE_VERSION}/
  fi
  
    
  cd qmc2_${PACKAGE_VERSION}
  #remove the .svn files
  for i in `find . -name .svn`
  do
    rm -rf $i
  done
  cd ..

   #create orig file
  mkdir qmc2_${QMC2_VERSION}.orig
  cp -a qmc2_${PACKAGE_VERSION}/* qmc2_${QMC2_VERSION}.orig/
  tar cvjf qmc2_${QMC2_VERSION}.orig.tar.bz2 qmc2_${QMC2_VERSION}.orig
  rm -rf qmc2_${QMC2_VERSION}.orig

  #copy content on pkg-spec/Debian folder to debian folder inside release dir
  mkdir qmc2_${PACKAGE_VERSION}/debian
  cp -a qmc2_${PACKAGE_VERSION}/pkg-specs/Debian/* qmc2_${PACKAGE_VERSION}/debian/
  rm qmc2_${PACKAGE_VERSION}/debian/*.sh

  #add the changelog only if not debian (since debian is already in changelog
  if [ x$1 != 'xdebian' ]
  then
  sed -i "1 i\\
qmc2 (${PACKAGE_VERSION}) ${UBUNTU_VERSION_NAME}; urgency=low\\
\\
  * ${UBUNTU_VERSION_NAME} port\\
\\
\\
 -- Marcelo Marzola Bossoni <mmbossoni@gmail.com>  ${CURRENT_DATE}\\
\\
" qmc2_${PACKAGE_VERSION}/debian/changelog
  fi

  #build the package
  cd qmc2_${PACKAGE_VERSION}
  debuild -S -sa
  cd ..

  if [ x$2 = 'xstable' ]
  then
    if [ x$1 = 'xdebian' ]
    then
      dput mentors qmc2_${PACKAGE_VERSION}_source.changes
    else
      dput ppa qmc2_${PACKAGE_VERSION}_source.changes
    fi
  else
    dput unstable-ppa qmc2_${PACKAGE_VERSION}_source.changes
  fi
fi
