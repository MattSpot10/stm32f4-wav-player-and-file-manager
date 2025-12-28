#include "main.h"
#include "fatfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ILI9481.h"
#include "TouchScreen.h"
#include "wav.h"




void File_Manager_App();
int scan_files (char * path);
int moveBack(char *dir);
const char *get_filename_ext(const char *filename);
int getFileState(const char *filename);
char * filterAstrid(const char *filename);
void musicPlayer(char * filename, int index);
void imageViewer(char * filename);
void fileManager(char * dir, int offset, int reload);
int file_Manager_menu(char * dir, int trigger);
int confirmationDelete(const char * dir, int f_state, const char * option1, const char * option2);

void copy_file(const char * dest, const char * src);
FRESULT rm_dir(char * dir);
FRESULT delete_node (
    TCHAR* path,    /* Path name buffer with the sub-directory to delete */
    UINT sz_buff,   /* Size of path name buffer (items) */
    FILINFO* fno    /* Name read buffer */
);

int keyBoard(char * output_text, const char * input_text, const char * option1, const char * option2, int arr_size, int reload);


//char files[128][32] = {};
char **files;
const int _max_files = 128;
const int _FileNameLength = 32;



void File_Manager_App() {
	//malloc space for files
	files = (char **)malloc(_max_files * sizeof(char *));
  for (int i = 0; i < _max_files; i++) {
  	files[i] = (char *)malloc(_FileNameLength * sizeof(char));
  }

	char dir[256] = "";
	fileManager(dir, 0, true);
	//free space for files
	free(files);
}

int scan_files (char * path) {
    FRESULT res;
    DIR dir;
    int i = 0;
    static FILINFO fno;
    SDState = BUSY;
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
            	//LOG(fno.fname);

            	sprintf(files[i], "*%s", fno.fname);

            	i++;
            }
            else {                                       /* It is a file. */
            	strcpy(files[i], fno.fname);
            	i++;
            }

            if (i >= 128) {
            	break;
            }
        }
        f_closedir(&dir);
    }
    SDState = IDLE;
    return i;
}
int moveBack(char *dir) {
	//char out_dir[256];
	int length = strlen(dir);
	if (length == 0) {
		return 0;
	}
	//int j = 0;
	for (int i = length; i >= 0; i--) {
		if (dir[i] == '/') {
			memmove(dir, dir, i);

			dir[i] = '\0';
			break;
		}
	}
	return 1;
	//*dir = out_dir;
}
void getName(char *name, const char *dir) {
	int length = strlen(dir);
	//int j = 0;
	for (int i = length; i >= 0; i--) {
		if (dir[i] == '/') {
			strcpy(name, dir + (i + 1));
			return;
		}
	}
	strcpy(name, dir);
	//*dir = out_dir;
}
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

#define FOLDER 	0
#define IMAGE 	1
#define AUDIO 	2
#define TEXT 		3
//#define EXEC 		4
#define ERROR 	5

int getFileState(const char *filename) {
	if (filename[0] == '*') {
		return FOLDER;
	}
  const char *ext = get_filename_ext(filename);
  if (strcmp(ext, "bimg") == 0 || strcmp(ext, "bin") == 0) {
  	return IMAGE;
  }
  if (strcmp(ext, "wav") == 0) {
  	return AUDIO;
  }
  if ((strcmp(ext, "txt") == 0) || (strcmp(ext, "c") == 0) || (strcmp(ext, "h") == 0)) {
  	return TEXT;
  }
  return ERROR;
}

char STRING[32] = {};
char * filterAstrid(const char *filename) {
	if (filename[0] == '*') {
		for (int i = 0; i < strlen(filename); i++) {
			STRING[i] = filename[i + 1];
		}
	}
	return STRING;
}


