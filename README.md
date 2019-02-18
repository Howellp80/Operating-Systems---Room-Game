# Operating-Systems---Room-Game
In the game, the player will begin in the "starting room" and will win the game automatically upon entering the "ending
room", which causes the game to exit, displaying the path taken by the player.
During the game, the player can also enter a command that returns the current time - this functionality utilizes mutexes
and multithreading.

Usage:
First to generate random room connections:
$ gcc –o howellp.buildrooms howellp.buildrooms.c
$ howellp.buildrooms

Then to play:
$ gcc –o howellp.adventure howellp.adventure.c -lpthread
$ howellp.adventure

e.g.:
CURRENT LOCATION: XYZZY
POSSIBLE CONNECTIONS: PLOVER, Dungeon, twisty.
WHERE TO? >Twisty

HUH? I DON’T UNDERSTAND THAT ROOM. TRY AGAIN.

CURRENT LOCATION: XYZZY
POSSIBLE CONNECTIONS: PLOVER, Dungeon, twisty.
WHERE TO? >time

1:03pm, Tuesday, September 13, 2016

WHERE TO? >twisty

CURRENT LOCATION: twisty
POSSIBLE CONNECTIONS: PLOVER, XYZZY, Dungeon, PLUGH.
WHERE TO? >Dungeon

YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!
YOU TOOK 2 STEPS. YOUR PATH TO VICTORY WAS:
twisty
Dungeon
$ echo $?
0
$ ls
currentTime.txt howellp.adventure howellp.adventure.c howellp.buildrooms
howellp.buildrooms.c howellp.rooms.19903
$ ls howellp.rooms.19903
Crowther_room Dungeon_room PLUGH_room PLOVER_room
twisty_room XYZZY_room Zork_room
$ cat howellp.rooms.19903/XYZZY_room
ROOM NAME: XYZZY
CONNECTION 1: PLOVER
CONNECTION 2: Dungeon
CONNECTION 3: twisty
ROOM TYPE: START_ROOM
