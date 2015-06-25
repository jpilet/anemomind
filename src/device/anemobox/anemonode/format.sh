
#!/bin/bash

set -e

if mount | grep /dev/mmcblk1p1 | grep -q vfat; then
  echo Formatting sdcard
  umount /media/sdcard
  mke2fs -j -F /dev/mmcblk1p1 
  mount /media/sdcard
  mkdir /media/sdcard/logs
  echo "DONE"
fi

