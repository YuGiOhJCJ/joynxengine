
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "config.h" // for CONFIG_CURRENT_DIR
#include "settings.h"
#include "replay.h"
#include "settings.fdh"

char *setfilename;
const uint16_t SETTINGS_VERSION = 0x1602;		// serves as both a version and magic

Settings normal_settings;
Settings replay_settings;
Settings *settings = &normal_settings;

void settings_setfilename()
{
	// set the setfilename variable
#ifndef CONFIG_CURRENT_DIR
	const char *setfilename_1 = getenv("HOME");
	const char *setfilename_2 = "/.joynxengine/settings.dat";
	setfilename = (char*) malloc((strlen(setfilename_1) + strlen(setfilename_2) + 1) * sizeof(char));
	sprintf(setfilename, "%s%s", setfilename_1, setfilename_2);
#else
	const char *setfilename_1 = "settings.dat";
	setfilename = (char*) malloc((strlen(setfilename_1) + 1) * sizeof(char));
	sprintf(setfilename, "%s", setfilename_1);
#endif
}

bool settings_load(Settings *setfile)
{
	if (!setfile) setfile = &normal_settings;
	
	if (tryload(settings))
	{
		stat("No saved settings; using defaults.");
		
		memset(setfile, 0, sizeof(Settings));
		setfile->resolution = 2;		// 640x480 Windowed, should be safe value
		setfile->last_save_slot = 0;
		setfile->multisave = true;
		
		setfile->enable_debug_keys = false;
		setfile->sound_enabled = true;
		setfile->music_enabled = 1;	// both Boss and Regular music
		setfile->volume = SDL_MIX_MAXVOLUME;
		
		setfile->instant_quit = false;
		setfile->emulate_bugs = false;
		setfile->no_quake_in_hell = false;
		setfile->inhibit_fullscreen = false;
		setfile->files_extracted = false;
		
		// I found that 8bpp->32bpp blits are actually noticably faster
		// than 32bpp->32bpp blits on several systems I tested. Not sure why
		// but calling SDL_DisplayFormat seems to actually be slowing things
		// down. This goes against established wisdom so if you want it back on,
		// run "displayformat 1" in the console and restart.
		setfile->displayformat = false;
		
		return 1;
	}
	else
	{
		#ifdef __SDLSHIM__
			stat("settings_load(): Hey FIXME!!!");
			settings->show_fps = true;
		#else
			input_set_mappings(settings->input_mappings);
			input_set_button_mappings(settings->input_button_mappings);
		#endif
	}
	
	return 0;
}

/*
void c------------------------------() {}
*/

static bool tryload(Settings *setfile)
{
FILE *fp;

	stat("Loading settings...");
	
	settings_setfilename();
	fp = fileopen(setfilename, "rb");
	if (!fp)
	{
		stat("Couldn't open file %s.", setfilename);
		free(setfilename);
		return 1;
	}
	free(setfilename);
	
	setfile->version = 0;
	fread(setfile, sizeof(Settings), 1, fp);
	if (setfile->version != SETTINGS_VERSION)
	{
		stat("Wrong settings version %04x.", setfile->version);
		return 1;
	}
	
	fclose(fp);
	return 0;
}


bool settings_save(Settings *setfile)
{
FILE *fp;

	if (!setfile)
		setfile = &normal_settings;
	
	stat("Writing settings...");
	settings_setfilename();
	fp = fileopen(setfilename, "wb");
	if (!fp)
	{
		stat("Couldn't open file %s.", setfilename);
		free(setfilename);
		return 1;
	}
	free(setfilename);
	
	for(int i=0;i<INPUT_COUNT;i++)
		setfile->input_mappings[i] = input_get_mapping(i);
	for(int i=0;i<BUTTON_COUNT;i++)
		setfile->input_button_mappings[i] = input_get_button_mapping(i);
	
	setfile->version = SETTINGS_VERSION;
	fwrite(setfile, sizeof(Settings), 1, fp);
	
	fclose(fp);
	return 0;
}




