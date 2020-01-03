cd /home/cklj/reps/flyos/src

nasm boot.asm -o  ./run/boot.bin 
cd ./run
dd if=boot.bin of=flyos.img conv=notrunc
bochs -q -f flyos.bochsrc -rc flyos.debug
