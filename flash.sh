 make veryclean && make APPLICATION=philosophers_dinner
riscv64-linux-gnu-objcopy -O binary img/philosophers_dinner.img epos
./vf2-imager -i epos
./vf2 firmware.img
