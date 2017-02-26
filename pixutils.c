#include "pixutils.h"

//private methods -> make static
static pixMap* pixMap_init(unsigned char arrayType);
static pixMap* pixMap_copy(pixMap *p);

/* The links are for personal use if I forgot how/why I did my code. */
//http://stackoverflow.com/questions/20102551/writing-init-function-for-c-struct
//https://github.com/TCSS333B-2017/multiarrays/blob/master/typeWriterGraphics/ascii.c
static pixMap* pixMap_init(unsigned char arrayType) {
	//initialize everything to zero except arrayType
	//This is a pointer init function so reference the vector normalize + init from assignment 1.
	pixMap *pM = malloc(sizeof(*pM));
	//Eclipse is saying NULL causes an error, why? Do I need a certain include or is this only in Java?	
	//if(pM != NULL) {
	pM->arrayType = arrayType;
	pM->image = 0;
	pM->imageWidth = 0;
	pM->imageHeight = 0;
	pM->pixArray_arrays = 0;
	pM->pixArray_blocks = 0;
	pM->pixArray_overlay = 0;
	//}
	return pM;
}
//Just like the example code.
//https://github.com/TCSS333B-2017/multiarrays/blob/master/typeWriterGraphics/ascii.c
void pixMap_destroy(pixMap **p) {
	//free all mallocs and put a zero pointer in *p
	pixMap *pp = *p;
	if (pp->pixArray_arrays)
		free(pp->pixArray_arrays);
	//I'm not sure about these parts but free them because they have a underline like pixArray_arrays
	if (pp->pixArray_blocks)
		free(pp->pixArray_blocks);
	if (pp->pixArray_overlay)
		free(pp->pixArray_overlay);
	if (*pp)
		free(*p);

}

pixMap *pixMap_read(char *filename, unsigned char arrayType) {
	//library call reads in the image into p->image and sets the width and height
	pixMap *p = pixMap_init(arrayType);
	int error;
	if ((error = lodepng_decode32_file(&(p->image), &(p->imageWidth),
			&(p->imageHeight), filename))) {
		fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
		return 0;
	}
	//allocate the 2-D rgba arrays

	if (arrayType == 0) {
		//can only allocate for the number of rows - each row will be an array of MAXWIDTH
		//allocates memory for the pixel arrays, the size is width * height. (Same as multiarrays example)
		//Except the width already has a assigned max value and type (rgba) so I used that. 
		//Why can't you use sizeof p-> imageWidth?.
		p->pixArray_arrays = malloc(p->imageHeight * sizeof(rgba[MAXWIDTH]));

		//copy each row of the image into each row <- so therefore use pixArrays_arrays[i][j] = image[n++] 
		//Substitute the number of rows and columns with image height and image width.
		int n = 0;
		for (int i = 0; i < p->imageHeight; i++) {
			for (int j = 0; j < p->imageWidth; j++) {
				//do I have to assign each unsigned char of the pixArray? 
				//(ex: p -> pixArray_arrays[i][j] = p -> image[n++]; or picArray_array[i][j].r )
				//I'm doing the former because I asked my boy Dino Hadzic.
				p->pixArray_arrays[i][j].r = p->image[n++];
				p->pixArray_arrays[i][j].g = p->image[n++];
				p->pixArray_arrays[i][j].b = p->image[n++];
				p->pixArray_arrays[i][j].a = p->image[n++];

			}
		}
	} else if (arrayType == 1) {
		//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
		//The keyword here is block of memory so therefore use pixArray_blocks

		//https://www.eskimo.com/~scs/cclass/int/sx8.html 
		//pixArray_blocks is a double pointer so that would mean I would call sizeof(*rgba) ?)
		//I originally had p -> imageHeight first before sizeof rgba* why did this cause a error?
		p->pixArray_blocks = malloc(sizeof(rgba*) * p->imageHeight);

		//use a loop allocate a block of memory for each row
		for (int i = 0; i < (p->imageHeight); i++) {
			p->pixArray_blocks[i] = malloc(sizeof(rgba) * p->imageWidth);
		}
		//copy each row of the image into the newly allocated block 
		//This sounds like a repeat of the last part except with blocks
		int n = 0;
		int rows = p->imageHeight;
		int cols = p->imageWidth;
		for (int i = 0; i < rows; i++) {
			for (int j = 0; j < cols; j++) {
				p->pixArray_blocks[i][j].r = p->image[n++];
				p->pixArray_blocks[i][j].g = p->image[n++];
				p->pixArray_blocks[i][j].b = p->image[n++];
				p->pixArray_blocks[i][j].a = p->image[n++];

			}
		}
	} else if (arrayType == 2) {
		//allocate a block of memory (dynamic array of p->imageHeight) to store the pointers
		int rows = p->imageHeight;
		int cols = p->imageWidth;
		//I'm using overlay because I saw it in the rotate code for (arrayType == 2) but wtf is this?
		p->pixArray_overlay = malloc(rows * cols * sizeof(rgba*));
		//set the first pointer to the start of p->image (so array at index 0)
		p->pixArray_overlay[0] = p->image;
		//each subsequent pointer is the previous pointer + p->imageWidth
		for (int i = 1; i < rows; i++) {
			p->pixArray_overlay[i] = p->pixArray_overlay[i - 1] + p->imageWidth;
		}
	} else {
		return 0;
	}
	return p;
}
int pixMap_write(pixMap *p, char *filename) {
	int error = 0;
//for arrayType 1 and arrayType 2 have to write out a controws  to the image using memcpy
//http://stackoverflow.com/questions/2225850/c-c-how-to-copy-a-multidimensional-char-array-without-nested-loops
	if (p->arrayType == 0) {
		for (int i = 0; i < p->imageHeight; i++) {

			memcpy(p->image, p->pixArray_arrays[i], p->imageWidth * sizeof(rgba));
		}
	} else if (p->arrayType == 1) {
		for (int i = 0; i < p->imageHeight; i++) {
			//If 1 then allocates block just like pixMap_read
			memcpy(p->image, p->pixArray_blocks[i], p->imageWidth * sizeof(rgba));
		}
		//have to copy each row of the array into the corresponding row of the image		
	}
//library call to write the image out  
	if (lodepng_encode32_file(filename, p->image, p->imageWidth,
			p->imageHeight)) {
		fprintf(stderr, "error %u: %s\n", error, lodepng_error_text(error));
		return 1;
	}
	return 0;
}

