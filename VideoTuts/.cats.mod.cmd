cmd_/home/vboxuser/Desktop/OSlab/VideoTuts/cats.mod := printf '%s\n'   cats.o | awk '!x[$$0]++ { print("/home/vboxuser/Desktop/OSlab/VideoTuts/"$$0) }' > /home/vboxuser/Desktop/OSlab/VideoTuts/cats.mod