int music_old_index = 0;
void update_musicPlayer(char * filename, int index, uint16_t *channels, uint16_t *SRate, uint16_t *BPS, uint32_t *fileSize, uint32_t *duration, int *preLocation, int *restart) {
	drawImage(0, 0, "System/APPS/File_Manager/Music_Player/background.bimg");
	drawImage((240 - 105) / 2, 40 + (240 - 105) / 2, "System/APPS/File_Manager/Music_Player/audio.bimg");
	drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/pause.bimg");
	setTextColor(WHITE);
	textAlign(LEFT, TOP);
	text(filename, 40 + 15, 15);

	music_old_index = index;
	if (wavState == BUSY) {
		if (strcmp(filename, _wav_filename) != 0) {
			wavStop();
			wavPlay(filename);
		}
	}
	else if (wavState == PAUSED) {
		wavStop();
		wavPlay(filename);
	}
	else {
		wavPlay(filename);
	}

	*channels = _numChannels;
	*SRate = _sampleRate;
	*BPS = _bitsPerSample;//Bits Per Sample
	*fileSize = _fileSize;

	//wavInfo(&channels, &SRate, &BPS, &fileSize);

	char STR[256] = {};

	if (*channels == 1) sprintf(STR, "%d BIT MONO", *BPS);
	if (*channels == 2) sprintf(STR, "%d BIT STEREO", *BPS);
	text(STR, 240, (160 - 5) - 30);

	sprintf(STR, "%d HZ", *SRate);
	text(STR, 240, (160 - 5) - 10);



	//snprintf(STR, 2, "%f", fileSize / 1048576);

	if (*fileSize < 1024) {
		sprintf(STR, "%d BYTES", (int)fileSize);
	}
	if (*fileSize < 1048576) {
		int intPart = *fileSize / 1024;
		int fPart = (((float)(*fileSize) / 1024) - ((int)(*fileSize) / 1024)) * 100;
		sprintf(STR, "%d.%d KB (%d BYTES)", intPart, fPart, (int)(*fileSize));
	}
	if ((*fileSize) >= 1048576) {
		int intPart = (*fileSize) / 1048576;
		int fPart = (((float)(*fileSize) / 1048576) - ((int)(*fileSize) / 1048576)) * 100;
		sprintf(STR, "%d.%d MB (%d BYTES)", intPart, fPart, (int)(*fileSize));
	}

	text(STR, 240, (160 - 5) + 10);

	text("WAV FILE", 240, (160 - 5) + 30);

	//uint32_t duration = 0;
	*duration = (*fileSize) / ((*SRate) * (*channels));

	sprintf(STR, "%02d:%02d", (int)(*duration) / 60, (int)(*duration) % 60);
	text(STR, 480-15-30, 320 - 25);

	*preLocation = -1;
	*restart = false;
}
void musicPlayer(char * filename, int index) {
	uint16_t channels = 0;
	uint16_t SRate = 0;
	uint16_t BPS = 0;//Bits Per Sample
	uint32_t fileSize = 0;

	uint32_t duration = 0;

	int preLocation = -1;
	int restart = false;

	char STR[32] = {};

	update_musicPlayer(filename, index, &channels, &SRate, &BPS, &fileSize, &duration, &preLocation, &restart);

	while(1) {
		//update location
		uint16_t location = ((_buff_count * WAV_BUFFER_LEN) / (SRate * channels));
		if (location != preLocation) {
			fillRect(15, 320-25, 40, 10, BLACK);
			sprintf(STR, "%02d:%02d", location / 60, location % 60);
			text(STR, 15, 320 - 25);
			int XL = map(location, 0, duration, 0, 480-4);
			fillRect(0, 320 - 40 - 2, XL, 4, color565(19, 146, 239));
			fillRect(XL, 320 - 40 - 2, 4, 4, WHITE);
			preLocation = location;
		}

		if (wavState == STOPED && location == duration) {//>=
			drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/restart.bimg");
			restart = true;
		}
		if (restart == true && location != duration) {//<
			drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/play.bimg");
			restart = false;
		}


		uint16_t x = 0, y = 0;
		if (touchscreen(&x, &y) == true) {
			//fillCircle(x, y, 3, GREEN);
			if (mouseOver(x, y, 0, 0, 60, 40)) {
				if (wavState == PAUSED) {
					wavStop();
				}
				return;
			}
			else if (mouseOver(x, y, 440, 0, 40, 40)) {//menu
				if (file_Manager_menu(filename, AUDIO) == true) {
					wavStop();
					return;
				}
				update_musicPlayer(filename, index, &channels, &SRate, &BPS, &fileSize, &duration, &preLocation, &restart);
			}
			else if (mouseOver(x, y, 0, 320 - 80, 480, 40)) {//back button
				if (wavState != STOPED) {
					wavStop();
					drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/play.bimg");
				}
				_buff_count = map(x, 0, 480, 0, fileSize / WAV_BUFFER_LEN);
				drawImage(0, 320 - 42, "System/APPS/File_Manager/Music_Player/seekBar.bimg");
				preLocation = -1;
			}
			else if (mouseOver(x, y, 240-80, 320-40, 60, 40)) {//Previous
				wavStop();
				moveBack(filename);
				int length = scan_files(filename);

				int i = index - 1;
				while(getFileState(files[i]) != AUDIO) {
					if (i <= 0) {
						i = length - 1;
					}
					else {
						i--;
					}
				}
				char STR[256] = {};
				sprintf(STR, "%s/%s", filename, files[i]);
				strcpy(filename, STR);
				index = i;
				update_musicPlayer(STR, i, &channels, &SRate, &BPS, &fileSize, &duration, &preLocation, &restart);
			}
			else if (mouseOver(x, y, 240+30, 320-40, 60, 40)) {//Next
				wavStop();
				moveBack(filename);
				int length = scan_files(filename);

				int i = index + 1;
				while(getFileState(files[i]) != AUDIO) {
					if (i > length - 1) {
						i = 0;
					}
					else {
						i++;
					}
				}
				char STR[256] = {};
				sprintf(STR, "%s/%s", filename, files[i]);
				strcpy(filename, STR);
				index = i;
				update_musicPlayer(STR, i, &channels, &SRate, &BPS, &fileSize, &duration, &preLocation, &restart);


			}
			else if (mouseOver(x, y, 240-30, 320-40, 60, 40)) {//pause / play

				if (restart == true) {
					restart = false;
					drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/pause.bimg");
					drawImage(0, 320 - 42, "System/APPS/File_Manager/Music_Player/seekBar.bimg");
					wavPlay(filename);
				}

				else if (wavState == STOPED) {
					drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/pause.bimg");
					wavPlayFromPos(filename, _buff_count * WAV_BUFFER_LEN);
				}
				else if (wavState == PAUSED) {
					drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/pause.bimg");
					wavResume();
				}
				else if (wavState == BUSY) {
					wavPause();
					drawImage(160, 320 - 38, "System/APPS/File_Manager/Music_Player/play.bimg");
				}
				HAL_Delay(500);
			}
		}
	}



}
void update_imageViewer(char * filename) {
	fillScreen(BLACK);
	setTextColor(WHITE);
	textAlign(LEFT, TOP);
	drawImage(0, 0, "System/APPS/File_Manager/assets/background_top.bimg");
	text(filename, 40 + 15, 15);

	if (wavState == BUSY || wavState == PAUSED) {
		drawImage(480-82, 0, "System/APPS/File_Manager/Music_Player/audio_small.bimg");
	}

	drawImage(0, 40, filename);

}
void imageViewer(char * filename) {
	update_imageViewer(filename);
	while(1) {
		uint16_t x = 0, y = 0;
		if (touchscreen(&x, &y) == true) {
			//fillCircle(x, y, 3, GREEN);
			if (mouseOver(x, y, 0, 0, 60, 40)) {
				return;
			}
			if (mouseOver(x, y, 440, 0, 40, 40)) {//menu
				if (file_Manager_menu(filename, IMAGE)) {
					return;
				}

				update_imageViewer(filename);
			}

			if (mouseOver(x, y, 400, 0, 40, 40)) {
				if (wavState == BUSY || wavState == PAUSED) {
					musicPlayer(_wav_filename, music_old_index);
					update_imageViewer(filename);
				}
			}
		}
	}
}

void drawIcon(int x, int y, FIL *fil) {
	const int buffLen = 60*60*2;
	unsigned char BUFFER[buffLen];

	UINT d;
	SDState = BUSY;
	f_lseek(&(*fil), 4);
	if (f_read(&(*fil), BUFFER, buffLen, &d) != FR_OK) {
		SDState = IDLE;
		return;
	}
	SDState = IDLE;

	int w = 60;
	int h = 60;

	h = constrain(h, 0, LCD_HEIGHT - y);

  CS_LOW;
  setAddress(x, y, x + w - 1, y + h - 1);
  RS_HIGH;

  pixelColor = (BUFFER[0] << 8) + BUFFER[1];
  for (int i = 0; i < w * h * 2; i+=2)
  {
		DATA_PORT->ODR = (BUFFER[i] << 8) + BUFFER[i + 1];

		WR_LOW;

		for (int i = 0; i < 3; i++) {
			__asm__("nop\n\t"); // kill 2 cpu cycles
		}
		WR_HIGH;
  }

  CS_HIGH;
}

