#!/system/bin/sh
rm log
touch log
mkdir /data/local/tmp 2> /dev/null
chown media /data/local/tmp
sleep 2
ps | grep mediaserver
pidof mediaserver
/data/local/tmp/hijack -p `pidof mediaserver` -l /data/local/tmp/libt.so -f /data/local/tmp/dump/ -d
