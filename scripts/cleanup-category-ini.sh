#!/bin/bash
#
# Important notes:
#
# 1) This script *assumes* that 'qmc2-sdlmess' has been run prior to using it and that
#    its XML cache database is *up-to-date* and stored as ~/.qmc2/mess-xml-cache.db.
#
# 2) It *requires* the command 'sqlite3', so make sure it's available!
#
echo "Cleaning up category.ini, please wait..."
cp data/cat/category.ini /tmp/category.ini
dos2unix -q /tmp/category.ini
sed 's/\s*$//g' /tmp/category.ini > /tmp/category.ini.new
rm /tmp/category.ini
mv /tmp/category.ini.new /tmp/category.ini
emuVersion=$(sqlite3 ~/.qmc2/mess-xml-cache.db "select emu_version from mess_xml_cache_metadata")
dateString="'Updated $(date "+%d-%b-%Y" | tr "[a-z]" "[A-Z]") (MESS $emuVersion)'"
sed "1s/^.*$/$dateString/g" /tmp/category.ini > /tmp/category.ini.new
rm /tmp/category.ini
mv /tmp/category.ini.new /tmp/category.ini
for i in $(cat /tmp/category.ini | grep -v "^\\[" | grep -v "^tr\\[" | grep "^[0-9a-z]"); do
	j=$(sqlite3 ~/.qmc2/mess-xml-cache.db "select id from mess_xml_cache where id='$i'")
	if [ "$j" != "$i" ]; then
		grep -v "^${i}$" /tmp/category.ini > /tmp/category.ini.new
		rm /tmp/category.ini
		mv /tmp/category.ini.new /tmp/category.ini
		echo "Removed invalid machine name '$i'"
	fi
done
unix2dos -q /tmp/category.ini
cp /tmp/category.ini data/cat/category.ini
rm /tmp/category.ini
echo "Done"