int length = 0;

void update_fileManager(char * dir, int offset, int reload) {
	if (reload == true) {
		fillScreen(BLACK);
		setTextColor(WHITE);
		textAlign(LEFT, TOP);
		drawImage(0, 0, "System/APPS/File_Manager/assets/background_top.bimg");
		text(dir, 40 + 15, 15);

		length = scan_files(dir);
	}

	fillRect(480 - 40, 42, 40, 320 - 42, color565(200, 200, 200));
	fillRect(480 - 40, 42, 40, 40, color565(200, 200, 200));
	fillRect(480 - 40, 320 - 40, 40, 40, color565(200, 200, 200));

	int T_X = 480 - 40;
	int T_Y = 42;
	fillTriangle(T_X + 10, T_Y + 25, T_X + 20, T_Y + 15, T_X + 30, T_Y + 25, color565(100, 100, 100));
	T_X = 480 - 40;
	T_Y = 320 - 40;
	fillTriangle(T_X + 10, T_Y + 15, T_X + 20, T_Y + 25, T_X + 30, T_Y + 15, color565(100, 100, 100));


	if (length > 4) {
		int w = ((float)4 / (float)length) * (float)198;
		fillRect(480 - 40, 82 + map(offset, 0, length - 4, 0, 198 - w), 40, w, color565(150, 150, 150));
	}
	if (length == 0) {
		textAlign(CENTER, CENTER);
		text("NO FILES OR FOLDERS", (480 - 40) / 2, (320 - 40) / 2 + 40);
		textAlign(LEFT, TOP);
	}

	int index = 0;
	int pre_F_State = -1;
	FIL fil; // File
	for (int i = offset; i < constrain(length, offset, offset + 5); i++) {
		index = i - offset;
		int F_State = getFileState(files[i]);

		if (F_State != pre_F_State) {
			SDState = BUSY;
			if (pre_F_State != -1) {
				if (f_close(&fil) != FR_OK) {//close file
					Error_Handler();
				}
			}
			pre_F_State = F_State;
			if (F_State == FOLDER) {
				if (f_open(&fil, "System/APPS/File_Manager/assets/folder.bimg", FA_READ) != FR_OK);
			}
			if (F_State == IMAGE) {
				if (f_open(&fil, "System/APPS/File_Manager/assets/photos.bimg", FA_READ) != FR_OK);
			}
			if (F_State == AUDIO) {
				if (f_open(&fil, "System/APPS/File_Manager/assets/audio.bimg", FA_READ) != FR_OK);
			}
			if (F_State == TEXT) {
				if (f_open(&fil, "System/APPS/File_Manager/assets/text.bimg", FA_READ) != FR_OK);
			}
			if (F_State == ERROR) {
				if (f_open(&fil, "System/APPS/File_Manager/assets/error.bimg", FA_READ) != FR_OK);
			}
			SDState = IDLE;
		}

		if (F_State == FOLDER) {
			//drawImage(0, 42 + index * 62, "System/APPS/File_Manager/assets/folder.bimg");
			drawIcon(0, 42 + index * 62, &fil);
			fillRect(60, 42 + index * 62, 480 - 60 - 42, 60, pixelColor);
			text(filterAstrid(files[i]), 62, 42 + index * 62 + 25);
			text("FOLDER", 480 - 100, 42 + index * 62 + 25);
		}
		else if (F_State == IMAGE) {
			//drawImage(0, 42 + index * 62, "System/APPS/File_Manager/assets/photos.bimg");
			drawIcon(0, 42 + index * 62, &fil);
			fillRect(60, 42 + index * 62, 480 - 60 - 42, 60, pixelColor);
			text(files[i], 62, 42 + index * 62 + 25);
			text("IMAGE", 480 - 100, 42 + index * 62 + 25);
		}
		else if (F_State == AUDIO) {
			//drawImage(0, 42 + index * 62, "System/APPS/File_Manager/assets/audio.bimg");
			drawIcon(0, 42 + index * 62, &fil);
			fillRect(60, 42 + index * 62, 480 - 60 - 42, 60, pixelColor);
			text(files[i], 62, 42 + index * 62 + 25);
			text("AUDIO", 480 - 100, 42 + index * 62 + 25);
		}
		else if (F_State == TEXT) {
			//drawImage(0, 42 + index * 62, "System/APPS/File_Manager/assets/text.bimg");
			drawIcon(0, 42 + index * 62, &fil);
			fillRect(60, 42 + index * 62, 480 - 60 - 42, 60, pixelColor);
			text(files[i], 62, 42 + index * 62 + 25);
			text("TEXT", 480 - 100, 42 + index * 62 + 25);
		}
		else if (F_State == ERROR) {
			//drawImage(0, 42 + index * 62, "System/APPS/File_Manager/assets/error.bimg");
			drawIcon(0, 42 + index * 62, &fil);
			fillRect(60, 42 + index * 62, 480 - 60 - 42, 60, pixelColor);
			text(files[i], 62, 42 + index * 62 + 25);
			text("UNKNOWN", 480 - 100, 42 + index * 62 + 25);
		}
	}
  SDState = BUSY;
	if (f_close(&fil) != FR_OK) {//close file
		Error_Handler();
	}
	SDState = IDLE;
	if (index < 5 - 1 && reload == false) {
		fillRect(0, 42 + (index + 1) * 62, 480 - 42, 60, BLACK);
	}

	if (wavState == BUSY || wavState == PAUSED) {
		drawImage(480-82, 0, "System/APPS/File_Manager/Music_Player/audio_small.bimg");
	}
	//*onScroll = false;
}

