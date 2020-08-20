/* See LICENSE file for copyright and license details. */

#include "theme.h"

/* appearance */
static const unsigned int borderpx  = 2;  /* border pixel of windows */
static const unsigned int snap      = 32; /* snap pixel */
static const int showsystray        = 1;     /* 0 means no systray */
static const int swallowfloating    = 1;   /* 1 means swallow floating windows by default */
static const unsigned int gappih    = 5;  /* horiz inner gap between windows */
static const unsigned int gappiv    = 5;  /* vert inner gap between windows */
static const unsigned int gappoh    = 5;  /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 5;  /* vert outer gap between windows and screen edge */
static const int smartgaps          = 0;  /* 1 means no outer gap when there is only one window */
static const int showbar            = 1;  /* 0 means no bar */
static const int topbar             = 1;  /* 0 means bottom bar */
static const int startontag         = 0;  /* 0 means no tag active on start */
static const int vertpad            = 5;  /* vertical padding of bar */
static const int sidepad            = 5;  /* horizontal padding of bar */
static const int barheight          = 0;  /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const char *fonts[]          = { 
	THEME_FONT,
#ifdef EXTRA_FONTS
	EXTRA_FONTS
#endif
};
static const char *colors[][3] = {
	/*                   fg           bg           border   */
	[SchemeNorm]     = { THEME_WHITE, THEME_BLACK, THEME_BRIGHTBLACK },
	[SchemeSel]      = { THEME_BLACK, THEME_BLUE,  THEME_BLUE  },
	[SchemeStatus]   = { THEME_BRIGHTWHITE, THEME_BLACK, NULL },
	[SchemeTagsSel]  = { THEME_BLACK, THEME_BLUE,  NULL },
	[SchemeTagsNorm] = { THEME_WHITE, THEME_BLACK, NULL },
	[SchemeInfoSel]  = { THEME_BLACK, THEME_BLUE,  NULL },
	[SchemeInfoNorm] = { THEME_WHITE, THEME_BLACK, NULL },
};

/* tagging */
static const char *tags[] = { "  ", "  ", "  ", "  ", "  ", "6", "7", "8", "9" };
static const char *tagsalt[] = { "1", "2", "3", "4", "5", "6", "7", "8", "9" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Firefox", NULL,     NULL,           1 << 8,    0,          0,          -1,        -1 },
	{ "st",      NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
	{ "[D]",      deck },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} }

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }
#define XDGPICS "~/pics"

/* commands */
enum {
	CmdDmenu, CmdSt, CmdPAMute, CmdPAVolUp, CmdPAVolUpU, CmdPAVolDown, CmdPAVolDownU,
	CmdMpcToggle, CmdMpcPrev, CmdMpcNext, CmdMpcSeekBack, CmdMpcSeekForw, CmdMpcSeekBackL,
	CmdMpcSeekForwL, CmdScrotScreen, CmdScrotRegion, CmdLast
};

static const char *cmds[][CmdLast] = {
	[CmdDmenu]        = { "dmenu_run", NULL },
	[CmdSt]           = { "st", NULL },
	[CmdPAMute]       = { "pamixer" , "-t", NULL },
	[CmdPAVolUp]      = { "pamixer" , "--max-volume=50", "-i5", NULL },
	[CmdPAVolUpU]     = { "pamixer" , "-i5", NULL },
	[CmdPAVolDown]    = { "pamixer" , "--max-volume=50", "-d5", NULL },
	[CmdPAVolDownU]   = { "pamixer" , "-d5", NULL },
	[CmdMpcToggle]    = { "mpc" , "toggle", NULL},
	[CmdMpcPrev]      = { "mpc" , "prev", NULL},
	[CmdMpcNext]      = { "mpc" , "next", NULL},
	[CmdMpcSeekBack]  = { "mpc" , "seek", "-10",  NULL},
	[CmdMpcSeekForw]  = { "mpc" , "seek", "+10", NULL},
	[CmdMpcSeekBackL] = { "mpc" , "seek", "-1:00",  NULL},
	[CmdMpcSeekForwL] = { "mpc" , "seek", "+1:00", NULL},
	[CmdScrotScreen]  = {"scrot", "-e", "xclip -selection clipboard -target image/png '$f'", "-e", "mv $f ~/pics/screenshots/",  NULL},
	[CmdScrotRegion]  = {"scrot", "-s", "-e", "xclip -selection clipboard -target image/png '$f'", "-e", "mv $f ~/pics/screenshots/", NULL},
};

static Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = cmds[CmdSt] } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY|ControlMask,           XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefloating, {0} },
	{ MODKEY,                       XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	{ MODKEY,                       XK_n,      togglealttag,   {0} },
	{ MODKEY|ALTKEY,                XK_1,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY|ALTKEY,                XK_2,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY|ALTKEY,                XK_3,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY|ALTKEY,                XK_4,      setlayout,      {.v = &layouts[3]} },
	{ MODKEY,                       XK_q,      killclient,     {0} },
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ MODKEY|ShiftMask,             XK_r,      quit,           {1} },
	{ MODKEY,                       XK_p,      spawnoverbar,   {.v = cmds[CmdDmenu] } },
	{ XK_NO_MOD,    XF86XK_AudioMute,          spawn,          {.v = cmds[CmdPAMute]} },
	{ XK_NO_MOD,    XF86XK_AudioLowerVolume,   spawn,          {.v = cmds[CmdPAVolDown]} },
	{ ShiftMask,    XF86XK_AudioLowerVolume,   spawn,          {.v = cmds[CmdPAVolDownU]} },
	{ XK_NO_MOD,    XF86XK_AudioRaiseVolume,   spawn,          {.v = cmds[CmdPAVolUp]} },
	{ ShiftMask,    XF86XK_AudioRaiseVolume,   spawn,          {.v = cmds[CmdPAVolUpU]} },
	{ XK_NO_MOD,    XF86XK_AudioPlay,          spawn,          {.v = cmds[CmdMpcToggle]} },
	{ XK_NO_MOD,    XF86XK_AudioPause,         spawn,          {.v = cmds[CmdMpcToggle]} },
	{ XK_NO_MOD,    XF86XK_AudioPrev,          spawn,          {.v = cmds[CmdMpcPrev]} },
	{ XK_NO_MOD,    XF86XK_AudioNext,          spawn,          {.v = cmds[CmdMpcNext]} },
	{ ControlMask,  XF86XK_AudioPrev,          spawn,          {.v = cmds[CmdMpcSeekBack]} },
	{ ControlMask,  XF86XK_AudioNext,          spawn,          {.v = cmds[CmdMpcSeekForw]} },
	{ ShiftMask,    XF86XK_AudioPrev,          spawn,          {.v = cmds[CmdMpcSeekBackL]} },
	{ ShiftMask,    XF86XK_AudioNext,          spawn,          {.v = cmds[CmdMpcSeekForwL]} },

	/* modifier     key            function     argument                        event */
	{ XK_NO_MOD,    XK_Print,      spawn,       {.v = cmds[CmdScrotScreen]},    KeyRelease },
	{ ControlMask,  XK_Print,      spawn,       {.v = cmds[CmdScrotRegion]},    KeyRelease },

	/*      trigger   tag */
	TAGKEYS(XK_1,     0),
	TAGKEYS(XK_2,     1),
	TAGKEYS(XK_3,     2),
	TAGKEYS(XK_4,     3),
	TAGKEYS(XK_5,     4),
	TAGKEYS(XK_6,     5),
	TAGKEYS(XK_7,     6),
	TAGKEYS(XK_8,     7),
	TAGKEYS(XK_9,     8),
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = cmds[CmdSt] } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

