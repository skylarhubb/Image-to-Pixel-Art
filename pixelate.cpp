//   Author: Skylar Hubbarth
// 	 Date: 12/9/2022
//	 CPSC4040 Final Project
// 	 Image to Pixel Art Program
//
//   OpenGL/GLUT Program using OpenImageIO to Read and Write Images, and OpenGL to display
// 	 Using various keyboard inputs, you can pixelate an image until it becomes
// 	 one singular pixel, write the image to a file, or reload the image back to
// 	 its original form. The program will first load and display an input image
//	 from an image file.
//
//   The program responds to the following keyboard commands:
//    w or W: prompt for an output image file name, read the display into a pixmap,
//	    and write from the pixmap to the file.
//
//		p or P: pixelates the image. The more times you press, the more pixelated
//		the image becomes.
//
//		r or R: reloads the image back to its original form.
//
//		i or I: invert the colors of the displayed image.
//
//    q, Q or ESC: quit.
//
//   When the window is resized by the user: If the size of the window becomes bigger than
//   the image, the image is centered in the window. If the size of the window becomes
//   smaller than the image, the image is uniformly scaled down to the largest size that
//   fits in the window.
//
// 	 Please see the README.txt and the Project Report for more details.


#include <OpenImageIO/imageio.h>
#include <iostream>
#include <GL/glut.h>

using namespace std;
OIIO_NAMESPACE_USING


struct Pixel{ // defines a pixel structure
	unsigned char r,g,b,a;
};

//
// Global variables and constants
//
const int DEFAULTWIDTH = 600;	// default window dimensions if no image
const int DEFAULTHEIGHT = 600;

int WinWidth, WinHeight;	// window width and height
int ImWidth, ImHeight;		// image width and height
int ImChannels;           // number of channels per image pixel

int VpWidth, VpHeight;		// viewport width and height
int Xoffset, Yoffset;     // viewport offset from lower left corner of window

Pixel **pixmap = NULL;  // the image pixmap used for OpenGL display
Pixel **origPixmap = NULL; // sets the original image to this variable for reloading
int pixformat; 			// the pixel format used to correctly draw the image

int keyPress = 1; 	// counts the amount of times image has been pixelated


//
//  Routine to cleanup the memory.
//
void destroy(){
 if (pixmap){
     delete pixmap[0];
	 delete pixmap;
  }
}


//
//  Routine to read an image file and store in a pixmap
//  returns the size of the image in pixels if correctly read, or 0 if failure
//
int readImage(string infilename){
  // Create the oiio file handler for the image, and open the file for reading the image.
  // Once open, the file spec will indicate the width, height and number of channels.
  std::unique_ptr<ImageInput> infile = ImageInput::open(infilename);
  if(!infile){
    cerr << "Could not input image file " << infilename << ", error = " << geterror() << endl;
    return 0;
  }

  // Record image width, height and number of channels in global variables
  ImWidth = infile->spec().width;
  ImHeight = infile->spec().height;
  ImChannels = infile->spec().nchannels;

  // allocate temporary structure to read the image
  unsigned char tmp_pixels[ImWidth * ImHeight * ImChannels];

  // read the image into the tmp_pixels from the input file, flipping it upside down using negative y-stride,
  // since OpenGL pixmaps have the bottom scanline first, and
  // oiio expects the top scanline first in the image file.
  int scanlinesize = ImWidth * ImChannels * sizeof(unsigned char);
  if(!infile->read_image(TypeDesc::UINT8, &tmp_pixels[0] + (ImHeight - 1) * scanlinesize, AutoStride, -scanlinesize)){
    cerr << "Could not read image from " << infilename << ", error = " << geterror() << endl;
    return 0;
  }

 // get rid of the old pixmap and make a new one of the new size
  destroy();

 // allocate space for the Pixmap (contiguous approach, 2d style access)
  pixmap = new Pixel*[ImHeight];
  if(pixmap != NULL)
	pixmap[0] = new Pixel[ImWidth * ImHeight];
  for(int i = 1; i < ImHeight; i++)
	pixmap[i] = pixmap[i - 1] + ImWidth;

	//doing the same for origPixmap
	origPixmap = new Pixel*[ImHeight];
	if(origPixmap != NULL)
	origPixmap[0] = new Pixel[ImWidth * ImHeight];
	for(int i = 1; i < ImHeight; i++)
	origPixmap[i] = origPixmap[i - 1] + ImWidth;

 //  assign the read pixels to the the data structure
 int index;
  for(int row = 0; row < ImHeight; ++row) {
    for(int col = 0; col < ImWidth; ++col) {
		index = (row*ImWidth+col)*ImChannels;

		if (ImChannels==1){
			pixmap[row][col].r = tmp_pixels[index];
			pixmap[row][col].g = tmp_pixels[index];
			pixmap[row][col].b = tmp_pixels[index];
			pixmap[row][col].a = 255;
		}
		else{
			pixmap[row][col].r = tmp_pixels[index];
			pixmap[row][col].g = tmp_pixels[index+1];
			pixmap[row][col].b = tmp_pixels[index+2];
			if (ImChannels <4) // no alpha value is present so set it to 255
				pixmap[row][col].a = 255;
			else // read the alpha value
				pixmap[row][col].a = tmp_pixels[index+3];
		}
    }
  }

	//setting origPixmap to what pixmap is. we will use this when reloading image
	for(int i = 0; i < ImHeight; ++i) {
		for(int j = 0; j < ImWidth; ++j) {
			origPixmap[i][j].r = pixmap[i][j].r;
			origPixmap[i][j].g = pixmap[i][j].g;
			origPixmap[i][j].b = pixmap[i][j].b;
			origPixmap[i][j].a = pixmap[i][j].a;
		}
	}

  // close the image file after reading, and free up space for the oiio file handler
  infile->close();

  // set the pixel format to GL_RGBA and fix the # channels to 4
  pixformat = GL_RGBA;
  ImChannels = 4;

  // return image size in pixels
  return ImWidth * ImHeight;
}


