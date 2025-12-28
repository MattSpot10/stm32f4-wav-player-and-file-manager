#ifndef  WAV_H
#define WAV_H

#include "main.h"
#include "fatfs.h"

#define WAV_OK 0
#define WAV_ERROR 1

DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_dac1;
DMA_HandleTypeDef hdma_dac2;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

volatile uint16_t count = 0;
volatile uint16_t _buff_count = 0;
volatile uint16_t buff = 0;
volatile int _buff_updated = false;

FATFS fs;  // file system
FIL _wav_file; // File
FILINFO fno;
FRESULT fresult;  // result
UINT d;


int wavState = STOPED;

#define WAV_BUFFER_LEN 		4096
uint8_t *WAV_BUFFER1;
uint8_t *WAV_BUFFER2;

uint8_t *WAV_BUFFER1C1;
uint8_t *WAV_BUFFER1C2;

uint8_t *WAV_BUFFER2C1;
uint8_t *WAV_BUFFER2C2;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void readNextWavBuffer();
static void RE_Init_TIM6(int Prescaler, int Period);
static void RE_Init_TIM7(int Prescaler, int Period);
void initTimers(uint16_t SampleRate);
uint32_t wavPlay(char * filename);
void wavStop(void);
void malloc_wav_buffers();
void free_wav_buffers();


volatile uint8_t interuptFlagWaiting = false;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6) {//if the source is tim 6
		count++;
		if (count >= WAV_BUFFER_LEN/2 && _buff_updated == true) {
			_buff_updated = false;
			if (buff == 0) {
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)WAV_BUFFER1C1, WAV_BUFFER_LEN/2, DAC_ALIGN_8B_R);
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t *)WAV_BUFFER1C2, WAV_BUFFER_LEN/2, DAC_ALIGN_8B_R);
			}
			else {
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)WAV_BUFFER2C1, WAV_BUFFER_LEN/2, DAC_ALIGN_8B_R);
				HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_2, (uint32_t *)WAV_BUFFER2C2, WAV_BUFFER_LEN/2, DAC_ALIGN_8B_R);
			}
			count = 0;
			//HAL_TIM_Base_Start_IT(&htim7);
		}
		if (count == 10) {
			HAL_TIM_Base_Start_IT(&htim7);//call interupt slightly later than HAL_DAC_Start_DMA...
		}
	}
	if(htim->Instance == TIM7) {
		if (SDState == IDLE) {
			interuptFlagWaiting = false;
			readNextWavBuffer();
			HAL_TIM_Base_Stop_IT(&htim7);
		}
		else {
			interuptFlagWaiting = true;
		}
		//HAL_TIM_Base_Stop_IT(&htim7);
	}

}
void readNextWavBuffer() {
	_buff_count++;
	int temp_buff = buff;

	if (temp_buff == 0) {
		f_read(&_wav_file, WAV_BUFFER2, WAV_BUFFER_LEN, &d);
		if (d != WAV_BUFFER_LEN) {
			wavStop();
		}
		for (int i = 0; i < WAV_BUFFER_LEN/2; i++) {//seperate channels
			WAV_BUFFER2C1[i] = WAV_BUFFER2[i * 2];
			WAV_BUFFER2C2[i] = WAV_BUFFER2[i * 2 + 1];
		}
		buff = 1;
	}
	else {
		f_read(&_wav_file, WAV_BUFFER1, WAV_BUFFER_LEN, &d);
		if (d != WAV_BUFFER_LEN) {
			wavStop();
		}
		for (int i = 0; i < WAV_BUFFER_LEN/2; i++) {//seperate channels
			WAV_BUFFER1C1[i] = WAV_BUFFER1[i * 2];
			WAV_BUFFER1C2[i] = WAV_BUFFER1[i * 2 + 1];
		}
		buff = 0;
	}
	_buff_updated = true;
}

