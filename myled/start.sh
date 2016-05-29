insmod rfbb.ko
mknod /dev/rfbb c 251 0
chown root:dialout /dev/rfbb
chmod g+rw /dev/rfbb
