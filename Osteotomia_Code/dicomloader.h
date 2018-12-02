#ifndef DICOMLOADER_H_
#define DICOMLOADER_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include "utils.h"

#include "library/imebra/include/imebra.h"
#include "library/imebra/include/codecFactory.h"

#include <fstream>
#include <string>
#include <iostream>
#include <codecvt>


#define MAX_IMG_WIDTH 0xFFFF
#define MAX_IMG_HEIGHT 0xFFFF

typedef struct sDicomImage {
	IplImage *image;
	std::string name;
    std::string gender;
	std::string birthday;

	sDicomImage()
	{
		image = NULL;
	}
	~sDicomImage()
	{
		if(image != NULL)
		{
			cvReleaseImage(&image);
		}
	}
} DicomImage;

class DicomLoader {
public:
	DicomLoader();
	virtual ~DicomLoader();
    std::shared_ptr<DicomImage> loadImage (const wchar_t* path);
};

#endif
