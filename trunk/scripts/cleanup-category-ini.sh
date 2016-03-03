#!/bin/bash
#
# Important notes:
#
# 1) This script *assumes* that 'qmc2-sdlmame' has been run prior to using it and that
#    its XML cache database is *up-to-date* and stored as ~/.qmc2/mame-xml-cache.db.
#
# 2) It *requires* the command 'sqlite3', so make sure it's available!
#

SQLITE3=sqlite3
XMLDB=~/.qmc2/mame-xml-cache.db
UNIX2DOS="unix2dos -q"
DOS2UNIX="dos2unix -q"
CP=cp
TMPCAT=/tmp/category.ini
TMPCATNEW=/tmp/category.ini.new
SRCCAT=data/cat/category.ini

function replaceCat {
	rm ${TMPCAT}
	mv ${TMPCATNEW} ${TMPCAT}
}

echo "Cleaning up category.ini, please wait..."
${CP} ${SRCCAT} ${TMPCAT}
${DOS2UNIX} ${TMPCAT}
sed 's/\s*$//g' ${TMPCAT} > ${TMPCATNEW}
replaceCat
emuVersion=$(${SQLITE3} ${XMLDB} "select emu_version from mame_xml_cache_metadata")
dateString="'Updated $(LC_TIME=C date "+%d-%b-%Y" | tr "[a-z]" "[A-Z]") (MAME ${emuVersion})'"
sed "1s/^.*$/${dateString}/g" ${TMPCAT} > ${TMPCATNEW}
replaceCat
for i in $(cat /tmp/category.ini | grep -v "^\\[" | grep -v "^tr\\[" | grep "^[0-9a-z]"); do
	id=$(${SQLITE3} ${XMLDB} "select id from mame_xml_cache where id='${i}'")
	if [ "${id}" != "${i}" ]; then
		grep -v "^${i}$" ${TMPCAT} > ${TMPCATNEW}
		replaceCat
		echo "Removed invalid machine '${i}'"
	else
		cloneCheck=$(${SQLITE3} ${XMLDB} "select xml from mame_xml_cache where id='${i}'" | grep "cloneof")
		if [ "${cloneCheck}" != "" ]; then
			grep -v "^${i}$" ${TMPCAT} > ${TMPCATNEW}
			replaceCat
			echo "Removed clone '${i}'"
		fi
	fi
done
${UNIX2DOS} ${TMPCAT}
${CP} ${TMPCAT} ${SRCCAT}
rm -f ${TMPCAT} ${TMPCATNEW}
echo "Done"