static void RE_Init_TIM6(int Prescaler, int Period)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = Prescaler-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = Period-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void RE_Init_TIM7(int Prescaler, int Period)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = Prescaler-1;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = Period-1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

void initTimers(uint16_t SampleRate) {
	int Prescaler = 21;
	int Period = 125;
	
	if (SampleRate == 8000) {
		Prescaler = 21;
		Period = 500;
	}
	if (SampleRate == 11025) {
		Prescaler = 21;
		Period = 362;
	}
	if (SampleRate == 16000) {
		Prescaler = 21;
		Period = 250;
	}
	if (SampleRate == 22050) {
		Prescaler = 21;
		Period = 182;
	}
	if (SampleRate == 32000) {
		Prescaler = 21;
		Period = 124;
	}
	if (SampleRate == 44100) {
		Prescaler = 7;
		Period = 272;
		//Prescaler = 21;
		//Period = 90;
	}
	
	RE_Init_TIM6(Prescaler, Period);
	RE_Init_TIM7(Prescaler, Period);
}

uint16_t _numChannels = 0;
uint16_t _sampleRate = 0;
uint16_t _bitsPerSample = 0;
uint32_t _fileSize = 0;
char _wav_filename[256];
uint32_t wavPlayFromPos(char * filename, uint32_t pos) {
	count = 0;
	_buff_count = pos / WAV_BUFFER_LEN;
	buff = 0;

	malloc_wav_buffers();

	strcpy(_wav_filename, filename);
	if (f_open(&_wav_file, filename, FA_READ) != FR_OK) {
		free_wav_buffers();
		return WAV_ERROR;
	}

	if (f_read(&_wav_file, WAV_BUFFER1, 44, &d) != FR_OK) {
		free_wav_buffers();
		return WAV_ERROR;
	}

  char wavStr[] = {'W','A','V','E'};
  for (uint8_t i = 0; i < 4; i++) {
	  if(WAV_BUFFER1[i + 8] != wavStr[i]){
			f_close(&_wav_file);
			return WAV_ERROR;
		}
  }
	_numChannels = WAV_BUFFER1[22];
	_sampleRate = (WAV_BUFFER1[24] << 0) + (WAV_BUFFER1[25] << 8) + (WAV_BUFFER1[26] << 16) + (WAV_BUFFER1[27] << 24);
	_bitsPerSample = WAV_BUFFER1[34];
	_fileSize = (WAV_BUFFER1[40] << 0) + (WAV_BUFFER1[41] << 8) + (WAV_BUFFER1[42] << 16) + (WAV_BUFFER1[43] << 24);

	if (_numChannels != 2) {
		return WAV_ERROR;
	}

	wavState = BUSY;

	f_lseek(&_wav_file, pos);

	initTimers(_sampleRate);

	//f_read(&_wav_file, WAV_BUFFER1, WAV_BUFFER_LEN, &d);//fill first buffer
	HAL_TIM_Base_Start(&htim6);
	HAL_TIM_Base_Start(&htim7);
	HAL_TIM_Base_Start_IT(&htim6);

	return WAV_OK;
}
uint32_t wavPlay(char * filename) {
	count = 0;
	_buff_count = 0;
	buff = 0;

	malloc_wav_buffers();

	strcpy(_wav_filename, filename);
	if (f_open(&_wav_file, filename, FA_READ) != FR_OK) {
		free_wav_buffers();
		return WAV_ERROR;
	}

	if (f_read(&_wav_file, WAV_BUFFER1, 44, &d) != FR_OK) {
		free_wav_buffers();
		return WAV_ERROR;
	}
	
  char wavStr[] = {'W','A','V','E'};
  for (uint8_t i = 0; i < 4; i++) {
	  if(WAV_BUFFER1[i + 8] != wavStr[i]){
			f_close(&_wav_file);
			return WAV_ERROR;
		}
  }
	_numChannels = WAV_BUFFER1[22];
	_sampleRate = (WAV_BUFFER1[24] << 0) + (WAV_BUFFER1[25] << 8) + (WAV_BUFFER1[26] << 16) + (WAV_BUFFER1[27] << 24);
	_bitsPerSample = WAV_BUFFER1[34];
	_fileSize = (WAV_BUFFER1[40] << 0) + (WAV_BUFFER1[41] << 8) + (WAV_BUFFER1[42] << 16) + (WAV_BUFFER1[43] << 24);
	
	if (_numChannels != 2) {
		return WAV_ERROR;
	}
	
	wavState = BUSY;

	initTimers(_sampleRate);
	
	//f_read(&_wav_file, WAV_BUFFER1, WAV_BUFFER_LEN, &d);//fill first buffer
	readNextWavBuffer();
	HAL_TIM_Base_Start(&htim6);
	HAL_TIM_Base_Start(&htim7);
	HAL_TIM_Base_Start_IT(&htim6);
	
	return WAV_OK;
}
void wavInfo(uint16_t * Ch, uint16_t * SR, uint16_t * BPS, uint32_t * FS) {
	(*Ch) = _numChannels;
	(*SR) = _sampleRate;
	(*BPS) = _bitsPerSample;
	(*FS) = _fileSize;
}
void wavStop(void) {
	//stop all timers and interupts
	wavState = STOPED;

	HAL_TIM_Base_Stop(&htim6);
	HAL_TIM_Base_Stop(&htim7);
	HAL_TIM_Base_Stop_IT(&htim6);
	HAL_TIM_Base_Stop_IT(&htim7);
	
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
	HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);

	f_close(&_wav_file);

	free_wav_buffers();
}
void wavPause() {
	if (wavState != PAUSED) {
		wavState = PAUSED;

		//HAL_TIM_Base_Stop(&htim6);
		//HAL_TIM_Base_Stop(&htim7);
		HAL_TIM_Base_Stop_IT(&htim6);
		HAL_TIM_Base_Stop_IT(&htim7);

		HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
		HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_2);
	}
}
void wavResume() {
	if (wavState != BUSY) {
		wavState = BUSY;

		count = 0;
		_buff_count++;

		//HAL_TIM_Base_Start(&htim6);
		//HAL_TIM_Base_Start(&htim7);
		HAL_TIM_Base_Start_IT(&htim6);
		HAL_TIM_Base_Start_IT(&htim7);
	}
}



