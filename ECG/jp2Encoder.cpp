#include "stdafx.h"
#include "jp2Encoder.h"
#include <assert.h>
#include <iostream>


using std::cerr;

jp2Encoder::jp2Encoder(int height, int width){
	jas_image_cmptparm_t img_parameter;
	pStream = NULL;
	pImage = NULL;
	ref_data = NULL;

	jp2Size = 0;


	if(jas_init())
		throw "Cannot Initiate jasper library";
	
	pStream = jas_stream_memopen(NULL,0); //The stream is to store the encoded image
	
	if(!pStream){
		
		throw "Cannot Creat jp2Encoder object: jas_steam_memopen() failed...";

	}

	img_parameter.tlx = 0;
	img_parameter.tly = 0;
	img_parameter.hstep = 1;
	img_parameter.vstep = 1;
	img_parameter.width = this->width = width;
	img_parameter.height = this->height = height;
	img_parameter.prec = 8;
	img_parameter.sgnd = false;
	
	pImage = jas_image_create(1, &img_parameter, 769); //Create an gray-scale RGB image data structure

	if(!pImage){
		throw "Cannot Creat jp2Encoder object: jas_image_create() failed...";

	}

	jas_image_setclrspc(pImage, JAS_CLRSPC_SGRAY);
	jas_image_setcmpttype(pImage, 0, JAS_IMAGE_CT_COLOR(JAS_CLRSPC_CHANIND_GRAY_Y));
	pImage->cmprof_ = jas_cmprof_createfromclrspc(pImage->clrspc_);




}

jp2Encoder::~jp2Encoder(){
	jas_stream_close(pStream);
	jas_image_destroy(pImage);
	
}


int jp2Encoder::Encode(){
	return Encode(NULL);
}



int jp2Encoder::Encode(char* optstr){


	jas_stream_rewind(pStream);  //hasn't been validate yet

	if(jas_image_encode(pImage, pStream, jas_image_strtofmt("jp2"), optstr) )
		throw "jas_image_encode error...";


	jp2Size = ( (jas_stream_memobj_t *) pStream->obj_ )->len_;
	p_jp2Data = ( (jas_stream_memobj_t *) pStream->obj_ )->buf_;

	//assert( jas_stream_length(pStream) == jp2Size );
	



	return jp2Size;
}



unsigned long jp2Encoder::getjp2Size(){
	return jp2Size;
}

unsigned char* jp2Encoder::getjp2Data(){
	return p_jp2Data;
}


int jp2Encoder::writeCompon(int* src, int row, int col){
	if(row < 0 ||row > height)
		throw "writeCompon: the row is out of available region...";
	if(col < 0 || col > width)
		throw "writeCompon: the col is out of available region...";
	if(row*col > width*height )
		throw "writeCompon: the pImage doesn't have enough space to store the in coming data";

/*
	if(x < 0 || x > width-1 )
		throw "writeCompon: the x coordinate is out of available region...";
	if(y < 0 || y > height-1)
		throw "writeCompon: the y coordinate is out of available region...";
*/	
	ref_data = jas_matrix_ref_create(row, col, src);

	if(!ref_data)
		throw "writeCompon: cannot create ref_data";
	
	if( jas_image_writecmpt(pImage, 0, 0, 0, col, row, ref_data) < 0)
		throw "writeCompon: jas_image_writecmpt() failed";


	jas_matrix_ref_destroy(ref_data);
	ref_data = NULL;

	return row*col;	
}
