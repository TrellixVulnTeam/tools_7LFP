/******************************************************************************

	    Copyright (c) 1994-2007 Macrovision Europe Ltd. and/or Macrovision Corporation. All Rights Reserved.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of
	Macrovision Inc and is protected by law.  It may
	not be copied or distributed in any form or medium, disclosed
	to third parties, reverse engineered or used in any manner not
	provided for in said License Agreement except with the prior
	written authorization from Macrovision Corporation.
 *****************************************************************************/
/*
 *
 *	Function:	get_dir
 *
 *	Description: 	Gets the directory the program was launched from
 *
 *	Parameters:	argc and argv from main()
 *
 *	Return:		(char *)path to directory.
 *
 *	Example:
 *
 *	char licpath[MAX_PATH_LEN+1];
 *
 *		sprintf(path, "%s/licensing/license.dat", get_dir(argc, argv));
 *		CHECKOUT(LM_RESTRICTIVE, feature, ver, licpath);
 *
 *	D. Birns
 *	4/5/96
 *
 *	Last changed:  4/5/96
 *
 */
#include <stdio.h>
#include <unistd.h>
char *get_dir lm_args((int, char **));

char *
next_path(nextpath)
char **nextpath;
{
  static char lbuf[2500];
  char *bp = lbuf, *cp;
  char *path = *nextpath;

	for (cp = path; *cp && *cp != ':'; cp++)
		*bp++ = *cp;
	cp++;
	if (!*lbuf && !*cp)
		return 0;
	*bp = 0;
	*nextpath = cp;
	return lbuf;
}

char *
get_dir(argc, argv)
int argc;
char **argv;
{
  static char buf[2500];
  char *path = 0, *currpath;
  int looking = 1;
  char *cp;

	if (*argv[0] == '.' || strchr(argv[0], '/'))
	{
		strcpy(buf, argv[0]);
	}
	else
	{
		*buf = 0;
		path = getenv("PATH");
		currpath = next_path(&path);
		for (looking = 1; looking && currpath;
						currpath = next_path(&path))
		{
			if (*currpath)
				sprintf(buf, "%s/%s", currpath, argv[0]);
			else
				strcpy(buf, argv[0]);
			if (!access(buf, 1))
				looking = 0;
			else
				*buf = 0;
		}
	}
	for(cp = &buf[strlen(buf)-1]; *cp && *cp != '/'; cp--)
		;
	*cp = 0;
	if (cp == buf) strcpy(buf, ".");
	strcat(buf, "/license.dat");
	return(buf);
}