int pixMap_rotate(pixMap *p, float theta) {
	pixMap *oldPixMap = pixMap_copy(p);
	if (!oldPixMap)
		return 1;
	const float ox = p->imageWidth / 2.0f;
	const float oy = p->imageHeight / 2.0f;
	const float s = sin(degreesToRadians(-theta));
	const float c = cos(degreesToRadians(-theta));
	for (int y = 0; y < p->imageHeight; y++) {
		for (int x = 0; x < p->imageWidth; x++) {
			float rotx = c * (x - ox) - s * (oy - y) + ox;
			float roty = -(s * (x - ox) + c * (oy - y) - oy);
			int rotj = rotx + .5;
			int roti = roty + .5;
			if (roti >= 0 && roti < oldPixMap->imageHeight && rotj >= 0
					&& rotj < oldPixMap->imageWidth) {
				if (p->arrayType == 0)
					memcpy(p->pixArray_arrays[y] + x,
							oldPixMap->pixArray_arrays[roti] + rotj,
							sizeof(rgba));
				else if (p->arrayType == 1)
					memcpy(p->pixArray_blocks[y] + x,
							oldPixMap->pixArray_blocks[roti] + rotj,
							sizeof(rgba));
				else if (p->arrayType == 2)
					memcpy(p->pixArray_overlay[y] + x,
							oldPixMap->pixArray_overlay[roti] + rotj,
							sizeof(rgba));
			} else {
				if (p->arrayType == 0)
					memset(p->pixArray_arrays[y] + x, 0, sizeof(rgba));
				else if (p->arrayType == 1)
					memset(p->pixArray_blocks[y] + x, 0, sizeof(rgba));
				else if (p->arrayType == 2)
					memset(p->pixArray_overlay[y] + x, 0, sizeof(rgba));
			}
		}
	}
	pixMap_destroy(&oldPixMap);
	return 0;
}
//Alot of the code here is reusing the pixMap_read
pixMap *pixMap_copy(pixMap *p) {
	int rows = p->imageHeight;
	int cols = p->imageWidth;
	pixMap *new = pixMap_init(p->arrayType);
	//allocate memory for new image of the same size a p->image and copy the image
	new->imageHeight = rows;
	new->imageWidth = cols;
	new->image = malloc(rows * cols * sizeof(p->image));
	memcpy(new->image, p->image, rows * cols * sizeof(rgba*));
	//allocate memory and copy the arrays. 
	if (new->arrayType == 0) {
		new->pixArray_arrays = malloc(rows * sizeof(rgba[MAXWIDTH]));
		memcpy(new->pixArray_arrays, p->pixArray_arrays,
				rows * sizeof(rgba[MAXWIDTH]));
	} else if (new->arrayType == 1) {
		new->pixArray_blocks = malloc(sizeof(rgba*) * rows);
		for (int i = 0; i < (p->imageHeight); i++) {
			p->pixArray_blocks[i] = malloc(sizeof(rgba*) * p->imageWidth);
			memcpy(new->pixArray_blocks[i], p->pixArray_blocks[i],
					new->imageWidth * sizeof(rgba*));
		}

	} else if (new->arrayType == 2) {
		new->pixArray_overlay = malloc(rows * cols * sizeof(rgba*));
		for (int i = 1; i < rows; i++) {
			p->pixArray_overlay[i] = p->pixArray_overlay[i - 1]
					+ new->imageWidth;
		}
		memcpy(new->image, p->image, rows * cols * sizeof(rgba));

	}
	return new;
}