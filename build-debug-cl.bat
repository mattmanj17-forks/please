cl "src/engine.c" "src/game.c" "src/platform_win32.c"^
	/Iinclude /Zi /DDEBUG /D_CRT_SECURE_NO_WARNINGS^
	/link user32.lib gdi32.lib hid.lib /OUT:game.exe /PDB:game.pdb /DEBUG
