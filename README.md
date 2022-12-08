# Image-to-Pixel-Art
This program turns any image into pixel art with a single click on the keyboard!

## DESCRIPTION
This program uses the OpenImageIO API to Read and Write Images,
and OpenGL/GLUT to display them.

Using various keyboard inputs, you can pixelate an image until it becomes
one singular pixel, write the image to a file, or reload the image back to
its original form. The program will first load and display an input image
from an image file.

NOTE: This program currently works best with images that have dimensions
that are 3^n px wide/tall. The sample images included meet this requirement.
See FUTURE IMPLEMENTATIONS for more details.

## EXAMPLES

### mario_characters.png
![](https://github.com/skylarhubb/Image-to-Pixel-Art/blob/main/examples/gifs/mario%20characters.gif)

### me.png
![](https://github.com/skylarhubb/Image-to-Pixel-Art/blob/main/examples/gifs/me.gif)

### tigerpaw.png (showcasing the color inversion as well!)
![](https://github.com/skylarhubb/Image-to-Pixel-Art/blob/main/examples/gifs/tiger%20paw%20(inversion).gif)

## DEPLOYMENT INSTRUCTIONS
First, ensure pixelate.cpp and the Makefile are in a directory together, along with any images you want to use.


Type 'make' to compile the program.

  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;./pixelate input.img : Displays the image in a resized window.
  

Pressing the following keys will perform the following commands:

  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'w' write the framebuffer to a file
  
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'i' to invert the colors of the image file
  
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'p' pixelates the image. The more times you press, the more pixelated the image becomes.
  
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'r' reloads the image back to its original form.
  
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;'q' or esc to exit the program
  


## KNOWN PROBLEMS
None!


## FUTURE IMPLEMENTATIONS
As mentioned earlier, this program currently works best with images that have dimensions
that are 3^n px wide/tall. With images that do not meet that requirement, there
is a portion on the top and right edge of the image that will not get pixelated the more
times the pixelation key is pressed. Future implementations of this program
will fix this issue.
