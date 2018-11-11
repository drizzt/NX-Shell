#include <switch.h>

#include "common.h"
#include "config.h"
#include "SDL_helper.h"
#include "status_bar.h"
#include "textures.h"
#include "touch_helper.h"

#define STATUS_BAR_LIGHT_TSP  FC_MakeColor(37, 79, 174, 127)
#define STATUS_BAR_DARK_TSP   FC_MakeColor(38, 50, 56, 127)
#define MENU_BAR_LIGHT_TSP    FC_MakeColor(51, 103, 214, 127)
#define MENU_BAR_DARK_TSP     FC_MakeColor(55, 71, 79, 127)

static char *Text_LoadFile(char *path) {
	SDL_RWops *file = SDL_RWFromFile(path, "rb");

	if (file == NULL) 
		return NULL;

	Sint64 res_size = SDL_RWsize(file);
	char *res = (char *)malloc(res_size + 1);
	Sint64 nb_read_total = 0, nb_read = 1;
	char *buf = res;

	while ((nb_read_total < res_size) && (nb_read != 0)) {
		nb_read = SDL_RWread(file, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}

	SDL_RWclose(file);
	
	if (nb_read_total != res_size) {
		free(res);
		return NULL;
	}

	res[nb_read_total] = '\0';
	return res;
}

void Menu_DisplayText(char *path) {
	int text_start_x = 20, text_start_y = 145;
	int text_pos_x = text_start_x, text_pos_y = text_start_y;	
	char *text = Text_LoadFile(path);

	u32 title_height = 0;
	SDL_GetTextDimensions(30, path, NULL, &title_height);

	FC_Rect text_rect = SDL_GetTextRect(text_pos_x, text_pos_y, 30, WHITE, text); // fix for bigger files

	int text_x = text_rect.x, text_y = text_rect.y;
	int text_width = text_rect.w, text_height = text_rect.h;
	
	TouchInfo touchInfo;
	Touch_Init(&touchInfo);

	u32 line_width = 0, line_height = 0;
	SDL_GetTextDimensions(30, "a", &line_width, &line_height);

	int endY = ((-text_height) + text_start_y) + 570;
	int speed = line_height;

	float percent = 0;

	int textViewTop = 140;
	int textViewHeight = 580;

	int scrollbarWidth = 10;

	while(appletMainLoop()) {
		SDL_ClearScreen(config.dark_theme? BLACK_BG : WHITE);

		SDL_DrawText(text_pos_x, text_pos_y, 30, config.dark_theme? WHITE : BLACK, text);

		SDL_DrawRect(0, 0, 1280, 40, config.dark_theme? STATUS_BAR_DARK : STATUS_BAR_LIGHT);	// Status bar
		SDL_DrawRect(0, 40, 1280, 100, config.dark_theme? MENU_BAR_DARK : MENU_BAR_LIGHT);	// Menu bar

		StatusBar_DisplayTime();

		SDL_DrawImage(icon_back, 40, 66);
		SDL_DrawText(128, 40 + ((100 - title_height)/2), 30, WHITE, path);

		hidScanInput();
		Touch_Process(&touchInfo);
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
		u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);

		percent = ((float)(-(text_pos_y-text_start_y))/(float)(-(endY-text_start_y)));

		if (text_height > 570)
		{
			SDL_DrawRect(1270, textViewTop+(percent*(textViewHeight-scrollbarWidth)), scrollbarWidth, scrollbarWidth, config.dark_theme? SELECTOR_COLOUR_DARK : SELECTOR_COLOUR_LIGHT);

			if ((kHeld & KEY_Y))
				speed = line_height * 2;
			else
				speed = line_height;

			if ((kHeld & KEY_UP)) {
				svcSleepThread(10000000);
				text_pos_y += speed;
			}
			else if ((kHeld & KEY_DOWN)) {
				svcSleepThread(10000000);
				text_pos_y -= speed;
			}

			if ((kDown & KEY_LEFT) || (kDown & KEY_L)) {
				svcSleepThread(10000000);
				text_pos_y += speed * 16;
			}
			else if ((kDown & KEY_RIGHT) || (kDown & KEY_R)) {
				svcSleepThread(10000000);
				text_pos_y -= speed * 16;
			}

			if ((kDown & KEY_ZL)) {
				svcSleepThread(10000000);
				text_pos_y = text_start_y;
			}
			else if ((kDown & KEY_ZR)) {
				svcSleepThread(10000000);
				text_pos_y = endY;
			}

			if (text_pos_y > text_start_y)
				text_pos_y = text_start_y;
			if (text_pos_y < endY)
				text_pos_y = endY;
		}

		if (touchInfo.state == TouchEnded && touchInfo.tapType != TapNone) {
			if (tapped_inside(touchInfo, 40, 66, 108, 114))
				break;
		}

		SDL_Renderdisplay();
		
		if (kDown & KEY_B)
			break;
	}

	MENU_DEFAULT_STATE = MENU_STATE_HOME;
}