//
// Routine to display a pixmap in the current window
//
void displayImage(){
  // if the window is smaller than the image, scale it down, otherwise do not scale
  if(WinWidth < ImWidth  || WinHeight < ImHeight)
    glPixelZoom(float(VpWidth) / ImWidth, float(VpHeight) / ImHeight);
  else
    glPixelZoom(1.0, 1.0);

  // display starting at the lower lefthand corner of the viewport
  glRasterPos2i(0, 0);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glDrawPixels(ImWidth, ImHeight, pixformat, GL_UNSIGNED_BYTE, pixmap[0]);
}


//
//   Display Callback Routine: clear the screen and draw the current image
//
void handleDisplay(){

  // specify window clear (background) color to be opaque black
  glClearColor(0, 0, 0, 1);
  // clear window to background color
  glClear(GL_COLOR_BUFFER_BIT);

  // only draw the image if it is of a valid size
  if(ImWidth > 0 && ImHeight > 0)
    displayImage();

  // flush the OpenGL pipeline to the viewport
  glFlush();
}


//
// Routine to write the current framebuffer to an image file
//
void writeImage(string outfilename){
  // make a pixmap that is the size of the window and grab OpenGL framebuffer into it
  // alternatively, you can read the pixmap into a 1d array and export this
   unsigned char local_pixmap[WinWidth * WinHeight * ImChannels];
   glReadPixels(0, 0, WinWidth, WinHeight, pixformat, GL_UNSIGNED_BYTE, local_pixmap);

  // create the oiio file handler for the image
  std::unique_ptr<ImageOutput> outfile = ImageOutput::create(outfilename);
  if(!outfile){
    cerr << "Could not create output image for " << outfilename << ", error = " << geterror() << endl;
    return;
  }

  // Open a file for writing the image. The file header will indicate an image of
  // width WinWidth, height WinHeight, and ImChannels channels per pixel.
  // All channels will be of type unsigned char
  ImageSpec spec(WinWidth, WinHeight, ImChannels, TypeDesc::UINT8);
  if(!outfile->open(outfilename, spec)){
    cerr << "Could not open " << outfilename << ", error = " << geterror() << endl;
    return;
  }

  // Write the image to the file. All channel values in the pixmap are taken to be
  // unsigned chars. While writing, flip the image upside down by using negative y stride,
  // since OpenGL pixmaps have the bottom scanline first, and oiio writes the top scanline first in the image file.
  int scanlinesize = WinWidth * ImChannels * sizeof(unsigned char);
  if(!outfile->write_image(TypeDesc::UINT8, local_pixmap + (WinHeight - 1) * scanlinesize, AutoStride, -scanlinesize)){
    cerr << "Could not write image to " << outfilename << ", error = " << geterror() << endl;
    return;
  }

  // close the image file after the image is written and free up space for the
  // ooio file handler
  outfile->close();
}


//
// Routine to pixelate the image.
//
void pixelate(int keypress){
	int total;
	total = keyPress*keyPress;

	// looping through the entire image
  for(int x = keyPress/2; x < ImHeight && x+keyPress/2 < ImHeight; x=x+keyPress) {
    for(int y = keyPress/2; y < ImWidth && y+keyPress/2 < ImWidth; y=y+keyPress) {

				int averageR = 0;
				int averageG = 0;
				int averageB = 0;

				// finding the average of the surrounding red pixels
				for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
					for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
						averageR += pixmap[i][j].r;
					}
				}
				averageR = averageR / total;

				// setting surrounding red pixels to average red color value
				for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
					for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
						pixmap[i][j].r = averageR;
					}
				}

				// finding the average of the surrounding green pixels
		    for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
		      for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
		        averageG += pixmap[i][j].g;
		      }
		    }
		    averageG = averageG / total;

				// setting surrounding green pixels to average green color value
		    for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
		      for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
		        pixmap[i][j].g = averageG;
		      }
		    }

				// finding the average of the surrounding blue pixels
		    for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
		      for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
		        averageB += pixmap[i][j].b;
		      }
		    }
		    averageB = averageB / total;

				// setting surrounding blue pixels to average blue color value
		    for(int i = x - (keyPress/2); i <= x + (keyPress/2); i++) {
		      for(int j = y - (keyPress/2); j <= y + (keyPress/2); j++) {
		        pixmap[i][j].b = averageB;
		      }
		    }

		}
  }

}


