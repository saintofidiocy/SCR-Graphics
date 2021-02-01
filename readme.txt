StarCraft: Remastered Graphics

This is currently kind of a mess and probably far from an ideal implementation, but it should hopefully provide functional sample code for reading and displaying HD graphics.

Command line options:
scrg [-w #] [-h #] [-t #] [-s name] [-c] ["path to raw chk map file"]

-w -- sets the window width
-h -- sets the window height
-t -- sets the texture detail: 0 = SD, 1 = HD2, 2 = HD
-s -- use skin "name" (valid options are "Carbot", "Presale")
-c -- use Carbot skin


carbot.bat and hd.bat will apply the appropriate settings without needing to run from command line, and both will also accept drag & dropped CHK files.
