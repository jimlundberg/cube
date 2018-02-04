// JPEGload.cpp - loads JPEG image files
//
// John Davin, Terrence Wong, Jim Lundberg
// Copyright (c) 2002-2004 JRL Innovations  All Rights Reserved
// 3D5 - The FIVE level 3D tic-tac-toe game
//

#include "StdAfx.h"
#include "jpeglib.h"

///////////////////////////////// DECODE JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This decodes the jpeg and fills in the tImageJPG structure
/////
///////////////////////////////// DECODE JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
void DecodeJPG(jpeg_decompress_struct* cinfo, tImageJPG *pImageData)
{
	// Read in the header of the jpeg file
	jpeg_read_header(cinfo, TRUE);
	
	// Start to decompress the jpeg file with our compression info
	jpeg_start_decompress(cinfo);

	// Get the image dimensions and row span to read in the pixel data
	pImageData->rowSpan = cinfo->image_width * cinfo->num_components;
	pImageData->sizeX   = cinfo->image_width;
	pImageData->sizeY   = cinfo->image_height;
	
	// Allocate memory for the pixel buffer
	pImageData->data = new unsigned char[pImageData->rowSpan * pImageData->sizeY];
		
	// Here we use the library's state variable cinfo.output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
	
	// Create an array of row pointers
	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];
	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i*pImageData->rowSpan]);

	// Now comes the juice of our work, here we extract all the pixel data
	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height) 
	{
		// Read in the current row of pixels and increase the rowsRead count
		rowsRead += jpeg_read_scanlines(cinfo, &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}
	
	// Delete the temporary row pointers
	delete [] rowPtr;

	// Finish decompressing the data
	jpeg_finish_decompress(cinfo);
}


///////////////////////////////// LOAD JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
/////
/////	This loads the JPG file and returns it's data in a tImageJPG struct
/////
///////////////////////////////// LOAD JPG \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

tImageJPG *LoadJPG(const char *filename)
{
	struct jpeg_decompress_struct cinfo;
	tImageJPG *pImageData = NULL;
	FILE *pFile;
	
	// This is the only function you should care about.  You don't need to
	// really know what all of this does (since you can't cause it's a library!) :)
	// Just know that you need to pass in the jpeg file name, and get a pointer
	// to a tImageJPG structure which contains the width, height and pixel data.
	// Be sure to free the data after you are done with it, just like a bitmap.
	
	// Open a file pointer to the jpeg file and check if it was found and opened 
	if((pFile = fopen(filename, "rb")) == NULL) 
	{
		// Display an error message saying the file was not found, then return NULL
		printf("Unable to load jpeg file %s!\n", filename);
		return NULL;
	}
	
	// Create an error handler
	jpeg_error_mgr jerr;

	// Have our compression info object point to the error handler address
	cinfo.err = jpeg_std_error(&jerr);
	
	// Initialize the decompression object
	jpeg_create_decompress(&cinfo);
	
	// Specify the data source (Our file pointer)	
	jpeg_stdio_src(&cinfo, pFile);
	
	// Allocate the structure that will hold our eventual jpeg data (must free it!)
	pImageData = (tImageJPG*)malloc(sizeof(tImageJPG));

	// Decode the jpeg file and fill in the image data structure to pass back
	DecodeJPG(&cinfo, pImageData);
	
	// This releases all the stored memory for reading and decoding the jpeg
	jpeg_destroy_decompress(&cinfo);
	
	// Close the file pointer that opened the file
	fclose(pFile);

	// Return the jpeg data (remember, you must free this data after you are done)
	return pImageData;
}
