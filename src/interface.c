#ifdef MACOSX
#include <CoreFoundation/CFString.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bzlib.h>
#include <math.h>
#include <time.h>
#include <http.h>
#include <md5.h>
#include <font.h>
#include <defines.h>
#include <powder.h>
#include <interface.h>
#include <misc.h>
#include <renderer.h>


//char pyready=1;
SDLMod sdl_mod;
int sdl_key, sdl_wheel, sdl_caps=0, sdl_ascii, sdl_zoom_trig=0;

char *shift_0="`1234567890-=[]\\;',./";
char *shift_1="~!@#$%^&*()_+{}|:\"<>?";

int svf_login = 0;
int svf_admin = 0;
int svf_mod = 0;
char svf_user[64] = "";
char svf_user_id[64] = "";
char svf_pass[64] = "";
char svf_session_id[64] = "";

int svf_open = 0;
int svf_own = 0;
int svf_myvote = 0;
int svf_publish = 0;
char svf_id[16] = "";
char svf_name[64] = "";
char svf_description[255] = "";
char svf_tags[256] = "";
void *svf_last = NULL;
int svf_lsize;

char *search_ids[GRID_X*GRID_Y];
char *search_dates[GRID_X*GRID_Y];
int   search_votes[GRID_X*GRID_Y];
int   search_publish[GRID_X*GRID_Y];
int	  search_scoredown[GRID_X*GRID_Y];
int	  search_scoreup[GRID_X*GRID_Y];
char *search_names[GRID_X*GRID_Y];
char *search_owners[GRID_X*GRID_Y];
void *search_thumbs[GRID_X*GRID_Y];
int   search_thsizes[GRID_X*GRID_Y];

int search_own = 0;
int search_fav = 0;
int search_date = 0;
int search_page = 0;
char search_expr[256] = "";

char *tag_names[TAG_MAX];
int tag_votes[TAG_MAX];

int zoom_en = 0;
int zoom_x=(XRES-ZSIZE_D)/2, zoom_y=(YRES-ZSIZE_D)/2;
int zoom_wx=0, zoom_wy=0;
unsigned char ZFACTOR = 256/ZSIZE_D;
unsigned char ZSIZE = ZSIZE_D;

void menu_count(void)//puts the number of elements in each section into .itemcount
{
	int i=0;
	msections[SC_WALL].itemcount = UI_WALLCOUNT-4;
	msections[SC_SPECIAL].itemcount = 4;
	for (i=0; i<PT_NUM; i++)
	{
		msections[ptypes[i].menusection].itemcount+=ptypes[i].menu;
	}

}

void Interface_GetSignPosition(int i, int *x0, int *y0, int *w, int *h)
{
	//Changing width if sign have special content
	if (strcmp(signs[i].text, "{p}")==0)
		*w = GetTextWidth("Pressure: -000.00");

	if (strcmp(signs[i].text, "{t}")==0)
		*w = GetTextWidth("Temp: 0000.00");

	if (sregexp(signs[i].text, "^{c:[0-9]*|.*}$")==0)
	{
		int sldr, startm;
		char buff[256];
		memset(buff, 0, sizeof(buff));
		for (sldr=3; signs[i].text[sldr-1] != '|'; sldr++)
			startm = sldr + 1;

		sldr = startm;
		while (signs[i].text[sldr] != '}')
		{
			buff[sldr - startm] = signs[i].text[sldr];
			sldr++;
		}
		*w = GetTextWidth(buff) + 5;
	}

	//Ususal width
	if (strcmp(signs[i].text, "{p}") && strcmp(signs[i].text, "{t}") && sregexp(signs[i].text, "^{c:[0-9]*|.*}$"))
		*w = GetTextWidth(signs[i].text) + 5;
	*h = 14;
	*x0 = (signs[i].ju == 2) ? signs[i].x - *w :
	      (signs[i].ju == 1) ? signs[i].x - *w/2 : signs[i].x;
	*y0 = (signs[i].y > 18) ? signs[i].y - 18 : signs[i].y + 4;
}

