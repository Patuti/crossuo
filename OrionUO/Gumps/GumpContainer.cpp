/***********************************************************************************
**
** GumpContainer.cpp
**
** Copyright (C) August 2016 Hotride
**
************************************************************************************
*/
//----------------------------------------------------------------------------------
#include "GumpContainer.h"
#include "../Container.h"
#include "../OrionUO.h"
#include "../OrionWindow.h"
#include "../ToolTip.h"
#include "../SelectedObject.h"
#include "../Game objects/GameWorld.h"
#include "../Game objects/ObjectOnCursor.h"
#include "../Game objects/GamePlayer.h"
#include "../ClickObject.h"
#include "../Target.h"
#include "../Managers/MouseManager.h"
#include "../PressedObject.h"
#include "../Gumps/GumpDrag.h"
#include "../Managers/GumpManager.h"
//----------------------------------------------------------------------------------
CGumpContainer::CGumpContainer(uint serial, short x, short y)
: CGump(GT_CONTAINER, serial, x, y), m_CorpseEyesTicks(0), m_CorpseEyesOffset(0),
m_IsGameBoard(false), m_TextRenderer(), m_CorpseEyes(NULL), m_DataBox(NULL)
{
	m_Page = 1;
	m_Locker.Serial = ID_GC_LOCK_MOVING;
}
//----------------------------------------------------------------------------------
CGumpContainer::~CGumpContainer()
{
	if (m_Graphic < CONTAINERS_COUNT)
	{
		ushort sound = g_ContainerOffset[m_Graphic].CloseSound;

		if (sound)
			g_Orion.PlaySoundEffect(sound);
	}
}
//----------------------------------------------------------------------------------
void CGumpContainer::CalculateGumpState()
{
	CGump::CalculateGumpState();

	if (g_GumpPressed && g_PressedObject.LeftObject() != NULL && g_PressedObject.LeftObject()->IsText())
	{
		g_GumpMovingOffset.Reset();

		if (m_Minimized)
		{
			g_GumpTranslate.X = (float)m_MinimizedX;
			g_GumpTranslate.Y = (float)m_MinimizedY;
		}
		else
		{
			g_GumpTranslate.X = (float)m_X;
			g_GumpTranslate.Y = (float)m_Y;
		}
	}
}
//----------------------------------------------------------------------------------
void CGumpContainer::PrepareTextures()
{
	CGump::PrepareTextures();
	g_Orion.ExecuteGumpPart(0x0045, 2); //Corpse eyes
}
//----------------------------------------------------------------------------------
void CGumpContainer::InitToolTip()
{
	if (!m_Minimized)
	{
		CGameObject *obj = g_World->FindWorldObject(g_SelectedObject.Serial);

		if (obj != NULL && obj->ClilocMessage.length())
			g_ToolTip.Set(obj->ClilocMessage, g_SelectedObject.Object());
	}
}
//----------------------------------------------------------------------------------
void CGumpContainer::PrepareContent()
{
	if (GetTopObjDistance(g_Player, g_World->FindWorldObject(Serial)) < 3 && g_PressedObject.LeftGump() == this && g_ObjectInHand == NULL && g_PressedObject.LeftSerial != ID_GC_MINIMIZE)
	{
		WISP_GEOMETRY::CPoint2Di offset = g_MouseManager.LeftDroppedOffset();

		if ((abs(offset.X) >= 3 || abs(offset.Y) >= 3) || (g_MouseManager.LastLeftButtonClickTimer + g_MouseManager.DoubleClickDelay < g_Ticks))
		{
			CGameItem *selobj = g_World->FindWorldItem(g_PressedObject.LeftSerial);

			if (selobj != NULL && selobj->IsStackable() && selobj->Count > 1)
			{
				CGumpDrag *newgump = new CGumpDrag(g_PressedObject.LeftSerial, g_MouseManager.Position.X - 80, g_MouseManager.Position.Y - 34);

				g_GumpManager.AddGump(newgump);
				g_OrionWindow.EmulateOnLeftMouseButtonDown();
				selobj->Dragged = true;
			}
			else if (selobj != NULL)
			{
				if (g_Target.IsTargeting())
					g_Target.SendCancelTarget();

				g_Orion.PickupItem(selobj, 0, m_IsGameBoard);

				g_PressedObject.ClearLeft();

				m_WantRedraw = false;
			}
		}
	}

	if (m_ID == 0x09 && m_CorpseEyes != NULL)
	{
		if (m_CorpseEyesTicks < g_Ticks)
		{
			m_CorpseEyesOffset = (uchar)!m_CorpseEyesOffset;
			m_CorpseEyesTicks = g_Ticks + 750;

			m_CorpseEyes->Graphic = 0x0045 + m_CorpseEyesOffset;
			m_WantRedraw = true;
		}
	}

	if (m_Minimized)
	{
		if (m_Page != 1)
		{
			m_Page = 1;
			m_WantRedraw = true;
		}
	}
	else
	{
		if (m_Page != 2)
		{
			m_Page = 2;
			m_WantRedraw = true;
		}
	}
}
//----------------------------------------------------------------------------------
void CGumpContainer::UpdateContent()
{
	CGameItem *container = g_World->FindWorldItem(Serial);

	if (container == NULL)
		return;

	if (m_Items == NULL)
	{
		Add(new CGUIPage(1));
		Add(new CGUIGumppic(0x0050, 0, 0));

		Add(new CGUIPage(2));
		Add(new CGUIGumppic((ushort)m_ID, 0, 0));

		if (m_ID == 0x09)
		{
			if (m_CorpseEyesTicks < g_Ticks)
			{
				m_CorpseEyesOffset = (BYTE)!m_CorpseEyesOffset;
				m_CorpseEyesTicks = g_Ticks + 750;
			}

			m_CorpseEyes = (CGUIGumppic*)Add(new CGUIGumppic(0x0045, 45, 30));
		}

		if (m_ID == 0x003C)
		{
			CGUIHitBox *box = (CGUIHitBox*)Add(new CGUIHitBox(ID_GC_MINIMIZE, 106, 162, 16, 16, true));
			box->ToPage = 1;
		}

		m_DataBox = (CGUIDataBox*)Add(new CGUIDataBox());
	}
	else
		m_DataBox->Clear();

	uint ignoreSerial = 0;

	if (g_ObjectInHand != NULL)
		ignoreSerial = g_ObjectInHand->Serial;

	m_DataBox->Add(new CGUIShader(g_ColorizerShader, true));

	QFOR(obj, container->m_Items, CGameItem*)
	{
		int count = obj->Count;

		if ((obj->Layer == OL_NONE || (container->IsCorpse() && LAYER_UNSAFE[obj->Layer])) && count > 0 && obj->Serial != ignoreSerial)
		{
			bool doubleDraw = false;
			ushort graphic = obj->GetDrawGraphic(doubleDraw);
			CGUIGumppicHightlighted *item = NULL;

			if (m_IsGameBoard)
			{
				item = (CGUIGumppicHightlighted*)m_DataBox->Add(new CGUIGumppicHightlighted(obj->Serial, graphic - GAME_FIGURE_GUMP_OFFSET, obj->Color, 0x0035, obj->X, obj->Y - 20));
				item->PartialHue = false;
			}
			else
			{
				item = (CGUIGumppicHightlighted*)m_DataBox->Add(new CGUITilepicHightlighted(obj->Serial, graphic, obj->Color, 0x0035, obj->X, obj->Y, doubleDraw));
				item->PartialHue = IsPartialHue(g_Orion.GetStaticFlags(graphic));
			}
		}
	}

	m_DataBox->Add(new CGUIShader(g_ColorizerShader, false));
}
//----------------------------------------------------------------------------------
void CGumpContainer::Draw()
{
	CalculateGumpState();
	
	//���������� ���������, ������������ � ��������� ������
	CRenderTextObject *rto = m_TextRenderer.m_TextItems;
	
	if (!m_Minimized)
	{
		m_TextRenderer.ClearRect();

		//���������� ��������� � ������������ ������
		for (; rto != NULL; rto = rto->m_NextDraw)
		{
			if (!rto->IsText())
			{
				if (rto->m_NextDraw == NULL)
					break;

				continue;
			}

			CTextData *td = (CTextData*)rto;

			if (td->Type == TT_OBJECT && td->Timer >= g_Ticks)
			{
				CGameObject *go = g_World->FindWorldObject(td->Serial);

				if (go != NULL && go->Graphic < 0x4000)
				{
					CGLTextTexture &tth = td->m_Texture;
					int drawX = td->X - go->GetTextOffsetX(td);
					int drawY = td->Y - go->GetTextOffsetY(td);

					CTextImageBounds ib(drawX, drawY, tth.Width, tth.Height, td);

					td->Transparent = m_TextRenderer.InRect(ib, go);

					m_TextRenderer.AddRect(ib);
				}
			}

			if (rto->m_NextDraw == NULL)
				break;
		}
	}

	CGump::Draw();

	if (!m_Minimized)
	{
		glTranslatef(g_GumpTranslate.X, g_GumpTranslate.Y, 0.0f);

		//��������� ������
		for (; rto != NULL; rto = rto->m_PrevDraw)
		{
			if (!rto->IsText())
				continue;

			CTextData *td = (CTextData*)rto;

			if (td->Type == TT_OBJECT && td->Timer >= g_Ticks)
			{
				CGLTextTexture &tth = td->m_Texture;

				int drawX = td->X - (tth.Width / 2);
				int drawY = td->Y;
				bool transparent = td->Transparent;

				while (td != NULL)
				{
					drawY -= td->m_Texture.Height;

					td = (CTextData*)td->m_Next;
				}

				if (transparent)
				{
					glEnable(GL_BLEND);
					glBlendFunc(GL_ONE, GL_DST_COLOR);

					tth.Draw(drawX, drawY);

					glDisable(GL_BLEND);
				}
				else
					tth.Draw(drawX, drawY);
			}
		}

		glTranslatef(-g_GumpTranslate.X, -g_GumpTranslate.Y, 0.0f);
	}
}
//----------------------------------------------------------------------------------
CRenderObject *CGumpContainer::Select()
{
	//���������� ���������, ������������ � ��������� ������
	CRenderTextObject *rto = m_TextRenderer.m_TextItems;

	if (!m_Minimized)
	{
		m_TextRenderer.ClearRect();

		//���������� ��������� � ������������ ������
		for (; rto != NULL; rto = rto->m_NextDraw)
		{
			if (!rto->IsText())
			{
				if (rto->m_NextDraw == NULL)
					break;

				continue;
			}

			if (rto->m_NextDraw == NULL)
				break;
		}
	}

	CRenderObject *selected = CGump::Select();

	WISP_GEOMETRY::CPoint2Di oldPos = g_MouseManager.Position;
	g_MouseManager.Position = WISP_GEOMETRY::CPoint2Di(oldPos.X - (int)g_GumpTranslate.X, oldPos.Y - (int)g_GumpTranslate.Y);

	if (!m_Minimized)
	{
		//�������� ������
		for (; rto != NULL; rto = rto->m_PrevDraw)
		{
			if (!rto->IsText())
				continue;

			CTextData *td = (CTextData*)rto;

			if (td->Type == TT_OBJECT && td->Timer >= g_Ticks)
			{
				CGLTextTexture &tth = td->m_Texture;

				int drawX = td->X - (tth.Width / 2);
				int drawY = td->Y;

				while (td != NULL)
				{
					drawY -= td->m_Texture.Height;

					td = (CTextData*)td->m_Next;
				}

				if (tth.UnderMouse(drawX, drawY))
				{
					g_SelectedObject.Init(rto, this);
					g_SelectedObject.Serial = rto->Serial;
				}
			}
		}
	}

	g_MouseManager.Position = oldPos;

	return selected;
}
//----------------------------------------------------------------------------------
void CGumpContainer::GUMP_BUTTON_EVENT_C
{
	if (!m_Minimized && serial == ID_GC_MINIMIZE && m_ID == 0x003C)
		m_Minimized = true;
	else if (serial == ID_GC_LOCK_MOVING)
	{
		m_LockMoving = !m_LockMoving;
		g_MouseManager.CancelDoubleClick = true;
	}
}
//----------------------------------------------------------------------------------
void CGumpContainer::OnLeftMouseButtonUp()
{
	CGump::OnLeftMouseButtonUp();

	uint dropContainer = m_Serial;
	uint selectedSerial = g_SelectedObject.Serial;

	bool canDrop = (GetTopObjDistance(g_Player, g_World->FindWorldObject(dropContainer)) < 3);

	if (canDrop && selectedSerial && selectedSerial != ID_GC_MINIMIZE && selectedSerial != ID_GC_LOCK_MOVING)
	{
		canDrop = false;

		if (g_Target.IsTargeting())
		{
			g_Target.SendTargetObject(selectedSerial);

			return;
		}
		else if (g_ObjectInHand != NULL)
		{
			canDrop = true;

			CGameItem *target = g_World->FindWorldItem(selectedSerial);

			if (target != NULL)
			{
				if (target->IsContainer())
					dropContainer = target->Serial;
				else if (target->IsStackable() && target->Graphic == g_ObjectInHand->Graphic)
					dropContainer = target->Serial;
			}
		}
	}

	if (!canDrop && g_ObjectInHand != NULL)
		g_Orion.PlaySoundEffect(0x0051);

	int x = g_MouseManager.Position.X - m_X;
	int y = g_MouseManager.Position.Y - m_Y;

	if (canDrop && g_ObjectInHand != NULL)
	{
		CONTAINER_OFFSET_RECT &r = g_ContainerOffset[Graphic].rect;

		bool doubleDraw = false;
		WORD graphic = g_ObjectInHand->GetDrawGraphic(doubleDraw);

		CGLTexture *th = g_Orion.ExecuteStaticArt(graphic);

		if (m_IsGameBoard)
		{
			th = g_Orion.ExecuteGump(graphic - GAME_FIGURE_GUMP_OFFSET);
			y += 20;
		}

		if (th != NULL)
		{
			x -= (th->Width / 2);
			y -= (th->Height / 2);

			if (x + th->Width > r.MaxX)
				x = r.MaxX - th->Width;

			if (y + th->Height > r.MaxY)
				y = r.MaxY - th->Height;
		}

		if (x < r.MinX)
			x = r.MinX;

		if (y < r.MinY)
			y = r.MinY;

		if (dropContainer != m_Serial)
		{
			CGameItem *target = g_World->FindWorldItem(selectedSerial);

			if (target->IsContainer())
			{
				x = -1;
				y = -1;
			}
		}

		g_Orion.DropItem(dropContainer, x, y, 0);
	}
	else if (g_ObjectInHand == NULL)
	{
		if (!g_ClickObject.Enabled)
		{
			CGameObject *clickTarget = g_World->FindWorldObject(selectedSerial);

			if (clickTarget != NULL)
			{
				g_ClickObject.Init(clickTarget);
				g_ClickObject.Timer = g_Ticks + g_MouseManager.DoubleClickDelay;
				g_ClickObject.X = x;
				g_ClickObject.Y = y;
			}
		}
	}
}
//----------------------------------------------------------------------------------
bool CGumpContainer::OnLeftMouseButtonDoubleClick()
{
	bool result = false;

	if (!g_PressedObject.LeftSerial && m_Minimized && m_ID == 0x003C)
	{
		m_Minimized = false;
		m_Page = 2;
		m_WantRedraw = true;

		result = true;
	}
	else if (g_PressedObject.LeftSerial && g_PressedObject.LeftSerial != ID_GC_MINIMIZE)
	{
		g_Orion.DoubleClick(g_PressedObject.LeftSerial);

		result = true;
	}

	return result;
}
//----------------------------------------------------------------------------------
