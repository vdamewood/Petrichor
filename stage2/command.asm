; command.asm: Command interpreter
;
; Copyright 2015, 2016 Vincent Damewood
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
;
; 1. Redistributions of source code must retain the above copyright
; notice, this list of conditions and the following disclaimer.
;
; 2. Redistributions in binary form must reproduce the above copyright
; notice, this list of conditions and the following disclaimer in the
; documentation and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
; LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
; A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
; HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
; SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
; LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
; DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
; THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
; OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

%include "functions.inc"

command_buffer_size  equ 32
command_buffer       times command_buffer_size db 0

%define buffer command_buffer
%define end    (command_buffer+command_buffer_size-1)

command_get:
	fprolog 0, edi
.start:
	call vidtxt_show_cursor
	mov edi, command_buffer

.loop:
	call keyboard_get_stroke

.check_special:
	cmp ah, 0x00
	jne .special

.printable:
	cmp edi, end ; if buffer is full
	je .loop ; ignore keypress

	push eax
	call vidtxt_putch
	pop eax ; smaller than add esp, 4
	call vidtxt_show_cursor
	stosb
	jmp .loop

.special:
	cmp ah, 0x01 ; Ignore ctrl-, alt- and errors.
	jne .loop

.check_esc:
	cmp al, 0x00 ; Escape
	jne .not_esc
.do_esc:
	cmp edi, buffer
	je .loop

	call vidtxt_delch
	call vidtxt_show_cursor
	dec edi
	jmp .do_esc
.not_esc:

.check_bksp:
	cmp al, 0x10 ; Backspace
	jne .not_bksp
.do_bksp:
	cmp edi, buffer ; if at the beginning of the buffer
	je .loop   ; ignore

	call vidtxt_delch
	call vidtxt_show_cursor
	dec edi
	jmp .loop
.not_bksp:

.check_enter:
	cmp al, 0x12 ; Enter
	jne .not_enter
.do_enter:
	mov al, 0
	stosb
	call vidtxt_breakline
	jmp .done
.not_enter:

.else:
	jmp .loop ; Ignore all other keystrokes

.done:
	mov eax, buffer
	freturn edi