void add_sign_ui(int mx, int my)
{
	int i, w, h, x, y, nm=0, ju;
	int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq;
	ui_edit ed;

	// check if it is an existing sign
	for (i=0; i<MAXSIGNS; i++)
		if (signs[i].text[0])
		{
			if (i == MSIGN)
			{
				MSIGN = -1;
				return;
			}
			Interface_GetSignPosition(i, &x, &y, &w, &h);
			if (mx>=x && mx<=x+w && my>=y && my<=y+h)
				break;
		}
	// else look for empty spot
	if (i >= MAXSIGNS)
	{
		nm = 1;
		for (i=0; i<MAXSIGNS; i++)
			if (!signs[i].text[0])
				break;
	}
	if (i >= MAXSIGNS)
		return;
	if (nm)
	{
		signs[i].x = mx;
		signs[i].y = my;
		signs[i].ju = 1;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	ed.x = x0+25;
	ed.y = y0+25;
	ed.w = 158;
	ed.nx = 1;
	ed.def = "[message]";
	ed.focus = 1;
	ed.hide = 0;
	ed.cursor = strlen(signs[i].text);
	ed.multiline = 0;
	strcpy(ed.str, signs[i].text);
	ju = signs[i].ju;

	Renderer_FillRectangle(-1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_DrawRectangle(x0, y0, 192, 80, 192, 192, 192, 255);
		Renderer_ClearRectangle(x0, y0, 192, 80);
		Graphics_RenderText(x0+8, y0+8, nm ? "New sign:" : "Edit sign:", 255, 255, 255, 255);
		Graphics_RenderText(x0+12, y0+23, "\xA1", 32, 64, 128, 255);
		Graphics_RenderText(x0+12, y0+23, "\xA0", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0+8, y0+20, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(&ed);
		Graphics_RenderText(x0+8, y0+46, "Justify:", 255, 255, 255, 255);
		Graphics_RenderIcon(x0+50, y0+42, 0x9D, ju == 0);
		Graphics_RenderIcon(x0+68, y0+42, 0x9E, ju == 1);
		Graphics_RenderIcon(x0+86, y0+42, 0x9F, ju == 2);





		if (!nm)
		{
			Graphics_RenderText(x0+138, y0+45, "\x86", 160, 48, 32, 255);
			Graphics_RenderText(x0+138, y0+45, "\x85", 255, 255, 255, 255);
			Graphics_RenderText(x0+152, y0+46, "Delete", 255, 255, 255, 255);
			Renderer_DrawRectangle(x0+134, y0+42, 50, 15, 255, 255, 255, 255);
			Renderer_DrawRectangle(x0+104,y0+42,26,15,255,255,255,255);
			Graphics_RenderText(x0+110, y0+48, "Mv.", 255, 255, 255, 255);
		}

		Graphics_RenderText(x0+5, y0+69, "OK", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+64, 192, 16, 192, 192, 192, 255);

		Renderer_Display();

		ui_edit_process(mx, my, b, &ed);

		if (b && !bq && mx>=x0+50 && mx<=x0+67 && my>=y0+42 && my<=y0+59)
			ju = 0;
		if (b && !bq && mx>=x0+68 && mx<=x0+85 && my>=y0+42 && my<=y0+59)
			ju = 1;
		if (b && !bq && mx>=x0+86 && mx<=x0+103 && my>=y0+42 && my<=y0+59)
			ju = 2;

		if (!nm && b && !bq && mx>=x0+104 && mx<=x0+130 && my>=y0+42 && my<=y0+59)
		{
			MSIGN = i;
			break;
		}
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (!nm && b && !bq && mx>=x0+134 && my>=y0+42 && mx<=x0+184 && my<=y0+59)
		{
			signs[i].text[0] = 0;
			return;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				return;
			ed.focus = 0;
		}
	}

	strcpy(signs[i].text, ed.str);
	signs[i].ju = ju;
}
//TODO: Finish text wrapping in text edits
void ui_edit_draw(ui_edit *ed)
{
	int cx, i, cy;
	char echo[256], *str;

	if (ed->hide)
	{
		for (i=0; ed->str[i]; i++)
			echo[i] = 0x8D;
		echo[i] = 0;
		str = echo;
	}
	else
		str = ed->str;

	if (ed->str[0])
	{
		if (ed->multiline) {
			Graphics_RenderWrapText(ed->x, ed->y, ed->w-14, str, 255, 255, 255, 255);
			Graphics_RenderText(ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
		} else {
			Graphics_RenderText(ed->x, ed->y, str, 255, 255, 255, 255);
			Graphics_RenderText(ed->x+ed->w-11, ed->y-1, "\xAA", 128, 128, 128, 255);
		}
	}
	else if (!ed->focus)
		Graphics_RenderText(ed->x, ed->y, ed->def, 128, 128, 128, 255);
	if (ed->focus)
	{
		if (ed->multiline) {
			GetTextNPos(str, ed->cursor, ed->w-14, &cx, &cy);
		} else {
			cx = GetTextNWidth(str, ed->cursor);
			cy = 0;
		}

		for (i=-3; i<9; i++)
			Renderer_BlendPixel(ed->x+cx, ed->y+i+cy, 255, 255, 255, 255);
	}
}

void ui_edit_process(int mx, int my, int mb, ui_edit *ed)
{
	char ch, ts[2], echo[256], *str;
	int l, i;
#ifdef RAWINPUT
	char *p;
#endif

	if (mb)
	{
		if (ed->hide)
		{
			for (i=0; ed->str[i]; i++)
				echo[i] = 0x8D;
			echo[i] = 0;
			str = echo;
		}
		else
			str = ed->str;

		if (ed->multiline) {
			if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = 0;
				ed->str[0] = 0;
			}
			else if (mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+ed->h)
			{
				ed->focus = 1;
				ed->cursor = GetTextPosXY(str, ed->w-14, mx-ed->x, my-ed->y);
			}
			else
				ed->focus = 0;
		} else {
			if (mx>=ed->x+ed->w-11 && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = 0;
				ed->str[0] = 0;
			}
			else if (mx>=ed->x-ed->nx && mx<ed->x+ed->w && my>=ed->y-5 && my<ed->y+11)
			{
				ed->focus = 1;
				ed->cursor = GetTextWidthX(str, mx-ed->x);
			}
			else
				ed->focus = 0;
		}
	}
	if (ed->focus && sdl_key)
	{
		if (ed->hide)
		{
			for (i=0; ed->str[i]; i++)
				echo[i] = 0x8D;
			echo[i] = 0;
			str = echo;
		}
		else
			str = ed->str;

		l = strlen(ed->str);
		switch (sdl_key)
		{
		case SDLK_HOME:
			ed->cursor = 0;
			break;
		case SDLK_END:
			ed->cursor = l;
			break;
		case SDLK_LEFT:
			if (ed->cursor > 0)
				ed->cursor --;
			break;
		case SDLK_RIGHT:
			if (ed->cursor < l)
				ed->cursor ++;
			break;
		case SDLK_DELETE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
				ed->str[ed->cursor] = 0;
			else if (ed->cursor < l)
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			break;
		case SDLK_BACKSPACE:
			if (sdl_mod & (KMOD_LCTRL|KMOD_RCTRL))
			{
				if (ed->cursor > 0)
					memmove(ed->str, ed->str+ed->cursor, l-ed->cursor+1);
				ed->cursor = 0;
			}
			else if (ed->cursor > 0)
			{
				ed->cursor--;
				memmove(ed->str+ed->cursor, ed->str+ed->cursor+1, l-ed->cursor);
			}
			break;
		default:
			if(sdl_mod & (KMOD_CTRL) && sdl_key=='c')//copy
			{
				clipboard_push_text(ed->str);
				break;
			}
			else if(sdl_mod & (KMOD_CTRL) && sdl_key=='v')//paste
			{
				char *paste = clipboard_pull_text();
				int pl = strlen(paste);
				if ((GetTextWidth(str)+GetTextWidth(paste) > ed->w-14 && !ed->multiline) || (pl+strlen(ed->str)>255) || (float)(((GetTextWidth(str)+GetTextWidth(paste))/(ed->w-14)*12) > ed->h && ed->multiline))
					break;
				memmove(ed->str+ed->cursor+pl, ed->str+ed->cursor, l+pl-ed->cursor);
				memcpy(ed->str+ed->cursor,paste,pl);
				ed->cursor += pl;
				break;
			}
#ifdef RAWINPUT
			if (sdl_key>=SDLK_SPACE && sdl_key<=SDLK_z && l<255)
			{
				ch = sdl_key;
				if ((sdl_mod & (KMOD_LSHIFT|KMOD_RSHIFT|KMOD_CAPS)))
				{
					if (ch>='a' && ch<='z')
						ch &= ~0x20;
					p = strchr(shift_0, ch);
					if (p)
						ch = shift_1[p-shift_0];
				}
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((GetTextWidth(str)+GetTextWidth(ts) > ed->w-14 && !ed->multiline) || (float)(((GetTextWidth(str)+GetTextWidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
			}
#else
			if (sdl_ascii>=' ' && sdl_ascii<127 && l<255)
			{
				ch = sdl_ascii;
				ts[0]=ed->hide?0x8D:ch;
				ts[1]=0;
				if ((GetTextWidth(str)+GetTextWidth(ts) > ed->w-14 && !ed->multiline) || (float)(((GetTextWidth(str)+GetTextWidth(ts))/(ed->w-14)*12) > ed->h && ed->multiline))
					break;
				memmove(ed->str+ed->cursor+1, ed->str+ed->cursor, l+1-ed->cursor);
				ed->str[ed->cursor] = ch;
				ed->cursor++;
			}
#endif
			break;
		}
	}
}

void ui_checkbox_draw(ui_checkbox *ed)
{
	int w = 12;
	if (ed->checked)
	{
		Graphics_RenderText(ed->x+2, ed->y+2, "\xCF", 128, 128, 128, 255);
	}
	if (ed->focus)
	{
		Renderer_DrawRectangle(ed->x, ed->y, w, w, 255, 255, 255, 255);
	}
	else
	{
		Renderer_DrawRectangle(ed->x, ed->y, w, w, 128, 128, 128, 255);
	}
}

void ui_checkbox_process(int mx, int my, int mb, int mbq, ui_checkbox *ed)
{
	int w = 12;

	if (mb && !mbq)
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->checked = (ed->checked)?0:1;
		}
	}
	else
	{
		if (mx>=ed->x && mx<=ed->x+w && my>=ed->y && my<=ed->y+w)
		{
			ed->focus = 1;
		}
		else
		{
			ed->focus = 0;
		}
	}
}

void ui_copytext_draw(ui_copytext *ed)
{
	int g = 180, i = 0;
	if (!ed->state) {
		if (ed->hover) {
			i = 0;
		} else {
			i = 100;
		}
		g = 255;
		Graphics_RenderText((ed->x+(ed->width/2))-(GetTextWidth("Click the box to copy the text")/2), ed->y-12, "Click the box to copy the text", 255, 255, 255, 255-i);
	} else {
		i = 0;
		Graphics_RenderText((ed->x+(ed->width/2))-(GetTextWidth("Copied!")/2), ed->y-12, "Copied!", 255, 255, 255, 255-i);
		g = 190;
	}

	Renderer_DrawRectangle(ed->x, ed->y, ed->width, ed->height, g, 255, g, 255-i);
	Renderer_DrawRectangle(ed->x+1, ed->y+1, ed->width-2, ed->height-2, g, 255, g, 100-i);
	Graphics_RenderText(ed->x+6, ed->y+5, ed->text, g, 255, g, 230-i);
}

void ui_copytext_process(int mx, int my, int mb, int mbq, ui_copytext *ed)
{
	if (my>=ed->y && my<=ed->y+ed->height && mx>=ed->x && mx<=ed->x+ed->width && !ed->state) {
		if (mb && !mbq) {
			clipboard_push_text(ed->text);
			ed->state = 1;
		}
		ed->hover = 1;
	} else {
		ed->hover = 0;
	}
}

void draw_svf_ui()// all the buttons at the bottom
{
	int c;

	//the open browser button
	Graphics_RenderText(4, YRES+(MENUSIZE-14), "\x81", 255, 255, 255, 255);
	Renderer_DrawRectangle(1, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

	// the reload button
	c = svf_open ? 255 : 128;
	Graphics_RenderText(23, YRES+(MENUSIZE-14), "\x91", c, c, c, 255);
	Renderer_DrawRectangle(19, YRES+(MENUSIZE-16), 16, 14, c, c, c, 255);

	// the save sim button
	c = svf_login ? 255 : 128;
	Graphics_RenderText(40, YRES+(MENUSIZE-14), "\x82", c, c, c, 255);
	if (svf_open)
		Graphics_RenderMaxText(58, YRES+(MENUSIZE-12), 125, svf_name, c, c, c, 255);
	else
		Graphics_RenderText(58, YRES+(MENUSIZE-12), "[untitled simulation]", c, c, c, 255);
	Renderer_DrawRectangle(37, YRES+(MENUSIZE-16), 150, 14, c, c, c, 255);
	if (svf_open && svf_own)
		Renderer_DrawDots(55, YRES+(MENUSIZE-15), 12, c, c, c, 255);

	c = (svf_login && svf_open) ? 255 : 128;

	//the vote buttons
	Renderer_DrawRectangle(189, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);
	Renderer_DrawRectangle(203, YRES+(MENUSIZE-16), 14, 14, c, c, c, 255);

	if (svf_myvote==1 && (svf_login && svf_open))
	{
		Renderer_FillRectangle(189, YRES+(MENUSIZE-16), 14, 14, 0, 108, 10, 255);
	}
	else if (svf_myvote==-1 && (svf_login && svf_open))
	{
		Renderer_FillRectangle(203, YRES+(MENUSIZE-16), 14, 14, 108, 10, 0, 255);
	}
	Graphics_RenderText(192, YRES+(MENUSIZE-12), "\xCB", 0, 187, 18, c);
	Graphics_RenderText(205, YRES+(MENUSIZE-14), "\xCA", 187, 40, 0, c);

	//the tags button
	Graphics_RenderText(222, YRES+(MENUSIZE-15), "\x83", c, c, c, 255);
	if (svf_tags[0])
		Graphics_RenderMaxText(240, YRES+(MENUSIZE-12), XRES+BARSIZE-405, svf_tags, c, c, c, 255);
	else
		Graphics_RenderText(240, YRES+(MENUSIZE-12), "[no tags set]", c, c, c, 255);

	Renderer_DrawRectangle(219, YRES+(MENUSIZE-16), XRES+BARSIZE-380, 14, c, c, c, 255);

	//the clear sim button------------some of the commented values are wrong
	Graphics_RenderText(XRES-139+BARSIZE/*371*/, YRES+(MENUSIZE-14), "\x92", 255, 255, 255, 255);
	Renderer_DrawRectangle(XRES-143+BARSIZE/*367*/, YRES+(MENUSIZE-16), 16, 14, 255, 255, 255, 255);

	//the login button
	Graphics_RenderText(XRES-122+BARSIZE/*388*/, YRES+(MENUSIZE-13), "\x84", 255, 255, 255, 255);
	if (svf_login)
		Graphics_RenderMaxText(XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), 66, svf_user, 255, 255, 255, 255);
	else
		Graphics_RenderText(XRES-104+BARSIZE/*406*/, YRES+(MENUSIZE-12), "[sign in]", 255, 255, 255, 255);
	Renderer_DrawRectangle(XRES-125+BARSIZE/*385*/, YRES+(MENUSIZE-16), 91, 14, 255, 255, 255, 255);

	//te pause button
	if (sys_pause)
	{
		Renderer_FillRectangle(XRES-17+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
		Graphics_RenderText(XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 0, 0, 0, 255);
	}
	else
	{
		Graphics_RenderText(XRES-14+BARSIZE/*496*/, YRES+(MENUSIZE-14), "\x90", 255, 255, 255, 255);
		Renderer_DrawRectangle(XRES-16+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//the heat sim button
	if (!legacy_enable)
	{
		Renderer_FillRectangle(XRES-160+BARSIZE/*493*/, YRES+(MENUSIZE-17), 16, 16, 255, 255, 255, 255);
		Graphics_RenderText(XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
		Graphics_RenderText(XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 0, 0, 0, 255);
	}
	else
	{
		Graphics_RenderText(XRES-154+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 255, 255, 255, 255);
		Renderer_DrawRectangle(XRES-159+BARSIZE/*494*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);
	}

	//the view mode button
	switch (cmode)
	{
	case CM_VEL:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x98", 128, 160, 255, 255);
		break;
	case CM_PRESS:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x99", 255, 212, 32, 255);
		break;
	case CM_PERS:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9A", 212, 212, 212, 255);
		break;
	case CM_FIRE:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9B", 255, 0, 0, 255);
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x9C", 255, 255, 64, 255);
		break;
	case CM_BLOB:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBF", 55, 255, 55, 255);
		break;
	case CM_HEAT:
		Graphics_RenderText(XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBE", 255, 0, 0, 255);
		Graphics_RenderText(XRES-27+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xBD", 255, 255, 255, 255);
		break;
	case CM_FANCY:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xC4", 100, 150, 255, 255);
		break;
	case CM_NOTHING:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x00", 100, 150, 255, 255);
		break;
	case CM_CRACK:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD4", 255, 55, 55, 255);
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD5", 55, 255, 55, 255);
		break;
	case CM_GRAD:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\xD3", 255, 50, 255, 255);
		break;
	case CM_LIFE:
		Graphics_RenderText(XRES-29+BARSIZE/*481*/, YRES+(MENUSIZE-13), "\x00", 255, 50, 255, 255);
		break;
	}
	Renderer_DrawRectangle(XRES-32+BARSIZE/*478*/, YRES+(MENUSIZE-16), 14, 14, 255, 255, 255, 255);

	// special icons for admin/mods
	if (svf_admin)
	{
		Graphics_RenderText(XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 232, 127, 35, 255);
		Graphics_RenderText(XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
		Graphics_RenderText(XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC8", 255, 255, 255, 255);
	}
	else if (svf_mod)
	{
		Graphics_RenderText(XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC9", 35, 127, 232, 255);
		Graphics_RenderText(XRES-45+BARSIZE/*463*/, YRES+(MENUSIZE-14), "\xC7", 255, 255, 255, 255);
	}//else if(amd)
	//	Graphics_RenderText(XRES-45/*465*/, YRES+(MENUSIZE-15), "\x97", 0, 230, 153, 255); Why is this here?
}

void error_ui(int err, char *txt)
{
	int x0=(XRES-240)/2,y0=YRES/2,b=1,bq,mx,my,textheight;
	char *msg;

	msg = malloc(strlen(txt)+16);
	if (err)
		sprintf(msg, "%03d %s", err, txt);
	else
		sprintf(msg, "%s", txt);
	textheight = GetTextWrapHeight(msg, 240);
	y0 -= (52+textheight)/2;
	if (y0<2)
		y0 = 2;
	if (y0+50+textheight>YRES)
		textheight = YRES-50-y0;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_ClearRectangle(x0-2, y0-2, 244, 52+textheight);
		Renderer_DrawRectangle(x0, y0, 240, 48+textheight, 192, 192, 192, 255);
		if (err)
			Graphics_RenderText(x0+8, y0+8, "HTTP error:", 255, 64, 32, 255);
		else
			Graphics_RenderText(x0+8, y0+8, "Error:", 255, 64, 32, 255);
		Graphics_RenderWrapText(x0+8, y0+26, 224, msg, 255, 255, 255, 255);
		Graphics_RenderText(x0+5, y0+textheight+37, "Dismiss", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+textheight+32, 240, 16, 192, 192, 192, 255);
		Renderer_Display();

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+textheight+32 && my<=y0+textheight+48)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	free(msg);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}

void info_ui(char *top, char *txt)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_ClearRectangle(x0-2, y0-2, 244, 64);
		Renderer_DrawRectangle(x0, y0, 240, 60, 192, 192, 192, 255);
		Graphics_RenderText(x0+8, y0+8, top, 160, 160, 255, 255);
		Graphics_RenderText(x0+8, y0+26, txt, 255, 255, 255, 255);
		Graphics_RenderText(x0+5, y0+49, "OK", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+44, 240, 16, 192, 192, 192, 255);
		Renderer_Display();

		if (b && !bq && mx>=x0 && mx<x0+240 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}

void info_box(char *msg)
{
	int w = GetTextWidth(msg)+16;
	int x0=(XRES-w)/2,y0=(YRES-24)/2;

	Renderer_ClearRectangle(x0-2, y0-2, w+4, 28);
	Renderer_DrawRectangle(x0, y0, w, 24, 192, 192, 192, 255);
	Graphics_RenderText(x0+8, y0+8, msg, 192, 192, 240, 255);
#ifndef RENDERER
	Renderer_Display();
#endif
}

void copytext_ui(char *top, char *txt, char *copytxt)
{
	int state = 0;
	int i;
	int g = 255;
	int xsize = 244;
	int ysize = 90;
	int x0=(XRES-xsize)/2,y0=(YRES-MENUSIZE-ysize)/2,b=1,bq,mx,my;
	int buttonx = 0;
	int buttony = 0;
	int buttonwidth = 0;
	int buttonheight = 0;
	ui_copytext ed;

	buttonwidth = GetTextWidth(copytxt)+12;
	buttonheight = 10+8;
	buttony = y0+50;
	buttonx = x0+(xsize/2)-(buttonwidth/2);

	ed.x = buttonx;
	ed.y = buttony;
	ed.width = buttonwidth;
	ed.height = buttonheight;
	ed.hover = 0;
	ed.state = 0;
	strcpy(ed.text, copytxt);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_ClearRectangle(x0-2, y0-2, xsize+4, ysize+4);
		Renderer_DrawRectangle(x0, y0, xsize, ysize, 192, 192, 192, 255);
		Graphics_RenderText(x0+8, y0+8, top, 160, 160, 255, 255);
		Graphics_RenderText(x0+8, y0+26, txt, 255, 255, 255, 255);

		ui_copytext_draw(&ed);
		ui_copytext_process(mx, my, b, bq, &ed);

		Graphics_RenderText(x0+5, y0+ysize-11, "OK", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+ysize-16, xsize, 16, 192, 192, 192, 255);

		Renderer_Display();

		if (b && !bq && mx>=x0 && mx<x0+xsize && my>=y0+ysize-16 && my<=y0+ysize)
			break;

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
}

int confirm_ui(char *top, char *msg, char *btn)
{
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2,b=1,bq,mx,my;
	int ret = 0;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_ClearRectangle(x0-2, y0-2, 244, 64);
		Renderer_DrawRectangle(x0, y0, 240, 60, 192, 192, 192, 255);
		Graphics_RenderText(x0+8, y0+8, top, 255, 216, 32, 255);
		Graphics_RenderText(x0+8, y0+26, msg, 255, 255, 255, 255);
		Graphics_RenderText(x0+5, y0+49, "Cancel", 255, 255, 255, 255);
		Graphics_RenderText(x0+165, y0+49, btn, 255, 216, 32, 255);
		Renderer_DrawRectangle(x0, y0+44, 160, 16, 192, 192, 192, 255);
		Renderer_DrawRectangle(x0+160, y0+44, 80, 16, 192, 192, 192, 255);
		Renderer_Display();

		if (b && !bq && mx>=x0+160 && mx<x0+240 && my>=y0+44 && my<=y0+60)
		{
			ret = 1;
			break;
		}
		if (b && !bq && mx>=x0 && mx<x0+160 && my>=y0+44 && my<=y0+60)
			break;

		if (sdl_key==SDLK_RETURN)
		{
			ret = 1;
			break;
		}
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	return ret;
}

void login_ui()
{
	int x0=(XRES-192)/2,y0=(YRES-80)/2,b=1,bq,mx,my,err;
	ui_edit ed1,ed2;
	char *res;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	ed1.x = x0+25;
	ed1.y = y0+25;
	ed1.w = 158;
	ed1.nx = 1;
	ed1.def = "[user name]";
	ed1.focus = 1;
	ed1.hide = 0;
	ed1.multiline = 0;
	ed1.cursor = strlen(svf_user);
	strcpy(ed1.str, svf_user);
	ed2.x = x0+25;
	ed2.y = y0+45;
	ed2.w = 158;
	ed2.nx = 1;
	ed2.def = "[password]";
	ed2.focus = 0;
	ed2.hide = 1;
	ed2.cursor = 0;
	ed2.multiline = 0;
	strcpy(ed2.str, "");

	Renderer_FillRectangle(-1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_DrawRectangle(x0, y0, 192, 80, 192, 192, 192, 255);
		Renderer_ClearRectangle(x0, y0, 192, 80);
		Graphics_RenderText(x0+8, y0+8, "Server login:", 255, 255, 255, 255);
		Graphics_RenderText(x0+12, y0+23, "\x8B", 32, 64, 128, 255);
		Graphics_RenderText(x0+12, y0+23, "\x8A", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0+8, y0+20, 176, 16, 192, 192, 192, 255);
		Graphics_RenderText(x0+11, y0+44, "\x8C", 160, 144, 32, 255);
		Graphics_RenderText(x0+11, y0+44, "\x84", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0+8, y0+40, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(&ed1);
		ui_edit_draw(&ed2);
		Graphics_RenderText(x0+5, y0+69, "Sign in", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+64, 192, 16, 192, 192, 192, 255);
		Renderer_Display();

		ui_edit_process(mx, my, b, &ed1);
		ui_edit_process(mx, my, b, &ed2);

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36)
			break;
		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+42 && my<y0+46)
			break;
		if (b && !bq && mx>=x0 && mx<x0+192 && my>=y0+64 && my<=y0+80)
			break;

		if (sdl_key==SDLK_RETURN || sdl_key==SDLK_TAB)
		{
			if (!ed1.focus)
				break;
			ed1.focus = 0;
			ed2.focus = 1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed1.focus && !ed2.focus)
				return;
			ed1.focus = 0;
			ed2.focus = 0;
		}
	}

	strcpy(svf_user, ed1.str);
	md5_ascii(svf_pass, (unsigned char *)ed2.str, 0);

	res = http_multipart_post(
	          "http://" SERVER "/Login.api",
	          NULL, NULL, NULL,
	          svf_user, svf_pass, NULL,
	          &err, NULL);
	if (err != 200)
	{
		error_ui(err, http_ret_text(err));
		if (res)
			free(res);
		goto fail;
	}
	if (res && !strncmp(res, "OK ", 3))
	{
		char *s_id,*u_e,*nres;
		printf("{%s}\n", res);
		s_id = strchr(res+3, ' ');
		if (!s_id)
			goto fail;
		*(s_id++) = 0;

		u_e = strchr(s_id, ' ');
		if (!u_e) {
			u_e = malloc(1);
			memset(u_e, 0, 1);
		}
		else
			*(u_e++) = 0;

		strcpy(svf_user_id, res+3);
		strcpy(svf_session_id, s_id);
		nres = mystrdup(u_e);

		printf("{%s} {%s} {%s}\n", svf_user_id, svf_session_id, nres);

		if (!strncmp(nres, "ADMIN", 5))
		{
			svf_admin = 1;
			svf_mod = 0;
		}
		else if (!strncmp(nres, "MOD", 3))
		{
			svf_admin = 0;
			svf_mod = 1;
		}
		else
		{
			svf_admin = 0;
			svf_mod = 0;
		}
		free(res);
		svf_login = 1;
		return;
	}
	if (!res)
		res = mystrdup("Unspecified Error");
	error_ui(0, res);
	free(res);

fail:
	strcpy(svf_user, "");
	strcpy(svf_pass, "");
	strcpy(svf_user_id, "");
	strcpy(svf_session_id, "");
	svf_login = 0;
	svf_own = 0;
	svf_admin = 0;
	svf_mod = 0;
}

int stamp_ui()
{
	int b=1,bq,mx,my,d=-1,i,j,k,x,gx,gy,y,w,h,r=-1,stamp_page=0,per_page=STAMP_X*STAMP_Y,page_count;
	char page_info[64];
	page_count = ceil((float)stamp_count/(float)per_page);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_ClearRectangle(-1, -1, XRES+1, YRES+MENUSIZE+1);
		k = stamp_page*per_page;//0;
		r = -1;
		d = -1;
		for (j=0; j<GRID_Y; j++)
			for (i=0; i<GRID_X; i++)
			{
				if (stamps[k].name[0] && stamps[k].thumb)
				{
					gx = ((XRES/GRID_X)*i) + (XRES/GRID_X-XRES/GRID_S)/2;
					gy = ((((YRES-MENUSIZE+20)+15)/GRID_Y)*j) + ((YRES-MENUSIZE+20)/GRID_Y-(YRES-MENUSIZE+20)/GRID_S+10)/2 + 18;
					x = (XRES*i)/GRID_X + XRES/(GRID_X*2);
					y = (YRES*j)/GRID_Y + YRES/(GRID_Y*2);
					gy -= 20;
					w = stamps[k].thumb_w;
					h = stamps[k].thumb_h;
					x -= w/2;
					y -= h/2;
					Renderer_DrawImage(stamps[k].thumb, gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h, 255);
					Renderer_XORRectangle(gx+(((XRES/GRID_S)/2)-(w/2)), gy+(((YRES/GRID_S)/2)-(h/2)), w, h);
					if (mx>=gx+XRES/GRID_S-4 && mx<(gx+XRES/GRID_S)+6 && my>=gy-6 && my<gy+4)
					{
						d = k;
						Renderer_DrawRectangle(gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					}
					else
					{
						if (mx>=gx && mx<gx+(XRES/GRID_S) && my>=gy && my<gy+(YRES/GRID_S))
						{
							r = k;
							Renderer_DrawRectangle(gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 210, 255);
						}
						else
						{
							Renderer_DrawRectangle(gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
						}
						Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x86", 150, 48, 32, 255);
					}
					Graphics_RenderText(gx+XRES/(GRID_S*2)-GetTextWidth(stamps[k].name)/2, gy+YRES/GRID_S+7, stamps[k].name, 192, 192, 192, 255);
					Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
				}
				k++;
			}

		sprintf(page_info, "Page %d of %d", stamp_page+1, page_count);

		Graphics_RenderText((XRES/2)-(GetTextWidth(page_info)/2), YRES+MENUSIZE-14, page_info, 255, 255, 255, 255);

		if (stamp_page)
		{
			Graphics_RenderText(4, YRES+MENUSIZE-14, "\x96", 255, 255, 255, 255);
			Renderer_DrawRectangle(1, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}
		if (stamp_page<page_count-1)
		{
			Graphics_RenderText(XRES-15, YRES+MENUSIZE-14, "\x95", 255, 255, 255, 255);
			Renderer_DrawRectangle(XRES-18, YRES+MENUSIZE-18, 16, 16, 255, 255, 255, 255);
		}

		if (b==1&&d!=-1)
		{
			if (confirm_ui("Do you want to delete?", stamps[d].name, "Delete"))
			{
				del_stamp(d);
			}
		}

		Renderer_Display();

		if (b==1&&r!=-1)
			break;
		if (b==4&&r!=-1)
		{
			r = -1;
			break;
		}

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel>0)
		{
			if (stamp_page)
			{
				stamp_page --;
			}
			sdl_wheel = 0;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-18 && my<YRES+MENUSIZE-2) || sdl_wheel<0)
		{
			if (stamp_page<page_count-1)
			{
				stamp_page ++;
			}
			sdl_wheel = 0;
		}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
		{
			r = -1;
			break;
		}
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	return r;
}

void tag_list_ui()
{
	int y,d,x0=(XRES-192)/2,y0=(YRES-256)/2,b=1,bq,mx,my,vp,vn;
	char *p,*q,s;
	char *tag=NULL, *op=NULL;
	ui_edit ed;
	struct strlist *vote=NULL,*down=NULL;

	ed.x = x0+25;
	ed.y = y0+221;
	ed.w = 158;
	ed.nx = 1;
	ed.def = "[new tag]";
	ed.focus = 0;
	ed.hide = 0;
	ed.cursor = 0;
	ed.multiline = 0;
	strcpy(ed.str, "");

	Renderer_FillRectangle(-1, -1, XRES, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		op = tag = NULL;

		Renderer_DrawRectangle(x0, y0, 192, 256, 192, 192, 192, 255);
		Renderer_ClearRectangle(x0, y0, 192, 256);
		Graphics_RenderText(x0+8, y0+8, "Current tags:", 255, 255, 255, 255);
		p = svf_tags;
		s = svf_tags[0] ? ' ' : 0;
		y = 36 + y0;
		while (s)
		{
			q = strchr(p, ' ');
			if (!q)
				q = p+strlen(p);
			s = *q;
			*q = 0;
			if (svf_own || svf_admin || svf_mod)
			{
				Graphics_RenderText(x0+20, y-1, "\x86", 160, 48, 32, 255);
				Graphics_RenderText(x0+20, y-1, "\x85", 255, 255, 255, 255);
				d = 14;
				if (b && !bq && mx>=x0+18 && mx<x0+32 && my>=y-2 && my<y+12)
				{
					op = "delete";
					tag = mystrdup(p);
				}
			}
			else
				d = 0;
			vp = strlist_find(&vote, p);
			vn = strlist_find(&down, p);
			if ((!vp && !vn && !svf_own) || svf_admin || svf_mod)
			{
				Graphics_RenderText(x0+d+20, y-1, "\x88", 32, 144, 32, 255);
				Graphics_RenderText(x0+d+20, y-1, "\x87", 255, 255, 255, 255);
				if (b && !bq && mx>=x0+d+18 && mx<x0+d+32 && my>=y-2 && my<y+12)
				{
					op = "vote";
					tag = mystrdup(p);
					strlist_add(&vote, p);
				}
				Graphics_RenderText(x0+d+34, y-1, "\x88", 144, 48, 32, 255);
				Graphics_RenderText(x0+d+34, y-1, "\xA2", 255, 255, 255, 255);
				if (b && !bq && mx>=x0+d+32 && mx<x0+d+46 && my>=y-2 && my<y+12)
				{
					op = "down";
					tag = mystrdup(p);
					strlist_add(&down, p);
				}
			}
			if (vp)
				Graphics_RenderText(x0+d+48+GetTextWidth(p), y, " - voted!", 48, 192, 48, 255);
			if (vn)
				Graphics_RenderText(x0+d+48+GetTextWidth(p), y, " - voted.", 192, 64, 32, 255);
			Graphics_RenderText(x0+d+48, y, p, 192, 192, 192, 255);
			*q = s;
			p = q+1;
			y += 16;
		}
		Graphics_RenderText(x0+11, y0+219, "\x86", 32, 144, 32, 255);
		Graphics_RenderText(x0+11, y0+219, "\x89", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0+8, y0+216, 176, 16, 192, 192, 192, 255);
		ui_edit_draw(&ed);
		Graphics_RenderText(x0+5, y0+245, "Close", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+240, 192, 16, 192, 192, 192, 255);
		Renderer_Display();

		ui_edit_process(mx, my, b, &ed);

		if (b && mx>=x0 && mx<=x0+192 && my>=y0+240 && my<y0+256)
			break;

		if (op)
		{
			d = execute_tagop(op, tag);
			free(tag);
			op = tag = NULL;
			if (d)
				goto finish;
		}

		if (b && !bq && mx>=x0+9 && mx<x0+23 && my>=y0+218 && my<y0+232)
		{
			d = execute_tagop("add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = 0;
			if (d)
				goto finish;
		}

		if (sdl_key==SDLK_RETURN)
		{
			if (!ed.focus)
				break;
			d = execute_tagop("add", ed.str);
			strcpy(ed.str, "");
			ed.cursor = 0;
			if (d)
				goto finish;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			strcpy(ed.str, "");
			ed.cursor = 0;
			ed.focus = 0;
		}
	}
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	sdl_key = 0;

finish:
	strlist_free(&vote);
}

int save_name_ui()
{
	int x0=(XRES-420)/2,y0=(YRES-68-YRES/4)/2,b=1,bq,mx,my,ths,idtxtwidth,nd=0;
	void *th;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	ui_edit ed;
	ui_edit ed2;
	ui_checkbox cb;
	ui_copytext ctb;

	th = build_thumb(&ths, 0);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	ed.x = x0+25;
	ed.y = y0+25;
	ed.w = 158;
	ed.nx = 1;
	ed.def = "[simulation name]";
	ed.focus = 1;
	ed.hide = 0;
	ed.cursor = strlen(svf_name);
	ed.multiline = 0;
	strcpy(ed.str, svf_name);

	ed2.x = x0+13;
	ed2.y = y0+45;
	ed2.w = 166;
	ed2.h = 85;
	ed2.nx = 1;
	ed2.def = "[simulation description]";
	ed2.focus = 0;
	ed2.hide = 0;
	ed2.cursor = strlen(svf_description);
	ed2.multiline = 1;
	strcpy(ed2.str, svf_description);

	ctb.x = 0;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = GetTextWidth(svf_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, svf_id);


	cb.x = x0+10;
	cb.y = y0+53+YRES/4;
	cb.focus = 0;
	cb.checked = svf_publish;

	Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	//memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		Renderer_DrawRectangle(x0, y0, 420, 90+YRES/4, 192, 192, 192, 255);
		Renderer_ClearRectangle(x0, y0, 420, 90+YRES/4);
		Graphics_RenderText(x0+8, y0+8, "New simulation name:", 255, 255, 255, 255);
		Graphics_RenderText(x0+10, y0+23, "\x82", 192, 192, 192, 255);
		Renderer_DrawRectangle(x0+8, y0+20, 176, 16, 192, 192, 192, 255);

		Renderer_DrawRectangle(x0+8, y0+40, 176, 95, 192, 192, 192, 255);

		ui_edit_draw(&ed);
		ui_edit_draw(&ed2);

		Renderer_DrawRectangle(x0+(205-XRES/3)/2-2+205, y0+30, XRES/3+3, YRES/3+3, 128, 128, 128, 255);
		Graphics_RenderThumbnail(th, ths, 0, x0+(205-XRES/3)/2+205, y0+32, 3);

		ui_checkbox_draw(&cb);
		Graphics_RenderText(x0+34, y0+50+YRES/4, "Publish? (Do not publish others'\nworks without permission)", 192, 192, 192, 255);

		Graphics_RenderText(x0+5, y0+79+YRES/4, "Save simulation", 255, 255, 255, 255);
		Renderer_DrawRectangle(x0, y0+74+YRES/4, 192, 16, 192, 192, 192, 255);

		Renderer_DrawLine(x0+192, y0, x0+192, y0+90+YRES/4, 150, 150, 150, XRES+BARSIZE);

		if (svf_id[0])
		{
			//Save ID text and copybox
			idtxtwidth = GetTextWidth("Current save ID: ");
			idtxtwidth += ctb.width;
			ctb.x = GetTextWidth("Current save ID: ")+(XRES+BARSIZE-idtxtwidth)/2;
			Graphics_RenderText((XRES+BARSIZE-idtxtwidth)/2, YRES+MENUSIZE-15, "Current save ID: ", 255, 255, 255, 255);

			ui_copytext_draw(&ctb);
			ui_copytext_process(mx, my, b, bq, &ctb);
		}

		Renderer_Display();

		//memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

		ui_edit_process(mx, my, b, &ed);
		ui_edit_process(mx, my, b, &ed2);
		ui_checkbox_process(mx, my, b, bq, &cb);

		if (b && !bq && ((mx>=x0+9 && mx<x0+23 && my>=y0+22 && my<y0+36) ||
		                 (mx>=x0 && mx<x0+192 && my>=y0+74+YRES/4 && my<y0+90+YRES/4)))
		{
			free(th);
			if (!ed.str[0])
				return 0;
			nd = strcmp(svf_name, ed.str) || !svf_own;
			strncpy(svf_name, ed.str, 63);
			svf_name[63] = 0;
			strncpy(svf_description, ed2.str, 254);
			svf_description[254] = 0;
			if (nd)
			{
				strcpy(svf_id, "");
				strcpy(svf_tags, "");
			}
			svf_open = 1;
			svf_own = 1;
			svf_publish = cb.checked;
			return nd+1;
		}

		if (sdl_key==SDLK_RETURN)
		{
			free(th);
			if (!ed.str[0])
				return 0;
			nd = strcmp(svf_name, ed.str) || !svf_own;
			strncpy(svf_name, ed.str, 63);
			svf_name[63] = 0;
			strncpy(svf_description, ed2.str, 254);
			svf_description[254] = 0;
			if (nd)
			{
				strcpy(svf_id, "");
				strcpy(svf_tags, "");
			}
			svf_open = 1;
			svf_own = 1;
			svf_publish = cb.checked;
			return nd+1;
		}
		if (sdl_key==SDLK_ESCAPE)
		{
			if (!ed.focus)
				break;
			ed.focus = 0;
		}
	}
	free(th);
	return 0;
}

//unused old function, with all the elements drawn at the bottom
/*
void menu_ui(int i, int *sl, int *sr)
{
	int b=1,bq,mx,my,h,x,y,n=0,height,width,sy,rows=0;
	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	Renderer_FillRectangle(-1, -1, XRES+1, YRES+MENUSIZE, 0, 0, 0, 192);
	memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;
		rows = ceil((float)msections[i].itemcount/16.0f);
		height = (ceil((float)msections[i].itemcount/16.0f)*18);
		width = restrict_flt(msections[i].itemcount*31, 0, 16*31);
		//Renderer_ClearRectangle(-1, -1, XRES+1, YRES+MENUSIZE+1);
		h = -1;
		x = XRES-BARSIZE-26;
		y = (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-(height/2)+(FONT_H/2)+1;
		sy = y;
		//Renderer_ClearRectangle((XRES-BARSIZE-width)+1, y-4, width+4, height+4+rows);
		Renderer_FillRectangle((XRES-BARSIZE-width)-7, y-10, width+16, height+16+rows, 0, 0, 0, 100);
		Renderer_DrawRectangle((XRES-BARSIZE-width)-7, y-10, width+16, height+16+rows, 255, 255, 255, 255);
		Renderer_FillRectangle((XRES-BARSIZE)+11, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-2, 15, FONT_H+3, 0, 0, 0, 100);
		Renderer_DrawRectangle((XRES-BARSIZE)+10, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-2, 16, FONT_H+3, 255, 255, 255, 255);
		Renderer_DrawRectangle((XRES-BARSIZE)+9, (((YRES/SC_TOTAL)*i)+((YRES/SC_TOTAL)/2))-1, 1, FONT_H+1, 0, 0, 0, 255);
		if (i==SC_WALL)
		{
			for (n = 122; n<122+UI_WALLCOUNT; n++)
			{
				if (n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= Graphics_RenderToolsXY(x, y, n, mwalls[n-122].colour)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}
		else if (i==SC_SPECIAL)
		{
			for (n = 122; n<122+UI_WALLCOUNT; n++)
			{
				if (n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= Graphics_RenderToolsXY(x, y, n, mwalls[n-122].colour)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
			for (n = 0; n<PT_NUM; n++)
			{
				if (ptypes[n].menusection==i&&ptypes[n].menu==1)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= Graphics_RenderToolsXY(x, y, n, ptypes[n].pcolors)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}
		else
		{
			for (n = 0; n<PT_NUM; n++)
			{
				if (ptypes[n].menusection==i&&ptypes[n].menu==1)
				{
					if (x-26<=60)
					{
						x = XRES-BARSIZE-26;
						y += 19;
					}
					x -= Graphics_RenderToolsXY(x, y, n, ptypes[n].pcolors)+5;
					if (mx>=x+32 && mx<x+58 && my>=y && my< y+15)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
						h = n;
					}
					else if (n==*sl)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					}
					else if (n==*sr)
					{
						Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
					}
				}
			}
		}

		if (h==-1)
		{
			Graphics_RenderText(XRES-GetTextWidth((char *)msections[i].name)-BARSIZE, sy+height+10, (char *)msections[i].name, 255, 255, 255, 255);
		}
		else if (i==SC_WALL||(i==SC_SPECIAL&&h>=122))
		{
			Graphics_RenderText(XRES-GetTextWidth((char *)mwalls[h-122].descs)-BARSIZE, sy+height+10, (char *)mwalls[h-122].descs, 255, 255, 255, 255);
		}
		else
		{
			Graphics_RenderText(XRES-GetTextWidth((char *)ptypes[h].descs)-BARSIZE, sy+height+10, (char *)ptypes[h].descs, 255, 255, 255, 255);
		}


		Renderer_Display();
		memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (!(mx>=(XRES-BARSIZE-width)-7 && my>=sy-10 && my<sy+height+9))
		{
			break;
		}

		if (b==1&&h!=-1)
		{
			*sl = h;
			break;
		}
		if (b==4&&h!=-1)
		{
			*sr = h;
			break;
		}
		//if(b==4&&h!=-1) {
		//	h = -1;
		//	break;
		//}

		if (sdl_key==SDLK_RETURN)
			break;
		if (sdl_key==SDLK_ESCAPE)
			break;
	}

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	//Graphics_RenderText(XRES+2, (12*i)+2, msections[i].icon, 255, 255, 255, 255);
}
*/
//current menu function
void menu_ui_v3(int i, int *sl, int *sr, int b, int bq, int mx, int my)
{
	int h,x,y,n=0,height,width,sy,rows=0,xoff=0,fwidth;
	SEC = SEC2;
	mx /= sdl_scale;
	my /= sdl_scale;
	rows = ceil((float)msections[i].itemcount/16.0f);
	height = (ceil((float)msections[i].itemcount/16.0f)*18);
	width = restrict_flt(msections[i].itemcount*31, 0, 16*31);
	fwidth = msections[i].itemcount*31;
	h = -1;
	x = XRES-BARSIZE-18;
	y = YRES+1;
	sy = y;
	if (i==SC_WALL)//wall menu
	{
		for (n = UI_WALLSTART; n<UI_WALLSTART+UI_WALLCOUNT; n++)
		{
			if (n!=SPC_AIR&&n!=SPC_HEAT&&n!=SPC_COOL&&n!=SPC_VACUUM)
			{
				/*if (x-18<=2)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= Graphics_RenderToolsXY(x, y, n, mwalls[n-UI_WALLSTART].colour)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT)))
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
				}
				else if (n==*sr)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
				}
			}
		}
	}
	else if (i==SC_SPECIAL)//special menu
	{
		for (n = UI_WALLSTART; n<UI_WALLSTART+UI_WALLCOUNT; n++)
		{
			if (n==SPC_AIR||n==SPC_HEAT||n==SPC_COOL||n==SPC_VACUUM)
			{
				/*if (x-18<=0)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= Graphics_RenderToolsXY(x, y, n, mwalls[n-UI_WALLSTART].colour)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT)))
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
				}
				else if (n==*sr)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
				}
			}
		}
		for (n = 0; n<PT_NUM; n++)
		{
			if (ptypes[n].menusection==i&&ptypes[n].menu==1)
			{
				/*if (x-18<=0)
				{
					x = XRES-BARSIZE-18;
					y += 19;
				}*/
				x -= Graphics_RenderToolsXY(x, y, n, ptypes[n].pcolors)+5;
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
					h = n;
				}
				if (!bq && mx>=x+32 && mx<x+58 && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT)))
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 255, 0, 0, 255);
				}
				else if (n==*sr)
				{
					Renderer_DrawRectangle(x+30, y-1, 29, 17, 0, 0, 255, 255);
				}
			}
		}
	}
	else //all other menus
	{
		if (fwidth > XRES-BARSIZE) { //fancy scrolling
			float overflow = fwidth-(XRES-BARSIZE), location = ((float)XRES-BARSIZE)/((float)(mx-(XRES-BARSIZE)));
			xoff = (int)(overflow / location);
		}
		for (n = 0; n<PT_NUM; n++)
		{
			if (ptypes[n].menusection==i&&ptypes[n].menu==1)
			{
				x -= Graphics_RenderToolsXY(x-xoff, y, n, ptypes[n].pcolors)+5;
				if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15)
				{
					Renderer_DrawRectangle(x+30-xoff, y-1, 29, 17, 255, 0, 0, 255);
					h = n;
				}
				if (!bq && mx>=x+32-xoff && mx<x+58-xoff && my>=y && my< y+15&&(sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT)))
				{
					Renderer_DrawRectangle(x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
					h = n;
				}
				else if (n==SLALT)
				{
					Renderer_DrawRectangle(x+30-xoff, y-1, 29, 17, 0, 255, 255, 255);
				}
				else if (n==*sl)
				{
					Renderer_DrawRectangle(x+30-xoff, y-1, 29, 17, 255, 0, 0, 255);
				}
				else if (n==*sr)
				{
					Renderer_DrawRectangle(x+30-xoff, y-1, 29, 17, 0, 0, 255, 255);
				}
			}
		}
	}
	if (!bq && mx>=((XRES+BARSIZE)-16) ) //highlight menu section
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))
			if (i>=0&&i<SC_TOTAL)
				SEC = i;

	if (h==-1)
	{
		Graphics_RenderText(XRES-GetTextWidth((char *)msections[i].name)-BARSIZE, sy-10, (char *)msections[i].name, 255, 255, 255, 255);
	}
	else if (i==SC_WALL||(i==SC_SPECIAL&&h>=UI_WALLSTART))
	{
		Graphics_RenderText(XRES-GetTextWidth((char *)mwalls[h-UI_WALLSTART].descs)-BARSIZE, sy-10, (char *)mwalls[h-UI_WALLSTART].descs, 255, 255, 255, 255);
	}
	else
	{
		Graphics_RenderText(XRES-GetTextWidth((char *)ptypes[h].descs)-BARSIZE, sy-10, (char *)ptypes[h].descs, 255, 255, 255, 255);
	}
	//these are click events, b=1 is left click, b=4 is right
	//h has the value of the element it is over, and -1 if not over an element
	if (b==1&&h==-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT) && SEC>=0)
		{
			SLALT = -1;
			SEC2 = SEC;
		}
	}
	if (b==1&&h!=-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))
		{
			SLALT = h;
			SEC2 = -1;
		}
		else {
			*sl = h;
		}
	}
	if (b==4&&h==-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT) && SEC>=0)
		{
			SLALT = -1;
			SEC2 = SEC;
		}
	}
	if (b==4&&h!=-1)
	{
		if (sdl_mod & (KMOD_LALT) && sdl_mod & (KMOD_SHIFT))
		{
			SLALT = h;
			SEC2 = -1;
		}
		else {
			*sr = h;
		}
	}
}

