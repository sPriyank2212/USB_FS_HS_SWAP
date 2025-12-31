/*
 * File_Handling_RTOS.c
 *
 *  Created on: 26-June-2020
 *      Author: Controllerstech.com
 */

#include "File_Handling.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"


extern UART_HandleTypeDef huart1;
#define UART &huart1  // MSC (Pendrive) operations output to UART1



/* =============================>>>>>>>> NO CHANGES AFTER THIS LINE =====================================>>>>>>> */



extern char USBHPath[4];   /* USBH logical drive path */
extern FATFS USBHFatFS;    /* File system object for USBH logical drive */
extern FIL USBHFile;       /* File object for USBH */

FILINFO USBHfno;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pUSBHFatFS;
DWORD fre_clust;
uint32_t total, free_space;


void Send_Uart (char *string)
{
	HAL_UART_Transmit(UART, (uint8_t *)string, strlen (string), HAL_MAX_DELAY);
}



void Mount_USB (void)
{
	fresult = f_mount(&USBHFatFS, USBHPath, 1);
	if (fresult != FR_OK) Send_Uart ("ERROR!!! in mounting USB ...\n\n");
	else Send_Uart("USB mounted successfully...\n");
}

void Unmount_USB (void)
{
	fresult = f_mount(NULL, USBHPath, 1);
	if (fresult == FR_OK) Send_Uart ("USB UNMOUNTED successfully...\n\n\n");
	else Send_Uart("ERROR!!! in UNMOUNTING USB \n\n\n");
}

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_USB (char* pat)
{
    DIR dir;
    UINT i;
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s",pat);

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &USBHfno);                   /* Read a directory item */
            if (fresult != FR_OK || USBHfno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (USBHfno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname))) continue;
            	if (!(strcmp("System Volume Information", USBHfno.fname))) continue;
            	char *buf = malloc(30*sizeof(char));
            	sprintf (buf, "Dir: %s\r\n", USBHfno.fname);
            	Send_Uart(buf);
            	free(buf);
                i = strlen(path);
                sprintf(&path[i], "/%s", USBHfno.fname);
                fresult = Scan_USB(path);                     /* Enter the directory */
                if (fresult != FR_OK) break;
                path[i] = 0;
            }
            else
            {   /* It is a file. */
           	   char *buf = malloc(30*sizeof(char));
               sprintf(buf,"File: %s/%s\n", path, USBHfno.fname);
               Send_Uart(buf);
               free(buf);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}

/* Only supports removing files from home directory */
FRESULT Format_USB (void)
{
    DIR dir;
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s","/");

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &USBHfno);                   /* Read a directory item */
            if (fresult != FR_OK || USBHfno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (USBHfno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", USBHfno.fname))) continue;
            	if (!(strcmp("System Volume Information", USBHfno.fname))) continue;
            	fresult = f_unlink(USBHfno.fname);
            	if (fresult == FR_DENIED) continue;
            }
            else
            {   /* It is a file. */
               fresult = f_unlink(USBHfno.fname);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}




FRESULT Write_File (char *name, char *data)
{

	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &USBHfno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\n\n", name);
		Send_Uart (buf);
	    free(buf);
	    return fresult;
	}

	else
	{
	    /* Create a file with read write access and open it */
	    fresult = f_open(&USBHFile, name, FA_OPEN_EXISTING | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\n\n", fresult, name);
	    	Send_Uart(buf);
	        free(buf);
	        return fresult;
	    }

	    else
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "Opening file-->  *%s*  To WRITE data in it\n", name);
	    	Send_Uart(buf);
	        free(buf);

	    	fresult = f_write(&USBHFile, data, strlen(data), &bw);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d while writing to the FILE *%s*\n\n", fresult, name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}

	    	/* Close file */
	    	fresult = f_close(&USBHFile);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d in closing file *%s* after writing it\n\n", fresult, name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}
	    	else
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "File *%s* is WRITTEN and CLOSED successfully\n\n", name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}
	    }
	    return fresult;
	}
}