void fileManager(char * dir, int offset, int reload) {
	uint16_t onScroll = false;
	update_fileManager(dir, offset, reload);


	while(1) {
		uint16_t x = 0, y = 0;
		if (touchscreen(&x, &y) == true) {
			if (mouseOver(x, y, 0, 0, 60, 40)) {
				moveBack(dir);
				update_fileManager(dir, offset = 0, true);
				continue;
				//fileManager(dir, 0, true);
			}
			if (mouseOver(x, y, 400-2, 0, 40, 40) && (wavState == BUSY || wavState == PAUSED)) {
				musicPlayer(_wav_filename, music_old_index);
				update_fileManager(dir, offset = 0, true);
				continue;
				//fileManager(dir, 0, true);
			}
			if (mouseOver(x, y, 440, 0, 40, 40)) {
				file_Manager_menu(dir, FOLDER);
				update_fileManager(dir, offset = 0, true);
				continue;
				//fileManager(dir, 0, true);
			}
			if (y > 40 && x < 480 - 40) {
				y -= 40;
				y /= 62;
				y += offset;
				if (y < length) {
					uint32_t t = HAL_GetTick();
					int Selected = false;
					char STR[256] = {};
					drawRect(0, 42 + (y - offset) * 62, 480 - 42, 60, RED);
					drawRect(1, 42 + (y - offset) * 62 + 1, 480 - 42 - 2, 60 - 2, RED);
					while (screenPressed()) {
						if (HAL_GetTick() - t > 1000) {
							Selected = true;
							break;
						}
					}
					if (Selected == true) {//held on screen
						if (getFileState(files[y]) == FOLDER) {
							sprintf(STR, "%s/%s", dir, filterAstrid(files[y]));
						}
						else {
							sprintf(STR, "%s/%s", dir, files[y]);
						}
						file_Manager_menu(STR, getFileState(files[y]));
						update_fileManager(dir, offset = 0, true);
						continue;
					}

					if (getFileState(files[y]) == FOLDER) {
						sprintf(STR, "%s/%s", dir, filterAstrid(files[y]));
						strcpy(dir, STR);
						update_fileManager(dir, offset = 0, true);
						continue;
						//fileManager(STR, 0, true);
					}
					if (getFileState(files[y]) == AUDIO) {
						sprintf(STR, "%s/%s", dir, files[y]);
						musicPlayer(STR, y);
						update_fileManager(dir, offset = 0, true);
						continue;
						//fileManager(dir, 0, true);
					}
					if (getFileState(files[y]) == IMAGE) {
						sprintf(STR, "%s/%s", dir, files[y]);
						imageViewer(STR);
						update_fileManager(dir, offset = 0, true);
						continue;
						//fileManager(dir, 0, true);
					}
				}

			}
			if (length > 4) {
				if (y > 40 && x > 480 - 40) {
					if (mouseOver(x, y, 480 - 40, 42, 40, 40)) {
						//fileManager(dir, constrain(offset - 1, 0, length - 1), false);
						update_fileManager(dir, offset = constrain(offset - 1, 0, length - 4), false);
						continue;
					}
					if (mouseOver(x, y, 480 - 40, 320 - 40, 40, 40)) {
						//fileManager(dir, constrain(offset + 1, 0, length - 4), false);
						update_fileManager(dir, offset = constrain(offset + 1, 0, length - 4), false);
						continue;
					}

					if (mouseOver(x, y, 480 - 40, 82, 40, 198)) {
						int temp_offset = offset;
						onScroll = true;
						y -= 82;
						int w = ((float)4 / (float)length) * (float)198;
						offset = map(y, w / 2, 198 - w / 2, 0, length - 4);
						offset = constrain(offset, 0, length - 4);

						if (offset != temp_offset) {
							fillRect(480 - 40, 82, 40, 198, color565(200, 200, 200));
							fillRect(480 - 40, 82 + map(offset, 0, length - 4, 0, 198 - w), 40, w, color565(150, 150, 150));
						}
					}
					//fillRect(480 - 40, 42, 40, 320 - 42, color565(200, 200, 200));
					//fillRect(480 - 40, 42, 40, 40, color565(200, 200, 200));
					//fillRect(480 - 40, 320 - 40, 40, 40, color565(200, 200, 200));
				}
			}

		}
		else if (onScroll == true) {
			//fileManager(dir, constrain(offset, 0, length - 4), false);
			onScroll = false;
			update_fileManager(dir, offset = constrain(offset, 0, length - 4), false);
			continue;
		}
	}
}
const char menu[6][7] = {
		"New",
		"Delete",
		"Rename",
		"Cut",
		"Copy",
		"Paste"
};
char clipBoard[256];
int clipBoardState = 0;
void updateFolderMenu() {
	textAlign(CENTER, CENTER);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			if (j == 0 && i == 2) {
				fillRect(80+(j*160)+1, 80+1+(i*60), 160-2, 60-2, color565(128, 0, 0));
				setTextColor(color565(200, 200, 200));
				text(menu[i * 2 + j], 80+(j * 160)+80, 80+(i*60)+30);
			}
			else if (j == 1 && i == 2 && clipBoardState == 0) {
				fillRect(80+(j*160)+1, 80+1+(i*60), 160-2, 60-2, color565(128, 0, 0));
				setTextColor(color565(200, 200, 200));
				text(menu[i * 2 + j], 80+(j * 160)+80, 80+(i*60)+30);
			}
			else {
				fillRect(80+(j*160)+1, 80+1+(i*60), 160-2, 60-2, RED);
				setTextColor(WHITE);
				text(menu[i * 2 + j], 80+(j * 160)+80, 80+(i*60)+30);
			}
		}
	}
}
void updateFileMenu() {
	textAlign(CENTER, CENTER);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			if ((j == 0 && i == 0) || (j == 1 && i == 2)) {
				fillRect(80+(j*160)+1, 80+1+(i*60), 160-2, 60-2, color565(0, 0, 128));
				setTextColor(color565(200, 200, 200));
				text(menu[i * 2 + j], 80+(j * 160)+80, 80+(i*60)+30);
			}
			else {
				fillRect(80+(j*160)+1, 80+1+(i*60), 160-2, 60-2, BLUE);
				setTextColor(WHITE);
				text(menu[i * 2 + j], 80+(j * 160)+80, 80+(i*60)+30);
			}
		}
	}
}

