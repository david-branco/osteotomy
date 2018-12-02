#include "dicomloader.h"

DicomLoader::DicomLoader()
{
	// Change max resolution to load bigger images
	puntoexe::ptr<puntoexe::imebra::codecs::codecFactory> factory(puntoexe::imebra::codecs::codecFactory::getCodecFactory());
    factory->setMaximumImageSize(MAX_IMG_WIDTH,MAX_IMG_HEIGHT);
}

DicomLoader::~DicomLoader()
{

}

std::shared_ptr<DicomImage> DicomLoader::loadImage(const wchar_t* path)
{
#ifdef _MSC_VER
    std::ifstream file(path, std::ios::binary | std::ios::ate);
#else
    //setup converter
    typedef std::codecvt_utf8<wchar_t> convert_type;
    std::wstring_convert<convert_type, wchar_t> converter;
    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::string converted_path = converter.to_bytes( std::wstring(path) );
    std::ifstream file(converted_path.c_str(), std::ios::binary | std::ios::ate);
#endif
    if(!file.is_open())
    {
        return NULL;
    }

    std::streamsize size = file.tellg();
    if(size == 0)
    {
        return NULL;
    }

    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(buffer.data(), size);

    const uint8_t* bufferPointer = reinterpret_cast<const uint8_t*>(buffer.data());
    puntoexe::ptr<puntoexe::memory> memory (new puntoexe::memory);
    memory->assign(bufferPointer, buffer.size() & 0xFFFFFFFF);
    puntoexe::ptr<puntoexe::memoryStream> readStream(new puntoexe::memoryStream(memory));

	puntoexe::ptr<puntoexe::streamReader> reader(new puntoexe::streamReader(readStream));
    puntoexe::ptr<puntoexe::imebra::dataSet> dataSet;
    try
    {
        dataSet = puntoexe::imebra::codecs::codecFactory::getCodecFactory()->load(reader);
    }
    catch (...)
    {
        return NULL;
    }

    // Just read the 1st image
    puntoexe::ptr<puntoexe::imebra::image> firstImage;
    try
    {
        firstImage = dataSet->getModalityImage(0);
    }
    catch (...)
    {
        return NULL;
    }

	assert (firstImage.get() != NULL);

	// Retrieve the image's size in pixels
	std::uint32_t sizeX, sizeY;
	firstImage->getSize(&sizeX, &sizeY);

	std::uint32_t rowSize, channelPixelSize, channelsNumber;
	puntoexe::ptr<puntoexe::imebra::handlers::dataHandlerNumericBase> myHandler = firstImage->getDataHandler(false, &rowSize, &channelPixelSize, &channelsNumber);

	// Create DicomImage struct
	std::shared_ptr<DicomImage> dicomImage(new DicomImage);
	dicomImage->image = cvCreateImage(cvSize(sizeX, sizeY), IPL_DEPTH_16U, channelsNumber);
	IplImage *img = dicomImage->image;
	int i = 0;

	dicomImage->name = dataSet->getString(0x0010, 0, 0x0010, 0);
    dicomImage->gender = dataSet->getString(0x0010, 0, 0x0040, 0);
	dicomImage->birthday = dataSet->getString(0x0010, 0, 0x0030, 0);

    std::stringstream ss;
    std::vector<std::string> strings = Utils::split(dicomImage->name, '^');

    //[family name; given names; middle name; prefix; suffixes.]
    //prefix
    if(strings.size() > 3){ss << " " << strings.at(3);}
    //given names
    if(strings.size() > 1){ss << " " << strings.at(1);}
    //middles names
    if(strings.size() > 2){ss << " " << strings.at(2);}
    //family name
    if(strings.size() > 0){ss << " " << strings.at(0);}
    //suffixes
    if(strings.size() > 4){ss << " " << strings.at(4);}
    dicomImage->name = ss.str().substr(1);


	// Scan all the rows
	std::uint32_t index(0);
	for(std::uint32_t scanY = 0; scanY < sizeY; ++scanY)
	{
		// Scan all the columns
		for(std::uint32_t scanX = 0; scanX < sizeX; ++scanX)
		{
			// Scan all the channels
			for(std::uint32_t scanChannel = 0; scanChannel < channelsNumber; ++scanChannel)
			{
				// Convert 12bit image to 16bit image
				std::uint16_t channelValue = 0xffff - ((myHandler->getUnsignedLong(index++) << 4) & 0xffff);
				img->imageData[i++] = (unsigned char) (channelValue & 0xFF);
				img->imageData[i++] = (unsigned char) ((channelValue >> 8) & 0xFF);
			}
		}
	}

	return dicomImage;
}
