/***********************************************************************************
**
** GUIChecktrans.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef GUICHECKTRANS_H
#define GUICHECKTRANS_H
//----------------------------------------------------------------------------------
#include "GUIPolygonal.h"
//----------------------------------------------------------------------------------
class CGUIChecktrans : public CGUIPolygonal
{
public:
	CGUIChecktrans(const int &x, const int &y, const int &width, const int &height);
	virtual ~CGUIChecktrans();

	virtual void Draw(const bool &checktrans = false);
	virtual bool Select() { return false; }
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
