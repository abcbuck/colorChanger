# What is this

This is a program that swaps the RGB values of an input image in every way possible. The RGB values of every pixel are changed to the six permutations RGB, RBG, BRG, BGR, GBR, GRB. For example, a pixel with the RGB value (123, 45, 67) under the permutation RGB→BGR will change to (67, 45, 123), as the values for R and B are swapped.

Because you only get five new images this way, I also made the program add all images where any of the color values are inverted so that 3!*2**3-1=47 new images are created.

# Usage

1. Transform your image file to a 24-bit .bmp image by opening it in a graphics editor (e.g. Microsoft Paint) and saving it as a .bmp file, selecting 24-bit as format.
2. Run `colorChanger [file]`, where [file] is your 24-bit .bmp image.
