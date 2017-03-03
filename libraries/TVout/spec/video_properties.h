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

/*
 This File contains the timing definitions for the TVout AVR composite video
 generation Library
*/
#ifndef VIDEO_TIMING_H
#define	VIDEO_TIMING_H

#define _CYCLES_PER_US			(F_CPU / 1000000)

// Timing settings for PAL
// Front porch (A):				1.65 µs		+0.4/-0.1
// Sync pulse length (B):		4.7 µs		+0.2/-0.2
// Back porch (C):				5.7 µs		+0.2/-0.2
// Active video (D):			51.95 µs	+0.4/-0.1
// Total horizontal sync time:	12.05 µs / 12.00 µs + 0.15 µs
// Total time for each line:	64.00 µs
//
// 312.5 - 287.5 = 25 lines (Vertical Blanking Width: Blank lines/field)
// Vertical lines:				312.5 / 312-313 (625 total)	??? 288 + 25 = 313
// Vertical lines visible:		287.5 / 288 (575 / 576 total)
// Vertical Frequency:			50 Hz = 25 * 2
// Vertical Period:				20 ms = 312.5 * 64 µs
// Vertical Equalizing Pulse Width: 2.35 µs	???
// Vertical sync polarity:		Negative (burst)
// Sync pulse length (F):		0.576 ms (burst)	???
// Active video (H):			18.4 ms

// Vertical "Equalizing pulses":
// Vert. “front porch” *b:		2.5 lines = 160 µs	???
// Vert sync. pulse width *b:	2.5 lines = 160 µs	???
// Vert. “back porch” *b,*c:	2.5 + 17.5 = 20 lines = 20 * 64 µs = 1.28 ms
// Vertical Blanking Width:		25 lines = 25 * 64 µs = 1.6 ms
// Total vertical sync time:	1.6 ms = 25 * 64 µs

// *b Due to the interlaced format used in both systems, the vertical blanking interval is somewhat more complex than this description would indicate. “Equalizing pulses,” of twice the normal rate of the horizontal synchronization pulses but roughly half the duration, replace the standard H. sync pulse during the vertical front porch, sync pulse, and the first few lines of the back porch.

// *c The “back porch” timings given here are separated into the time during which equalization pulses are produced (thepostequalization period) and the remaining “normal” line times.

// Analog Video Vertical Sync: https://www.youtube.com/watch?v=NY2rIjkH1Xw
// PAL video timing specification: http://martin.hinner.info/vga/pal.html
// NTSC format description: http://martin.hinner.info/vga/pal.gif

// Timing settings for PAL
#define _PAL_LINE_FULL_FRAME		625		// lines
#define _PAL_LINE_FRAME				(_PAL_LINE_FULL_FRAME / 2)		// lines

#define _PAL_TIME_RENDERING_LINE	46		// µs
#define _PAL_LINE_DISPLAY			260		// lines
#define _PAL_LINE_MID				((_PAL_LINE_FRAME - _PAL_LINE_DISPLAY) / 2 + _PAL_LINE_DISPLAY / 2) // lines

#define _PAL_TIME_SCANLINE			64		// µs
#define _PAL_TIME_HORZ_SYNC			4.7		// µs
#define _PAL_TIME_OUTPUT_START		12.5	// µs

#define _PAL_CYCLES_SCANLINE		((_PAL_TIME_SCANLINE * _CYCLES_PER_US) - 1)
#define _PAL_CYCLES_HORZ_SYNC		((_PAL_TIME_HORZ_SYNC * _CYCLES_PER_US) - 1)
#define _PAL_CYCLES_OUTPUT_START	((_PAL_TIME_OUTPUT_START * _CYCLES_PER_US) - 1)

#define _PAL_TIME_VSYNC_SCANLINE			(_PAL_TIME_SCANLINE / 2)	// µs
#define _PAL_TIME_VSYNC_PRE_EQUALIZING		2.3		// µs 2.4
#define _PAL_TIME_VSYNC_INVERTED_EQUALIZING	4.7		// µs

#define _PAL_CYCLES_VSYNC_SCANLINE			((_PAL_TIME_VSYNC_SCANLINE * _CYCLES_PER_US) - 1)
#define _PAL_CYCLES_VSYNC_PRE_EQUALIZING	((_PAL_TIME_VSYNC_PRE_EQUALIZING * _CYCLES_PER_US) - 1)
#define _PAL_CYCLES_VSYNC_EQUALIZING		(((_PAL_TIME_VSYNC_SCANLINE - _PAL_TIME_VSYNC_INVERTED_EQUALIZING) * _CYCLES_PER_US) - 1)

#define _PAL_LINE_NUMBER_TO_START_SECONDFRAME_VSYNC		310		// line# 311
#define _PAL_LINE_NUMBER_TO_START_SECONDFRAME_BLANKING	318		// line# 318

#define _PAL_LINE_NUMBER_TO_START_FIRSTFRAME_VSYNC		622		// line# 623
#define _PAL_LINE_NUMBER_TO_START_FIRSTFRAME_BLANKING	5		// line# 6

#endif