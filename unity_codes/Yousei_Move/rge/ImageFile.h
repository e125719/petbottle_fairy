/*
 *  ImageFile.h
 *  collada_test
 *
 *  Created by Yuhei Akamine on 07/06/26.
 *  Copyright 2007 Yuhei Akamine. All rights reserved.
 *
 */
#pragma once

#include <stdio.h>
#include "Utils.h"

extern "C" {
#include <jpeglib.h>
}

namespace rge {
class ImageFileReader
{
public:
	ImageFileReader() : mRawData() {}

	unsigned char* getRawData() { return mRawData; }

	int getSizx() { return mSizx; }
	int getSizy() { return mSizy; }
protected:
	virtual ~ImageFileReader() { delete[] mRawData; }
	
	void alloc(int x, int y) { mRawData = new unsigned char[x*y*4]; mSizx = x; mSizy = y; }
	void setPixel(int x, int y, int r, int g, int b, int a) {
		mRawData[(mSizx*y + x)*4  ] = r;
		mRawData[(mSizx*y + x)*4+1] = g;
		mRawData[(mSizx*y + x)*4+2] = b;
		mRawData[(mSizx*y + x)*4+3] = a;
	}
	
	unsigned char* mRawData;

private:
	int mSizx;
	int mSizy;
};

class CheckerImage : public ImageFileReader
{
public:
	CheckerImage() {};
	
	void create(int sx, int sy) {
		alloc(sx, sy);
		
		for(int y=0; y<sy; ++y) {
			for(int x=0; x<sx; ++x) {
				setPixel(x, y, (x/16+y/16)%2==0? 255 : 0, 0,0, 255);
			}
		}
	}
};

class JpegFileReader : public ImageFileReader 
{
public:
	JpegFileReader() {}
	
	bool read(const char* filename) {
		if(strcmp(filename+strlen(filename)-3, "jpg") != 0) {
			printf("Not a jpeg %s\n", filename);
			return false;
		}
	
		jpeg_decompress_struct cinfo;
		jpeg_error_mgr jerr;
		
		cinfo.err = jpeg_std_error(&jerr);
		jpeg_create_decompress(&cinfo);
		
		FILE* infile = 0;
		if((infile = fopen(filename, "rb")) == NULL) {
			printf("JpegFileReader:can't open %s\n", filename);
			return false;
		}
		jpeg_stdio_src(&cinfo, infile);
		if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK)
			return false;
		jpeg_start_decompress(&cinfo);
		
		alloc(cinfo.image_width,cinfo.image_height);
		
		JSAMPLE *line = new JSAMPLE[cinfo.image_width*cinfo.output_components];
		
		for(u_int y=0; y<cinfo.image_height; ++y) {
			jpeg_read_scanlines(&cinfo, &line, 1);
			
			for(u_int x=0; x<cinfo.image_width; ++x) {
				int r = line[x*cinfo.output_components];
				int g = line[x*cinfo.output_components+1];
				int b = line[x*cinfo.output_components+2];
				
				setPixel(x, cinfo.image_height-y-1, 
						 r, g, b, 255);
			}
		}
		delete[] line;
		
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		
		return true;
	}
};

}