int sdl_poll(void)
{
	SDL_Event event;
	sdl_key=sdl_wheel=sdl_ascii=0;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			sdl_key=event.key.keysym.sym;
			sdl_ascii=event.key.keysym.unicode;
			if (event.key.keysym.sym == SDLK_CAPSLOCK)
				sdl_caps = 1;
			if (event.key.keysym.sym=='z')
			{
				sdl_zoom_trig = 1;
			}
			if ( event.key.keysym.sym == SDLK_PLUS)
			{
				sdl_wheel++;
			}
			if ( event.key.keysym.sym == SDLK_MINUS)
			{
				sdl_wheel--;
			}
			//  4
			//1 8 2
			if (event.key.keysym.sym == SDLK_RIGHT)
			{
				player[0] = (int)(player[0])|0x02;  //Go right command
			}
			if (event.key.keysym.sym == SDLK_LEFT)
			{
				player[0] = (int)(player[0])|0x01;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_DOWN && ((int)(player[0])&0x08)!=0x08)
			{
				player[0] = (int)(player[0])|0x08;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_UP && ((int)(player[0])&0x04)!=0x04)
			{
				player[0] = (int)(player[0])|0x04;  //Jump command
			}

			if (event.key.keysym.sym == SDLK_d)
			{
				player2[0] = (int)(player2[0])|0x02;  //Go right command
			}
			if (event.key.keysym.sym == SDLK_a)
			{
				player2[0] = (int)(player2[0])|0x01;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_s && ((int)(player2[0])&0x08)!=0x08)
			{
				player2[0] = (int)(player2[0])|0x08;  //Go left command
			}
			if (event.key.keysym.sym == SDLK_w && ((int)(player2[0])&0x04)!=0x04)
			{
				player2[0] = (int)(player2[0])|0x04;  //Jump command
			}
			break;

		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_CAPSLOCK)
				sdl_caps = 0;
			if (event.key.keysym.sym == 'z')
				sdl_zoom_trig = 0;
			if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT)
			{
				player[1] = player[0];  //Saving last movement
				player[0] = (int)(player[0])&12;  //Stop command
			}
			if (event.key.keysym.sym == SDLK_UP)
			{
				player[0] = (int)(player[0])&11;
			}
			if (event.key.keysym.sym == SDLK_DOWN)
			{
				player[0] = (int)(player[0])&7;
			}

			if (event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a)
			{
				player2[1] = player2[0];  //Saving last movement
				player2[0] = (int)(player2[0])&12;  //Stop command
			}
			if (event.key.keysym.sym == SDLK_w)
			{
				player2[0] = (int)(player2[0])&11;
			}
			if (event.key.keysym.sym == SDLK_s)
			{
				player2[0] = (int)(player2[0])&7;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_WHEELUP)
				sdl_wheel++;
			if (event.button.button == SDL_BUTTON_WHEELDOWN)
				sdl_wheel--;
			break;
		case SDL_QUIT:
			return 1;
		}
	}
	sdl_mod = SDL_GetModState();
	return 0;
}

