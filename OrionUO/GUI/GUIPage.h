/***********************************************************************************
**
** GUIPage.h
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#ifndef GUIPAGE_H
#define GUIPAGE_H
//----------------------------------------------------------------------------------
#include "BaseGUI.h"
//----------------------------------------------------------------------------------
class CGUIPage : public CBaseGUI
{
	SETGET(int, Index);

public:
	CGUIPage(const int &index);
	virtual ~CGUIPage();
};
//----------------------------------------------------------------------------------
#endif
//----------------------------------------------------------------------------------
