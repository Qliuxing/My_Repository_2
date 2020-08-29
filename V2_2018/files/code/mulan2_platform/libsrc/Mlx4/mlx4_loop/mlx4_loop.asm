Mlx4di

; ---------------------------------------------------------------------------
;
; Description:
;  Endless Loop firmware for MLX4
;
;
; Copyright (c) Melexis Digital Competence Center
;
; ---------------------------------------------------------------------------

				words

;--------------- linker informations --------------
				segment word at 0-FFF 'all'	;The total memory map consist of:
				contains 'reserved'		;- reserved locations (at 0..3)
				contains 'task0vects'	;- vectors of task 0 (at 4..7)
				contains 'task1vects'	;- vectors of task 1 (at 8..0Bh)
				contains 'code'			;- normal code (no placement requirements)

;--------------- reserved memory locations and vector table

				words					;ROM addresses are 16 bits wide

				segment word at 0 'reserved'	;reserved memory locations in ROM

				; 5AA5h is a signature of valid program in extrnal memory (RAM)
				; Image with such signature can be put into both RAM and ROM
				dc.w 5AA5h				;ROM address 0 -- signature of valid image (MLX16 reads this as 0xA55A word)
				dc.w 0A55Ah				;ROM address 1
				dc.w 0					;ROM address 2 is reserved
				dc.w 0					;ROM address 3 is reserved

				segment word at 4 'task0vects'	;vector table of task 0
				jmp sleep				;power-on reset vector of task 0
				jmp sleep				;task reset vector of task 0
				jmp sleep				;external interrupt vector of task 0
				jmp sleep				;attention interrupt vector of task 0

				segment word at 8 'task1vects'	;vector table of task 1
				dc.w 0
				dc.w 0
				dc.w 0
				dc.w 0

;--------------- ROM tables -----------------------
				segment word at 0C 'code'	;normal code (no calls into this code)

sleep			nop
				jmp sleep

;--------------- program end ------------------------------------------------------------------------------------------
prog_end		end
