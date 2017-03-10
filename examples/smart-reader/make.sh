export IP=192.168.1.3
#make && scp bin/smart-reader.hex root@$IP: && ssh root@$IP "./isp -b 200 -f smart-reader.hex && gpio -g mode 4 out"

#make && scp bin/smart-reader.hex root@$IP: && ssh root@$IP "fastboot/FBoot-Linux/src/bootloader -d /dev/ttyUSB0 -b 115200 -r -p ~/smart-reader.hex"
make && ~/smart-meter-reader/bootloader/FBoot-Linux/src/bootloader -d /dev/ttyUSB0 -b 115200 -r -p bin/smart-reader.hex

