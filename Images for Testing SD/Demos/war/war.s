
.z
	move.l	#$11,d0			; set sound command
	lea		.sound,a3		; load sound message payload pointer to a3
	trap 	#1				; call dosound

	; SIDE EFFECT: the sound call inits D4!

	move.w	#256-1,d7		; 256 v-lines
	lea		$20000,a0		; load screen address
.ol
	moveq	#32-1,d5		; 32 * 4 bytes per scanline
.il							; 
		move.l	d4,(a0)+	; write garbage
		roxr.l	d5,d4		; roxrl the garbage
	dbra	d5,.il			; loop for x
		dbra	d7,.ol		; loop for y

	addq.b	#3,12(a3)		; change sound parameter slightly
	jmp	.z					; reinit the sound
.sound
	dc.b	$a       		;0 	0 command, play sound
	dc.b	8	     		;1 	1 bytes to follow
	dc.l	$0000aaaa		;2 	2,3,4,5 byte parameters						omitted, because baked in the code
	dc.b	$00      		;6	6 pitch1                  
	dc.b	$df        		;7 7 pitch2
	
	dc.b	0				;8 interval between steps  
	dc.b	0				;9

	dc.b	0				;10 du	r
	dc.b	0				;11

	dc.b	$70				;12 step in pitch          
	dc.b	$99				;13 randomness
;	dc.b	1				;14 no return
