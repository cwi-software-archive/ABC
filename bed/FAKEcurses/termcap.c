/* SccsId: %W% %G% */
/*
 * Dummy termcap-compatible routines.
 * (That is, just enough to let the B editor work
 * FAKE curses package for the HP 2621 terminal.)
 */

tgetent()
{
	return 1;
}

char *
tgetstr(key)
	char *key;
{
	if (key && key[0] == 'k') {
		if (key[1] == 's')
			return "\33&jB";
		if (key[1] == 'e')
			return "\33&jA";
	}
	return (char *)0;
}
