#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>   
#include <string.h>
#include <math.h>
#include <inttypes.h>
#include "./constants.h"

// Constants needed for the wav header
#define BITS_PR_SAMPLE  16 * CHANNELS
#define BYTE_RATE  SAMPLE_RATE * CHANNELS * BITS_PR_SAMPLE/8
#define BLOCK_ALIGN CHANNELS * BITS_PR_SAMPLE / 8
// Using max 9999 output files, "output" (6 characters) + 9999 (4 characters) + ".waw" (4 charactters)
#define FILENAME_LEN 14

// File pointer is used by all functions
FILE *output;

void start_record() {

    printf("Start recording!\n");

    // Check if file exist, if it does, 
    // add number to the end
    char filename[FILENAME_LEN] = "output.wav";
    // stat buffer is used to check if filename alredy exists
    struct stat buffer;
    int counter = 1;
    while(stat(filename, &buffer) == 0 && counter <= 9999) {
        // make sure we start from the beginning
        filename[6] = '\0';
        // Number of digits in counter + 1 for trailing zero.
        int postfix_len = (int) floor(log10(counter)) + 2;
        char postfix[postfix_len];
        sprintf(postfix, "%d", counter);
        // Add counter to filename
        strncat(filename,postfix,postfix_len);
        strncat(filename, ".wav", 5);
        // Increment counter
        counter += 1;
    }

    // Header for the waw file
    char header[44] = {
         'R','I','F','F', // ChunkID
         0x00, 0x00, 0x00, 0x00, // ChunkSize
         'W', 'A', 'V', 'E', // Format
         'f','m','t', ' ', // SubchunkID
         0x10, 0x00, 0x00, 0x00,// Subchunk1Size
         0x01, 0x00, // AudioFormat
         0x01, 0x00, // NumChannels
         SAMPLE_RATE & 0xFF, (SAMPLE_RATE >> 8) & 0xFF, (SAMPLE_RATE >> 16) & 0xFF, (SAMPLE_RATE >> 24) & 0xFF, // SampleRate
         BYTE_RATE  & 0xFF, (BYTE_RATE >> 8) & 0xFF, (BYTE_RATE >> 16) & 0xFF, (BYTE_RATE >> 24) & 0xFF, // ByteRate
         (BLOCK_ALIGN) & 0xFF, (BLOCK_ALIGN >> 8) & 0xFF, // BlockAlign
         BITS_PR_SAMPLE & 0xFF, (BITS_PR_SAMPLE >> 8) & 0xFF, // BitsPrSample
        'd','a','t','a', // Subchunk2ID
        0x00, 0x00, 0x00, 0x00 // Subchunk2Size
    };
    output = fopen(filename, "wb");
    // write header to file
    fwrite(header, sizeof(char), 44, output);
}

void stop_record() {


    printf("Stop recording\n");
    // Update the chunksize field in header
    int32_t len = ftell(output) - 8;
    fseek(output, 4, SEEK_SET);
    fwrite(&len, sizeof(int32_t),1, output);

    // Update the Subchunk2size in the header
    fseek(output, 0, SEEK_END);
    int32_t subchunksize = ftell(output) - 44;
    fseek(output, 40, SEEK_SET);
    fwrite(&subchunksize, sizeof(int32_t), 1, output);

    // Close file
    fclose(output);
}

void record_sample(int16_t sample) {
    fwrite(&sample, sizeof(int16_t), 1, output);

}

