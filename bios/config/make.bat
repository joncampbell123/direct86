@echo off
..\maketool\nasm16 -f bin config.asm -o config.com
cd ..
call make
cd config