FRESULT Read_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &USBHfno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERRROR!!! *%s* does not exists\n\n", name);
		Send_Uart (buf);
		free(buf);
	    return fresult;
	}

	else
	{
		/* Open file to read */
		fresult = f_open(&USBHFile, name, FA_READ);

		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in opening file *%s*\n\n", fresult, name);
		    Send_Uart(buf);
		    free(buf);
		    return fresult;
		}

		/* Read data from the file
		* see the function details for the arguments */

    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "Opening file-->  *%s*  To READ data from it\n", name);
    	Send_Uart(buf);
        free(buf);

		char *buffer = malloc(sizeof(f_size(&USBHFile)));
		fresult = f_read (&USBHFile, buffer, f_size(&USBHFile), &br);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			free(buffer);
		 	sprintf (buf, "ERROR!!! No. %d in reading file *%s*\n\n", fresult, name);
		  	Send_Uart(buffer);
		  	free(buf);
		}

		else
		{
			Send_Uart(buffer);
			free(buffer);

			/* Close file */
			fresult = f_close(&USBHFile);
			if (fresult != FR_OK)
			{
				char *buf = malloc(100*sizeof(char));
				sprintf (buf, "ERROR!!! No. %d in closing file *%s*\n\n", fresult, name);
				Send_Uart(buf);
				free(buf);
			}
			else
			{
				char *buf = malloc(100*sizeof(char));
				sprintf (buf, "File *%s* CLOSED successfully\n\n", name);
				Send_Uart(buf);
				free(buf);
			}
		}
	    return fresult;
	}
}

FRESULT Create_File (char *name)
{
	fresult = f_stat (name, &USBHfno);
	if (fresult == FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* already exists!!!!\n use Update_File \n\n",name);
		Send_Uart(buf);
		free(buf);
	    return fresult;
	}
	else
	{
		fresult = f_open(&USBHFile, name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in creating file *%s*\n\n", fresult, name);
			Send_Uart(buf);
			free(buf);
		    return fresult;
		}
		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "*%s* created successfully\n Now use Write_File to write data\n",name);
			Send_Uart(buf);
			free(buf);
		}

		fresult = f_close(&USBHFile);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in closing file *%s*\n\n", fresult, name);
			Send_Uart(buf);
			free(buf);
		}
		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "File *%s* CLOSED successfully\n\n", name);
			Send_Uart(buf);
			free(buf);
		}
	}
    return fresult;
}

FRESULT Update_File (char *name, char *data)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &USBHfno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\n\n", name);
		Send_Uart (buf);
		free(buf);
	    return fresult;
	}

	else
	{
		 /* Create a file with read write access and open it */
	    fresult = f_open(&USBHFile, name, FA_OPEN_APPEND | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\n\n", fresult, name);
	    	Send_Uart(buf);
	        free(buf);
	        return fresult;
	    }

    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "Opening file-->  *%s*  To UPDATE data in it\n", name);
    	Send_Uart(buf);
        free(buf);

	    /* Writing text */
	    fresult = f_write(&USBHFile, data, strlen (data), &bw);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in writing file *%s*\n\n", fresult, name);
	    	Send_Uart(buf);
	    	free(buf);
	    }

	    else
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "*%s* UPDATED successfully\n", name);
	    	Send_Uart(buf);
	    	free(buf);
	    }

	    /* Close file */
	    fresult = f_close(&USBHFile);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in closing file *%s*\n\n", fresult, name);
	    	Send_Uart(buf);
	    	free(buf);
	    }
	    else
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "File *%s* CLOSED successfully\n\n", name);
	    	Send_Uart(buf);
	    	free(buf);
	     }
	}
    return fresult;
}

