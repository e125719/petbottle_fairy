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
#include <memory>

#include "Utils.h"

extern "C" {
#include <jpeglib.h>
}

#ifdef WITH_OPENCV
#   include <opencv2/opencv.hpp>
#endif

namespace rge {
    
    class Image2D
    {
        int mSizx;
        int mSizy;
        std::shared_ptr<unsigned char> mRawData ;
    public:
        Image2D() {}
        ~Image2D() {}
        
        const unsigned char* getRawData() const { return mRawData.get(); }
        
        int getSizx() { return mSizx; }
        int getSizy() { return mSizy; }
        
        void alloc(int x, int y) {
            mRawData = std::shared_ptr<unsigned char>(new unsigned char[x*y*4]);
            mSizx = x; mSizy = y;
        }
        void setPixel(int x, int y, int r, int g, int b, int a) {
            mRawData.get()[(mSizx*y + x)*4  ] = r;
            mRawData.get()[(mSizx*y + x)*4+1] = g;
            mRawData.get()[(mSizx*y + x)*4+2] = b;
            mRawData.get()[(mSizx*y + x)*4+3] = a;
        }
        
        // resize to n^2xn^2
        Image2D normalized() {
            int w = pow(2, (int)log2( mSizx ));
            int h = pow(2, (int)log2( mSizy ));
            
            if(w>h) {
                h = w;
            }else {
                w = h;
            }
            Image2D normalized;
            
            normalized.alloc(w, h);
            
            for(int y=0; y<h; ++y) {
                for(int x=0; x<w; ++x) {
                   
                    size_t i = index((float)x/w*mSizx,(float)y/h*mSizy);
                            
                    int r,g,b,a;
                    r = mRawData.get()[i];
                    g = mRawData.get()[i+1];
                    b = mRawData.get()[i+2];
                    a = mRawData.get()[i+3];
                    
                    normalized.setPixel(x, y, r, g, b, a);
                }
            }
            
            return normalized;
        }
        
        Image2D halfImage() {
            Image2D half;
            half.alloc(getSizx()/2, getSizy()/2);
            
            for(int y=0; y<mSizy/2; ++y) {
                for(int x=0; x<mSizx/2; ++x) {
                    
                    int r=0, g=0, b=0, a=0;
                    for(int yy=0; yy<2; ++yy) {
                        for(int xx=0; xx<2; ++xx) {
                            size_t i = index(x*2+xx,y*2+yy);
                            
                            r += mRawData.get()[i];
                            g += mRawData.get()[i+1];
                            b += mRawData.get()[i+2];
                            a += mRawData.get()[i+3];
                        }
                    }
                    
                    half.setPixel(x, y, r/4, g/4, b/4, a/4);
                }
            }
            return half;
        }
    private:
        size_t index(int x, int y) { return (y*mSizx + x)*4; }
    };
    
    class CheckerImage : public Image2D
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
    
    class JpegImage : public Image2D
    {
    public:
        JpegImage() {}
        
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