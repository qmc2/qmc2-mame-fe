#!/bin/bash
# WARNING: This is just a "make things easier" script. It is not intended to 
# overwrite any general script. 
# 
# @param: version of release (f.i. 0.2.b20)
# @param: 0 for trunk, 1 for stable
# 
# This script creates a debian package from the source folder

IFS='
'
if [ $# -ne 2 ]
then
  echo "use: $0 release_version src_line
  where src_line is
    0 for trunk
    1 for stable tags"
else
  RELEASE_FOLDER=qmc2-$1
  #entering basis (have trunk and tags folder)
  cd ../../../
  if [ $2 -eq 0 ]
  then
    mkdir ${RELEASE_FOLDER}
    cp -a trunk/* ${RELEASE_FOLDER}/
  else
    #return 1 more dir
    cd ../
    if [ -e tags/$1 ]
    then
      mkdir ${RELEASE_FOLDER}
      cp -a tags/$1/* ${RELEASE_FOLDER}/
    else
      echo "Source folder does not exist. Exiting..."
      exit 1
    fi
  fi
  
  cd ${RELEASE_FOLDER}
  #remove the .svn files
  for i in `find . -name .svn`
  do
    rm -rf $i
  done
  cd ..

   #create orig file
  mkdir ${RELEASE_FOLDER}.orig
  cp -a ${RELEASE_FOLDER}/* ${RELEASE_FOLDER}.orig/
  tar cvjf qmc2_$1.orig.tar.bz2 ${RELEASE_FOLDER}.orig

  #copy content on pkg-spec/Debian folder to debian folder inside release dir
  mkdir ${RELEASE_FOLDER}/debian
  cp -a ${RELEASE_FOLDER}/pkg-specs/Debian/* ${RELEASE_FOLDER}/debian/
  rm ${RELEASE_FOLDER}/debian/do_debian_pkg.sh

fi
