#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "common.h"
#include "types.h"

/* Read and validate command line arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    char *ptr;   

    /* Validate stego image extension */
    ptr = strstr(argv[2], ".bmp");     
    if (ptr != NULL && strcmp(ptr, ".bmp") == 0)     
    {
        decInfo->stego_image_fname = argv[2];   // store stego image filename
    }
    else
    {
        printf("ERROR: Invalid Stego Image\n");   
        return e_failure;    
    }

    /* Output file name (default = "decoded") */
    if (argv[3] != NULL)   
    {
        decInfo->output_fname = argv[3];   // user‑provided output filename
    }
    else
    {
        decInfo->output_fname = "decoded";   // fallback default
    }

    return e_success;
}

/* Open stego image file */
Status open_decode_files(DecodeInfo *decInfo)
{
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r"); // open in read mode
    if (decInfo->fptr_stego_image == NULL)      
    {
        perror("fopen");       
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);                  
        return e_failure;
    }

    /* Output file will be opened later after decoding extension */
    return e_success;    
}

/* Decode one byte from 8 LSBs */
Status decode_byte_from_lsb(char *data, char *image_buffer)
{
    *data = 0;   // reset data
    for (int i = 0; i < 8; i++)
    {
        *data = *data << 1;              // shift left
        *data = *data | (image_buffer[i] & 1); // add LSB bit
    }
    return e_success;
}

/* Decode integer size from 32 LSBs */
Status decode_size_from_lsb(int *size, char *imageBuffer)
{
    *size = 0;   // reset size
    for (int i = 0; i < 32; i++)
    {
        *size = *size << 1;              // shift left
        *size = *size | (imageBuffer[i] & 1); // add LSB bit
    }
    return e_success;
}

/* Decode magic string */
Status decode_magic_string(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char magic_str[strlen(MAGIC_STRING) + 1]; // buffer for decoded string

    for (int i = 0; i < strlen(MAGIC_STRING); i++)
    {
        if (fread(image_buffer, 8, 1, decInfo->fptr_stego_image) != 1) // read 8 bytes
        {
            return e_failure;
        }
        decode_byte_from_lsb(&magic_str[i], image_buffer); // decode one char
    }

    magic_str[strlen(MAGIC_STRING)] = '\0'; // null terminate

    if (strcmp(magic_str, MAGIC_STRING) == 0) // compare with expected
        return e_success;

    return e_failure;
}

/* Decode extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)
{
    char image_buffer[32];    
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image); // read 32 bytes
    decode_size_from_lsb(size, image_buffer);              // decode size
    return e_success;
}

/* Decode extension string */
Status decode_secret_file_extn(char *file_extn, int size, DecodeInfo *decInfo)
{
    char image_buffer[8];
    for (int i = 0; i < size; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image); // read 8 bytes
        decode_byte_from_lsb(&file_extn[i], image_buffer);    // decode one char
    }
    file_extn[size] = '\0'; // null terminate
    strcpy(decInfo->extn_secret_file, file_extn); // store in struct
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(long *file_size, DecodeInfo *decInfo)
{
    char image_buffer[32];
    int size;
    fread(image_buffer, 32, 1, decInfo->fptr_stego_image); // read 32 bytes
    decode_size_from_lsb(&size, image_buffer);             // decode size
    *file_size = size;
    decInfo->size_secret_file = size;                      // store in struct
    return e_success;
}

/* Decode secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char image_buffer[8];
    char ch;
    for(int i = 0; i < decInfo->size_secret_file; i++)
    {
        fread(image_buffer, 8, 1, decInfo->fptr_stego_image); // read 8 bytes
        decode_byte_from_lsb(&ch, image_buffer);              // decode one char
        fputc(ch, decInfo->fptr_output);                      // write to output file
    }
    return e_success;
}

/* Main decoding workflow */
Status do_decoding(DecodeInfo *decInfo)
{
    int extn_size; // variable to hold extension size
    char extn_secret_file[50]; // buffer to hold decoded extension
    long file_size; // variable to hold secret file size
    char filename[50]; // buffer to hold output filename

    /* Step 1: Open stego image */
    if (open_decode_files(decInfo) == e_success) // try opening stego image
        printf("INFO: Files opened successfully\n");
    else
    {
        printf("ERROR: Unable to open files\n");
        return e_failure; // return failure if file not opened
    }

    /* Step 2: Skip BMP header */
    fseek(decInfo->fptr_stego_image, 54, SEEK_SET); // skip first 54 bytes of BMP header

    /* Step 3: Decode magic string */
    if (decode_magic_string(decInfo) == e_success) // decode and verify magic string
        printf("INFO: Magic String Decoded Successfully\n");
    else
    {
        printf("ERROR: Magic String Not Matched\n");
        fclose(decInfo->fptr_stego_image); // close file if mismatch
        return e_failure; // return failure if magic string not matched
    }

    /* Step 4: Decode extension size */
    if (decode_secret_file_extn_size(&extn_size, decInfo) == e_success) // decode extension size
        printf("INFO: Extension Size Decoded Successfully\n");
    else
    {
        printf("ERROR: Extension Size Decoding Failed\n");
        fclose(decInfo->fptr_stego_image); // close file if failed
        return e_failure; // return failure if extension size fails
    }

    /* Step 5: Decode extension */
    if (decode_secret_file_extn(extn_secret_file, extn_size, decInfo) == e_success) // decode extension string
        printf("INFO: Extension Decoded Successfully\n");
    else
    {
        printf("ERROR: Extension Decoding Failed\n");
        fclose(decInfo->fptr_stego_image); // close file if failed
        return e_failure; // return failure if extension decoding fails
    }

    /* Step 6: Create output file */
    strcpy(filename, decInfo->output_fname); // copy base output name
    strcat(filename, decInfo->extn_secret_file); // append decoded extension
    decInfo->fptr_output = fopen(filename, "w"); // open output file
    if (decInfo->fptr_output == NULL)
    {
        perror("fopen"); // print system error
        printf("ERROR: Unable to create output file\n");
        fclose(decInfo->fptr_stego_image); // close stego image
        return e_failure; // return failure if output file not created
    }

    /* Step 7: Decode secret file size */
    if (decode_secret_file_size(&file_size, decInfo) == e_success) // decode secret file size
        printf("INFO: Secret File Size Decoded Successfully\n");
    else
    {
        printf("ERROR: Secret File Size Decoding Failed\n");
        fclose(decInfo->fptr_output); // close output file
        fclose(decInfo->fptr_stego_image); // close stego image
        return e_failure; // return failure if size decoding fails
    }

    /* Step 8: Decode secret file data */
    if (decode_secret_file_data(decInfo) == e_success) // decode secret file contents
        printf("INFO: Secret File Data Decoded Successfully\n");
    else
    {
        printf("ERROR: Secret File Data Decoding Failed\n");
        fclose(decInfo->fptr_output); // close output file
        fclose(decInfo->fptr_stego_image); // close stego image
        return e_failure; // return failure if data decoding fails
    }

    fclose(decInfo->fptr_output); // close output file after success
    fclose(decInfo->fptr_stego_image); // close stego image after success

    return e_success; // return success if all steps completed
}
