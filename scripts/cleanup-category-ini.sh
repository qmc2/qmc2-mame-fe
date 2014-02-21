#!/bin/sh
echo "Cleaning up category.ini, please wait..."
cp data/cat/category.ini /tmp/category.ini
dos2unix /tmp/category.ini 2&>1 >/dev/null
sed 's/\s*$//g' /tmp/category.ini > /tmp/category.ini.new
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
unix2dos /tmp/category.ini 2&>1 >/dev/null
cp /tmp/category.ini data/cat/category.ini
rm /tmp/category.ini
echo "Done"
