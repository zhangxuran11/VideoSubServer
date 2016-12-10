#!/bin/sh
#ping 127.0.0.1
ping $1 -w 2 -c 2  2>$1 1>/dev/null
if [ $? = 0 ];then
	echo yes > /tmp/isARC
else
	echo no	> /tmp/isARC
fi
