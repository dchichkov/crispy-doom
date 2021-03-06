//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
// Copyright(C) 2016 Fabian Greffrath
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	[crispy] Archiving: Extended SaveGame I/O.
//

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "doomstat.h"
#include "doomtype.h"
#include "m_misc.h"
#include "p_local.h"
#include "p_extsaveg.h"
#include "p_saveg.h"
#include "z_zone.h"

static char line[260];
static char string[80];

static void P_WritePackageTarname (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %s\n", key, PACKAGE_VERSION);
	fputs(line, save_stream);
}

// maplumpinfo->wad_file->name

char *savewadfilename = NULL;

static void P_WriteWadFileName (const char *key)
{
	M_snprintf(line, sizeof(line), "%s %s\n", key, maplumpinfo->wad_file->name);
	fputs(line, save_stream);
}

static void P_ReadWadFileName (const char *key)
{
	if (!savewadfilename)
	{
		if (sscanf(line, "%s", string) == 1 &&
			!strncmp(string, key, sizeof(string)))
		{
			if (sscanf(line, "%*s %s", string) == 1)
			{
				savewadfilename = strdup(string);
			}
		}
	}
}

// extrakills

static void P_WriteExtraKills (const char *key)
{
	if (extrakills)
	{
		M_snprintf(line, sizeof(line), "%s %d\n", key, extrakills);
		fputs(line, save_stream);
	}
}

static void P_ReadExtraKills (const char *key)
{
	int value;

	if (sscanf(line, "%s %d", string, &value) == 2 &&
	    !strncmp(string, key, sizeof(string)))
	{
		extrakills = value;
	}
}

// totalleveltimes

static void P_WriteTotalLevelTimes (const char *key)
{
	if (totalleveltimes)
	{
		M_snprintf(line, sizeof(line), "%s %d\n", key, totalleveltimes);
		fputs(line, save_stream);
	}
}

static void P_ReadTotalLevelTimes (const char *key)
{
	int value;

	if (sscanf(line, "%s %d", string, &value) == 2 &&
	    !strncmp(string, key, sizeof(string)))
	{
		totalleveltimes = value;
	}
}

// T_FireFlicker()

extern void T_FireFlicker (fireflicker_t* flick);

static void P_WriteFireFlicker (const char *key)
{
	thinker_t* th;

	for (th = thinkercap.next; th != &thinkercap; th = th->next)
	{
		if (th->function.acp1 == (actionf_p1)T_FireFlicker)
		{
			fireflicker_t *flick = (fireflicker_t *)th;

			M_snprintf(line, sizeof(line), "%s %d %d %d %d\n",
			           key,
			           (int)(flick->sector - sectors),
			           (int)flick->count,
			           (int)flick->maxlight,
			           (int)flick->minlight);
			fputs(line, save_stream);
		}
	}
}

static void P_ReadFireFlicker (const char *key)
{
	int sector, count, maxlight, minlight;

	if (sscanf(line, "%s %d %d %d %d\n",
	           string,
	           &sector,
	           &count,
	           &maxlight,
	           &minlight) == 5 &&
	    !strncmp(string, key, sizeof(string)))
	{
		fireflicker_t *flick;

		flick = Z_Malloc(sizeof(*flick), PU_LEVEL, NULL);

		flick->sector = &sectors[sector];
		flick->count = count;
		flick->maxlight = maxlight;
		flick->minlight = minlight;

		flick->thinker.function.acp1 = (actionf_p1)T_FireFlicker;

		P_AddThinker(&flick->thinker);
	}
}

// markpoints[]

extern void AM_GetMarkPoints (int *n, long *p);
extern void AM_SetMarkPoints (int n, long *p);

static void P_WriteMarkPoints (const char *key)
{
	int n;
	long p[20];

	AM_GetMarkPoints(&n, p);

	if (p[0] != -1)
	{
		M_snprintf(line, sizeof(line), "%s %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
		           key, n,
		           p[0], p[1], p[2], p[3], p[4],
		           p[5], p[6], p[7], p[8], p[9],
		           p[10], p[11], p[12], p[13], p[14],
		           p[15], p[16], p[17], p[18], p[19]);
		fputs(line, save_stream);
	}
}

