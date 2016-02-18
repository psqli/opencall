cwd="`pwd`"
xdir="`dirname $0`"

cd $xdir

common_files="network.c audio.c setsched.c"
common_libs="-lasound"

cc $common_libs -o server $common_files server.c
cc $common_libs -o client $common_files client.c
cc $common_libs -lpthread -o ptp $common_files ptp.c

cd $cwd