int file_Manager_menu(char * dir, int trigger) {
	textAlign(CENTER, CENTER);
	setTextColor(WHITE);

	fillRect(80 - 1, 60 - 1, 480 - 160 + 2, 320 - 120 + 2, BLACK);
	drawRect(80 - 2, 60 - 2, 480 - 160 + 4 , 320 - 120 + 4, WHITE);
	text(dir, 480 / 2, 60 + 10);

	if (trigger == FOLDER) {
		updateFolderMenu();
		while(screenPressed());//wait if screen is pressed
		while(1) {
			uint16_t x = 0, y = 0;
			if (touchscreen(&x, &y) == true) {
				if (mouseOver(x, y, 80, 80, 320, 180)) {
					x -= 80;
					y -= 80;

					x /= 160;
					y /= 60;
					if ((x == 0 || x == 1) && (y >= 0 && y <= 2)) {
						fillRect(80+(x*160)+1, 80+1+(y*60), 160-2, 60-2, color565(50, 50, 50));
						setTextColor(WHITE);
						text(menu[y * 2 + x], 80+(x * 160)+80, 80+(y*60)+30);
						while(screenPressed());
						updateFolderMenu();
					}
					if (x == 0 && y == 0) {
						char STR[256] = {};
						char FOLDER_NAME[32] = "New Folder";

						if (keyBoard(FOLDER_NAME, "New Folder", "Cancel", "Create", 32, true) == true) {
							sprintf(STR, "%s/%s", dir, FOLDER_NAME);
							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}

							f_mkdir(STR);// make dir

							if (temp == BUSY) {
								wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
							}
						}
						return false;
					}
					if (x == 1 && y == 0) {

						int des = confirmationDelete(dir, FOLDER, "Cancel", "Delete");
						if (des) {
							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}
							int res = rm_dir(dir);
							if (temp == BUSY) {
								wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
							}
							if (res != FR_OK) {
								fillRect(80 - 1, 60 - 1, 480 - 160 + 2, 320 - 120 + 2, BLUE);
								textAlign(LEFT, TOP);
								if (res == 18) {
									text("FR_TOO_MANY_OPEN_FILES", 80 + 10, 60 + 10);
									char STR[32] = {};
									sprintf(STR, "<_FS_LOCK> SET TO <%d> IN <Inc/ffconf.h>", _FS_LOCK);
									text(STR, 80 + 10, 60 + 30);
								}
								else {
									text("AN UNKNOWN ERROR OCCURED", 80 + 10, 60 + 10);
								}
								HAL_Delay(4000);
							}
							else {
								moveBack(dir);//if successful: move out of deleted dir
							}
						}

						//char STR[256] = {};
						//char FOLDER_NAME[32] = {};
						return false;
					}
					if (x == 0 && y == 1) {
						char NEW_DIR[256] = {};
						char NEW_NAME[32] = {};
						getName(NEW_NAME, dir);
						if (keyBoard(NEW_NAME, "Rename Folder", "Cancel", "Rename", 32, true) == true) {
							strcpy(NEW_DIR, dir);
							moveBack(NEW_DIR);
							sprintf(NEW_DIR, "%s/%s", NEW_DIR, NEW_NAME);
							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}

							//f_mkdir(STR);// make dir
							f_rename(dir, NEW_DIR);

							if (temp == BUSY) {
								wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
							}
							strcpy(dir, NEW_DIR);
						}
						return false;
					}
					if (x == 1 && y == 1) {
						strcpy(clipBoard, dir);
						clipBoardState = 3;
						return false;
					}
					if (x == 1 && y == 2) {
						if (clipBoardState == 1) {
							char STR[256];
							char F_Name[32];
							getName(F_Name, clipBoard);
							sprintf(STR, "%s/%s", dir, F_Name);

							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}
							copy_file(STR, clipBoard);
							if (temp == BUSY) {
								wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
							}
							return false;
						}
						if (clipBoardState == 2 || clipBoardState == 3) {
							char STR[256];
							char F_Name[32];
							getName(F_Name, clipBoard);
							sprintf(STR, "%s/%s", dir, F_Name);

							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}
							int res = f_rename(clipBoard, STR);
							if (res != FR_OK) {
								Error_Handler();
							}
							if (temp == BUSY) {
								if (clipBoard != _wav_filename) {
									wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
								}
								else {
									//don't do anything - got changed
								}
							}

							clipBoardState = 0;
							return false;
						}
					}

				}
				else {
					if (!mouseOver(x, y, 440, 0, 40, 40)) {
						return false;
					}
				}
			}
		}
	}
	else {//if its not a folder
		updateFileMenu();
		while(screenPressed());//wait if screen is pressed
		while(1) {
			uint16_t x = 0, y = 0;
			if (touchscreen(&x, &y) == true) {
				if (mouseOver(x, y, 80, 80, 320, 180)) {
					x -= 80;
					y -= 80;

					x /= 160;
					y /= 60;
					if ((x == 0 || x == 1) && (y >= 0 && y <= 2)) {
						fillRect(80+(x*160)+1, 80+1+(y*60), 160-2, 60-2, color565(50, 50, 50));
						setTextColor(WHITE);
						text(menu[y * 2 + x], 80+(x * 160)+80, 80+(y*60)+30);
						while(screenPressed());
						updateFileMenu();
					}
					if (x == 1 && y == 0) {
						if (confirmationDelete(dir, trigger, "Cancel", "Delete") == true) {
							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}
							int res = f_unlink(dir);
							if (res != FR_OK) {
								Error_Handler();
							}
							if (temp == BUSY) {
								if (dir != _wav_filename) {
									wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
								}
							}
							return true;
						}
						return false;
					}
					if (x == 0 && y == 1) {
						char NEW_DIR[256] = {};
						char NEW_NAME[32] = {};
						getName(NEW_NAME, dir);
						if (keyBoard(NEW_NAME, "Rename File", "Cancel", "Rename", 32, true) == true) {
							strcpy(NEW_DIR, dir);
							moveBack(NEW_DIR);
							sprintf(NEW_DIR, "%s/%s", NEW_DIR, NEW_NAME);
							int temp = 0;
							if (wavState == BUSY) {
								temp = BUSY;
								wavStop();//stop playback
							}

							//f_mkdir(STR);// make dir
							int res = f_rename(dir, NEW_DIR);
							if (res != FR_OK) {
								Error_Handler();
							}

							if (temp == BUSY) {
								if (dir != _wav_filename) {
									wavPlayFromPos(_wav_filename, _buff_count * WAV_BUFFER_LEN);//restart playback
								}
								else {
									wavPlayFromPos(NEW_DIR, _buff_count * WAV_BUFFER_LEN);//restart playback
								}
							}
							strcpy(dir, NEW_DIR);
						}
						return false;
					}
					if (x == 0 && y == 2) {
						strcpy(clipBoard, dir);
						clipBoardState = 1;
						return false;
					}
					if (x == 1 && y == 1) {
						strcpy(clipBoard, dir);
						clipBoardState = 2;
						return false;
					}
				}
				else {
					if (!mouseOver(x, y, 440, 0, 40, 40)) {
						return false;
					}
				}
			}
		}
	}
	textAlign(LEFT, TOP);
	return false;
}
int confirmationDelete(const char * dir, int f_state, const char * option1, const char * option2) {
	fillScreen(BLACK);
	setTextColor(WHITE);

	fillRect(0, 0, 480, 44, color565(150, 150, 170));
	fillRect(480-82-82, 2, 80, 40, color565(180, 180, 200));
	fillRect(480-82, 2, 80, 40, color565(180, 180, 200));
	textAlign(CENTER, CENTER);
	text(option1, (480-82-82) + 40, 22);
	text(option2, (480-82) + 40, 22);
	textAlign(LEFT, CENTER);

	text("Confirmation Delete", 22, 22);

	if (f_state == FOLDER) {
		drawImage(22, 42 + 22, "System/APPS/File_Manager/assets/folder.bimg");
		text(dir, 22 + 60 + 22, 42 + 22 + 30);
	}
	if (f_state == AUDIO) {
		drawImage(22, 42 + 22, "System/APPS/File_Manager/assets/audio.bimg");
		text(dir, 22 + 60 + 22, 42 + 22 + 30);
	}
	if (f_state == IMAGE) {
		drawImage(22, 42 + 22, "System/APPS/File_Manager/assets/photos.bimg");
		text(dir, 22 + 60 + 22, 42 + 22 + 30);
	}



	uint16_t x = 0, y = 0;
	while(1) {
		if (touchscreen(&x, &y) == true) {
			if (mouseOver(x, y, 480-82-82, 2, 80, 40)) {//option1 - cancel
				fillRect(480-82-82, 2, 80, 40, BLACK);
				textAlign(CENTER, CENTER);
				text(option1, (480-82-82) + 40, 22);
				while(touchscreen(&x, &y) == true) {
					//wait until screen in not pressed
				}
				return false;
			}
			if (mouseOver(x, y, 480-82, 2, 80, 40)) {//option2 - okay
				fillRect(480-82, 2, 80, 40, BLACK);
				textAlign(CENTER, CENTER);
				text(option2, (480-82) + 40, 22);
				while(touchscreen(&x, &y) == true) {
					//wait until screen in not pressed
				}
				return true;
			}
		}
	}
	return false;
}