void malloc_wav_buffers() {
	//_wav_filename = (char*)malloc(256 * sizeof(char));

	WAV_BUFFER1 = (uint8_t*)malloc(WAV_BUFFER_LEN * sizeof(uint8_t));
	WAV_BUFFER2 = (uint8_t*)malloc(WAV_BUFFER_LEN * sizeof(uint8_t));

	WAV_BUFFER1C1 = (uint8_t*)malloc((WAV_BUFFER_LEN / 2) * sizeof(uint8_t));
	WAV_BUFFER1C2 = (uint8_t*)malloc((WAV_BUFFER_LEN / 2) * sizeof(uint8_t));

	WAV_BUFFER2C1 = (uint8_t*)malloc((WAV_BUFFER_LEN / 2) * sizeof(uint8_t));
	WAV_BUFFER2C2 = (uint8_t*)malloc((WAV_BUFFER_LEN / 2) * sizeof(uint8_t));

	if (WAV_BUFFER1 == NULL || WAV_BUFFER2 == NULL || WAV_BUFFER1C1 == NULL || WAV_BUFFER1C2 == NULL || WAV_BUFFER2C1 == NULL || WAV_BUFFER2C2 == NULL) {
		Error_Handler();
	}
}
void free_wav_buffers() {
	//free(_wav_filename);

	free(WAV_BUFFER1);
	free(WAV_BUFFER2);

	free(WAV_BUFFER1C1);
	free(WAV_BUFFER1C2);

	free(WAV_BUFFER2C1);
	free(WAV_BUFFER2C2);
}





















#endif
