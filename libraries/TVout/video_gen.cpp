/*
 Copyright (c) 2010 Myles Metzer

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

#include <avr/interrupt.h>
#include <avr/io.h>

#include "video_gen.h"
#include "spec/video_properties.h"
#include "spec/asm_macros.h"
#include "spec/hardware_setup.h"

//#define REMOVE6C
//#define REMOVE5C
//#define REMOVE4C
//#define REMOVE3C

int renderLine;
TVout_vid display;
void (*render_line)();			//remove me
void (*line_handler)();			//remove me

Callback frame_callback=0;

void render_setup(uint8_t mode, uint8_t x, uint8_t y, uint8_t *scrnptr) {
	
	display.screen = scrnptr;
	display.hres = x;
	display.vres = y;
	display.frames = 0;
	
	display.vscale_const = _PAL_LINE_DISPLAY / display.vres - 1;
	display.vscale = display.vscale_const;
	
	//selects the widest render method that fits in 46us
	//as of 9/16/10 rendermode 3 will not work for resolutions lower than
	//192(display.hres lower than 24)
	unsigned char rmethod = (_PAL_TIME_RENDERING_LINE * _CYCLES_PER_US) / (display.hres * 8); // re: 46 µs * 16 / (16 * 8) = 5.75 ticks per pixel
	switch(rmethod) {
		case 6:
			render_line = &render_line6c;
			break;
		case 5:
			render_line = &render_line5c;
			break;
		case 4:
			render_line = &render_line4c;
			break;
		case 3:
			render_line = &render_line3c;
			break;
		default:
			if (rmethod > 6)
				render_line = &render_line6c;
			else
				render_line = &render_line3c;
	}
	
	DDR_VID |= _BV(VID_PIN);
	DDR_SYNC |= _BV(SYNC_PIN);
	PORT_VID &= ~_BV(VID_PIN);
	PORT_SYNC |= _BV(SYNC_PIN);
	
	// inverted fast pwm mode on timer 1:
	TCCR1A = _BV(COM1A1) | _BV(COM1A0) | _BV(WGM11); 
	TCCR1B = _BV(WGM13) | _BV(WGM12) | _BV(CS10);
	
	display.output_delay = _PAL_CYCLES_OUTPUT_START;
	display.lines_frame = _PAL_LINE_FRAME;
	
	display.first_half_frame_start_render = _PAL_LINE_MID - (display.vres * (display.vscale_const + 1)) / 2;
	display.first_half_frame_end_render = display.first_half_frame_start_render + (display.vres * (display.vscale_const + 1));
	display.second_half_frame_start_render = display.lines_frame + display.first_half_frame_start_render;
	display.second_half_frame_end_render = display.lines_frame + display.first_half_frame_end_render;
	
	display.scanLine = 0;
	display.vsyncScanLine = 0;
	line_handler = &first_half_frame_vsync_lines;
	
	ICR1  = _PAL_CYCLES_VSYNC_SCANLINE;
	OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
	
	TIMSK1 = _BV(TOIE1);
	sei();
}

// render a line
ISR(TIMER1_OVF_vect) {
	line_handler();
}

void first_half_frame_vsync_lines() {
	
	display.vsyncScanLine++;
	
	if (display.vsyncScanLine == 5) {
		OCR1A = _PAL_CYCLES_VSYNC_EQUALIZING;
	}
	else if (display.vsyncScanLine == 10) {
		OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
	}
	else if (display.vsyncScanLine == 15) {
		ICR1  = _PAL_CYCLES_SCANLINE;
		OCR1A = _PAL_CYCLES_HORZ_SYNC;
		
		display.scanLine = _PAL_LINE_NUMBER_TO_START_FIRSTFRAME_BLANKING;
		line_handler = &first_half_frame_blank_line;
	}
}

void first_half_frame_blank_line() {
	
	display.scanLine++;
	
	if ( display.scanLine == display.first_half_frame_start_render) {
		renderLine = 0;
		display.vscale = display.vscale_const;
		line_handler = &first_half_frame_active_line;
	}
	else if (display.scanLine == _PAL_LINE_NUMBER_TO_START_SECONDFRAME_VSYNC) {
		ICR1  = _PAL_CYCLES_VSYNC_SCANLINE;
		OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
		
		line_handler = &second_half_frame_vsync_lines;
		display.vsyncScanLine = 0;
		display.frames++;
		if(frame_callback) frame_callback();
	}
}

void first_half_frame_active_line() {
	
	display.scanLine++;
	
	wait_until(display.output_delay);
	render_line();
	
	if (display.vscale == 0) {
		display.vscale = display.vscale_const;
		renderLine += display.hres;
	}
	else {
		display.vscale--;
	}
		
	if (display.scanLine ==  display.first_half_frame_end_render) {
		line_handler = &first_half_frame_blank_line;
	}
}

void second_half_frame_vsync_lines() {
	
	display.vsyncScanLine++;
	
	if (display.vsyncScanLine == 5) {
		OCR1A = _PAL_CYCLES_VSYNC_EQUALIZING;
	}
	else if (display.vsyncScanLine == 10) {
		OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
	}
	else if (display.vsyncScanLine == 14) {
		ICR1  = _PAL_CYCLES_SCANLINE;
		// OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
	}
	else if (display.vsyncScanLine == 15) {
		OCR1A = _PAL_CYCLES_HORZ_SYNC;
		
		display.scanLine = _PAL_LINE_NUMBER_TO_START_SECONDFRAME_BLANKING;
		line_handler = &second_half_frame_blank_line;
	}
}

void second_half_frame_blank_line() {
	
	display.scanLine++;
	
	if (display.scanLine == display.second_half_frame_start_render) {
		renderLine = 0;
		display.vscale = display.vscale_const;
		line_handler = &second_half_frame_active_line;
	}
	else if (display.scanLine == _PAL_LINE_NUMBER_TO_START_FIRSTFRAME_VSYNC) {
		ICR1  = _PAL_CYCLES_VSYNC_SCANLINE;
		OCR1A = _PAL_CYCLES_HORZ_SYNC;
		
		line_handler = &second_half_frame_half_blank_line;
	}
}

void second_half_frame_active_line() {
	
	display.scanLine++;
	
	wait_until(display.output_delay);
	render_line();
	
	if (display.vscale == 0) {
		display.vscale = display.vscale_const;
		renderLine += display.hres;
	}
	else {
		display.vscale--;
	}
		
	if (display.scanLine ==  display.second_half_frame_end_render) {
		line_handler = &second_half_frame_blank_line;
	}
}

void second_half_frame_half_blank_line() {
	
	ICR1  = _PAL_CYCLES_VSYNC_SCANLINE;
	OCR1A = _PAL_CYCLES_VSYNC_PRE_EQUALIZING;
	
	line_handler = &first_half_frame_vsync_lines;
	display.vsyncScanLine = 0;
	display.frames++;
	if(frame_callback) frame_callback();

}


static void inline wait_until(uint8_t time) {
	__asm__ __volatile__ (
			"subi	%[time], 10\n"
			"sub	%[time], %[tcnt1l]\n\t"
		"100:\n\t"
			"subi	%[time], 3\n\t"
			"brcc	100b\n\t"
			"subi	%[time], 0-3\n\t"
			"breq	101f\n\t"
			"dec	%[time]\n\t"
			"breq	102f\n\t"
			"rjmp	102f\n"
		"101:\n\t"
			"nop\n" 
		"102:\n"
		:
		: [time] "a" (time),
		[tcnt1l] "a" (TCNT1L)
	);
}

void render_line6c() {
	#ifndef REMOVE6C
	__asm__ __volatile__ (
		"ADD	r26,r28\n\t"
		"ADC	r27,r29\n\t"
		//save PORTB
		"svprt	%[port]\n\t"
		
		"rjmp	enter6\n"
	"loop6:\n\t"
		"bst	__tmp_reg__,0\n\t"			//8
		"o1bs	%[port]\n"
	"enter6:\n\t"
		"LD		__tmp_reg__,X+\n\t"			//1
		"delay1\n\t"
		"bst	__tmp_reg__,7\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//2
		"bst	__tmp_reg__,6\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//3
		"bst	__tmp_reg__,5\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//4
		"bst	__tmp_reg__,4\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//5
		"bst	__tmp_reg__,3\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//6
		"bst	__tmp_reg__,2\n\t"
		"o1bs	%[port]\n\t"
		"delay3\n\t"						//7
		"bst	__tmp_reg__,1\n\t"
		"o1bs	%[port]\n\t"
		"dec	%[hres]\n\t"
		"brne	loop6\n\t"					//go too loopsix
		"delay2\n\t"
		"bst	__tmp_reg__,0\n\t"			//8
		"o1bs	%[port]\n"
		
		"svprt	%[port]\n\t"
		BST_HWS
		"o1bs	%[port]\n\t"
		:
		: [port] "i" (_SFR_IO_ADDR(PORT_VID)),
		"x" (display.screen),
		"y" (renderLine),
		[hres] "d" (display.hres)
		: "r16" // try to remove this clobber later...
	);
	#endif
}

void render_line5c() {
	#ifndef REMOVE5C
	__asm__ __volatile__ (
		"ADD	r26,r28\n\t"
		"ADC	r27,r29\n\t"
		//save PORTB
		"svprt	%[port]\n\t"
		
		"rjmp	enter5\n"
	"loop5:\n\t"
		"bst	__tmp_reg__,0\n\t"			//8
		"o1bs	%[port]\n"
	"enter5:\n\t"
		"LD		__tmp_reg__,X+\n\t"			//1
		"bst	__tmp_reg__,7\n\t"
		"o1bs	%[port]\n\t"
		"delay2\n\t"						//2
		"bst	__tmp_reg__,6\n\t"
		"o1bs	%[port]\n\t"
		"delay2\n\t"						//3
		"bst	__tmp_reg__,5\n\t"
		"o1bs	%[port]\n\t"
		"delay2\n\t"						//4
		"bst	__tmp_reg__,4\n\t"
		"o1bs	%[port]\n\t"
		"delay2\n\t"						//5
		"bst	__tmp_reg__,3\n\t"
		"o1bs	%[port]\n\t"
		"delay2\n\t"						//6
		"bst	__tmp_reg__,2\n\t"
		"o1bs	%[port]\n\t"
		"delay1\n\t"						//7
		"dec	%[hres]\n\t"
		"bst	__tmp_reg__,1\n\t"
		"o1bs	%[port]\n\t"
		"brne	loop5\n\t"					//go too loop5
		"delay1\n\t"
		"bst	__tmp_reg__,0\n\t"			//8
		"o1bs	%[port]\n"
		
		"svprt	%[port]\n\t"
		BST_HWS
		"o1bs	%[port]\n\t"
		:
		: [port] "i" (_SFR_IO_ADDR(PORT_VID)),
		"x" (display.screen),
		"y" (renderLine),
		[hres] "d" (display.hres)
		: "r16" // try to remove this clobber later...
	);
	#endif
}

void render_line4c() {
	#ifndef REMOVE4C
	__asm__ __volatile__ (
		"ADD	r26,r28\n\t"
		"ADC	r27,r29\n\t"
		
		"rjmp	enter4\n"
	"loop4:\n\t"
		"lsl	__tmp_reg__\n\t"			//8
		"out	%[port],__tmp_reg__\n\t"
	"enter4:\n\t"
		"LD		__tmp_reg__,X+\n\t"			//1
		"delay1\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay2\n\t"						//2
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay2\n\t"						//3
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay2\n\t"						//4
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay2\n\t"						//5
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay2\n\t"						//6
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay1\n\t"						//7
		"lsl	__tmp_reg__\n\t"
		"dec	%[hres]\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"brne	loop4\n\t"					//go too loop4
		"delay1\n\t"						//8
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"
		"delay3\n\t"
		"cbi	%[port],7\n\t"
		:
		: [port] "i" (_SFR_IO_ADDR(PORT_VID)),
		"x" (display.screen),
		"y" (renderLine),
		[hres] "d" (display.hres)
		: "r16" // try to remove this clobber later...
	);
	#endif
}

// only 16mhz right now!!!
void render_line3c() {
	#ifndef REMOVE3C
	__asm__ __volatile__ (
	".macro byteshift\n\t"
		"LD		__tmp_reg__,X+\n\t"
		"out	%[port],__tmp_reg__\n\t"	//0
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//1
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//2
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//3
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//4
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//5
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//6
		"nop\n\t"
		"lsl	__tmp_reg__\n\t"
		"out	%[port],__tmp_reg__\n\t"	//7
	".endm\n\t"
	
		"ADD	r26,r28\n\t"
		"ADC	r27,r29\n\t"
		
		"cpi	%[hres],30\n\t"		//615
		"breq	skip0\n\t"
		"cpi	%[hres],29\n\t"
		"breq	jumpto1\n\t"
		"cpi	%[hres],28\n\t"
		"breq	jumpto2\n\t"
		"cpi	%[hres],27\n\t"
		"breq	jumpto3\n\t"
		"cpi	%[hres],26\n\t"
		"breq	jumpto4\n\t"
		"cpi	%[hres],25\n\t"
		"breq	jumpto5\n\t"
		"cpi	%[hres],24\n\t"
		"breq	jumpto6\n\t"
	"jumpto1:\n\t"
		"rjmp	skip1\n\t"
	"jumpto2:\n\t"
		"rjmp	skip2\n\t"
	"jumpto3:\n\t"
		"rjmp	skip3\n\t"
	"jumpto4:\n\t"
		"rjmp	skip4\n\t"
	"jumpto5:\n\t"
		"rjmp	skip5\n\t"
	"jumpto6:\n\t"
		"rjmp	skip6\n\t"
	"skip0:\n\t"
		"byteshift\n\t"	//1		\\643
	"skip1:\n\t"
		"byteshift\n\t"	//2
	"skip2:\n\t"
		"byteshift\n\t"	//3
	"skip3:\n\t"
		"byteshift\n\t"	//4
	"skip4:\n\t"
		"byteshift\n\t"	//5
	"skip5:\n\t"
		"byteshift\n\t"	//6
	"skip6:\n\t"
		"byteshift\n\t"	//7
		"byteshift\n\t"	//8
		"byteshift\n\t"	//9
		"byteshift\n\t"	//10
		"byteshift\n\t"	//11
		"byteshift\n\t"	//12
		"byteshift\n\t"	//13
		"byteshift\n\t"	//14
		"byteshift\n\t"	//15
		"byteshift\n\t"	//16
		"byteshift\n\t"	//17
		"byteshift\n\t"	//18
		"byteshift\n\t"	//19
		"byteshift\n\t"	//20
		"byteshift\n\t"	//21
		"byteshift\n\t"	//22
		"byteshift\n\t"	//23
		"byteshift\n\t"	//24
		"byteshift\n\t"	//25
		"byteshift\n\t"	//26
		"byteshift\n\t"	//27
		"byteshift\n\t"	//28
		"byteshift\n\t"	//29
		"byteshift\n\t"	//30
		
		"delay2\n\t"
		"cbi	%[port],7\n\t"
		:
		: [port] "i" (_SFR_IO_ADDR(PORT_VID)),
		"x" (display.screen),
		"y" (renderLine),
		[hres] "d" (display.hres)
		: "r16" // try to remove this clobber later...
	);
	#endif
}