void convertSizeToSTR(char * output, uint32_t fileSize, int dispBytes) {
	if (fileSize < 1024) {
		sprintf(&(*output), "%d BYTES", (int)(fileSize));
	}
	if (fileSize < 1048576) {
		int intPart = fileSize / 1024;
		int fPart = (((float)(fileSize) / 1024) - ((int)(fileSize) / 1024)) * 100;
		if (dispBytes) {
			sprintf(&(*output), "%d.%d KB (%d BYTES)", intPart, fPart, (int)(fileSize));
		}
		else {
			sprintf(&(*output), "%d.%d KB", intPart, fPart);
		}
	}
	if ((fileSize) >= 1048576) {
		int intPart = (fileSize) / 1048576;
		int fPart = (((float)(fileSize) / 1048576) - ((int)(fileSize) / 1048576)) * 100;
		if (dispBytes) {
			sprintf(&(*output), "%d.%d MB (%d BYTES)", intPart, fPart, (int)(fileSize));
		}
		else {
			sprintf(&(*output), "%d.%d MB", intPart, fPart);
		}
	}
}

void copy_file(const char * dest, const char * src) {
	FIL fil; // File
	FRESULT res;
	UINT br, bw;
	int buff_size = 32768;

	uint8_t *BUFFER = (uint8_t*)malloc(buff_size * sizeof(uint8_t));
	if (BUFFER == NULL) {
		Error_Handler();
	}

	uint16_t location = 0;

	res = f_open(&fil, src, FA_OPEN_EXISTING | FA_READ);
	FSIZE_t size = f_size(&fil);
	f_close(&fil);
	uint32_t totalBuffCount = size / buff_size;

	fillRect(40, 80, 480 - 80, 320 - 160, WHITE);
	drawRect(40, 80, 480 - 80, 320 - 160, BLACK);
	fillRect(40 + 20, 320 / 2 - 15, 400 - 40, 30, BLACK);
	setTextColor(BLACK);
	textAlign(RIGHT, TOP);
	char STR[256];
	convertSizeToSTR(STR, size, false);
	text(STR, 440 - 20, 320 - 80 - 20 - 10);

	while(1) {
		//read buffer_______________________________________________________________
		res = f_open(&fil, src, FA_OPEN_EXISTING | FA_READ);
		if (res != FR_OK) break;
		f_lseek(&fil, location * buff_size);
		res = f_read(&fil, BUFFER, buff_size, &br);
		if (res != FR_OK || br == 0) break;
		f_close(&fil);

		//write buffer to new file__________________________________________________
		res = f_open(&fil, dest, FA_OPEN_ALWAYS | FA_WRITE);
		if (res != FR_OK) break;
		f_lseek(&fil, location * buff_size);
		res = f_write(&fil, BUFFER, br, &bw);
		if (res != FR_OK || bw < br) break;
		f_close(&fil);

		fillRect(40 + 20, 320 / 2 - 15, map(location, 0, totalBuffCount, 0, 400 - 40), 30, GREEN);

		fillRect(40 + 20, 80 + 20, 6*4, 10, WHITE);
		fillRect(40 + 20, 320 - 80 - 20 - 10, 100, 10, WHITE);

		textAlign(LEFT, TOP);
		sprintf(STR, "%d\%%", (int)map(location, 0, totalBuffCount, 0, 100));
		text(STR, 40 + 20, 80 + 20);
		convertSizeToSTR(STR, location * buff_size, false);
		text(STR, 40 + 20, 320 - 80 - 20 - 10);

		location++;//Increment location
	}

	f_close(&fil);
	free(BUFFER);

	setTextColor(WHITE);
}

FRESULT rm_dir(char * dir) {
	//_FS_LOCK 		4 	in Inc/ffconf.h
  FRESULT fr;
  //FATFS fs;
  TCHAR buff[256];
  FILINFO fno;

  strcpy(buff, _T(dir));

  fr = delete_node(buff, sizeof buff / sizeof buff[0], &fno);
  return fr;
}

