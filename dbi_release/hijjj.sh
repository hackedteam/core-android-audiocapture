#!/system/bin/sh
for i in 1 2; do
  n=0
  for a in `ps | grep mediaserver` then; 
  do
    if [ n -eq 1 ]
    then
      PID=$a
      break
    fi
    n=$((n+1))
  done

  if [ -z "$PID" ]
  then
    echo "no mediaserver found"
    exit 1
  else
    echo "mediaserver at \"$PID\""
    if [ $i -le 1 ]
    then
      echo "kill process before re-hijack"
      kill -9 $PID
      sleep 1
    else
      echo running /data/local/tmp/hijack -p $PID -l /data/local/tmp/libt.so -f /data/local/tmp/dump/ -d
      /data/local/tmp/hijack -p $PID -l /data/local/tmp/libt.so -f /data/local/tmp/dump/ -d
    fi
  fi
done