FRESULT Remove_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &USBHfno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\n\n", name);
		Send_Uart (buf);
		free(buf);
		return fresult;
	}

	else
	{
		fresult = f_unlink (name);
		if (fresult == FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "*%s* has been removed successfully\n\n", name);
			Send_Uart (buf);
			free(buf);
		}

		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in removing *%s*\n\n",fresult, name);
			Send_Uart (buf);
			free(buf);
		}
	}
	return fresult;
}

FRESULT Create_Dir (char *name)
{
    fresult = f_mkdir(name);
    if (fresult == FR_OK)
    {
    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "*%s* has been created successfully\n\n", name);
    	Send_Uart (buf);
    	free(buf);
    }
    else
    {
    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "ERROR No. %d in creating directory *%s*\n\n", fresult,name);
    	Send_Uart(buf);
    	free(buf);
    }
    return fresult;
}

void Check_USB_Details (void)
{
    /* Check free space */
    f_getfree("", &fre_clust, &pUSBHFatFS);

    total = (uint32_t)((pUSBHFatFS->n_fatent - 2) * pUSBHFatFS->csize * 0.5);
    char *buf = malloc(30*sizeof(char));
    sprintf (buf, "USB  Total Size: \t%lu\n",total);
    Send_Uart(buf);
    free(buf);
    free_space = (uint32_t)(fre_clust * pUSBHFatFS->csize * 0.5);
    buf = malloc(30*sizeof(char));
    sprintf (buf, "USB Free Space: \t%lu\n",free_space);
    Send_Uart(buf);
    free(buf);
}

/* ===== BENCHMARK FUNCTIONS FOR PENDRIVE SPEED TEST ===== */

/* Write benchmark: Create 5MB file with dummy data and measure time */
void Benchmark_Write_Test(void)
{
	char *buf = malloc(150*sizeof(char));
	uint32_t start_time, end_time, elapsed_time;
	uint32_t file_size = 5 * 1024 * 1024;  // 5 MB
	uint32_t chunk_size = 4096;  // 4KB chunks
	uint32_t chunks_written = 0;

	// LED: Start blinking (will be handled in callback)

	sprintf(buf, "\r\n[BENCHMARK] Starting WRITE test (5MB file)...\r\n");
	Send_Uart(buf);

	// Create/Open file for writing
	fresult = f_open(&USBHFile, "benchmark.bin", FA_CREATE_ALWAYS | FA_WRITE);
	if (fresult != FR_OK) {
		sprintf(buf, "[ERROR] Failed to create benchmark file! Error: %d\r\n", fresult);
		Send_Uart(buf);
		free(buf);
		return;
	}

	// Allocate buffer for dummy data
	uint8_t *write_buffer = malloc(chunk_size);
	if (write_buffer == NULL) {
		sprintf(buf, "[ERROR] Memory allocation failed!\r\n");
		Send_Uart(buf);
		f_close(&USBHFile);
		free(buf);
		return;
	}

	// Fill buffer with pattern (0x55 alternating with chunk number)
	for (uint32_t i = 0; i < chunk_size; i++) {
		write_buffer[i] = 0x55;
	}

	// Start timing
	start_time = HAL_GetTick();

	// Write 5MB in chunks
	uint32_t bytes_to_write = file_size;
	while (bytes_to_write > 0) {
		uint32_t write_size = (bytes_to_write > chunk_size) ? chunk_size : bytes_to_write;

		fresult = f_write(&USBHFile, write_buffer, write_size, &bw);
		if (fresult != FR_OK || bw != write_size) {
			sprintf(buf, "[ERROR] Write failed at chunk %lu! Error: %d\r\n", chunks_written, fresult);
			Send_Uart(buf);
			break;
		}

		bytes_to_write -= write_size;
		chunks_written++;

		// Toggle LED every 256KB
		if (chunks_written % 64 == 0) {
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		}
	}

	// End timing
	end_time = HAL_GetTick();
	elapsed_time = end_time - start_time;

	// Close file
	f_close(&USBHFile);
	free(write_buffer);

	// Calculate speed
	float speed_kbps = (float)(file_size / 1024) / ((float)elapsed_time / 1000.0f);

	sprintf(buf, "[BENCHMARK] WRITE Complete!\r\n");
	Send_Uart(buf);
	sprintf(buf, "  File Size: 5 MB (%lu bytes)\r\n", file_size);
	Send_Uart(buf);
	sprintf(buf, "  Time: %lu ms\r\n", elapsed_time);
	Send_Uart(buf);
	sprintf(buf, "  Speed: %.2f KB/s\r\n", speed_kbps);
	Send_Uart(buf);
	sprintf(buf, "  Chunks written: %lu\r\n\r\n", chunks_written);
	Send_Uart(buf);

	free(buf);
}

