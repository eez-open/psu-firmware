REM In C:\Users\mvladic\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\platform.txt
REM replace "-g" with "-g3 -gdwarf-2"

call "C:\Program Files (x86)\Arduino\arduino_debug.exe" --verify --board arduino:sam:arduino_due_x "C:\Users\mvladic\Dropbox\Code\EEZ\psu-firmware\eez_psu_sketch\eez_psu_sketch.ino" --pref build.path="C:\Users\mvladic\Documents\Arduino\build\psu-firmware"
pause