Simple MLX4 aplication which waits in the endless loop

From Nico Bollen (NIO):
It is a work around for a problem which exists in 81300/81310 chips (when MLX4 is in reset state MLX16 will not wake up after halt command),
of course it could be useful on other chips if these chips are having the same problem.

The idea was just to have some known code running in the MLX4 so that we could use the halt command on the MLX16.
As Marcel and me are already using this code for some while and it is working fine I don't think the code will be changed soon...