FRESULT delete_node (
    TCHAR* path,    /* Path name buffer with the sub-directory to delete */
    UINT sz_buff,   /* Size of path name buffer (items) */
    FILINFO* fno    /* Name read buffer */
)
{
    UINT i, j;
    FRESULT fr;
    DIR dir;


    fr = f_opendir(&dir, path); /* Open the directory */
    if (fr != FR_OK) return fr;

    for (i = 0; path[i]; i++) ; /* Get current path length */
    path[i++] = _T('/');

    for (;;) {
        fr = f_readdir(&dir, fno);  /* Get a directory item */
        if (fr != FR_OK || !fno->fname[0]) break;   /* End of directory? */
        j = 0;
        do {    /* Make a path name */
            if (i + j >= sz_buff) { /* Buffer over flow? */
                fr = 100; break;    /* Fails with 100 when buffer overflow */
            }
            path[i + j] = fno->fname[j];
        } while (fno->fname[j++]);
        if (fno->fattrib & AM_DIR) {    /* Item is a directory */
            fr = delete_node(path, sz_buff, fno);
        } else {                        /* Item is a file */
            fr = f_unlink(path);
        }
        if (fr != FR_OK) break;
    }

    path[--i] = 0;  /* Restore the path name */
    f_closedir(&dir);

    if (fr == FR_OK) fr = f_unlink(path);  /* Delete the empty directory */
    return fr;
}






//keyboard______________________________________________________________________________________________________________________________________________________

#define _N		0
#define _D		1
#define _L		2

#define w			48
#define hw		40

#define _BS		8
#define _CR		13

#define _UP		128		//uppercase
#define _LW		129		//lowercase
#define _ST		130		//Standard (abc)
#define _SP		131		//Special keys (!@#$)


