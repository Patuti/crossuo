/***********************************************************************************
**
** GumpBuff.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "GumpBuff.h"
#include "../SelectedObject.h"
#include "../ToolTip.h"
#include "../OrionUO.h"
#include "../Managers/FontsManager.h"
//----------------------------------------------------------------------------------
CGumpBuff::CGumpBuff(uint serial, short x, short y)
: CGump(GT_BUFF, serial, x, y)
{
	m_Graphic = 0x7580;
	m_Locker.Serial = ID_GB_LOCK_MOVING;

	Add(new CGUIGumppic(m_Graphic, 0, 0));
	Add(new CGUIHitBox(0, 0, 0, 0, 0));
	Add(new CGUIButton(ID_GB_NEXT_WINDOW_DIRECTION, 0x7585, 0x7589, 0x7589, 0, 0));
	Add(new CGUIAlphaBlending(true, 0.1f));
	Add(new CGUIAlphaBlending(false, 0.1f));
}
//----------------------------------------------------------------------------------
CGumpBuff::~CGumpBuff()
{
}
//----------------------------------------------------------------------------------
void CGumpBuff::UpdateBuffIcons()
{
	for (CBaseGUI *item = (CBaseGUI*)m_Items; item != NULL;)
	{
		CBaseGUI *next = (CBaseGUI*)item->m_Next;

		if (item->Type == GOT_BUFF)
		{
			CGUIBuff *buff = (CGUIBuff*)item;

			int delta = (int)(buff->Timer - g_Ticks);

			if (buff->Timer != 0xFFFFFFFF && delta < USE_ALPHA_BLENDING_WHEN_TIMER_LESS)
			{
				m_WantRedraw = true;

				if (delta <= 0)
				{
					Delete(buff);
					m_WantUpdateContent = true;
				}
				else
				{
					int alpha = buff->Alpha;
					int addVal = (USE_ALPHA_BLENDING_WHEN_TIMER_LESS - delta) / ALPHA_CHANGE_KOEFF;

					if (buff->DecAlpha)
					{
						alpha -= addVal;

						if (alpha <= MUNIMUM_ICON_ALPHA)
						{
							buff->DecAlpha = false;
							alpha = MUNIMUM_ICON_ALPHA;
						}
					}
					else
					{
						alpha += addVal;

						if (alpha >= 255)
						{
							buff->DecAlpha = true;
							alpha = 255;
						}
					}

					buff->Alpha = alpha;
				}
			}
		}

		item = next;
	}
}
//----------------------------------------------------------------------------------
void CGumpBuff::AddBuff(const ushort &id, const ushort &timer, const wstring &text)
{
	DWORD ticks = 0xFFFFFFFF;

	if (timer)
		ticks = g_Ticks + (timer * 1000);

	QFOR(item, m_Items, CBaseGUI*)
	{
		if (item->Type == GOT_BUFF && item->Graphic == id)
		{
			CGUIBuff *buff = (CGUIBuff*)item;

			buff->Timer = ticks;
			buff->Text = text;
			buff->DecAlpha = true;
			buff->Alpha = 0xFF;
			buff->TooltipTimer = 0;

			MoveToBack(buff);
			m_WantUpdateContent = true;

			return;
		}
	}

	Add(new CGUIBuff(id, ticks, text));
}
//----------------------------------------------------------------------------------
void CGumpBuff::DeleteBuff(const ushort &id)
{
	QFOR(item, m_Items, CBaseGUI*)
	{
		if (item->Type == GOT_BUFF && item->Graphic == id)
		{
			Delete(item);
			m_WantUpdateContent = true;

			break;
		}
	}
}
//----------------------------------------------------------------------------------
void CGumpBuff::InitToolTip()
{
	if (g_SelectedObject.Serial == ID_GB_NEXT_WINDOW_DIRECTION)
	{
	}
	else if (g_SelectedObject.Object())
	{
		QFOR(item, m_Items, CBaseGUI*)
		{
			if (item != g_SelectedObject.Object())
				continue;

			if (item->Type == GOT_BUFF)
			{
				CGUIBuff *buff = (CGUIBuff*)item;

				g_FontManager.SetUseHTML(true);

				if (buff->Timer != 0xFFFFFFFF && buff->TooltipTimer < g_Ticks)
				{
					buff->TooltipTimer = g_Ticks + ((buff->Timer - g_Ticks) % 1000);
					g_ToolTip.Reset();

					wchar_t buf[512] = { 0 };
					wsprintf(buf, L"%s\nTimeLeft: %i seconds.", buff->Text.c_str(), (buff->Timer - g_Ticks) / 1000);

					g_ToolTip.Set(buf, buff);

					g_ToolTip.Timer = 0;
					g_ToolTip.Use = true;
				}
				else
					g_ToolTip.Set(buff->Text, buff);

				g_FontManager.SetUseHTML(false);
			}
			else
				g_ToolTip.Set(L"Buffs and Debuffs will appear here.", item, 120);

			break;
		}
	}
}
//----------------------------------------------------------------------------------
void CGumpBuff::GetGumpStatus(WISP_GEOMETRY::CPoint2Di &ball, WISP_GEOMETRY::CPoint2Di &items, bool &useX, bool &decX, bool &decY, WISP_GEOMETRY::CPoint2Di &startGump, WISP_GEOMETRY::CSize &endGump)
{
	startGump.X = 0;
	startGump.Y = 0;

	endGump = g_Orion.GetGumpDimension(m_Graphic);

	switch (m_Graphic)
	{
		case 0x757F: //v
		{
			ball.X = 0;
			ball.Y = 0;
			items.X = 25;
			items.Y = 25;
			decY = false;
			decX = false;
			useX = false;
			break;
		}
		case 0x7581: //^
		{
			ball.X = 34;
			ball.Y = 78;
			items.X = 7;
			items.Y = 52;
			decY = true;
			decX = false;
			useX = false;
			break;
		}
		case 0x7582: //<
		{
			ball.X = 76;
			ball.Y = 36;
			items.X = 52;
			items.Y = 7;
			decY = false;
			decX = true;
			useX = true;
			break;
		}
		case 0x7580: //>
		default:
		{
			ball.X = -2;
			ball.Y = 36;
			items.X = 20;
			items.Y = 7;
			decY = false;
			decX = false;
			useX = true;
			break;
		}
	}

	WISP_GEOMETRY::CPoint2Di itemsOfs = items;

	QFOR(item, m_Items, CBaseGUI*)
	{
		if (item->Type != GOT_BUFF)
			continue;

		bool moved = false;

		WISP_GEOMETRY::CSize gumpDim = g_Orion.GetGumpDimension(item->Graphic);

		if (useX)
		{
			if (decX)
				itemsOfs.X -= gumpDim.Width + BUFF_ITEM_STEP_OFFSET_X;
			else
				itemsOfs.X += gumpDim.Width + BUFF_ITEM_STEP_OFFSET_X;
		}
		else
		{
			if (decY)
				itemsOfs.Y -= gumpDim.Height + BUFF_ITEM_STEP_OFFSET_Y;
			else
				itemsOfs.Y += gumpDim.Height + BUFF_ITEM_STEP_OFFSET_Y;
		}
	}

	if (useX)
	{
		if (decX)
			itemsOfs.X -= 20;
		else
			itemsOfs.X += 20;
	}
	else
	{
		if (decY)
			itemsOfs.Y -= 20;
		else
			itemsOfs.Y += 20;
	}

	if (itemsOfs.X < startGump.X)
		startGump.X = itemsOfs.X;

	if (itemsOfs.Y < startGump.Y)
		startGump.Y = itemsOfs.Y;

	if (itemsOfs.X > endGump.Width)
		endGump.Width = itemsOfs.X;

	if (itemsOfs.Y > endGump.Height)
		endGump.Height = itemsOfs.Y;
}
//----------------------------------------------------------------------------------
void CGumpBuff::PrepareContent()
{
	if (m_Graphic < 0x757F || m_Graphic > 0x7582)
	{
		m_Graphic = 0x7580;
		m_WantUpdateContent = true;
	}
}
//----------------------------------------------------------------------------------
void CGumpBuff::UpdateContent()
{
	bool decX = false;
	bool decY = false;
	bool useX = true;

	WISP_GEOMETRY::CPoint2Di ballCoordinates;
	WISP_GEOMETRY::CPoint2Di startCoordinates;
	WISP_GEOMETRY::CPoint2Di startGump;
	WISP_GEOMETRY::CSize endGump;

	GetGumpStatus(ballCoordinates, startCoordinates, useX, decX, decY, startGump, endGump);

	//Body
	CBaseGUI *gui = (CBaseGUI*)m_Items;
	gui->Graphic = m_Graphic;

	//Selection zone
	gui = (CBaseGUI*)gui->m_Next;
	gui->X = startGump.X;
	gui->Y = startGump.Y;
	((CGUIPolygonal*)gui)->Width = endGump.Width;
	((CGUIPolygonal*)gui)->Height = endGump.Height;

	//Crystall ball
	gui = (CBaseGUI*)gui->m_Next;
	gui->X = ballCoordinates.X;
	gui->Y = ballCoordinates.Y;

	gui = NULL;

	QFOR(item, m_Items, CBaseGUI*)
	{
		if (item->Type != GOT_BUFF)
		{
			if (item->Type == GOT_BLENDING && !item->Enabled)
				gui = item;

			continue;
		}

		CGUIBuff *buff = (CGUIBuff*)item;

		WISP_GEOMETRY::CSize gumpDim = g_Orion.GetGumpDimension(buff->Graphic);
		buff->X = startCoordinates.X;
		buff->Y = startCoordinates.Y;

		if (useX)
		{
			if (decX)
				startCoordinates.X -= gumpDim.Width + BUFF_ITEM_STEP_OFFSET_X;
			else
				startCoordinates.X += gumpDim.Width + BUFF_ITEM_STEP_OFFSET_X;
		}
		else
		{
			if (decY)
				startCoordinates.Y -= gumpDim.Height + BUFF_ITEM_STEP_OFFSET_Y;
			else
				startCoordinates.Y += gumpDim.Height + BUFF_ITEM_STEP_OFFSET_Y;
		}
	}

	//���������� ������������ � ������� �������
	if (gui != NULL)
		MoveToBack(gui);
}
//----------------------------------------------------------------------------------
void CGumpBuff::GUMP_BUTTON_EVENT_C
{
	if (serial == ID_GB_NEXT_WINDOW_DIRECTION)
	{
		switch (m_Graphic)
		{
			case 0x7580:
			{
				m_Graphic = 0x7582;
				break;
			}
			case 0x7581:
			{
				m_Graphic = 0x757F;
				break;
			}
			case 0x7582:
			{
				m_Graphic = 0x7581;
				break;
			}
			case 0x757F:
			default:
			{
				m_Graphic = 0x7580;
				break;
			}
		}

		m_WantUpdateContent = true;
	}
	else if (serial == ID_GB_LOCK_MOVING)
		m_LockMoving = !m_LockMoving;
}
//----------------------------------------------------------------------------------
