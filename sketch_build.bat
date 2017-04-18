REM In C:\Users\martin\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\platform.txt
REM replace "-g" with "-g3 -gdwarf-2"

call "C:\Program Files (x86)\Arduino\arduino_debug.exe" --verify --board arduino:sam:arduino_due_x "C:\Users\martin\Dropbox\Code\EEZ\psu-firmware\eez_psu_sketch\eez_psu_sketch.ino" --pref build.path="C:\Users\martin\Documents\Arduino\build\psu-firmware"
pause