void set_cmode(int cm) // sets to given view mode
{
	cmode = cm;
	itc = 51;
	if (cmode==CM_BLOB)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Blob Display");
	}
	else if (cmode==CM_HEAT)
	{
		strcpy(itc_msg, "Heat Display");
	}
	else if (cmode==CM_FANCY)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fancy Display");
	}
	else if (cmode==CM_FIRE)
	{
		memset(fire_r, 0, sizeof(fire_r));
		memset(fire_g, 0, sizeof(fire_g));
		memset(fire_b, 0, sizeof(fire_b));
		strcpy(itc_msg, "Fire Display");
	}
	else if (cmode==CM_PERS)
	{
		Renderer_ClearSecondaryBuffer();
		strcpy(itc_msg, "Persistent Display");
	}
	else if (cmode==CM_PRESS)
	{
		strcpy(itc_msg, "Pressure Display");
	}
	else if (cmode==CM_NOTHING)
	{
		strcpy(itc_msg, "Nothing Display");
	}
	else if (cmode==CM_CRACK)
	{
		strcpy(itc_msg, "Alternate Velocity Display");
	}
	else if (cmode==CM_GRAD)
	{
		strcpy(itc_msg, "Heat Gradient Display");
	}
	else if (cmode==CM_LIFE)
	{
		if (DEBUG_MODE) //can only get to Life view in debug mode
		{
			strcpy(itc_msg, "Life Display");
		}
		else
		{
			set_cmode(CM_CRACK);
		}
	}
	else //if no special text given, it will display this.
	{
		strcpy(itc_msg, "Velocity Display");
	}
	save_presets(0);
}

char *download_ui(char *uri, int *len)
{
	int dstate = 0;
	void *http = http_async_req_start(NULL, uri, NULL, 0, 0);
	int x0=(XRES-240)/2,y0=(YRES-MENUSIZE)/2;
	int done, total, i, ret, zlen, ulen;
	char str[16], *tmp, *res;

	while (!http_async_req_status(http))
	{
		sdl_poll();

		http_async_get_length(http, &total, &done);

		Renderer_ClearRectangle(x0-2, y0-2, 244, 64);
		Renderer_DrawRectangle(x0, y0, 240, 60, 192, 192, 192, 255);
		Graphics_RenderText(x0+8, y0+8, "Please wait", 255, 216, 32, 255);
		Graphics_RenderText(x0+8, y0+26, "Downloading update...", 255, 255, 255, 255);

		if (total)
		{
			i = (236*done)/total;
			Renderer_FillRectangle(x0+1, y0+45, i+1, 14, 255, 216, 32, 255);
			i = (100*done)/total;
			sprintf(str, "%d%%", i);
			if (i<50)
				Graphics_RenderText(x0+120-GetTextWidth(str)/2, y0+48, str, 192, 192, 192, 255);
			else
				Graphics_RenderText(x0+120-GetTextWidth(str)/2, y0+48, str, 0, 0, 0, 255);
		}
		else
			Graphics_RenderText(x0+120-GetTextWidth("Waiting...")/2, y0+48, "Waiting...", 255, 216, 32, 255);

		Renderer_DrawRectangle(x0, y0+44, 240, 16, 192, 192, 192, 255);
		Renderer_Display();
	}

	tmp = http_async_req_stop(http, &ret, &zlen);
	if (ret!=200)
	{
		error_ui(ret, http_ret_text(ret));
		if (tmp)
			free(tmp);
		return NULL;
	}
	if (!tmp)
	{
		error_ui(0, "Server did not return data");
		return NULL;
	}

	if (zlen<16)
	{
		printf("ZLen is not 16!\n");
		goto corrupt;
	}
	if (tmp[0]!=0x42 || tmp[1]!=0x75 || tmp[2]!=0x54 || tmp[3]!=0x54)
	{
		printf("Tmperr %d, %d, %d, %d\n", tmp[0], tmp[1], tmp[2], tmp[3]);
		goto corrupt;
	}

	ulen  = (unsigned char)tmp[4];
	ulen |= ((unsigned char)tmp[5])<<8;
	ulen |= ((unsigned char)tmp[6])<<16;
	ulen |= ((unsigned char)tmp[7])<<24;

	res = (char *)malloc(ulen);
	if (!res)
	{
		printf("No res!\n");
		goto corrupt;
	}
	dstate = BZ2_bzBuffToBuffDecompress((char *)res, (unsigned *)&ulen, (char *)(tmp+8), zlen-8, 0, 0);
	if (dstate)
	{
		printf("Decompression failure: %d!\n", dstate);
		free(res);
		goto corrupt;
	}

	free(tmp);
	if (len)
		*len = ulen;
	return res;

corrupt:
	error_ui(0, "Downloaded update is corrupted");
	free(tmp);
	return NULL;
}

