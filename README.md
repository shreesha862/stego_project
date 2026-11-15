# LSB Image Steganography

A command-line application written in C that hides secret text files inside `.bmp` images using the Least Significant Bit (LSB) steganography technique and extracts them without visible changes to the image.

## Features
- **Encode Secret Data:** Embeds a secret text file inside a `.bmp` carrier image by modifying the least significant bits of pixel bytes.
- **Capacity Check:** Automatically verifies whether the `.bmp` image has enough pixel capacity to hold the secret file before encoding.
- **Magic String Verification:** Embeds a magic string signature to quickly verify whether an image contains hidden data during decoding.
- **Decode Secret Data:** Extracts the hidden message from a stego `.bmp` image and restores it to an output text file.
- **Command-Line Interface:** Accepts all modes and file paths directly via command-line arguments.

## Tech Stack
- **Language:** C
- **Concepts:** Bitwise Operations, Binary File I/O, BMP Header Parsing, Command-Line Arguments

## Usage
- **Encode a secret file:** `./stego -e <source.bmp> <secret.txt> <stego.bmp>`
- **Decode a stego image:** `./stego -d <stego.bmp> <output.txt>`