/* Read benchmark: Read the 5MB file and measure time */
void Benchmark_Read_Test(void)
{
	char *buf = malloc(150*sizeof(char));
	uint32_t start_time, end_time, elapsed_time;
	uint32_t chunk_size = 4096;  // 4KB chunks
	uint32_t chunks_read = 0;
	uint32_t total_bytes_read = 0;

	sprintf(buf, "[BENCHMARK] Starting READ test...\r\n");
	Send_Uart(buf);

	// Check if file exists
	fresult = f_stat("benchmark.bin", &USBHfno);
	if (fresult != FR_OK) {
		sprintf(buf, "[ERROR] Benchmark file not found!\r\n");
		Send_Uart(buf);
		free(buf);
		return;
	}

	// Open file for reading
	fresult = f_open(&USBHFile, "benchmark.bin", FA_READ);
	if (fresult != FR_OK) {
		sprintf(buf, "[ERROR] Failed to open benchmark file! Error: %d\r\n", fresult);
		Send_Uart(buf);
		free(buf);
		return;
	}

	uint32_t file_size = f_size(&USBHFile);

	// Allocate read buffer
	uint8_t *read_buffer = malloc(chunk_size);
	if (read_buffer == NULL) {
		sprintf(buf, "[ERROR] Memory allocation failed!\r\n");
		Send_Uart(buf);
		f_close(&USBHFile);
		free(buf);
		return;
	}

	// Start timing
	start_time = HAL_GetTick();

	// Read entire file in chunks
	while (total_bytes_read < file_size) {
		fresult = f_read(&USBHFile, read_buffer, chunk_size, &br);
		if (fresult != FR_OK) {
			sprintf(buf, "[ERROR] Read failed at chunk %lu! Error: %d\r\n", chunks_read, fresult);
			Send_Uart(buf);
			break;
		}

		if (br == 0) break;  // End of file

		total_bytes_read += br;
		chunks_read++;

		// Toggle LED every 256KB
		if (chunks_read % 64 == 0) {
			HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_0);
		}
	}

	// End timing
	end_time = HAL_GetTick();
	elapsed_time = end_time - start_time;

	// Close file
	f_close(&USBHFile);
	free(read_buffer);

	// Calculate speed
	float speed_kbps = (float)(total_bytes_read / 1024) / ((float)elapsed_time / 1000.0f);

	sprintf(buf, "[BENCHMARK] READ Complete!\r\n");
	Send_Uart(buf);
	sprintf(buf, "  File Size: %lu bytes (%.2f MB)\r\n", total_bytes_read, (float)total_bytes_read / (1024.0f * 1024.0f));
	Send_Uart(buf);
	sprintf(buf, "  Time: %lu ms\r\n", elapsed_time);
	Send_Uart(buf);
	sprintf(buf, "  Speed: %.2f KB/s\r\n", speed_kbps);
	Send_Uart(buf);
	sprintf(buf, "  Chunks read: %lu\r\n\r\n", chunks_read);
	Send_Uart(buf);

	free(buf);
}

