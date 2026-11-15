#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include <string.h>

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height; 
    fseek(fptr_image, 18, SEEK_SET); // Seek to 18th byte

    fread(&width, sizeof(int), 1, fptr_image); // Read the width (an int)
    printf("width = %u\n", width);

    fread(&height, sizeof(int), 1, fptr_image);  // Read the height (an int)
    printf("height = %u\n", height);

    return width * height * 3;  // Return image capacity
}


uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END); // Move to end of file
    uint size = ftell(fptr);  // Get file size
    rewind(fptr);             // Reset pointer

    return size;
}


/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */

Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    char *ptr;

    ptr = strstr(argv[2], ".bmp"); //Check source image 
    if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
    {
        encInfo->src_image_fname = argv[2]; // Store source image name
    }
    else
    {
        printf("ERROR: Source image should be .bmp\n");
        return e_failure; // Return failure if not bmp
    }

   ptr = strrchr(argv[3], '.'); // Find extension in secret file
      if(ptr != NULL)
      {
         encInfo->secret_fname = argv[3]; // Store secret file name
      }
      else
      {
         printf("ERROR: Secret file has no extension\n");
         return e_failure; // Return failure if no extension
      }

    if(argv[4] != NULL) //Destination image
    {
        ptr = strstr(argv[4], ".bmp"); // Check destination extension
        if(ptr != NULL && strcmp(ptr, ".bmp") == 0)
        {
            encInfo->dest_image_fname = argv[4]; // Store destination image name
        }
        else
        {
            printf("ERROR: Output image should be .bmp\n");
            return e_failure; // Return failure if not bmp
        }
    }
    else
    {
        encInfo->dest_image_fname = "default.bmp"; // Default output name
    }

    return e_success; // Return success if all validations pass
}


Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r"); // Open source BMP
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen"); // Print system error
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname); // Custom error

        return e_failure; // Return failure if source not opened
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r"); // Open secret file
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen"); // Print system error
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname); // Custom error

        return e_failure; // Return failure if secret not opened
    }

    // destination Image file
    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "w"); // Open destination BMP
    // Do Error handling
    if (encInfo->fptr_dest_image == NULL)
    {
        perror("fopen"); // Print system error
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname); // Custom error

        return e_failure; // Return failure if destination not opened
    }

   
    return e_success;   // No failure return e_success
}

Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity =
        get_image_size_for_bmp(encInfo->fptr_src_image); // Get BMP capacity

    encInfo->size_secret_file =
        get_file_size(encInfo->fptr_secret); // Get secret file size

    if(encInfo->image_capacity >
        (16 + 32 + 32 + 32 + (encInfo->size_secret_file * 8))) // Check required bits
    {
        return e_success; // Return success if capacity sufficient
    }
    else
    {
        printf("ERROR: Image does not have enough capacity\n");
        return e_failure; // Return failure if capacity insufficient
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
   
    rewind(fptr_src_image); // Reset source pointer

    char image_buffer[54]; // BMP header buffer

    fread(image_buffer, 54, 1, fptr_src_image); // Read header
    fwrite(image_buffer, 54, 1, fptr_dest_image); // Write header
    if ((ftell(fptr_src_image) == 54) && (ftell(fptr_dest_image) == 54)) // Verify positions
    {
        return e_success; // Return success if header copied
    }
    else
    {
        return e_failure; // Return failure if mismatch
    }
}

Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    char imageBuffer[8];

    int len = strlen(magic_string); // Length of magic string

    for(int i = 0; i < len; i++)
    {
        
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);  // Read 8 bytes from source image 
        encode_byte_to_lsb(magic_string[i], imageBuffer);   // Encode one character 
        fwrite(imageBuffer, 8, 1, encInfo->fptr_dest_image); // Write encoded bytes
    }

    /* Verify both file pointers advanced correctly */
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        return e_success; // Return success if positions match
    }
    else
    {
        return e_failure; // Return failure if mismatch
    }
}


Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    char imageBuffer[32];

    /* Read 32 bytes from source image */
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);

    /* Encode extension size */
    encode_size_to_lsb(size, imageBuffer);

    /* Write modified bytes to destination image */
    fwrite(imageBuffer, 32, 1, encInfo->fptr_dest_image);

    /* Check whether both file pointers are at the same position */
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        return e_success; //return e_success
    }
    else
    {
        return e_failure;//return e_failure
    }
}

Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    char imageBuffer[8];

    int len = strlen(file_extn);

    for(int i = 0; i < len; i++)
    {
        /* Read 8 bytes from source image */
        fread(imageBuffer, 8, 1, encInfo->fptr_src_image);

        /* Encode one extension character */
        encode_byte_to_lsb(file_extn[i], imageBuffer);

        /* Write modified bytes */
        fwrite(imageBuffer, 8, 1, encInfo->fptr_dest_image);
    }

    /* Verify both file pointers are at the same position */
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    char imageBuffer[32];

    /* Read 32 bytes from source image */
    fread(imageBuffer, 32, 1, encInfo->fptr_src_image);

    /* Encode secret file size */
    encode_size_to_lsb(file_size, imageBuffer);

    /* Write modified bytes to destination image */
    fwrite(imageBuffer, 32, 1, encInfo->fptr_dest_image);

    /* Verify both file pointers advanced equally */
    if (ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    /* Move to the beginning of the secret file */
    rewind(encInfo->fptr_secret);

    /* Read the complete secret file */
    char file_data[encInfo->size_secret_file];

    fread(file_data, encInfo->size_secret_file,  1, encInfo->fptr_secret);

    char image_buffer[8];

    for(int i = 0; i < encInfo->size_secret_file; i++)
    {
        /* Read 8 bytes from source image */
        fread(image_buffer, 8, 1, encInfo->fptr_src_image);

        /* Encode one character */
        encode_byte_to_lsb(file_data[i], image_buffer);

        /* Write encoded bytes */
        fwrite(image_buffer, 8, 1, encInfo->fptr_dest_image);
    }

    /* Verify file pointer positions */
    if(ftell(encInfo->fptr_src_image) == ftell(encInfo->fptr_dest_image))
    {
        return e_success;
    }

    return e_failure;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    int bit;

    for(int i = 0; i < 8; i++)
    {
        /* Extract one bit from the character (MSB to LSB) */
        bit = (data >> (7 - i)) & 1;

         /* Clear the LSB */
        image_buffer[i] &= ~1;

         /* Set the bit */
        image_buffer[i] |= bit;
    }

    return e_success;
}