const unsigned char keyBoardText[3][5][15][6] = {
		{
				{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
				{"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"},
				{" ", "a", "s", "d", "f", "g", "h", "j", "k", "l", " "},
				{"UP", "z", "x", "c", "v", "b", "n", "m", "<"},
				{"!@#", ".", "SPACE", ",", "RETURN"}
		},

		{
				{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
				{"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"},
				{" ", "A", "S", "D", "F", "G", "H", "J", "K", "L", " "},
				{"DOWN", "Z", "X", "C", "V", "B", "N", "M", "<"},
				{"!@#", ".", "SPACE", ",", "RETURN"}
		},
		{
				{"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"},
				{"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"},
				{"-", "=", "_", "=", "[", "]", "\\", "{", "}", "|"},
				{";", "'", ":", "\"", "<", ">", "/", "?", "<"},
				{"ABC", ".", "SPACE", ",", "RETURN"}
		}

};
const unsigned char keyBoardChars[3][5][15] = {
		{
				{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
				{'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p'},
				{' ', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ' '},
				{_UP, 'z', 'x', 'c', 'v', 'b', 'n', 'm', _BS},
				{_SP, '.', ' ', ',', _CR}
		},
		{
				{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
				{'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'},
				{' ', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ' '},
				{_LW, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', _BS},
				{_SP, '.', ' ', ',', _CR}
		},
		{
				{'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'},
				{'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'},
				{'-', '=', '_', '=', '[', ']', '\\', '{', '}', '|'},
				{';', '\'', ':', '\"', '<', '>', '/', '?', _BS},
				{_ST, '.', ' ', ',', _CR}
		}

};
const uint8_t keyBoardOffsets[2][5][11] = {//to be divided by ten
		{
				{10,10,10,10,10,10,10,10,10,10},//used for both keystate 0 and 1
				{10,10,10,10,10,10,10,10,10,10},
				{5,10,10,10,10,10,10,10,10,10,5},
				{15,10,10,10,10,10,10,10,15},
				{15,10,50,10,15}
		},
		{
				{10,10,10,10,10,10,10,10,10,10},
				{10,10,10,10,10,10,10,10,10,10},
				{10,10,10,10,10,10,10,10,10,10},
				{10,10,10,10,10,10,10,10,20},
				{15,10,50,10,15}
		}
};
const uint8_t keyBoardColors[2][5][11] = {
		{
				{_D,_D,_D,_D,_D,_D,_D,_D,_D,_D},//used for both keystate 0 and 1
				{_L,_L,_L,_L,_L,_L,_L,_L,_L,_L},
				{_N,_L,_L,_L,_L,_L,_L,_L,_L,_L,_N},
				{_D,_L,_L,_L,_L,_L,_L,_L,_D},
				{_D,_D,_L,_D,_D}
		},
		{
				{_D,_D,_D,_D,_D,_D,_D,_D,_D,_D},
				{_L,_L,_L,_L,_L,_L,_L,_L,_L,_L},
				{_L,_L,_L,_L,_L,_L,_L,_L,_L,_L},
				{_L,_L,_L,_L,_L,_L,_L,_L,_D},
				{_D,_D,_L,_D,_D}
		}
};
int keyState = 0;


uint16_t keyBoardGetColor(int i, int j) {
	if (keyBoardColors[constrain(keyState - 1, 0, 1)][i][j] == _N) {
		return color565(150, 150, 170);
	}
	else if (keyBoardColors[constrain(keyState - 1, 0, 1)][i][j] == _D) {
		return color565(120, 120, 140);
	}
	else if (keyBoardColors[constrain(keyState - 1, 0, 1)][i][j] == _L) {
		return color565(180, 180, 200);
	}
	return BLACK;
}
int updateKeyboard(int Pressed, uint16_t x, uint16_t y) {
	textAlign(CENTER, CENTER);
	uint16_t fillColor = 0x00;
	int output_char = -1;

	for (int i = 0; i < 5; i++) {
		int length = 11;//strlen(keyBoardText[keyState][i]);

		int X = 0;
		for (int j = 0; j < length; j++) {
			float Width = ((float) keyBoardOffsets[constrain(keyState - 1, 0, 1)][i][j]) / 10.0;
			Width = Width * w;

			if (Pressed && mouseOver(x, y, X + 1, i * hw + 1  + (320 - hw * 5), Width, hw)) {
				fillColor = BLACK;
				setTextColor(WHITE);
				output_char = keyBoardChars[keyState][i][j];
			}
			else {
				fillColor = keyBoardGetColor(i, j);
				setTextColor(WHITE);
			}

			fillRect(X + 1, i * hw + 1  + (320 - hw * 5), Width - 2, hw - 2, fillColor);

			if (i == 4) {
				text((char*) keyBoardText[keyState][i][j], X + 1 + (Width / 2), i * hw + 1 + (hw / 2) + (320 - hw * 5));
			}
			else {
				//char STR[2];
				//STR[0] = keyBoardText[keyState][i][j];
				//STR[1] = '\0';
				text((char*) keyBoardText[keyState][i][j], X + 1 + (Width / 2), i * hw + 1 + (hw / 2) + (320 - hw * 5));
			}
			X += (Width);
		}
	}
	return output_char;
}

int str_cut(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}

int keyBoard(char * output_text, const char * input_text, const char * option1, const char * option2, int arr_size, int reload) {
	fillScreen(BLACK);
	setTextColor(WHITE);

	fillRect(0, 0, 480, 44, color565(150, 150, 170));
	fillRect(480-82-82, 2, 80, 40, color565(180, 180, 200));
	fillRect(480-82, 2, 80, 40, color565(180, 180, 200));
	textAlign(CENTER, CENTER);
	text(option1, (480-82-82) + 40, 22);
	text(option2, (480-82) + 40, 22);
	textAlign(LEFT, CENTER);

	keyState = 0;
	int Cursor = strlen(output_text);

	char STR[64];
	sprintf(STR, "%s  -  %d char max", input_text, arr_size);
	text(STR, 22, 22);

	fillRect(0, 320-40*5-1, 480, 40*5+1, color565(150, 150, 170));


	//output_text[0] = '\0';
	textAlign(LEFT, CENTER);
	text(output_text, 22, 42 + 39);
	fillRect(22+Cursor * 6-1, 42 + 39 - 7, 1, 14, WHITE);//display cursor

	textAlign(LEFT, TOP);
	updateKeyboard(false, 0, 0);//display keyboard

	uint16_t x = 0, y = 0;
	char ch = -1;

	while(1) {
		ch = -1;
		if (touchscreen(&x, &y) == true) {
			if (mouseOver(x, y, 0, 320-40*5-1, 480, 40*5+1)) {
				ch = updateKeyboard(true, x, y);
				if (ch >= 0) {
					while(screenPressed()) {//wait if screen is pressed

					}
					updateKeyboard(false, 0, 0);

					if (ch == _BS) {//check for backspace
						if (strlen(output_text) >= arr_size - 1) {
							fillRect(240 - 75, 42+15, 150, 10, BLACK);
						}
						if (Cursor > 0) {//strlen(output_text)
							//output_text[strlen(output_text) - 1] = '\0';

							char subStr1[32];//first string
							strcpy(subStr1, output_text + Cursor);//get second string
							//subStr1[strlen(subStr1) - 1] = '\0';

							int i = Cursor;
							while(subStr1[i - Cursor] != '\0') {//copy second string into output  with negative offset of 1
								output_text[i-1] = subStr1[i - Cursor];
								i++;
							}
							output_text[i-1] = '\0';//terminate string

							Cursor--;//decrement Cursor
						}
					}
					else if (ch == _CR) {
						return true;
					}
					else if (ch == _UP) {
						keyState = 1;
						updateKeyboard(false, 0, 0);
					}
					else if (ch == _LW) {
						keyState = 0;
						updateKeyboard(false, 0, 0);
					}
					else if (ch == _SP) {
						keyState = 2;
						fillRect(0, 320-40*5-1, 480, 40*5+1, color565(150, 150, 170));
						updateKeyboard(false, 0, 0);
					}
					else if (ch == _ST) {
						keyState = 0;
						fillRect(0, 320-40*5-1, 480, 40*5+1, color565(150, 150, 170));
						updateKeyboard(false, 0, 0);
					}
					else {
						if (strlen(output_text) < arr_size - 1) {
							char subStr1[32];//first string
							char subStr2[32];//second string
							strcpy(subStr1, output_text);//get first string
							subStr1[Cursor] = '\0';//trim string to desired length

							strcpy(subStr2, output_text + Cursor);//get second string

							int i = 0;
							while(subStr1[i] != '\0') {//copy first string into output
								output_text[i] = subStr1[i];
								i++;
							}
							output_text[i] = ch;//insert char
							i++;//increment i
							int offset = i;
							while(subStr2[i - offset] != '\0') {//copy second string into output
								output_text[i] = subStr2[i - offset];
								i++;
							}
							output_text[i] = '\0';//terminate string

							Cursor++;//increment Cursor
						}
						else {
							setTextColor(RED);
							textAlign(CENTER, CENTER);
							sprintf(STR, "Reached %d char limit", arr_size);
							text(STR, 480 / 2, 42+20);
							setTextColor(WHITE);
							//textAlign(CENTER, CENTER);
						}
					}

					//update text on screen
					textAlign(LEFT, CENTER);
					fillRect(0, 42 + 39 - 7, 240, 14, BLACK);
					text(output_text, 22, 42 + 39);
					fillRect(22+Cursor * 6-1, 42 + 39 - 7, 1, 14, WHITE);//display cursor
				}
			}
			else {
				if (mouseOver(x, y, 480-82-82, 2, 80, 40)) {//option1 - cancel
					fillRect(480-82-82, 2, 80, 40, BLACK);
					textAlign(CENTER, CENTER);
					text(option1, (480-82-82) + 40, 22);
					while(screenPressed()) {//wait if screen is pressed

					}
					return false;
				}
				if (mouseOver(x, y, 480-82, 2, 80, 40)) {//option2 - okay
					fillRect(480-82, 2, 80, 40, BLACK);
					textAlign(CENTER, CENTER);
					text(option2, (480-82) + 40, 22);
					while(screenPressed()) {//wait if screen is pressed

					}
					return true;
				}
				if (mouseOver(x, y, 0, 42 + 39 - 25, 240, 50)) {//move cursor
					int X = x - 22;
					X /= 6;
					Cursor = constrain(X, 0, strlen(output_text));

					textAlign(LEFT, CENTER);
					fillRect(0, 42 + 39 - 7, 240, 14, BLACK);
					text(output_text, 22, 42 + 39);
					fillRect(22+Cursor * 6-1, 42 + 39 - 7, 1, 14, WHITE);//display cursor
				}
			}
		}

	}
	return false;
}






