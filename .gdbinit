add-symbol-file ~/fieldkit/bootloaders/build/m0/zero/fk-bootloaders-samd21large.elf 0x0000
target extended-remote :4331
monitor reset
continue
