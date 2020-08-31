This folder should only contain build results, and thus no source-code.
The files in this folder should result from the 'build_app_install'.
A technology file (.cal) shall not be added, this file shall be at "<projectroot>/digital/release/tapeout/romxxxx.cal".
 
eg.

- <firmware>.elf
- <firmware>.lss
- <firmware>.map
- <firmware>.sym
- <firmware>.hex
- <firmware>.txt


In order to release the software for rom-mask back-end data generation,
following files need to be compressed to a <firmware_final_release_tag>.zip archive and send to:
hotline@xfab.com

- <firmware>.txt  --
- ROM5120X16.cal  -- > <firmware_final_release_tag>.zip
