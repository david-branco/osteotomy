/*

Imebra community build 20151130-002

Imebra: a C++ Dicom library

Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015
by Paolo Brandoli/Binarno s.p.

All rights reserved.

This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2 as published by
 the Free Software Foundation.

This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

-------------------

If you want to use Imebra commercially then you have to buy the commercial
 license available at http://imebra.com

After you buy the commercial license then you can use Imebra according
 to the terms described in the Imebra Commercial License Version 2.
A copy of the Imebra Commercial License Version 2 is available in the
 documentation pages.

Imebra is available at http://imebra.com

The author can be contacted by email at info@binarno.com or by mail at
 the following address:
 Binarno s.p., Paolo Brandoli
 Rakuseva 14
 1000 Ljubljana
 Slovenia



*/

/*! \file viewHelper.cpp
    \brief Implementation of the class view.

*/

#include "../include/viewHelper.h"
#include "../include/image.h"

namespace puntoexe
{

namespace imebra
{

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// cursorLine
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Default constructors
//
///////////////////////////////////////////////////////////
cursorLine::cursorLine():
	m_x0(0), m_y0(0), m_x1(0), m_y1(0),
		m_red(255), m_green(0), m_blue(0),
		m_style(line), m_width(0)
{
}

///////////////////////////////////////////////////////////
//
// Construct and assign
//
///////////////////////////////////////////////////////////
cursorLine::cursorLine(
		std::int32_t x0,
		std::int32_t y0,
		std::int32_t x1,
		std::int32_t y1,
		std::uint8_t red,
		std::uint8_t green,
		std::uint8_t blue,
		tStyle style,
		std::uint32_t width):
	m_x0(x0), m_y0(y0), m_x1(x1), m_y1(y1),
		m_red(red), m_green(green), m_blue(blue),
		m_style(style), m_width(width)
{
}

///////////////////////////////////////////////////////////
//
// Copy constructor
//
///////////////////////////////////////////////////////////
cursorLine::cursorLine(const cursorLine& right): baseObject()
{
	copy(right);
}

///////////////////////////////////////////////////////////
//
// Operator =
//
///////////////////////////////////////////////////////////
cursorLine& cursorLine::operator=(const cursorLine& right)
{
	copy(right);
	return *this;
}

///////////////////////////////////////////////////////////
//
// Copy
//
///////////////////////////////////////////////////////////
void cursorLine::copy(const cursorLine& right)
{
	m_x0 = right.m_x0;
	m_y0 = right.m_y0;
	m_x1 = right.m_x1;
	m_y1 = right.m_y1;

	m_red = right.m_red;
	m_green = right.m_green;
	m_blue = right.m_blue;

	m_style = right.m_style;
	m_width = right.m_width;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
//
// view
//
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Constructor
//
///////////////////////////////////////////////////////////
view::view():
	m_leftPosition(0),
	m_topPosition(0),
	m_rightPosition(0),
	m_bottomPosition(0),
	m_backgroundRed(192),
	m_backgroundGreen(192),
	m_backgroundBlue(192),
        m_zoom(1.0),
        m_bUpdateImage(true),
        m_cachedWidth(0),
        m_cachedHeight(0),
        m_cachedVisibleTop(0),
	m_cachedVisibleLeft(0),
	m_cachedVisibleBottom(0),
        m_cachedVisibleRight(0)
{
}


///////////////////////////////////////////////////////////
//
// Get the background color
//
///////////////////////////////////////////////////////////
void view::getBackgroundColor(std::uint8_t* pRed, std::uint8_t* pGreen, std::uint8_t* pBlue)
{
	*pRed   = m_backgroundRed;
	*pGreen = m_backgroundGreen;
	*pBlue  = m_backgroundBlue;
}


///////////////////////////////////////////////////////////
//
// Set the background color
//
///////////////////////////////////////////////////////////
void view::setBackgroundColor(std::uint8_t red, std::uint8_t green, std::uint8_t blue)
{
	m_backgroundRed   = red;
	m_backgroundGreen = green;
	m_backgroundBlue  = blue;
	invalidate();
}


///////////////////////////////////////////////////////////
//
// Get the image drawing area
//
///////////////////////////////////////////////////////////
void view::getImageRect(std::int32_t* pLeft, std::int32_t* pTop, std::int32_t* pRight, std::int32_t* pBottom)
{
	*pLeft   = m_leftPosition;
	*pTop    = m_topPosition;
	*pRight  = m_rightPosition;
	*pBottom = m_bottomPosition;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Zoom
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Get the active zoom factor
//
///////////////////////////////////////////////////////////
double view::getZoomFactor()
{
	return m_zoom;
}


///////////////////////////////////////////////////////////
//
// Set the active zoom factor
//
///////////////////////////////////////////////////////////
void view::setZoomFactor(double zoomFactor, std::int32_t centerPointX /* =-1 */, std::int32_t centerPointY /* =-1 */)
{
	// Update the zoom factor
	///////////////////////////////////////////////////////////
	m_zoom = zoomFactor;

	// If the image hasn't been set, then don't do anything
	///////////////////////////////////////////////////////////
	if(m_originalImage == 0)
	{
		return;
	}

	// Not automatic zoom
	///////////////////////////////////////////////////////////
	if(m_zoom > 0)
	{
		updateImageRect(centerPointX, centerPointY);
		return;
	}

	// Automatic zoom
	///////////////////////////////////////////////////////////
	setScrollSize(1, 1, true);
	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	setZoomRect(0, 0, imageSizeX, imageSizeY);
}


///////////////////////////////////////////////////////////
//
// Zoom to the selected image's area. The rectangle measure
//  units are in image's pixels
//
///////////////////////////////////////////////////////////
void view::setZoomRect(std::int32_t left, std::int32_t top, std::int32_t right, std::int32_t bottom)
{
	if(m_originalImage == 0)
	{
		return;
	}

	if(left > right)
	{
		std::int32_t temp = left;
		left = right;
		right = temp;
	}

	if(top > bottom)
	{
		std::int32_t temp = top;
		top = bottom;
		bottom = temp;

	}
	std::int32_t centerPointX = (right - left)/2 + left;
	std::int32_t centerPointY = (bottom - top)/2 + top;

	std::uint32_t sizeX, sizeY;
	getWindowSize(&sizeX, &sizeY);

	double imageSizeMmX, imageSizeMmY;
	m_originalImage->getSizeMm(&imageSizeMmX, &imageSizeMmY);

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	std::uint32_t horzDPI, vertDPI;
	getScreenDPI(&horzDPI, &vertDPI);

	if(imageSizeMmX == 0)
	{
		imageSizeMmX = (double)imageSizeX * 25.4 / (double)horzDPI;
	}
	if(imageSizeMmY == 0)
	{
		imageSizeMmY = (double)imageSizeY * 25.4 / (double)vertDPI;
	}

	double displaySizeMmX = imageSizeMmX * ((double)(right - left) / (double)imageSizeX);
	double displaySizeMmY = imageSizeMmY * ((double)(bottom - top) / (double)imageSizeY);
	double horzZoom = 0.95*(sizeX*25.4)/(displaySizeMmX*(double)horzDPI);
	double vertZoom = 0.95*(sizeY*25.4)/(displaySizeMmY*(double)vertDPI);

	m_zoom = (horzZoom < vertZoom) ? horzZoom : vertZoom;

	updateImageRect(centerPointX, centerPointY);

}


///////////////////////////////////////////////////////////
//
// Zoom in/out. The selected pixel will be centered into the
//  window
//
///////////////////////////////////////////////////////////
void view::zoomInOut(bool bZoomIn, std::int32_t centerPointX /* =-1 */, std::int32_t centerPointY /* =-1 */)
{
	if(bZoomIn)
	{
		m_zoom *= 2;
	}
	else
	{
		m_zoom /= 2;
	}
	updateImageRect(centerPointX, centerPointY);
}


///////////////////////////////////////////////////////////
//
// Returns true if the auto zoom is enabled
//
///////////////////////////////////////////////////////////
bool view::isAutoZoom()
{
	if(m_originalImage == 0 || m_leftPosition == m_rightPosition || m_topPosition == m_bottomPosition)
	{
		return false;
	}
	std::uint32_t sizeX, sizeY;
	getWindowSize(&sizeX, &sizeY);
	return (m_leftPosition >= 0 && m_rightPosition <= (std::int32_t)sizeX && m_topPosition >= 0 && m_bottomPosition <= (std::int32_t)sizeY);
}


///////////////////////////////////////////////////////////
//
// Get the current image's pixel displayed in the center of
//  the window
//
///////////////////////////////////////////////////////////
void view::getCenterPoint(std::int32_t* pCenterPointX, std::int32_t* pCenterPointY)
{
	// Reset the result values
	///////////////////////////////////////////////////////////
	*pCenterPointX = 0;
	*pCenterPointY = 0;

	if(m_originalImage == 0)
	{
		return;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	// Nothing happens if the image is not valid
	///////////////////////////////////////////////////////////
	if(m_rightPosition - m_leftPosition == 0 || m_bottomPosition - m_topPosition == 0)
	{
		return;
	}

	// Get the window's width
	///////////////////////////////////////////////////////////
	std::uint32_t windowWidth  = 0;
	std::uint32_t windowHeight = 0;
	getWindowSize(&windowWidth, &windowHeight);

	// Get the scroll position
	///////////////////////////////////////////////////////////
	std::int32_t scrollX = 0;
	std::int32_t scrollY = 0;
	getScrollPosition(&scrollX, &scrollY);

	// Calculate the actual center point
	///////////////////////////////////////////////////////////
	*pCenterPointX = windowPosToImageX(scrollX + windowWidth / 2);
	*pCenterPointY = windowPosToImageY(scrollY + windowHeight / 2);
}


///////////////////////////////////////////////////////////
//
// Center the desidered image's pixel in the window
//
///////////////////////////////////////////////////////////
void view::setCenterPoint(std::int32_t centerPointX, std::int32_t centerPointY)
{
	if(m_originalImage == 0)
	{
		return;
	}

	// Calculate the size of the area occupied by the image on
	//  the screen
	///////////////////////////////////////////////////////////
	std::int32_t imageAreaWidth = m_rightPosition - m_leftPosition;
	std::int32_t imageAreaHeight = m_bottomPosition - m_topPosition;

	// Get the window's width
	///////////////////////////////////////////////////////////
	std::uint32_t windowWidth  = 0;
	std::uint32_t windowHeight = 0;
	getWindowSize(&windowWidth, &windowHeight);

	// Get the scroll position
	///////////////////////////////////////////////////////////
	std::int32_t oldScrollPosX = 0;
	std::int32_t oldScrollPosY = 0;
	getScrollPosition(&oldScrollPosX, &oldScrollPosY);

	std::int32_t newScrollPosX = oldScrollPosX;
	std::int32_t newScrollPosY = oldScrollPosY;

	// Get the image's size
	///////////////////////////////////////////////////////////
	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	// Calculate the new scroll position
	///////////////////////////////////////////////////////////
	if(centerPointX>=0)
	{
		newScrollPosX = std::int32_t((centerPointX * imageAreaWidth)/imageSizeX) + m_leftPosition - windowWidth/2;
	}

	if(centerPointY>=0)
	{
		newScrollPosY = std::int32_t((centerPointY * imageAreaHeight)/imageSizeY) + m_topPosition - windowHeight/2;
	}

	// Check if the scroll position is valid
	///////////////////////////////////////////////////////////
	if(newScrollPosX+(std::int32_t)windowWidth>imageAreaWidth)
	{
		newScrollPosX=imageAreaWidth-(std::int32_t)windowWidth;
	}
	if(newScrollPosY+(std::int32_t)windowHeight>imageAreaHeight)
	{
		newScrollPosY=imageAreaHeight-(std::int32_t)windowHeight;
	}
	if(newScrollPosX < 0)
	{
		newScrollPosX = 0;
	}
	if(newScrollPosY < 0)
	{
		newScrollPosY = 0;
	}
	if(oldScrollPosX!=newScrollPosX || oldScrollPosY!=newScrollPosY)
	{
		setScrollPosition(newScrollPosX, newScrollPosY);
	}

	// Set the new scroll position
	///////////////////////////////////////////////////////////
	if(newScrollPosX != oldScrollPosX || newScrollPosY != oldScrollPosY)
	{
		setScrollPosition(newScrollPosX, newScrollPosY);
	}
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Cursor
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Begin the cursor definition
//
///////////////////////////////////////////////////////////
void view::startCursorDef()
{
	m_tempCursorLines.clear();
}

///////////////////////////////////////////////////////////
//
// End the cursor definition
//
///////////////////////////////////////////////////////////
void view::endCursorDef(std::int32_t cursorHotSpotX, std::int32_t cursorHotSpotY)
{
	// Copy the temporary lines into cursor's lines
	///////////////////////////////////////////////////////////
	m_cursorLines.clear();
	for(tCursorLinesList::iterator copyLines = m_tempCursorLines.begin(); copyLines != m_tempCursorLines.end(); ++copyLines)
	{
		m_cursorLines.push_back(*copyLines);
	}
	m_tempCursorLines.clear();


	// Do we have to scroll the window because the hotspot is
	//  outside the visible area?
	///////////////////////////////////////////////////////////
	if(!isMouseCaptured() || m_originalImage == 0)
	{
		invalidate();
		return;
	}

	// Convert the hotspot into window's coordinates
	///////////////////////////////////////////////////////////
	std::int32_t scrollX, scrollY;
	getScrollPosition(&scrollX, &scrollY);

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	std::int32_t windowHotSpotX = cursorHotSpotX * (m_rightPosition - m_leftPosition) / imageSizeX +m_leftPosition - scrollX;
	std::int32_t windowHotSpotY = cursorHotSpotY * (m_bottomPosition - m_topPosition) / imageSizeY +m_topPosition - scrollY;

	// Find the amount of scroll to execute
	///////////////////////////////////////////////////////////
	std::uint32_t windowSizeX, windowSizeY;
	getWindowSize(&windowSizeX, &windowSizeY);

	std::int32_t limitX = windowSizeX / 10;
	std::int32_t limitY = windowSizeY / 10;

	std::int32_t executeScrollX = 0;
	std::int32_t executeScrollY = 0;

	if(windowHotSpotX < limitX)
	{
		executeScrollX = windowHotSpotX - limitX;
	}
	if(windowHotSpotY < limitY)
	{
		executeScrollY = windowHotSpotY - limitY;
	}
	if(windowHotSpotX > ((std::int32_t)windowSizeX - limitX) )
	{
		executeScrollX = windowHotSpotX - (std::int32_t)windowSizeX + limitX;
	}
	if(windowHotSpotY > ((std::int32_t)windowSizeY - limitY) )
	{
		executeScrollY = windowHotSpotY - (std::int32_t)windowSizeY + limitY;
	}

	if(executeScrollX != 0 && executeScrollY != 0)
	{
		updateWindow();
		setScrollPosition(scrollX + executeScrollX, scrollY + executeScrollY);
	}

	invalidate();
}


///////////////////////////////////////////////////////////
//
// End the cursor definition
//
///////////////////////////////////////////////////////////
void view::endCursorDef()
{
	// Copy the temporary lines into cursor's lines
	///////////////////////////////////////////////////////////
	m_cursorLines.clear();
	for(tCursorLinesList::iterator copyLines = m_tempCursorLines.begin(); copyLines != m_tempCursorLines.end(); ++copyLines)
	{
		m_cursorLines.push_back(*copyLines);
	}
	m_tempCursorLines.clear();

	// Invalidate the new lines
	///////////////////////////////////////////////////////////
	invalidate();
}


///////////////////////////////////////////////////////////
//
// Define a cursor's line
//
///////////////////////////////////////////////////////////
void view::defCursorLine(std::int32_t startPointX, std::int32_t startPointY, std::int32_t endPointX, std::int32_t endPointY, std::uint8_t colorRed, std::uint8_t colorGreen, std::uint8_t colorBlue, cursorLine::tStyle style /* = cursorLine::line */, int width /* =0 */)
{
	cursorLine newLine(startPointX, startPointY, endPointX, endPointY, colorRed, colorGreen, colorBlue, style, width);
	m_tempCursorLines.push_back(newLine);
}


///////////////////////////////////////////////////////////
//
// Translate the cursor's position into a
//  coordinate relative to the image's pixels
//
///////////////////////////////////////////////////////////
std::int32_t view::windowPosToImageX(std::int32_t windowPosX)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	std::int32_t returnValue = ((windowPosX - m_leftPosition) * (std::int32_t)imageSizeX) / (m_rightPosition - m_leftPosition);
	if(returnValue < 0)
	{
		return 0;
	}
	if(returnValue > (std::int32_t)imageSizeX)
	{
		return (std::int32_t)imageSizeX;
	}
	return returnValue;
}

///////////////////////////////////////////////////////////
//
// Translate the cursor's position into a
//  coordinate relative to the image's pixels
//
///////////////////////////////////////////////////////////
std::int32_t view::windowPosToImageY(std::int32_t windowPosY)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	std::int32_t returnValue = ((windowPosY - m_topPosition) * (std::int32_t)imageSizeY) / (m_bottomPosition - m_topPosition);
	if(returnValue < 0)
	{
		return 0;
	}
	if(returnValue > (std::int32_t)imageSizeY)
	{
		return (std::int32_t)imageSizeY;
	}
	return returnValue;
}


///////////////////////////////////////////////////////////
//
// Return a pixel's horizontal position in millimiters
//
///////////////////////////////////////////////////////////
double view::imagePosToMillimitersX(std::int32_t imagePosX)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	if(imageSizeX == 0)
	{
		return 0;
	}

	double mmSizeX, mmSizeY;
	m_originalImage->getSizeMm(&mmSizeX, &mmSizeY);

	return mmSizeX * (double)imagePosX / (double)imageSizeX;
}


///////////////////////////////////////////////////////////
//
// Return a pixel's vertical position in millimiters
//
///////////////////////////////////////////////////////////
double view::imagePosToMillimitersY(std::int32_t imagePosY)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	if(imageSizeY == 0)
	{
		return 0;
	}

	double mmSizeX, mmSizeY;
	m_originalImage->getSizeMm(&mmSizeX, &mmSizeY);

	return mmSizeY * (double)imagePosY / (double)imageSizeY;
}


///////////////////////////////////////////////////////////
//
// Return the pixel horizontal index at the specified
//  position in millimeters
//
///////////////////////////////////////////////////////////
std::int32_t view::millimitersToImagePosX(double millimitersX)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	double mmSizeX, mmSizeY;
	m_originalImage->getSizeMm(&mmSizeX, &mmSizeY);
	if(mmSizeX == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	return (std::int32_t)((double)imageSizeX * millimitersX / mmSizeX);
}


///////////////////////////////////////////////////////////
//
// Return the pixel vertical index at the specified
//  position in millimeters
//
///////////////////////////////////////////////////////////
std::int32_t view::millimitersToImagePosY(double millimitersY)
{
	if(m_originalImage == 0)
	{
		return 0;
	}

	double mmSizeX, mmSizeY;
	m_originalImage->getSizeMm(&mmSizeX, &mmSizeY);
	if(mmSizeY == 0)
	{
		return 0;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	return (std::int32_t)((double)imageSizeY * millimitersY / mmSizeY);
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Drawing functions
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Draw the cursor
//
///////////////////////////////////////////////////////////
void view::drawCursor(void* pDeviceContext)
{
	if(m_originalImage == 0)
	{
		return;
	}

	std::uint32_t imageSizeX, imageSizeY;
	m_originalImage->getSize(&imageSizeX, &imageSizeY);

	for(tCursorLinesList::iterator scanLines = m_cursorLines.begin(); scanLines != m_cursorLines.end(); ++scanLines)
	{
		std::int32_t x0 = scanLines->m_x0 * (m_rightPosition - m_leftPosition) / imageSizeX + m_leftPosition;
		std::int32_t y0 = scanLines->m_y0 * (m_bottomPosition - m_topPosition) / imageSizeY + m_topPosition;
		std::int32_t x1 = scanLines->m_x1 * (m_rightPosition - m_leftPosition) / imageSizeX + m_leftPosition;
		std::int32_t y1 = scanLines->m_y1 * (m_bottomPosition - m_topPosition) / imageSizeY + m_topPosition;

		drawCursorLine(pDeviceContext,
			x0, y0, x1, y1,
			scanLines->m_red, scanLines->m_green, scanLines->m_blue,
			scanLines->m_style, scanLines->m_width);
	}
}


///////////////////////////////////////////////////////////
//
// Calculate the rectancle to use to draw the image
//
///////////////////////////////////////////////////////////
void view::updateImageRect(std::int32_t centerPointX, std::int32_t centerPointY)
{
	std::int32_t tempCenterPointX = 0;
	std::int32_t tempCenterPointY = 0;
	getCenterPoint(&tempCenterPointX, &tempCenterPointY);

	if(centerPointX < 0) centerPointX = tempCenterPointX;
	if(centerPointY < 0) centerPointY = tempCenterPointY;

	std::int32_t leftPosition, topPosition, rightPosition, bottomPosition;
	leftPosition=
		topPosition=
		rightPosition=
		bottomPosition = 0;

	// Get the window's size
	///////////////////////////////////////////////////////////
	std::uint32_t windowSizeX = 0;
	std::uint32_t windowSizeY = 0;
	getWindowSize(&windowSizeX, &windowSizeY);

	// Get the scroll size
	///////////////////////////////////////////////////////////
	std::uint32_t scrollSizeX = 0;
	std::uint32_t scrollSizeY = 0;
	getScrollSize(&scrollSizeX, &scrollSizeY);

	// Get the scroll position
	///////////////////////////////////////////////////////////
	std::int32_t scrollPosX = 0;
	std::int32_t scrollPosY = 0;
	getScrollPosition(&scrollPosX, &scrollPosY);

	// For now, the new scroll size and position are the same
	//  as the old ones
	///////////////////////////////////////////////////////////
	std::uint32_t newScrollSizeX=scrollSizeX;
	std::uint32_t newScrollSizeY=scrollSizeY;

	if(m_originalImage != 0)
	{
		std::uint32_t imageSizeX(0), imageSizeY(0);
		m_originalImage->getSize(&imageSizeX, &imageSizeY);

		// Retrieve the screen's resolution
		///////////////////////////////////////////////////////////
		std::uint32_t screenHorzDPI, screenVertDPI;
		screenHorzDPI=screenVertDPI=75;
		getScreenDPI(&screenHorzDPI, &screenVertDPI);

		// Get the image's size (in mms and pixels)
		///////////////////////////////////////////////////////////
		double imageSizeMmX = 0;
		double imageSizeMmY = 0;
		m_originalImage->getSizeMm(&imageSizeMmX, &imageSizeMmY);

		if(imageSizeMmX == 0)
		{
			imageSizeMmX = (double)imageSizeX * 25.4 / (double)screenHorzDPI;
		}
		if(imageSizeMmY == 0)
		{
			imageSizeMmY = (double)imageSizeY * 25.4 / (double)screenVertDPI;
		}


		// Calculate the area occupied by the image, in screen's
		//  pixels
		///////////////////////////////////////////////////////////
		std::uint32_t displayAreaWidth=(std::uint32_t)((double)imageSizeMmX*m_zoom*(double)screenHorzDPI/25.4+0.5);
		std::uint32_t displayAreaHeight=(std::uint32_t)((double)imageSizeMmY*m_zoom*(double)screenVertDPI/25.4+0.5);

		if(displayAreaWidth>windowSizeX)
		{
			rightPosition = displayAreaWidth;
		}
		else
		{
			leftPosition = (windowSizeX-displayAreaWidth)>>1;
			rightPosition = leftPosition+displayAreaWidth;
		}

		if(displayAreaHeight>windowSizeY)
		{
			bottomPosition = displayAreaHeight;
		}
		else
		{
			topPosition = (windowSizeY-displayAreaHeight)>>1;
			bottomPosition = topPosition+displayAreaHeight;
		}
		newScrollSizeX = displayAreaWidth;
		newScrollSizeY = displayAreaHeight;
	}

	m_leftPosition = leftPosition;
	m_topPosition = topPosition;
	m_rightPosition = rightPosition;
	m_bottomPosition = bottomPosition;

	if(scrollSizeX!=newScrollSizeX || scrollSizeY!=newScrollSizeY)
	{
		setScrollSize(newScrollSizeX, newScrollSizeY, false);
	}

	setCenterPoint(centerPointX, centerPointY);

	invalidate();
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//
//
// Set/get the active image
//
//
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
//
// Retrieve the active image
//
///////////////////////////////////////////////////////////
ptr<image> view::getImage()
{
	return m_originalImage;
}

///////////////////////////////////////////////////////////
//
// Set the active image
//
///////////////////////////////////////////////////////////
void view::setImage(ptr<image> pImage, ptr<transforms::transformsChain> pChain)
{
	m_bUpdateImage = true;
	if(pImage == 0)
	{
		m_originalImage = 0;
		setScrollSize(1, 1, true);
		m_drawBitmap.release();
		return;
	}
	m_drawBitmap = new imebra::drawBitmap(pImage, pChain);

	std::uint32_t oldSizeX = 0;
	std::uint32_t oldSizeY = 0;
	double oldSizeMmX = 0;
	double oldSizeMmY = 0;

	if(m_originalImage != 0)
	{
		m_originalImage->getSize(&oldSizeX, &oldSizeY);
		m_originalImage->getSizeMm(&oldSizeMmX, &oldSizeMmY);
	}

	m_originalImage = pImage;

	std::uint32_t newSizeX, newSizeY;
	m_originalImage->getSize(&newSizeX, &newSizeY);
	double newSizeMmX, newSizeMmY;
	m_originalImage->getSizeMm(&newSizeMmX, &newSizeMmY);

	if(oldSizeX == newSizeX &&
		oldSizeY == newSizeY &&
		oldSizeMmX == newSizeMmX &&
		oldSizeMmY == newSizeMmY)
	{
		invalidate();
		return;
	}

	setZoomFactor(-1, -1, -1);
}


} // namespace imebra

} // namespace puntoexe