int search_ui()
{
	int uih=0,nyu,nyd,b=1,bq,mx=0,my=0,mxq=0,myq=0,mmt=0,gi,gj,gx,gy,pos,i,mp,dp,dap,own,last_own=search_own,last_fav=search_fav,page_count=0,last_page=0,last_date=0,j,w,h,st=0,lv;
	int is_p1=0, exp_res=GRID_X*GRID_Y, tp, view_own=0;
	int thumb_drawn[GRID_X*GRID_Y];
	pixel *v_buf = (pixel *)malloc(((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
	float ry;
	time_t http_last_use=HTTP_TIMEOUT;
	ui_edit ed;


	void *http = NULL;
	int active = 0;
	char *last = NULL;
	int search = 0;
	int lasttime = TIMEOUT;
	char *uri;
	int status;
	char *results;
	char *tmp, ts[64];

	void *img_http[IMGCONNS];
	char *img_id[IMGCONNS];
	void *thumb, *data;
	int thlen, dlen;

	memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

	memset(img_http, 0, sizeof(img_http));
	memset(img_id, 0, sizeof(img_id));

	memset(search_ids, 0, sizeof(search_ids));
	memset(search_dates, 0, sizeof(search_dates));
	memset(search_names, 0, sizeof(search_names));
	memset(search_scoreup, 0, sizeof(search_scoreup));
	memset(search_scoredown, 0, sizeof(search_scoredown));
	memset(search_publish, 0, sizeof(search_publish));
	memset(search_owners, 0, sizeof(search_owners));
	memset(search_thumbs, 0, sizeof(search_thumbs));
	memset(search_thsizes, 0, sizeof(search_thsizes));

	memset(thumb_drawn, 0, sizeof(thumb_drawn));

	do_open = 0;

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	ed.x = 65;
	ed.y = 13;
	ed.w = XRES-200;
	ed.nx = 1;
	ed.def = "[search terms]";
	ed.focus = 1;
	ed.hide = 0;
	ed.cursor = strlen(search_expr);
	ed.multiline = 0;
	strcpy(ed.str, search_expr);

	sdl_wheel = 0;

	while (!sdl_poll())
	{
		uih = 0;
		bq = b;
		mxq = mx;
		myq = my;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		if (mx!=mxq || my!=myq || sdl_wheel || b)
			mmt = 0;
		else if (mmt<TIMEOUT)
			mmt++;

		Renderer_ClearRectangle(-1, -1, (XRES+BARSIZE)+1, YRES+MENUSIZE+1);

		//memcpy(vid_buf, v_buf, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);

		Graphics_RenderText(11, 13, "Search:", 192, 192, 192, 255);
		if (!last || (!active && strcmp(last, ed.str)))
			Graphics_RenderText(51, 11, "\x8E", 192, 160, 32, 255);
		else
			Graphics_RenderText(51, 11, "\x8E", 32, 64, 160, 255);
		Graphics_RenderText(51, 11, "\x8F", 255, 255, 255, 255);
		Renderer_DrawRectangle(48, 8, XRES-182, 16, 192, 192, 192, 255);

		if (!svf_login)
		{
			search_own = 0;
			Renderer_DrawRectangle(XRES-64+16, 8, 56, 16, 96, 96, 96, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x94", 96, 80, 16, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x93", 128, 128, 128, 255);
			Graphics_RenderText(XRES-46+16, 13, "My Own", 128, 128, 128, 255);
		}
		else if (search_own)
		{
			Renderer_FillRectangle(XRES-65+16, 7, 58, 18, 255, 255, 255, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x94", 192, 160, 64, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x93", 32, 32, 32, 255);
			Graphics_RenderText(XRES-46+16, 13, "My Own", 0, 0, 0, 255);
		}
		else
		{
			Renderer_DrawRectangle(XRES-64+16, 8, 56, 16, 192, 192, 192, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x94", 192, 160, 32, 255);
			Graphics_RenderText(XRES-61+16, 11, "\x93", 255, 255, 255, 255);
			Graphics_RenderText(XRES-46+16, 13, "My Own", 255, 255, 255, 255);
		}

		if (search_fav)
		{
			Renderer_FillRectangle(XRES-134, 7, 18, 18, 255, 255, 255, 255);
			Graphics_RenderText(XRES-130, 11, "\xCC", 192, 160, 64, 255);
		}
		else
		{
			Renderer_DrawRectangle(XRES-134, 8, 16, 16, 192, 192, 192, 255);
			Graphics_RenderText(XRES-130, 11, "\xCC", 192, 160, 32, 255);
		}

		if (search_date)
		{
			Renderer_FillRectangle(XRES-130+16, 7, 62, 18, 255, 255, 255, 255);
			Graphics_RenderText(XRES-126+16, 11, "\xA6", 32, 32, 32, 255);
			Graphics_RenderText(XRES-111+16, 13, "By date", 0, 0, 0, 255);
		}
		else
		{
			Renderer_DrawRectangle(XRES-129+16, 8, 60, 16, 192, 192, 192, 255);
			Graphics_RenderText(XRES-126+16, 11, "\xA9", 144, 48, 32, 255);
			Graphics_RenderText(XRES-126+16, 11, "\xA8", 32, 144, 32, 255);
			Graphics_RenderText(XRES-126+16, 11, "\xA7", 255, 255, 255, 255);
			Graphics_RenderText(XRES-111+16, 13, "By votes", 255, 255, 255, 255);
		}

		if (search_page)
		{
			Graphics_RenderText(4, YRES+MENUSIZE-16, "\x96", 255, 255, 255, 255);
			Renderer_DrawRectangle(1, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}
		if (page_count > 9)
		{
			Graphics_RenderText(XRES-15, YRES+MENUSIZE-16, "\x95", 255, 255, 255, 255);
			Renderer_DrawRectangle(XRES-18, YRES+MENUSIZE-20, 16, 16, 255, 255, 255, 255);
		}

		ui_edit_draw(&ed);

		if ((b && !bq && mx>=1 && mx<=17 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel>0)
		{
			if (search_page)
			{
				search_page --;
				lasttime = TIMEOUT;
			}
			sdl_wheel = 0;
			uih = 1;
		}
		if ((b && !bq && mx>=XRES-18 && mx<=XRES-1 && my>=YRES+MENUSIZE-20 && my<YRES+MENUSIZE-4) || sdl_wheel<0)
		{
			if (page_count>exp_res)
			{
				lasttime = TIMEOUT;
				search_page ++;
				page_count = exp_res;
			}
			sdl_wheel = 0;
			uih = 1;
		}

		tp = -1;
		if (is_p1)
		{
			Graphics_RenderText((XRES-GetTextWidth("Popular tags:"))/2, 31, "Popular tags:", 255, 192, 64, 255);
			for (gj=0; gj<((GRID_Y-GRID_P)*YRES)/(GRID_Y*14); gj++)
				for (gi=0; gi<GRID_X; gi++)
				{
					pos = gi+GRID_X*gj;
					if (pos>TAG_MAX || !tag_names[pos])
						break;
					if (tag_votes[0])
						i = 127+(128*tag_votes[pos])/tag_votes[0];
					else
						i = 192;
					w = GetTextWidth(tag_names[pos]);
					if (w>XRES/GRID_X-5)
						w = XRES/GRID_X-5;
					gx = (XRES/GRID_X)*gi;
					gy = gj*14 + 46;
					if (mx>=gx && mx<gx+(XRES/GRID_X) && my>=gy && my<gy+14)
					{
						j = (i*5)/6;
						tp = pos;
					}
					else
						j = i;
					Graphics_RenderMaxText(gx+(XRES/GRID_X-w)/2, gy, XRES/GRID_X-5, tag_names[pos], j, j, i, 255);
				}
		}

		mp = dp = -1;
		dap = -1;
		st = 0;
		for (gj=0; gj<GRID_Y; gj++)
			for (gi=0; gi<GRID_X; gi++)
			{
				if (is_p1)
				{
					pos = gi+GRID_X*(gj-GRID_Y+GRID_P);
					if (pos<0)
						break;
				}
				else
					pos = gi+GRID_X*gj;
				if (!search_ids[pos])
					break;
				gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
				gy = ((((YRES-(MENUSIZE-20))+15)/GRID_Y)*gj) + ((YRES-(MENUSIZE-20))/GRID_Y-(YRES-(MENUSIZE-20))/GRID_S+10)/2 + 18;
				if (GetTextWidth(search_names[pos]) > XRES/GRID_X-10)
				{
					tmp = malloc(strlen(search_names[pos])+4);
					strcpy(tmp, search_names[pos]);
					j = GetTextWidthX(tmp, XRES/GRID_X-15);
					strcpy(tmp+j, "...");
					Graphics_RenderText(gx+XRES/(GRID_S*2)-GetTextWidth(tmp)/2, gy+YRES/GRID_S+7, tmp, 192, 192, 192, 255);
					free(tmp);
				}
				else
					Graphics_RenderText(gx+XRES/(GRID_S*2)-GetTextWidth(search_names[pos])/2, gy+YRES/GRID_S+7, search_names[pos], 192, 192, 192, 255);
				j = GetTextWidth(search_owners[pos]);
				if (mx>=gx+XRES/(GRID_S*2)-j/2 && mx<=gx+XRES/(GRID_S*2)+j/2 &&
				        my>=gy+YRES/GRID_S+18 && my<=gy+YRES/GRID_S+31)
				{
					st = 1;
					Graphics_RenderText(gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 160, 255);
				}
				else
					Graphics_RenderText(gx+XRES/(GRID_S*2)-j/2, gy+YRES/GRID_S+20, search_owners[pos], 128, 128, 128, 255);
				if (search_thumbs[pos]&&thumb_drawn[pos]==0)
				{
					Graphics_RenderThumbnail(search_thumbs[pos], search_thsizes[pos], 1, gx, gy, GRID_S);
					thumb_drawn[pos] = 1;
				}
				own = svf_login && (!strcmp(svf_user, search_owners[pos]) || svf_admin || svf_mod);
				if (mx>=gx-2 && mx<=gx+XRES/GRID_S+3 && my>=gy-2 && my<=gy+YRES/GRID_S+30)
					mp = pos;
				if (own)
				{
					if (mx>=gx+XRES/GRID_S-4 && mx<=gx+XRES/GRID_S+6 && my>=gy-6 && my<=gy+4)
					{
						mp = -1;
						dp = pos;
					}
					if (!search_dates[pos] && mx>=gx-6 && mx<=gx+4 && my>=gy+YRES/GRID_S-4 && my<=gy+YRES/GRID_S+6)
					{
						mp = -1;
						dap = pos;
					}
				}
				Renderer_DrawRectangle(gx-2+(XRES/GRID_S)+5, gy-2, 6, YRES/GRID_S+3, 128, 128, 128, 255);
				Renderer_FillRectangle(gx-2+(XRES/GRID_S)+5, gy-2, 6, 1+(YRES/GRID_S+3)/2, 0, 107, 10, 255);
				Renderer_FillRectangle(gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 6, 1+(YRES/GRID_S+3)/2, 107, 10, 0, 255);

				if (mp==pos && !st)
					Renderer_DrawRectangle(gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 160, 160, 192, 255);
				else
					Renderer_DrawRectangle(gx-2, gy-2, XRES/GRID_S+3, YRES/GRID_S+3, 128, 128, 128, 255);
				if (own && search_fav!=1)
				{
					if (dp == pos)
						Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x86", 255, 48, 32, 255);
					else
						Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x86", 160, 48, 32, 255);
					Graphics_RenderText(gx+XRES/GRID_S-4, gy-6, "\x85", 255, 255, 255, 255);
				}
				if (!search_publish[pos])
				{
					Graphics_RenderText(gx-6, gy-6, "\xCD", 255, 255, 255, 255);
					Graphics_RenderText(gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (!search_dates[pos] && own)
				{
					Renderer_FillRectangle(gx-5, gy+YRES/GRID_S-3, 7, 8, 255, 255, 255, 255);
					if (dap == pos) {
						Graphics_RenderText(gx-6, gy+YRES/GRID_S-4, "\xA6", 200, 100, 80, 255);
					} else {
						Graphics_RenderText(gx-6, gy+YRES/GRID_S-4, "\xA6", 160, 70, 50, 255);
					}
					//Graphics_RenderText(gx-6, gy-6, "\xCE", 212, 151, 81, 255);
				}
				if (view_own || svf_admin || svf_mod)
				{
					sprintf(ts+1, "%d", search_votes[pos]);
					ts[0] = 0xBB;
					for (j=1; ts[j]; j++)
						ts[j] = 0xBC;
					ts[j-1] = 0xB9;
					ts[j] = 0xBA;
					ts[j+1] = 0;
					w = gx+XRES/GRID_S-2-GetTextWidth(ts);
					h = gy+YRES/GRID_S-11;
					Graphics_RenderText(w, h, ts, 16, 72, 16, 255);
					for (j=0; ts[j]; j++)
						ts[j] -= 14;
					Graphics_RenderText(w, h, ts, 192, 192, 192, 255);
					sprintf(ts, "%d", search_votes[pos]);
					for (j=0; ts[j]; j++)
						ts[j] += 127;
					Graphics_RenderText(w+3, h, ts, 255, 255, 255, 255);
				}
				if (search_scoreup[pos]>0||search_scoredown[pos]>0)
				{
					lv = (search_scoreup[pos]>search_scoredown[pos]?search_scoreup[pos]:search_scoredown[pos]);

					if (((YRES/GRID_S+3)/2)>lv)
					{
						ry = ((float)((YRES/GRID_S+3)/2)/(float)lv);
						if (lv<8)
						{
							ry =  ry/(8-lv);
						}
						nyu = search_scoreup[pos]*ry;
						nyd = search_scoredown[pos]*ry;
					}
					else
					{
						ry = ((float)lv/(float)((YRES/GRID_S+3)/2));
						nyu = search_scoreup[pos]/ry;
						nyd = search_scoredown[pos]/ry;
					}


					Renderer_FillRectangle(gx-1+(XRES/GRID_S)+5, gy-1+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 57, 187, 57, 255);
					Renderer_FillRectangle(gx-1+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2), 4, nyd, 187, 57, 57, 255);
					//Renderer_DrawRectangle(gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)-nyu, 4, nyu, 0, 107, 10, 255);
					//Renderer_DrawRectangle(gx-2+(XRES/GRID_S)+5, gy-2+((YRES/GRID_S+3)/2)+1, 4, nyd, 107, 10, 0, 255);
				}
			}

		if (mp!=-1 && mmt>=TIMEOUT/5 && !st)
		{
			gi = mp % GRID_X;
			gj = mp / GRID_X;
			if (is_p1)
				gj += GRID_Y-GRID_P;
			gx = ((XRES/GRID_X)*gi) + (XRES/GRID_X-XRES/GRID_S)/2;
			gy = (((YRES+15)/GRID_Y)*gj) + (YRES/GRID_Y-YRES/GRID_S+10)/2 + 18;
			i = w = GetTextWidth(search_names[mp]);
			h = YRES/GRID_Z+30;
			if (w<XRES/GRID_Z) w=XRES/GRID_Z;
			gx += XRES/(GRID_S*2)-w/2;
			gy += YRES/(GRID_S*2)-h/2;
			if (gx<2) gx=2;
			if (gx+w>=XRES-2) gx=XRES-3-w;
			if (gy<32) gy=32;
			if (gy+h>=YRES+(MENUSIZE-2)) gy=YRES+(MENUSIZE-3)-h;
			Renderer_ClearRectangle(gx-2, gy-3, w+4, h);
			Renderer_DrawRectangle(gx-2, gy-3, w+4, h, 160, 160, 192, 255);
			if (search_thumbs[mp])
				Graphics_RenderThumbnail(search_thumbs[mp], search_thsizes[mp], 1, gx+(w-(XRES/GRID_Z))/2, gy, GRID_Z);
			Graphics_RenderText(gx+(w-i)/2, gy+YRES/GRID_Z+4, search_names[mp], 192, 192, 192, 255);
			Graphics_RenderText(gx+(w-GetTextWidth(search_owners[mp]))/2, gy+YRES/GRID_Z+16, search_owners[mp], 128, 128, 128, 255);
		}

		Renderer_Display();

		ui_edit_process(mx, my, b, &ed);

		if (sdl_key==SDLK_RETURN)
		{
			if (!last || (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page)))
				lasttime = TIMEOUT;
			else if (search_ids[0] && !search_ids[1])
			{
				bq = 0;
				b = 1;
				mp = 0;
			}
		}
		if (sdl_key==SDLK_ESCAPE)
			goto finish;

		if (b && !bq && mx>=XRES-64+16 && mx<=XRES-8+16 && my>=8 && my<=24 && svf_login)
		{
			search_own = !search_own;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-129+16 && mx<=XRES-65+16 && my>=8 && my<=24)
		{
			search_date = !search_date;
			lasttime = TIMEOUT;
		}
		if (b && !bq && mx>=XRES-134 && mx<=XRES-134+16 && my>=8 && my<=24)
		{
			search_fav = !search_fav;
			lasttime = TIMEOUT;
		}

		if (b && !bq && dp!=-1 && search_fav==0)
			if (confirm_ui("Do you want to delete?", search_names[dp], "Delete"))
			{
				execute_delete(search_ids[dp]);
				lasttime = TIMEOUT;
				if (last)
				{
					free(last);
					last = NULL;
				}
			}
		if (b && !bq && dap!=-1)
		{
			sprintf(ed.str, "history:%s", search_ids[dap]);
			lasttime = TIMEOUT;
		}

		if (b && !bq && tp!=-1)
		{
			strncpy(ed.str, tag_names[tp], 255);
			lasttime = TIMEOUT;
		}

		if (b && !bq && mp!=-1 && st)
		{
			sprintf(ed.str, "user:%s", search_owners[mp]);
			lasttime = TIMEOUT;
		}

		if (do_open==1)
		{
			mp = 0;
		}

		if ((b && !bq && mp!=-1 && !st && !uih) || do_open==1)
		{
			if (open_ui(search_ids[mp], search_dates[mp]?search_dates[mp]:NULL)==1) {
				goto finish;
			}
		}

		if (!last)
		{
			search = 1;
		}
		else if (!active && (strcmp(last, ed.str) || last_own!=search_own || last_date!=search_date || last_page!=search_page || last_fav!=search_fav))
		{
			search = 1;
			if (strcmp(last, ed.str) || last_own!=search_own || last_fav!=search_fav || last_date!=search_date)
			{
				search_page = 0;
				page_count = 0;
			}
			free(last);
			last = NULL;
		}
		else
			search = 0;

		if (search && lasttime>=TIMEOUT)
		{
			lasttime = 0;
			last = mystrdup(ed.str);
			last_own = search_own;
			last_date = search_date;
			last_page = search_page;
			last_fav = search_fav;
			active = 1;
			uri = malloc(strlen(last)*3+180+strlen(SERVER)+strlen(svf_user)+20); //Increase "padding" from 80 to 180 to fix the search memory corruption bug
			if (search_own || svf_admin || svf_mod)
				tmp = "&ShowVotes=true";
			else
				tmp = "";
			if (!search_own && !search_date && !*last)
			{
				if (search_page)
				{
					exp_res = GRID_X*GRID_Y;
					sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", (search_page-1)*GRID_X*GRID_Y+GRID_X*GRID_P, exp_res+1, tmp);
				}
				else
				{
					exp_res = GRID_X*GRID_P;
					sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d&t=%d%s&Query=", 0, exp_res+1, ((GRID_Y-GRID_P)*YRES)/(GRID_Y*14)*GRID_X, tmp);
				}
			}
			else
			{
				exp_res = GRID_X*GRID_Y;
				sprintf(uri, "http://" SERVER "/Search.api?Start=%d&Count=%d%s&Query=", search_page*GRID_X*GRID_Y, exp_res+1, tmp);
			}
			strcaturl(uri, last);
			if (search_own)
			{
				strcaturl(uri, " user:");
				strcaturl(uri, svf_user);
			}
			if (search_fav)
			{
				strcaturl(uri, " cat:favs");
			}
			if (search_date)
				strcaturl(uri, " sort:date");

			http = http_async_req_start(http, uri, NULL, 0, 1);
			if (svf_login)
			{
				//http_auth_headers(http, svf_user, svf_pass);
				http_auth_headers(http, svf_user_id, NULL, svf_session_id);
			}
			http_last_use = time(NULL);
			free(uri);
		}

		if (active && http_async_req_status(http))
		{
			http_last_use = time(NULL);
			results = http_async_req_stop(http, &status, NULL);
			view_own = last_own;
			if (status == 200)
			{
				page_count = search_results(results, last_own||svf_admin||svf_mod);
				memset(thumb_drawn, 0, sizeof(thumb_drawn));
				memset(v_buf, 0, ((YRES+MENUSIZE)*(XRES+BARSIZE))*PIXELSIZE);
			}
			is_p1 = (exp_res < GRID_X*GRID_Y);
			if (results)
				free(results);
			active = 0;
		}

		if (http && !active && (time(NULL)>http_last_use+HTTP_TIMEOUT))
		{
			http_async_req_close(http);
			http = NULL;
		}

		for (i=0; i<IMGCONNS; i++)
		{
			if (img_http[i] && http_async_req_status(img_http[i]))
			{
				thumb = http_async_req_stop(img_http[i], &status, &thlen);
				if (status != 200)
				{
					if (thumb)
						free(thumb);
					thumb = calloc(1,4);
					thlen = 4;
				}
				thumb_cache_add(img_id[i], thumb, thlen);
				for (pos=0; pos<GRID_X*GRID_Y; pos++) {
					if (search_dates[pos]) {
						char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						//img_id[i] = mystrdup(id_d_temp);
						if (id_d_temp && !strcmp(id_d_temp, img_id[i])) {
							break;
						}
					} else {
						if (search_ids[pos] && !strcmp(search_ids[pos], img_id[i])) {
							break;
						}
					}
				}
				if (pos<GRID_X*GRID_Y)
				{
					search_thumbs[pos] = thumb;
					search_thsizes[pos] = thlen;
				}
				else
					free(thumb);
				free(img_id[i]);
				img_id[i] = NULL;
			}
			if (!img_id[i])
			{
				for (pos=0; pos<GRID_X*GRID_Y; pos++)
					if (search_ids[pos] && !search_thumbs[pos])
					{
						for (gi=0; gi<IMGCONNS; gi++)
							if (img_id[gi] && !strcmp(search_ids[pos], img_id[gi]))
								break;
						if (gi<IMGCONNS)
							continue;
						break;
					}
				if (pos<GRID_X*GRID_Y)
				{
					if (search_dates[pos]) {
						char *id_d_temp = malloc(strlen(search_ids[pos])+strlen(search_dates[pos])+1);
						uri = malloc(strlen(search_ids[pos])*3+strlen(search_dates[pos])*3+strlen(SERVER)+71);
						strcpy(uri, "http://" SERVER "/Get.api?Op=thumb&ID=");
						strcaturl(uri, search_ids[pos]);
						strappend(uri, "&Date=");
						strcaturl(uri, search_dates[pos]);

						strcpy(id_d_temp, search_ids[pos]);
						strappend(id_d_temp, "_");
						strappend(id_d_temp, search_dates[pos]);
						img_id[i] = mystrdup(id_d_temp);
					} else {
						uri = malloc(strlen(search_ids[pos])*3+strlen(SERVER)+64);
						strcpy(uri, "http://" SERVER "/Get.api?Op=thumb&ID=");
						strcaturl(uri, search_ids[pos]);
						img_id[i] = mystrdup(search_ids[pos]);
					}

					img_http[i] = http_async_req_start(img_http[i], uri, NULL, 0, 1);
					free(uri);
				}
			}
			if (!img_id[i] && img_http[i])
			{
				http_async_req_close(img_http[i]);
				img_http[i] = NULL;
			}
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}

finish:
	if (last)
		free(last);
	if (http)
		http_async_req_close(http);
	for (i=0; i<IMGCONNS; i++)
		if (img_http[i])
			http_async_req_close(img_http[i]);

	search_results("", 0);

	strcpy(search_expr, ed.str);

	return 0;
}

int report_ui(char *save_id)
{
	int b=1,bq,mx,my;
	ui_edit ed;
	ed.x = 209;
	ed.y = 159;
	ed.w = (XRES+BARSIZE-400)-18;
	ed.h = (YRES+MENUSIZE-300)-36;
	ed.nx = 1;
	ed.def = "Report details";
	ed.focus = 0;
	ed.hide = 0;
	ed.multiline = 1;
	ed.cursor = 0;
	strcpy(ed.str, "");

	Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	while (!sdl_poll()) {
		Renderer_FillRectangle(200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 0,0,0, 255);
		Renderer_DrawRectangle(200, 150, (XRES+BARSIZE-400), (YRES+MENUSIZE-300), 255, 255, 255, 255);

		Renderer_DrawRectangle(205, 155, (XRES+BARSIZE-400)-10, (YRES+MENUSIZE-300)-28, 255, 255, 255, 170);

		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;


		Renderer_DrawRectangle(200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		Graphics_RenderText(213, (YRES+MENUSIZE-150)-13, "Cancel", 255, 255, 255, 255);

		Renderer_DrawRectangle((XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 255);
		Graphics_RenderText((XRES+BARSIZE-400)+163, (YRES+MENUSIZE-150)-13, "Report", 255, 255, 255, 255);
		if (mx>(XRES+BARSIZE-400)+150 && my>(YRES+MENUSIZE-150)-18 && mx<(XRES+BARSIZE-400)+200 && my<(YRES+MENUSIZE-150)) {
			Renderer_FillRectangle((XRES+BARSIZE-400)+150, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b) {
				if (execute_report(save_id, ed.str)) {
					info_ui("Success", "This save has been reported");
					return 1;
				} else {
					return 0;
				}
			}
		}
		if (mx>200 && my>(YRES+MENUSIZE-150)-18 && mx<250 && my<(YRES+MENUSIZE-150)) {
			Renderer_FillRectangle(200, (YRES+MENUSIZE-150)-18, 50, 18, 255, 255, 255, 40);
			if (b)
				return 0;
		}
		ui_edit_draw(&ed);
		Renderer_Display();
		ui_edit_process(mx, my, b, &ed);
	}
	return 0;
}

int open_ui(char *save_id, char *save_date)
{
	int b=1,bq,mx,my,ca=0,thumb_w,thumb_h,active=0,active_2=0,cc=0,ccy=0,cix=0,hasdrawninfo=0,hasdrawnthumb=0,authoritah=0,myown=0,queue_open=0,data_size=0,retval=0,bc=255,openable=1;
	int nyd,nyu,ry,lv;
	float ryf;

	char *uri, *uri_2, *o_uri;
	void *data, *info_data;
	save_info *info = malloc(sizeof(save_info));
	void *http = NULL, *http_2 = NULL;
	int lasttime = TIMEOUT;
	int status, status_2, info_ready = 0, data_ready = 0;
	time_t http_last_use = HTTP_TIMEOUT,  http_last_use_2 = HTTP_TIMEOUT;
	pixel *save_pic;// = malloc((XRES/2)*(YRES/2));
	ui_edit ed;
	ui_copytext ctb;

	pixel *old_vid=(pixel *)calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);

	Renderer_FillRectangle(50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 0, 0, 0, 255);
	Renderer_DrawRectangle(50, 50, XRES+BARSIZE-100, YRES+MENUSIZE-100, 255, 255, 255, 255);
	Renderer_DrawRectangle(50, 50, (XRES/2)+1, (YRES/2)+1, 255, 255, 255, 155);
	Renderer_DrawRectangle(50+(XRES/2)+1, 50, XRES+BARSIZE-100-((XRES/2)+1), YRES+MENUSIZE-100, 155, 155, 155, 255);
	Graphics_RenderText(50+(XRES/4)-GetTextWidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);

	ed.x = 57+(XRES/2)+1;
	ed.y = YRES+MENUSIZE-118;
	ed.w = XRES+BARSIZE-114-((XRES/2)+1);
	ed.h = 48;
	ed.nx = 1;
	ed.def = "Add comment";
	ed.focus = 1;
	ed.hide = 0;
	ed.multiline = 1;
	ed.cursor = 0;
	strcpy(ed.str, "");

	ctb.x = 100;
	ctb.y = YRES+MENUSIZE-20;
	ctb.width = GetTextWidth(save_id)+12;
	ctb.height = 10+7;
	ctb.hover = 0;
	ctb.state = 0;
	strcpy(ctb.text, save_id);

	//memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);

	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}

	//Begin Async loading of data
	if (save_date) {
		// We're loading an historical save
		uri = malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(SERVER)+71);
		strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
		strcaturl(uri, save_id);
		strappend(uri, "&Date=");
		strcaturl(uri, save_date);

		uri_2 = malloc(strlen(save_id)*3+strlen(save_date)*3+strlen(SERVER)+71);
		strcpy(uri_2, "http://" SERVER "/Info.api?ID=");
		strcaturl(uri_2, save_id);
		strappend(uri_2, "&Date=");
		strcaturl(uri_2, save_date);
	} else {
		//We're loading a normal save
		uri = malloc(strlen(save_id)*3+strlen(SERVER)+64);
		strcpy(uri, "http://" SERVER "/Get.api?Op=save&ID=");
		strcaturl(uri, save_id);

		uri_2 = malloc(strlen(save_id)*3+strlen(SERVER)+64);
		strcpy(uri_2, "http://" SERVER "/Info.api?ID=");
		strcaturl(uri_2, save_id);
	}
	http = http_async_req_start(http, uri, NULL, 0, 1);
	http_2 = http_async_req_start(http_2, uri_2, NULL, 0, 1);
	if (svf_login)
	{
		http_auth_headers(http, svf_user_id, NULL, svf_session_id);
		http_auth_headers(http_2, svf_user_id, NULL, svf_session_id);
	}
	http_last_use = time(NULL);
	http_last_use_2 = time(NULL);
	free(uri);
	free(uri_2);
	active = 1;
	active_2 = 1;
	while (!sdl_poll())
	{
		bq = b;
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;

		if (active && http_async_req_status(http))
		{
			int imgh, imgw, nimgh, nimgw;
			http_last_use = time(NULL);
			data = http_async_req_stop(http, &status, &data_size);
			if (status == 200)
			{
				pixel *full_save;
				if (!data||!data_size) {
					error_ui(0, "Save data is empty (may be corrupt)");
					break;
				}
				full_save = Graphics_PrerenderSave(data, data_size, &imgw, &imgh);
				if (full_save!=NULL) {
					save_pic = rescale_img(full_save, imgw, imgh, &thumb_w, &thumb_h, 2);
					data_ready = 1;
					free(full_save);
				} else {
					error_ui(0, "Save may be from a newer version");
					break;
				}
			}
			active = 0;
			free(http);
			http = NULL;
		}
		if (active_2 && http_async_req_status(http_2))
		{
			http_last_use_2 = time(NULL);
			info_data = http_async_req_stop(http_2, &status_2, NULL);
			if (status_2 == 200 || !info_data)
			{
				info_ready = info_parse(info_data, info);
				if (info_ready<=0) {
					error_ui(0, "Save info not found");
					break;
				}
			}
			if (info_data)
				free(info_data);
			active_2 = 0;
			free(http_2);
			http_2 = NULL;
		}

		if (data_ready && !hasdrawnthumb) {
			Renderer_DrawImage(save_pic, 51, 51, thumb_w, thumb_h, 255);
			hasdrawnthumb = 1;
			//memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		}
		if (info_ready && !hasdrawninfo) {
			//Render all the save information
			cix = Graphics_RenderText(60, (YRES/2)+60, info->name, 255, 255, 255, 255);
			cix = Graphics_RenderText(60, (YRES/2)+72, "Author:", 255, 255, 255, 155);
			cix = Graphics_RenderText(cix+4, (YRES/2)+72, info->author, 255, 255, 255, 255);
			cix = Graphics_RenderText(cix+4, (YRES/2)+72, "Date:", 255, 255, 255, 155);
			cix = Graphics_RenderText(cix+4, (YRES/2)+72, info->date, 255, 255, 255, 255);
			Graphics_RenderWrapText(62, (YRES/2)+86, (XRES/2)-24, info->description, 255, 255, 255, 200);

			//Draw the score bars
			if (info->voteup>0||info->votedown>0)
			{
				lv = (info->voteup>info->votedown)?info->voteup:info->votedown;
				lv = (lv>10)?lv:10;

				if (50>lv)
				{
					ryf = 50.0f/((float)lv);
					//if(lv<8)
					//{
					//	ry =  ry/(8-lv);
					//}
					nyu = info->voteup*ryf;
					nyd = info->votedown*ryf;
				}
				else
				{
					ryf = ((float)lv)/50.0f;
					nyu = info->voteup/ryf;
					nyd = info->votedown/ryf;
				}
				nyu = nyu>50?50:nyu;
				nyd = nyd>50?50:nyd;

				Renderer_FillRectangle(48+(XRES/2)-51, (YRES/2)+53, 52, 6, 0, 107, 10, 255);
				Renderer_FillRectangle(48+(XRES/2)-51, (YRES/2)+59, 52, 6, 107, 10, 0, 255);
				Renderer_DrawRectangle(48+(XRES/2)-51, (YRES/2)+53, 52, 6, 128, 128, 128, 255);
				Renderer_DrawRectangle(48+(XRES/2)-51, (YRES/2)+59, 52, 6, 128, 128, 128, 255);

				Renderer_FillRectangle(48+(XRES/2)-nyu, (YRES/2)+54, nyu, 4, 57, 187, 57, 255);
				Renderer_FillRectangle(48+(XRES/2)-nyd, (YRES/2)+60, nyd, 4, 187, 57, 57, 255);
			}

			ccy = 0;
			for (cc=0; cc<info->comment_count; cc++) {
				if ((ccy + 72 + ((GetTextWidth(info->comments[cc])/(XRES+BARSIZE-100-((XRES/2)+1)-20)))*12)<(YRES+MENUSIZE-50)) {
					Graphics_RenderText(60+(XRES/2)+1, ccy+60, info->commentauthors[cc], 255, 255, 255, 255);
					ccy += 12;
					ccy += Graphics_RenderWrapText(60+(XRES/2)+1, ccy+60, XRES+BARSIZE-100-((XRES/2)+1)-20, info->comments[cc], 255, 255, 255, 185);
					ccy += 10;
					if (ccy+52<YRES+MENUSIZE-50) { //Try not to draw off the screen.
						Renderer_DrawLine(50+(XRES/2)+2, ccy+52, XRES+BARSIZE-50, ccy+52, 100, 100, 100, XRES+BARSIZE);
					}
				}
			}
			hasdrawninfo = 1;
			myown = svf_login && !strcmp(info->author, svf_user);
			authoritah = svf_login && (!strcmp(info->author, svf_user) || svf_admin || svf_mod);
			//memcpy(old_vid, vid_buf, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		}
		if (info_ready && svf_login) {
			//Render the comment box.
			Renderer_FillRectangle(50+(XRES/2)+1, YRES+MENUSIZE-125, XRES+BARSIZE-100-((XRES/2)+1), 75, 0, 0, 0, 255);
			Renderer_DrawRectangle(50+(XRES/2)+1, YRES+MENUSIZE-125, XRES+BARSIZE-100-((XRES/2)+1), 75, 200, 200, 200, 255);

			Renderer_DrawRectangle(54+(XRES/2)+1, YRES+MENUSIZE-121, XRES+BARSIZE-108-((XRES/2)+1), 48, 255, 255, 255, 200);

			ui_edit_draw(&ed);

			Renderer_DrawRectangle(XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 255);
			Graphics_RenderText(XRES+BARSIZE-90, YRES+MENUSIZE-63, "Submit", 255, 255, 255, 255);
		}

		//Save ID text and copybox
		cix = GetTextWidth("Save ID: ");
		cix += ctb.width;
		ctb.x = GetTextWidth("Save ID: ")+(XRES+BARSIZE-cix)/2;
		//ctb.x =
		Graphics_RenderText((XRES+BARSIZE-cix)/2, YRES+MENUSIZE-15, "Save ID: ", 255, 255, 255, 255);
		ui_copytext_draw(&ctb);
		ui_copytext_process(mx, my, b, bq, &ctb);

		//Open Button
		bc = openable?255:150;
		Renderer_DrawRectangle(50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		Graphics_RenderText(73, YRES+MENUSIZE-63, "Open", 255, 255, 255, bc);
		Graphics_RenderText(58, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);
		//Fav Button
		bc = svf_login?255:150;
		Renderer_DrawRectangle(100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		Graphics_RenderText(122, YRES+MENUSIZE-63, "Fav.", 255, 255, 255, bc);
		Graphics_RenderText(107, YRES+MENUSIZE-64, "\xCC", 255, 255, 255, bc);
		//Report Button
		bc = (svf_login && info_ready)?255:150;
		Renderer_DrawRectangle(150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		Graphics_RenderText(168, YRES+MENUSIZE-63, "Report", 255, 255, 255, bc);
		Graphics_RenderText(158, YRES+MENUSIZE-63, "!", 255, 255, 255, bc);
		//Delete Button
		bc = authoritah?255:150;
		Renderer_DrawRectangle(200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, bc);
		Graphics_RenderText(218, YRES+MENUSIZE-63, "Delete", 255, 255, 255, bc);
		Graphics_RenderText(206, YRES+MENUSIZE-64, "\xAA", 255, 255, 255, bc);
		//Open in browser button
		bc = 255;
		Renderer_DrawRectangle(250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, bc);
		Graphics_RenderText(273, YRES+MENUSIZE-63, "Open in Browser", 255, 255, 255, bc);
		Graphics_RenderText(258, YRES+MENUSIZE-64, "\x81", 255, 255, 255, bc);

		//Open Button
		if (sdl_key==SDLK_RETURN && openable) {
			queue_open = 1;
		}
		if (mx > 50 && mx < 50+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && openable && !queue_open) {
			Renderer_FillRectangle(50, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				queue_open = 1;
			}
		}
		//Fav Button
		if (mx > 100 && mx < 100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && !queue_open) {
			Renderer_FillRectangle(100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
				info_box("Adding to favourites...");
				execute_fav(save_id);
			}
		}
		//Report Button
		if (mx > 150 && mx < 150+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
			Renderer_FillRectangle(150, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				if (report_ui(save_id)) {
					retval = 0;
					break;
				}
			}
		}
		//Delete Button
		if (mx > 200 && mx < 200+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && (authoritah || myown) && !queue_open) {
			Renderer_FillRectangle(200, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				if (myown || !info->publish) {
					if (confirm_ui( "Are you sure you wish to delete this?", "You will not be able recover it.", "Delete")) {
						Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box("Deleting...");
						if (execute_delete(save_id)) {
							retval = 0;
							break;
						}
					}
				} else {
					if (confirm_ui("Are you sure?", "This save will be removed from the search index.", "Remove")) {
						Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
						info_box("Removing...");
						if (execute_delete(save_id)) {
							retval = 0;
							break;
						}
					}
				}
			}
		}
		//Open in browser button
		if (mx > 250 && mx < 250+107 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50  && !queue_open) {
			Renderer_FillRectangle(250, YRES+MENUSIZE-68, 107, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				o_uri = malloc(7+strlen(SERVER)+41+strlen(save_id)*3);
				strcpy(o_uri, "http://" SERVER "/Browse/View.html?ID=");
				strcaturl(o_uri, save_id);
				open_link(o_uri);
				free(o_uri);
			}
		}
		//Submit Button
		if (mx > XRES+BARSIZE-100 && mx < XRES+BARSIZE-100+50 && my > YRES+MENUSIZE-68 && my < YRES+MENUSIZE-50 && svf_login && info_ready && !queue_open) {
			Renderer_FillRectangle(XRES+BARSIZE-100, YRES+MENUSIZE-68, 50, 18, 255, 255, 255, 40);
			if (b && !bq) {
				//Button Clicked
				Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 192);
				info_box("Submitting Comment...");
				execute_submit(save_id, ed.str);
				ed.str[0] = 0;
			}
		}
		//If mouse was clicked outsite of the window bounds.
		if (!(mx>50 && my>50 && mx<XRES+BARSIZE-50 && my<YRES+MENUSIZE-50) && b && !queue_open && my<YRES+MENUSIZE-21) {
			retval = 0;
			break;
		}

		//User opened the save, wait until we've got all the data first...
		if (queue_open) {
			if (info_ready && data_ready) {
				// Do Open!
				status = parse_save(data, data_size, 1, 0, 0, bmap, fvx, fvy, signs, parts, pmap);
				if (!status) {
					//if(svf_last)
					//free(svf_last);
					svf_last = data;
					svf_lsize = data_size;

					svf_open = 1;
					svf_own = svf_login && !strcmp(info->author, svf_user);
					svf_publish = info->publish && svf_login && !strcmp(info->author, svf_user);

					strcpy(svf_id, save_id);
					strcpy(svf_name, info->name);
					strcpy(svf_description, info->description);
					if (info->tags)
					{
						strncpy(svf_tags, info->tags, 255);
						svf_tags[255] = 0;
					} else {
						svf_tags[0] = 0;
					}
					svf_myvote = info->myvote;
					retval = 1;
					break;
				} else {
					queue_open = 0;

					svf_open = 0;
					svf_publish = 0;
					svf_own = 0;
					svf_myvote = 0;
					svf_id[0] = 0;
					svf_name[0] = 0;
					svf_description[0] = 0;
					svf_tags[0] = 0;
					if (svf_last)
						free(svf_last);
					svf_last = NULL;
					error_ui(0, "An Error Occurred");
				}
			} else {
				Renderer_FillRectangle(-1, -1, XRES+BARSIZE, YRES+MENUSIZE, 0, 0, 0, 190);
				Graphics_RenderText(50+(XRES/4)-GetTextWidth("Loading...")/2, 50+(YRES/4), "Loading...", 255, 255, 255, 128);
			}
		}
		if (!info_ready || !data_ready) {
			info_box("Loading");
		}
		Renderer_Display();
		//memcpy(vid_buf, old_vid, ((XRES+BARSIZE)*(YRES+MENUSIZE))*PIXELSIZE);
		if (info_ready && svf_login) {
			ui_edit_process(mx, my, b, &ed);
		}

		if (sdl_key==SDLK_ESCAPE) {
			retval = 0;
			break;
		}

		if (lasttime<TIMEOUT)
			lasttime++;
	}
	//Prevent those mouse clicks being passed down.
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		if (!b)
			break;
	}
	//Close open connections
	if (http)
		http_async_req_close(http);
	if (http_2)
		http_async_req_close(http_2);
	return retval;
}

int info_parse(char *info_data, save_info *info)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	memset(info, 0, sizeof(save_info));

	if (!info_data || !*info_data)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*info_data)
			break;
		p = strchr(info_data, '\n');
		if (!p)
			p = info_data + strlen(info_data);
		else
			*(p++) = 0;

		if (!strncmp(info_data, "TITLE ", 6))
		{
			info->title = mystrdup(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "NAME ", 5))
		{
			info->name = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "AUTHOR ", 7))
		{
			info->author = mystrdup(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "DATE ", 5))
		{
			info->date = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "DESCRIPTION ", 12))
		{
			info->description = mystrdup(info_data+12);
			j++;
		}
		else if (!strncmp(info_data, "VOTEUP ", 7))
		{
			info->voteup = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "VOTEDOWN ", 9))
		{
			info->votedown = atoi(info_data+9);
			j++;
		}
		else if (!strncmp(info_data, "VOTE ", 5))
		{
			info->vote = atoi(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "MYVOTE ", 7))
		{
			info->myvote = atoi(info_data+7);
			j++;
		}
		else if (!strncmp(info_data, "MYFAV ", 6))
		{
			info->myfav = atoi(info_data+6);
			j++;
		}
		else if (!strncmp(info_data, "PUBLISH ", 8))
		{
			info->publish = atoi(info_data+8);
			j++;
		}
		else if (!strncmp(info_data, "TAGS ", 5))
		{
			info->tags = mystrdup(info_data+5);
			j++;
		}
		else if (!strncmp(info_data, "COMMENT ", 8))
		{
			if (info->comment_count>=6) {
				info_data = p;
				continue;
			} else {
				q = strchr(info_data+8, ' ');
				*(q++) = 0;
				info->commentauthors[info->comment_count] = mystrdup(info_data+8);
				info->comments[info->comment_count] = mystrdup(q);
				info->comment_count++;
			}
			j++;
		}
		info_data = p;
	}
	if (j>=8) {
		return 1;
	} else {
		return -1;
	}
}

int search_results(char *str, int votes)
{
	int i,j;
	char *p,*q,*r,*s,*vu,*vd,*pu,*sd;

	for (i=0; i<GRID_X*GRID_Y; i++)
	{
		if (search_ids[i])
		{
			free(search_ids[i]);
			search_ids[i] = NULL;
		}
		if (search_names[i])
		{
			free(search_names[i]);
			search_names[i] = NULL;
		}
		if (search_dates[i])
		{
			free(search_dates[i]);
			search_dates[i] = NULL;
		}
		if (search_owners[i])
		{
			free(search_owners[i]);
			search_owners[i] = NULL;
		}
		if (search_thumbs[i])
		{
			free(search_thumbs[i]);
			search_thumbs[i] = NULL;
			search_thsizes[i] = 0;
		}
	}
	for (j=0; j<TAG_MAX; j++)
		if (tag_names[j])
		{
			free(tag_names[j]);
			tag_names[j] = NULL;
		}

	if (!str || !*str)
		return 0;

	i = 0;
	j = 0;
	s = NULL;
	do_open = 0;
	while (1)
	{
		if (!*str)
			break;
		p = strchr(str, '\n');
		if (!p)
			p = str + strlen(str);
		else
			*(p++) = 0;
		if (!strncmp(str, "OPEN ", 5))
		{
			do_open = 1;
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str+5, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+5);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+5, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "HISTORY ", 8))
		{
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				sd = strchr(str+8, ' ');
				if (!sd)
					return i;
				*(sd++) = 0;
				pu = strchr(sd, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str+8);

			search_dates[i] = mystrdup(sd);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str+8, search_thumbs+i, search_thsizes+i);
			i++;
		}
		else if (!strncmp(str, "TAG ", 4))
		{
			if (j >= TAG_MAX)
			{
				str = p;
				continue;
			}
			q = strchr(str+4, ' ');
			if (!q)
			{
				str = p;
				continue;
			}
			*(q++) = 0;
			tag_names[j] = mystrdup(str+4);
			tag_votes[j] = atoi(q);
			j++;
		}
		else
		{
			if (i>=GRID_X*GRID_Y)
				break;
			if (votes)
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				s = strchr(pu, ' ');
				if (!s)
					return i;
				*(s++) = 0;
				vu = strchr(s, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			else
			{
				pu = strchr(str, ' ');
				if (!pu)
					return i;
				*(pu++) = 0;
				vu = strchr(pu, ' ');
				if (!vu)
					return i;
				*(vu++) = 0;
				vd = strchr(vu, ' ');
				if (!vd)
					return i;
				*(vd++) = 0;
				q = strchr(vd, ' ');
			}
			if (!q)
				return i;
			*(q++) = 0;
			r = strchr(q, ' ');
			if (!r)
				return i;
			*(r++) = 0;
			search_ids[i] = mystrdup(str);

			search_publish[i] = atoi(pu);
			search_scoreup[i] = atoi(vu);
			search_scoredown[i] = atoi(vd);

			search_owners[i] = mystrdup(q);
			search_names[i] = mystrdup(r);

			if (s)
				search_votes[i] = atoi(s);
			thumb_cache_find(str, search_thumbs+i, search_thsizes+i);
			i++;
		}
		str = p;
	}
	if (*str)
		i++;
	return i;
}

int execute_tagop(char *op, char *tag)
{
	int status;
	char *result;

	char *names[] = {"ID", "Tag", NULL};
	char *parts[2];

	char *uri = malloc(strlen(SERVER)+strlen(op)+36);
	sprintf(uri, "http://" SERVER "/Tag.api?Op=%s", op);

	parts[0] = svf_id;
	parts[1] = tag;

	result = http_multipart_post(
	             uri,
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	free(uri);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return 1;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return 1;
	}

	if (result && result[2])
	{
		strncpy(svf_tags, result+3, 255);
		svf_id[15] = 0;
	}

	if (result)
		free(result);

	return 0;
}

void execute_save()
{
	int status;
	char *result;

	char *names[] = {"Name","Description", "Data:save.bin", "Thumb:thumb.bin", "Publish", "ID", NULL};
	char *uploadparts[6];
	int plens[6];

	uploadparts[0] = svf_name;
	plens[0] = strlen(svf_name);
	uploadparts[1] = svf_description;
	plens[1] = strlen(svf_description);
	uploadparts[2] = build_save(plens+2, 0, 0, XRES, YRES, bmap, fvx, fvy, signs, parts);
	uploadparts[3] = build_thumb(plens+3, 1);
	uploadparts[4] = (svf_publish==1)?"Public":"Private";
	plens[4] = strlen((svf_publish==1)?"Public":"Private");

	if (svf_id[0])
	{
		uploadparts[5] = svf_id;
		plens[5] = strlen(svf_id);
	}
	else
		names[5] = NULL;

	result = http_multipart_post(
	             "http://" SERVER "/Save.api",
	             names, uploadparts, plens,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (svf_last)
		free(svf_last);
	svf_last = uploadparts[2];
	svf_lsize = plens[2];

	free(uploadparts[3]);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (!result || strncmp(result, "OK", 2))
	{
		if (!result)
			result = mystrdup("Could not save - no reply from server");
		error_ui(0, result);
		free(result);
		return;
	}

	if (result && result[2])
	{
		strncpy(svf_id, result+3, 15);
		svf_id[15] = 0;
	}

	if (!svf_id[0])
	{
		error_ui(0, "No ID supplied by server");
		free(result);
		return;
	}

	thumb_cache_inval(svf_id);

	svf_own = 1;
	if (result)
		free(result);
}

int execute_delete(char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Delete.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "INFO: ", 6)==0)
	{
		info_ui("Info", result+6);
		free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

void execute_submit(char *id, char *message)
{
	int status;
	char *result;

	char *names[] = {"ID", "Message", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = message;

	result = http_multipart_post(
	             "http://" SERVER "/Comment.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

int execute_report(char *id, char *reason)
{
	int status;
	char *result;

	char *names[] = {"ID", "Reason", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = reason;

	result = http_multipart_post(
	             "http://" SERVER "/Report.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}

void execute_fav(char *id)
{
	int status;
	char *result;

	char *names[] = {"ID", NULL};
	char *parts[1];

	parts[0] = id;

	result = http_multipart_post(
	             "http://" SERVER "/Favourite.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return;
	}

	if (result)
		free(result);
}

int execute_vote(char *id, char *action)
{
	int status;
	char *result;

	char *names[] = {"ID", "Action", NULL};
	char *parts[2];

	parts[0] = id;
	parts[1] = action;

	result = http_multipart_post(
	             "http://" SERVER "/Vote.api",
	             names, parts, NULL,
	             svf_user_id, /*svf_pass*/NULL, svf_session_id,
	             &status, NULL);

	if (status!=200)
	{
		error_ui(status, http_ret_text(status));
		if (result)
			free(result);
		return 0;
	}
	if (result && strncmp(result, "OK", 2))
	{
		error_ui(0, result);
		free(result);
		return 0;
	}

	if (result)
		free(result);
	return 1;
}
void open_link(char *uri) {
#ifdef WIN32
	ShellExecute(0, "OPEN", uri, NULL, NULL, 0);
#elif MACOSX
	char *cmd = malloc(7+strlen(uri));
	strcpy(cmd, "open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN32
	char *cmd = malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#elif LIN64
	char *cmd = malloc(11+strlen(uri));
	strcpy(cmd, "xdg-open ");
	strappend(cmd, uri);
	system(cmd);
#else
	printf("Cannot open browser\n");
#endif
}
struct command_history {
	void *prev_command;
	char *command;
};
typedef struct command_history command_history;
command_history *last_command = NULL;
command_history *last_command2 = NULL;
char *console_ui(char error[255],char console_more) {
	int mx,my,b,cc,ci = -1,i;
	pixel *old_buf=calloc((XRES+BARSIZE)*(YRES+MENUSIZE), PIXELSIZE);
	command_history *currentcommand;
	command_history *currentcommand2;
	ui_edit ed;
	ed.x = 15;
	ed.y = 207;
	ed.w = XRES;
	ed.nx = 1;
	ed.def = "";
	strcpy(ed.str, "");
	ed.focus = 1;
	ed.hide = 0;
	ed.multiline = 0;
	ed.cursor = 0;
	//Renderer_FillRectangle(-1, -1, XRES, 220, 0, 0, 0, 190);
	//memcpy(old_buf,vid_buf,(XRES+BARSIZE)*YRES*PIXELSIZE);

	Renderer_FillRectangle(-1, -1, XRES, 220, 0, 0, 0, 190);

	currentcommand2 = malloc(sizeof(command_history));
	memset(currentcommand2, 0, sizeof(command_history));
	currentcommand2->prev_command = last_command2;
	currentcommand2->command = mystrdup(error);
	last_command2 = currentcommand2;

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);//enable keyrepeat for console (is disabled on console close later)
	cc = 0;
	while (cc < 80) {
		Renderer_FillRectangle(-1, -1+cc, XRES+BARSIZE, 2, 0, 0, 0, 160-(cc*2));
		cc++;
	}
	while (!sdl_poll())
	{
		b = SDL_GetMouseState(&mx, &my);
		mx /= sdl_scale;
		my /= sdl_scale;
		ed.focus = 1;

		//memcpy(vid_buf,old_buf,(XRES+BARSIZE)*YRES*PIXELSIZE);
		Renderer_DrawLine(0, 219, XRES+BARSIZE-1, 219, 228, 228, 228, XRES+BARSIZE);
#ifdef PYCONSOLE
		if (pygood)
			i=255;
		else
			i=0;
		if (pyready)
			Graphics_RenderText(15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python by Doxin)", 255, i, i, 255);
		else
			Graphics_RenderText(15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python disabled)", 255, i, i, 255);
#else
		Graphics_RenderText(15, 15, "Welcome to The Powder Toy console v.3 (by cracker64, python disabled)", 255, 255, 255, 255);
#endif

		cc = 0;
		currentcommand = last_command;
		while (cc < 10)
		{
			if (currentcommand==NULL)
				break;
			Graphics_RenderText(15, 175-(cc*12), currentcommand->command, 255, 255, 255, 255);
			if (currentcommand->prev_command!=NULL)
			{
				if (cc<9) {
					currentcommand = currentcommand->prev_command;
				} else if (currentcommand->prev_command!=NULL) {
					free(currentcommand->prev_command);
					currentcommand->prev_command = NULL;
				}
				cc++;
			}
			else
			{
				break;
			}
		}
		cc = 0;
		currentcommand2 = last_command2;
		while (cc < 10)
		{
			if (currentcommand2==NULL)
				break;
			Graphics_RenderText(215, 175-(cc*12), currentcommand2->command, 255, 225, 225, 255);
			if (currentcommand2->prev_command!=NULL)
			{
				if (cc<9) {
					currentcommand2 = currentcommand2->prev_command;
				} else if (currentcommand2->prev_command!=NULL) {
					free(currentcommand2->prev_command);
					currentcommand2->prev_command = NULL;
				}
				cc++;
			}
			else
			{
				break;
			}
		}

		//if(error && ed.str[0]=='\0')
		//Graphics_RenderText(20, 207, error, 255, 127, 127, 200);
		if (console_more==0)
			Graphics_RenderText(5, 207, ">", 255, 255, 255, 240);
		else
			Graphics_RenderText(5, 207, "...", 255, 255, 255, 240);

		ui_edit_draw(&ed);
		ui_edit_process(mx, my, b, &ed);
		Renderer_Display();
		if (sdl_key==SDLK_RETURN)
		{
			currentcommand = malloc(sizeof(command_history));
			memset(currentcommand, 0, sizeof(command_history));
			currentcommand->prev_command = last_command;
			currentcommand->command = mystrdup(ed.str);
			last_command = currentcommand;
			free(old_buf);
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
			return currentcommand->command;
		}
		if (sdl_key==SDLK_ESCAPE || sdl_key==SDLK_BACKQUOTE)
		{
			console_mode = 0;
			free(old_buf);
			SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
			return NULL;
		}
		if (sdl_key==SDLK_UP || sdl_key==SDLK_DOWN)
		{
			ci += sdl_key==SDLK_UP?1:-1;
			if (ci<-1)
				ci = -1;
			if (ci==-1)
			{
				strcpy(ed.str, "");
				ed.cursor = strlen(ed.str);
			}
			else
			{
				if (last_command!=NULL) {
					currentcommand = last_command;
					for (cc = 0; cc<ci; cc++) {
						if (currentcommand->prev_command==NULL)
							ci = cc;
						else
							currentcommand = currentcommand->prev_command;
					}
					strcpy(ed.str, currentcommand->command);
					ed.cursor = strlen(ed.str);
				}
				else
				{
					ci = -1;
					strcpy(ed.str, "");
					ed.cursor = strlen(ed.str);
				}
			}
		}
	}
	console_mode = 0;
	free(old_buf);
	SDL_EnableKeyRepeat(0, SDL_DEFAULT_REPEAT_INTERVAL);
	return NULL;
}

//takes a a string and compares it to element names, and puts it value into element.
int console_parse_type(char *txt, int *element, char *err)
{
	int i = -1;
	// alternative names for some elements
	if (strcasecmp(txt,"C4")==0) i = PT_PLEX;
	else if (strcasecmp(txt,"C5")==0) i = PT_C5;
	else if (strcasecmp(txt,"NONE")==0) i = PT_NONE;
	if (i>=0)
	{
		*element = i;
		strcpy(err,"");
		return 1;
	}
	for (i=1; i<PT_NUM; i++) {
		if (strcasecmp(txt,ptypes[i].name)==0)
		{
			*element = i;
			strcpy(err,"");
			return 1;
		}
	}
	strcpy(err, "Particle type not recognised");
	return 0;
}
//takes a string of coords "x,y" and puts the values into x and y.
int console_parse_coords(char *txt, int *x, int *y, char *err)
{
	// TODO: use regex?
	int nx = -1, ny = -1;
	if (sscanf(txt,"%d,%d",&nx,&ny)!=2 || nx<0 || nx>=XRES || ny<0 || ny>=YRES)
	{
		strcpy(err,"Invalid coordinates");
		return 0;
	}
	*x = nx;
	*y = ny;
	return 1;
}
//takes a string of either coords or a particle number, and puts the particle number into *which
int console_parse_partref(char *txt, int *which, char *err)
{
	int i = -1, nx, ny;
	strcpy(err,"");
	// TODO: use regex?
	if (strchr(txt,',') && console_parse_coords(txt, &nx, &ny, err))
	{
		i = pmap[ny][nx];
		if (!i || (i>>8)>=NPART)
			i = -1;
		else
			i = i>>8;
	}
	else if (txt)
	{
		char *num = (char*)malloc(strlen(txt)+3);
		i = atoi(txt);
		sprintf(num,"%d",i);
		if (!txt || strcmp(txt,num)!=0)
			i = -1;
		free(num);
	}
	if (i>=0 && i<NPART && parts[i].type)
	{
		*which = i;
		strcpy(err,"");
		return 1;
	}
	if (strcmp(err,"")==0) strcpy(err,"Particle does not exist");
	return 0;
}