//
//	Routine to reload the original image.
//
void reloadImage() {
	for(int x = 0; x < ImHeight; x++) {
		for(int y = 0; y < ImWidth; y++) {
			pixmap[x][y].r = origPixmap[x][y].r;
			pixmap[x][y].g = origPixmap[x][y].g;
			pixmap[x][y].b = origPixmap[x][y].b;
			pixmap[x][y].a = origPixmap[x][y].a;
		}
	}
	keyPress = 1;
}


//
// Routine to invert the colors of the displayed pixmap. Just for fun!
//
void invertImage(){
  for(int row = 0; row < ImHeight; ++row) {
    for(int col = 0; col < ImWidth; ++col) {
		pixmap[row][col].r = 255 - pixmap[row][col].r;
		pixmap[row][col].g = 255 - pixmap[row][col].g;
		pixmap[row][col].b = 255 - pixmap[row][col].b;
	}
  }
}


//
//  Keyboard Callback Routine: 'r' - read and display a new image,
//  'w' - write the current window to an image file, 'q' or ESC - quit
//
void handleKey(unsigned char key, int x, int y){
  string infilename, outfilename;
  int ok;

  switch(toupper(key)){
		// 'W' - write the image to a file
    case 'W':
      cout << "Output image filename? ";  // prompt user for output filename
      cin >> outfilename;
      writeImage(outfilename);
      break;

		// 'P' - pixelate the image
    case 'P':
			keyPress*=3;
      pixelate(keyPress);
	  glutPostRedisplay();
      break;

		// 'R' - reload original image
		case 'R':
			reloadImage();
			glutPostRedisplay();
			break;

		// 'I' - inverts the colors
		case 'I':
			invertImage();
			glutPostRedisplay();
			break;

		// q or ESC - quit
    case 'Q':
    case 27:
      destroy();
      exit(0);

    default:		// not a valid key -- just ignore it
      return;
  }
}


//
//  Reshape Callback Routine: If the window is too small to fit the image,
//  make a viewport of the maximum size that maintains the image proportions.
//  Otherwise, size the viewport to match the image size. In either case, the
//  viewport is centered in the window.
//
void handleReshape(int w, int h){
  float imageaspect = (float)ImWidth / (float)ImHeight;	// aspect ratio of image
  float newaspect = (float)w / (float)h; // new aspect ratio of window

  // record the new window size in global variables for easy access
  WinWidth = w;
  WinHeight = h;

  // if the image fits in the window, viewport is the same size as the image
  if(w >= ImWidth && h >= ImHeight){
    Xoffset = (w - ImWidth) / 2;
    Yoffset = (h - ImHeight) / 2;
    VpWidth = ImWidth;
    VpHeight = ImHeight;
  }
  // if the window is wider than the image, use the full window height
  // and size the width to match the image aspect ratio
  else if(newaspect > imageaspect){
    VpHeight = h;
    VpWidth = int(imageaspect * VpHeight);
    Xoffset = int((w - VpWidth) / 2);
    Yoffset = 0;
  }
  // if the window is narrower than the image, use the full window width
  // and size the height to match the image aspect ratio
  else{
    VpWidth = w;
    VpHeight = int(VpWidth / imageaspect);
    Yoffset = int((h - VpHeight) / 2);
    Xoffset = 0;
  }

  // center the viewport in the window
  glViewport(Xoffset, Yoffset, VpWidth, VpHeight);

  // viewport coordinates are simply pixel coordinates
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, VpWidth, 0, VpHeight);
  glMatrixMode(GL_MODELVIEW);
}


//
// Main program to scan the commandline, set up GLUT and OpenGL, and start Main Loop
//
int main(int argc, char* argv[]){
  // scan command line and process
  // only one parameter allowed, an optional image filename and extension
  if(argc != 2){
    cout << "usage: ./pixelate input.img\nPlease see the README.txt for more info!" << endl;
    exit(1);
  }

  // set up the default window and empty pixmap if no image or image fails to load
  //WinWidth = DEFAULTWIDTH;
  //WinHeight = DEFAULTHEIGHT;
  ImWidth = 0;
  ImHeight = 0;

  // load the image if present, and size the window to match
  if(argc == 2){
    if(readImage(argv[1])){
      WinWidth = ImWidth;
      WinHeight = ImHeight;
			//cout<<WinWidth<<" "<<WinHeight<<endl;
    }
  }

  // start up GLUT
  glutInit(&argc, argv);

  // create the graphics window, giving width, height, and title text
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
  glutInitWindowSize(WinWidth, WinHeight);
  glutCreateWindow("Image Viewer");

  // set up the callback routines
  glutDisplayFunc(handleDisplay); // display update callback
  glutKeyboardFunc(handleKey);	  // keyboard key press callback
  glutReshapeFunc(handleReshape); // window resize callback


  // Enter GLUT's event loop
  glutMainLoop();
  return 0;
}
