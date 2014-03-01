#!/bin/sh
echo "Cleaning up category.ini, please wait..."
cp data/cat/category.ini /tmp/category.ini
dos2unix -q /tmp/category.ini
sed 's/\s*$//g' /tmp/category.ini > /tmp/category.ini.new
rm /tmp/category.ini
mv /tmp/category.ini.new /tmp/category.ini
dateString="'Updated $(date "+%B %Y")'"
sed "1s/^.*$/$dateString/g" /tmp/category.ini > /tmp/category.ini.new
rm /tmp/category.ini
mv /tmp/category.ini.new /tmp/category.ini
for i in $(cat /tmp/category.ini | grep -v "^\\[" | grep "^[0-9a-z]"); do
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