static void P_ReadMarkPoints (const char *key)
{
	int n;
	long p[20];

	if (sscanf(line, "%s %d %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld\n",
	           string, &n,
	           &p[0], &p[1], &p[2], &p[3], &p[4],
	           &p[5], &p[6], &p[7], &p[8], &p[9],
	           &p[10], &p[11], &p[12], &p[13], &p[14],
	           &p[15], &p[16], &p[17], &p[18], &p[19]) == 22 &&
	    !strncmp(string, key, sizeof(string)))
	{
		AM_SetMarkPoints(n, p);
	}
}

// players[]->lookdir

static void P_WritePlayersLookdir (const char *key)
{
	int i;

	for (i = 0; i < MAXPLAYERS; i++)
	{
		if (playeringame[i] && players[i].lookdir)
		{
			M_snprintf(line, sizeof(line), "%s %d %d\n", key, i, players[i].lookdir);
			fputs(line, save_stream);
		}
	}
}

static void P_ReadPlayersLookdir (const char *key)
{
	int i, value;

	if (sscanf(line, "%s %d %d", string, &i, &value) == 3 &&
	    !strncmp(string, key, sizeof(string)) &&
	    i < MAXPLAYERS)
	{
		players[i].lookdir = value;
	}
}

typedef struct
{
	const char *key;
	void (* extsavegwritefn) (const char *key);
	void (* extsavegreadfn) (const char *key);
} extsavegdata_t;

static const extsavegdata_t extsavegdata[] =
{
	{PACKAGE_TARNAME, P_WritePackageTarname, NULL},
	{"wadfilename", P_WriteWadFileName, P_ReadWadFileName},
	{"extrakills", P_WriteExtraKills, P_ReadExtraKills},
	{"totalleveltimes", P_WriteTotalLevelTimes, P_ReadTotalLevelTimes},
	{"fireflicker", P_WriteFireFlicker, P_ReadFireFlicker},
	{"markpoints", P_WriteMarkPoints, P_ReadMarkPoints},
	{"playerslookdir", P_WritePlayersLookdir, P_ReadPlayersLookdir},
};

void P_WriteExtendedSaveGameData (void)
{
	int i;

	for (i = 0; i < arrlen(extsavegdata); i++)
	{
		extsavegdata[i].extsavegwritefn(extsavegdata[i].key);
	}
}

static void P_ReadKeyValuePairs (void)
{
	while (fgets(line, sizeof(line), save_stream))
	{
		if (sscanf(line, "%s", string) == 1)
		{
			int i;

			for (i = 1; i < arrlen(extsavegdata); i++)
			{
				if (extsavegdata[i].extsavegreadfn &&
				    !strncmp(string, extsavegdata[i].key, sizeof(string)))
				{
					extsavegdata[i].extsavegreadfn(extsavegdata[i].key);
				}
			}
		}
	}
}

void P_ReadExtendedSaveGameData (void)
{
	long p, curpos, endpos;

	curpos = ftell(save_stream);

	fseek(save_stream, 0, SEEK_END);
	endpos = ftell(save_stream);

	for (p = endpos - 1; p > 0; p--)
	{
		byte curbyte;

		fseek(save_stream, p, SEEK_SET);

		if (fread(&curbyte, 1, 1, save_stream) < 1)
		{
			break;
		}

		if (curbyte == SAVEGAME_EOF)
		{
			if (!fgets(line, sizeof(line), save_stream))
			{
				continue;
			}

			if (sscanf(line, "%s", string) == 1 &&
			    !strncmp(string, extsavegdata[0].key, sizeof(string)))
			{
				P_ReadKeyValuePairs();
				break;
			}
		}
	}

	fseek(save_stream, curpos, SEEK_SET);
}
