/*
Steganography Project Documentation

Introduction :
 This project implements a Steganography Encoder/Decoder in C.It hides a secret file inside a BMP image using Least Significant
Bit (LSB) encoding, and later extracts the hidden file back.Concealing secret data (like text, images, audio, or video) inside a cover medium (such as an image, audio file, or video).
Goal is to make the hidden message invisible to unintended observers.Cryptography scrambles a message so it can’t be read without a key.
Steganography hides the existence of the message itself.

Suppose if we want to hide the word "HELLO" inside an image:
Each pixel in an image has values like (R, G, B).
By slightly altering the least significant bit of each color channel, you can encode the binary representation of "HELLO".
the image looks unchanged, but the hidden message can be extracted with the right algorithm.

Usage:

Encoding:
    ./program -e <src.bmp> <secret.txt> <stego.bmp>

Decoding:
    ./program -d <stego.bmp> <output>

 Modules :
- types.h   : Defines enums (Status, OperationType) and typedefs.
- common.h  : Defines constants like MAGIC_STRING.
- encode.c  : Implements encoding functions:
              * get_image_size_for_bmp
              * get_file_size
              * read_and_validate_encode_args
              * open_files
              * check_capacity
              * copy_bmp_header
              * encode_magic_string
              * encode_secret_file_extn_size
              * encode_secret_file_extn
              * encode_secret_file_size
              * encode_secret_file_data
              * copy_remaining_img_data
              * do_encoding
- decode.c  : Implements decoding functions:
              * read_and_validate_decode_args
              * open_decode_files
              * decode_magic_string
              * decode_secret_file_extn_size
              * decode_secret_file_extn
              * decode_secret_file_size
              * decode_secret_file_data
              * do_decoding
- main.c    : Entry point. Decides whether to encode or decode
              based on command-line arguments.

Workflow :

Encoding:
    - Validate arguments
    - Open files
    - Check image capacity
    - Copy BMP header
    - Encode magic string, extension size, extension, file size, data
    - Copy remaining image data

Decoding:
    - Validate arguments
    - Open stego image
    - Skip BMP header
    - Decode magic string, extension size, extension, file size, data
    - Save recovered file

Key Points :

- Works only with BMP images
- Uses LSB encoding
- Includes capacity check
- Uses MAGIC_STRING to verify hidden data
- Provides INFO/ERROR messages for clarity


*/

#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"

/* Function Prototypes */
OperationType check_operation_type(char *symbol);

int main(int argc, char *argv[])
{
    /* Check minimum arguments */  
    if (argc < 3) // need at least operation type and input files
    {
        printf("ERROR: Insufficient arguments\n");
        return e_failure; // return failure if not enough arguments
    }

    OperationType op = check_operation_type(argv[1]);   // determine operation type

    /* Encoding */
    if (op == e_encode) // if user selected encoding
    {
        if (argc < 4) // encoding requires at least 4 arguments
        {
            printf("ERROR: Insufficient arguments for encoding\n");
            return e_failure; // return failure if insufficient args
        }

        printf("INFO: User selected Encoding\n");

        EncodeInfo encInfo; // structure to hold encoding info

        if (read_and_validate_encode_args(argv, &encInfo) == e_success) // validate arguments
        {
            printf("INFO: Read and Validate Successful\n");

            if (do_encoding(&encInfo) == e_success) // perform encoding
            {
                printf("INFO: Encoding completed successfully\n");
            }
            else
            {
                printf("ERROR: Encoding failed\n");
                return e_failure; // return failure if encoding fails
            }
        }
        else
        {
            printf("ERROR: Invalid command line arguments\n");
            return e_failure; // return failure if validation fails
        }
    }

    /* Decoding */
    else if (op == e_decode) // if user selected decoding
    {
        printf("INFO: User selected Decoding\n");

        DecodeInfo decInfo; // structure to hold decoding info

        if (read_and_validate_decode_args(argv, &decInfo) == e_success) // validate arguments
        {
            printf("INFO: Read and Validate Successful\n");

            if (do_decoding(&decInfo) == e_success) // perform decoding
            {
                printf("INFO: Decoding completed successfully\n");
            }
            else
            {
                printf("ERROR: Decoding failed\n");
                return e_failure; // return failure if decoding fails
            }
        }
        else
        {
            printf("ERROR: Invalid command line arguments\n");
            return e_failure; // return failure if validation fails
        }
    }

    /* Unsupported */
    else // if operation type is unsupported
    {
        printf("ERROR: Unsupported Operation\n");
        return e_failure; // return failure for unsupported operation
    }

    return 0; // return success if program completes
}

/* Check Operation Type */
OperationType check_operation_type(char *symbol)
{
    if (strcmp(symbol, "-e") == 0) // check for encoding flag
    {
        return e_encode;
    }
    else if (strcmp(symbol, "-d") == 0) // check for decoding flag
    {
        return e_decode;
    }
    else
    {
        return e_unsupported; // return unsupported if neither
    }
}

