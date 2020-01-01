cd /home/cklj/reps/flyos/src

nasm boot.asm -o  ./run/boot.bin 
cd ./run
dd if=boot.bin of=a.img conv=notrunc
bochs -q -rc debug
