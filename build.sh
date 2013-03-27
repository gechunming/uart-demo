/opt/arm-2009q3/bin/arm-none-linux-gnueabi-gcc -o uarttest uarttest.c -static
gzip -d test.img.gz
sudo mount test.img tmp -o loop

sudo cp -rf uarttest tmp/
sync
sudo umount tmp
gzip -9 test.img