Status encode_size_to_lsb(int size, char *imageBuffer)
{
    int bit;

    for(int i = 0; i < 32; i++)
    {
        /* Extract one bit from size (MSB to LSB) */
        bit = (size >> (31 - i)) & 1;

        /* Clear the LSB */
        imageBuffer[i] &= ~1;

        /* Set the bit */
        imageBuffer[i] |= bit;
    }

    return e_success;
}

Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char buffer[1024]; // buffer to hold chunks of image data
    size_t bytes_read; // variable to store number of bytes read

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0) // read until EOF
    {
        fwrite(buffer, 1, bytes_read, fptr_dest); // write the read bytes to destination
    }

    return e_success; // return success after copying
}

Status do_encoding(EncodeInfo *encInfo)
{
    /* Open files */
    if (open_files(encInfo) == e_success) // try opening files
    {
        printf("INFO: Files opened successfully\n");
    }
    else
    {
        printf("ERROR: Unable to open files\n");
        return e_failure; // return failure if files not opened
    }

    /* Check image capacity */
    if (check_capacity(encInfo) == e_success) // check if image can hold secret
    {
        printf("INFO: Image has enough capacity\n");
    }
    else
    {
        printf("ERROR: Image does not have enough capacity\n");
        return e_failure; // return failure if capacity insufficient
    }

    /* Copy BMP header */
    if (copy_bmp_header(encInfo->fptr_src_image,
                        encInfo->fptr_dest_image) == e_success) // copy header
    {
        printf("INFO: BMP header copied\n");
    }
    else
    {
        printf("ERROR: Failed to copy BMP header\n");
        return e_failure; // return failure if header copy fails
    }

    /* Encode magic string */
    if (encode_magic_string(MAGIC_STRING, encInfo) == e_success) // encode magic string
    {
        printf("INFO: Magic string encoded\n");
    }
    else
    {
        printf("ERROR: Magic string encoding failed\n");
        return e_failure; // return failure if magic string fails
    }

    /* Store secret file extension */
    char *ptr = strrchr(encInfo->secret_fname, '.'); // find extension in secret file name

    if (ptr != NULL)
    {
      strcpy(encInfo->extn_secret_file, ptr); // copy extension into struct
    }

    /* Encode extension size */
    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),
                                     encInfo) == e_success) // encode extension size
    {
        printf("INFO: Extension size encoded\n");
    }
    else
    {
        printf("ERROR: Extension size encoding failed\n");
        return e_failure; // return failure if extension size fails
    }

    /* Encode extension */
    if (encode_secret_file_extn(encInfo->extn_secret_file,
                                encInfo) == e_success) // encode extension characters
    {
        printf("INFO: Extension encoded\n");
    }
    else
    {
        printf("ERROR: Extension encoding failed\n");
        return e_failure; // return failure if extension fails
    }

    /* Encode secret file size */
    if (encode_secret_file_size(encInfo->size_secret_file,
                                encInfo) == e_success) // encode secret file size
    {
        printf("INFO: Secret file size encoded\n");
    }
    else
    {
        printf("ERROR: Secret file size encoding failed\n");
        return e_failure; // return failure if file size fails
    }

    /* Encode secret file data */
    if (encode_secret_file_data(encInfo) == e_success) // encode secret file contents
    {
        printf("INFO: Secret file data encoded\n");
    }
    else
    {
        printf("ERROR: Secret file data encoding failed\n");
        return e_failure; // return failure if data encoding fails
    }

    /* Copy remaining image data */
    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_dest_image) == e_success) // copy rest of image
    {
        printf("INFO: Remaining image copied\n");
    }
    else
    {
        printf("ERROR: Failed to copy remaining image data\n");
        return e_failure; // return failure if remaining copy fails
    }

    return e_success; // return success if all steps completed
}

