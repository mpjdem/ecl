#include "stdafx.h"
#include <windows.h>

#include "ecl.h"
#include "ecl_daq_ni.h"
#include "ecl_response_keyboard.h"
#include "ecl_response_pport.h"
#include "ecl_region_circle.h"
#include "ecl_monitor_dx.h"
#include "ecl_config.h"
#include "ecl_process.h"
#include "ecl_output_pfile.h"


int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	
	// Declaration of ECL objects
	ecl_daq_ni 				daq;
	ecl_output_pfile 		pfile;
	ecl_monitor_dx 			subj_mon;
	ecl_monitor_dx 			ctrl_mon;
	ecl_response_keyboard	keyb;
	ecl_response_pport 		breakers(0x378);
	ecl_config 				cfg; 
	ecl_process 			DPI;

	ecl_region_circle		regf(250,300,20);		// Fixation region
	ecl_region_circle		regs1(550,300,80);		// Stimulus region 1
	ecl_region_circle		regs2(550,310,80);		// Stimulus region 2

	
	// Initialization and setup of auxiliary ECL objects
	daq.init(4);

	subj_mon.init(hInstance, "SUBJECT SCREEN", 2, 800, 600, 200, 24, 400, 300, 1000);
	subj_mon.set_bg_color(RGB(128,128,128));
	subj_mon.set_fg_color(RGB(200,200,200));
	ctrl_mon.init(hInstance, "CONTROL SCREEN", 1, 800, 600, 85, 24, 200, 150, 500);
	
	keyb.init();
	breakers.init();
	breakers.add_button("BRK_LEFT",0);				// On pin 0
	breakers.add_button("BRK_RIGHT",1);				// On pin 1

	pfile.set_fname("mydata");					
	pfile.create_buffer(5000);						// Allocate a 5 second buffer

	cfg.load("ecl_default_cfg.txt");


	// Initialization and setup of the main ecl_process object
	DPI.init(cfg, &daq, &keyb, &breakers, &subj_mon, &ctrl_mon);
	DPI.add_region(&regf, "REG_FIX");
	DPI.add_region(&regs1, "REG_STIM_1");
	DPI.add_region(&regs2, "REG_STIM_2");
	DPI.ctrl_disable_region("REG_STIM_2");			// Do not draw this region yet
	DPI.add_stim_flag(2);							// Stim flags for the output file
	DPI.add_key_flag(&breakers, "BRK_LEFT");		// Keys flags for the output file
	DPI.add_key_flag(&breakers, "BRK_RIGHT");


	// Execute the calibration routine
	DPI.calibrate();


	// Preload both stimuli (circle and square)
	long s1_h = subj_mon.create_preloaded_image(80, 80);
	long s2_h = subj_mon.create_preloaded_image(80, 80);
	subj_mon.draw_oval(s1_h,40,40,15,15,-1,4,-1);
	subj_mon.draw_rect(s2_h,40,40,15,15,-1,4,-1);


	// Draw the fixation point without preloading
	subj_mon.draw_oval(0,250,300,3,3);
	subj_mon.present();


	// Start sampling
	DPI.start_sampling();


	// Apply drift correction when a button is pressed
	bool done = false;
	do
	{
		DPI.sample();							// Collect and process a sample
		DPI.ctrl_update_screen();				// Update the control screen in video memory
		ctrl_mon.present();						// Show the updated control screen

		if(breakers.is_pressed("BRK_RIGHT") || breakers.is_pressed("BRK_LEFT"))
			if(DPI.drift_correction(250,300))
				done = true;
		
		if(keyb.is_pressed("KB_ESCAPE"))		// Allow recalibration
			DPI.calibrate();

	} while (!done);


	// Start saving the data from now on
	DPI.attach_output_file(pfile);
	

	// Main loop
	long start_time = DPI.get_sampling_time();
	long resp = 0, s2_time = 0, stim_flag_on = 0;

	done = false;
	while(!done)
	{
		DPI.sample();					
		DPI.ctrl_update_screen();
		ctrl_mon.present();
		
		// Fixation period of 500 ms + minimum 150 ms saccadic reaction time
		if (DPI.get_sampling_time() < start_time + 500 + 150)
		{
			if (!DPI.on_region("REG_FIX"))
			{			
				DPI.stop_sampling();
				DPI.add_to_msgbuf("ERROR: Participant did not keep fixation");
				done = true;
			}

			if (DPI.get_sampling_time() > start_time + 500 && !stim_flag_on)
			{
				subj_mon.blit_preloaded_image(s1_h, 550, 300);
				subj_mon.present();
				DPI.stim_flag_on(1);
				stim_flag_on = 1;
			}
		}

		// Then require a saccade, and switch the stimulus when it occurs
		if (DPI.get_sampling_time() >= start_time + 500 + 150 && stim_flag_on == 1)
		{
			if (DPI.is_in_saccade())
			{
				subj_mon.blit_preloaded_image(s2_h, 550, 305);
				subj_mon.present();
				s2_time = DPI.get_sampling_time();
				DPI.stim_flag_off(1); DPI.stim_flag_on(2);
				DPI.ctrl_disable_region("REG_STIM_1");
				DPI.ctrl_enable_region("REG_STIM_2");
				stim_flag_on = 2;
			}
		}

		// Abort if no saccade was made
		if (DPI.get_sampling_time() >= start_time + 500 + 400 && stim_flag_on == 1)
		{	
			DPI.stop_sampling();
			DPI.add_to_msgbuf("ERROR: Participant did not saccade in time");
			done = true;
		}

		// Show the second stimulus for 100 ms
		if (DPI.get_sampling_time() > s2_time + 100 && stim_flag_on == 2)
		{
			DPI.stop_sampling();
			subj_mon.clear_screen();
			subj_mon.present();
			DPI.stim_flag_off(2);

			// Collect response
			if (breakers.is_pressed("BRK_LEFT"))
				resp = 1; done = true;

			if (breakers.is_pressed("BRK_RIGHT"))
				resp = 2; done = true;
		}
	}

	DPI.stop_sampling();


	// If a valid response was obtained, save the data
	if (resp)
		DPI.save_output_file();
	DPI.detach_output_file();


	// Print the result to the control screen
	char msg[50]; sprintf(msg, "Response given: %d", resp);
	DPI.add_to_msgbuf(msg);


	// Refresh the control monitor one last time
	DPI.ctrl_update_screen();
	ctrl_mon.wait_for_present();
	ctrl_mon.present();

	return 0;
}