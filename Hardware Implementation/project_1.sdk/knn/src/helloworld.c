/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "distip.h"
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <xbasic_types.h>
#include "platform.h"
#include "xparameters.h"  // Zynq parameters
#include "xgpiops.h"      // Zynq GPIO operations
#include <math.h>
#include <time.h>
#include <xscutimer.h>
#include <xscugic.h>

#define _POSIX_C_SOURCE >= 199309L

//using the TTC (Triple Timer Counter


Xuint32 *baseaddr_p = (Xuint32 *)XPAR_DISTIP_0_S00_AXI_BASEADDR;
//Xuint32 *baseaddr_q = (Xuint32 *)(0x40 + 0x43C00000) ;
Xuint32 *baseaddr_q = (Xuint32 *) XPAR_DISTIP_0_S00_AXI_BASEADDR + 0x40;

int main ()
{
	init_platform();
	//timer initialization
	static XScuTimer substitute_tick_timer;
	XScuTimer_Config* pTimerConfig = XScuTimer_LookupConfig(XPAR_XSCUTIMER_0_DEVICE_ID);

	if (pTimerConfig)
	{
		XScuTimer_CfgInitialize(&substitute_tick_timer, pTimerConfig, pTimerConfig->BaseAddr);
		XScuTimer_SelfTest(&substitute_tick_timer); // hmmm

		//load the timer
		XScuTimer_LoadTimer(&substitute_tick_timer, 0);
		XScuTimer_EnableAutoReload(&substitute_tick_timer);

		// Lookup GPIO config table
		//XGpioPs_Config * ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);


		Xint32 desc11[64] = {-2436,-630,5569,2226,1678,-752,4092,2436,1331,124,
			 	 	 	 	1561,570,-586,-58,825,215,-830,-1543,2513,9406,
			 	 	 	 	151,-3609,7066,12525,4842,-4749,6019,8570,-499,-1983,
			 	 	 	 	1061,2958,-6292,5220,6813,7426,254,6681,3459,10250,
			 	 	 	 	1584,4294,3099,8613,-6,1404,510,3624,-4916,187,
			 	 	 	 	5570,929,185,240,1585,932,289,-45,1093,618,
			 	 	 	 	-357,173,642,338};

		Xint32 desc22[64] = {-29,-188,554,406,214,13,966,489,181,-130,
			 	 	 	 	373,357,-44,-83,60,192,-104,3410,1308,4313,
			 	 	 	 	-645,5928,2373,6054,819,6799,2162,6913,1293,5138,
			 	 	 	 	2165,7366,101,81,1863,9739,-1683,-1365,3282,15269,
			 	 	 	 	-800,-3562,2848,16236,-941,-3679,2386,9224,-280,-2046,
			 	 	 	 	668,2717,-1060,-2583,1192,2954,-339,-1729,953,1912,
			 	 	 	 	140,-301,374,418};

		int i=0;
		XScuTimer_Start(&substitute_tick_timer);
		*(baseaddr_p+129) = 0x40000000;
		for (i=0; i<64; i++)
			 {
				*(baseaddr_p+i) = desc11[i];
				*(baseaddr_q+i) = desc22[i];
				xil_printf("Read : %d \n \r", *(baseaddr_p+i));
			 }
		*(baseaddr_p+129) = 0x80000000;

		XScuTimer_Stop(&substitute_tick_timer);
		xil_printf("Hardware count = %d\n",XScuTimer_GetCounterValue(substitute_tick_timer));

		// software knn
		XScuTimer_LoadTimer(&substitute_tick_timer, 0);
		XScuTimer_EnableAutoReload(&substitute_tick_timer);
		XScuTimer_Start(&substitute_tick_timer);

		int t;
		int32_t buf = 0;
		for (i=0; i<64; i++)
			 {
				t = desc11[i] - desc22[i];
				buf += t*t;
			 }
		buf = sqrt(buf);
		XScuTimer_Stop(&substitute_tick_timer);
		xil_printf("Software count = %d\n",XScuTimer_GetCounterValue(substitute_tick_timer));
		xil_printf("\n buf: %ld\n", buf);
		xil_printf("Read (Distance): %d \n \r", *(baseaddr_p+128));

	}
	 return 0;
}
// #include "ff.h"		// Declarations of FatFs API

/* FATFS FatFs;		// FatFs work area needed for each volume
FRESULT fr;			// FatFs function common result code
FIL fsrc;			// File object needed for source file
FIL fdst;			// File object needed for destination file
XGpioPs Gpio;

int main (void)
{
	UINT br, bw;		// File read/write count
	int retval;			// Return value for Gpio functions
	BYTE line[4096]; 		// Line buffer
	char lines[2048]; //line buffer for fgets
	int i;

	init_platform();

	// Lookup GPIO config table
	XGpioPs_Config * ConfigPtr = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);

	// Initialize GPIO
    if ( (retval = XGpioPs_CfgInitialize(&Gpio, ConfigPtr, ConfigPtr->BaseAddr)) != XST_SUCCESS) {
   		printf("Error initalizing GPIO fails\n"); return retval;}
	if ( (retval = XGpioPs_SelfTest(&Gpio)) != XST_SUCCESS) {
		printf("GPIO Self test fails\n"); return 1; }


	 //  Write files to SDC using FatFs SPI protocol


	f_mount(0,&FatFs);		// Give a work area to the default drive

	// Open or create a file to write
		if (f_open(&fdst, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
		{
			f_write(&fdst, "It works!\r\n", 11, &bw);	// Write data to the file
			f_close(&fdst);								// Close the file
		}

	// Check result
		if(bw == 11) // how many bytes written?
					printf("File written successfully\n");
				else
					printf("File write error\n");

	// Open a file to read and open or create a file to write what was read
	if ((f_open(&fsrc, "testfile.txt", FA_READ | FA_OPEN_EXISTING ) == FR_OK) &
			(f_open(&fdst, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS) == FR_OK))
	{
		// while (f_gets(lines, sizeof lines, &fsrc))
		  //      printf(lines);
		for (;;)
		{
			fr = f_read(&fsrc, line, sizeof line, &br); // Read a chunk of source file

			if (fr || br == 0)
				break; 									// error or EOF
			fr = f_write(&fdst, line, br, &bw);         // Write it to the destination file
			if (fr || bw < br)
				break; 									// error or disk full
		}

		for (i=0; i<sizeof line; i++)
			lines[i] = (char)line[i];
		printf (lines);

		// Close the files
		f_close(&fsrc);
		f_close(&fdst);
	}

	// Check result
	if(bw >= br)
		printf("File written successfully\n");
	else
		printf("File write error\n");

	exit(1);

}*/
