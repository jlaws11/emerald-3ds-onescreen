# Single screen

For a player who sees only the top screen: an emulator set to show one
screen (Azahar's Single Screen layout), a handheld PC such as the Steam Deck,
or a TV. OFF by default: **OPTIONS → GENERAL → INTERFACE → SINGLE SCREEN**.

With it ON, the top screen shows the game as usual, and shows the bottom
screen's picture instead whenever the player has to look down there:

- **X** in the field gives the dock and the header the buttons, as it always
  does: the bottom screen comes up on the top one. LEFT and RIGHT walk the
  dock, UP goes to the header, A opens, B or X goes back to the game.
- Everything opened from there: the party, the bag, the Pokédex, the
  PokéNav, the trainer card, SAVE and OPTIONS, and the Fly map.
- The game's own screens that live down there: the PC's boxes, the shops'
  BUY screen, trades, the summary.
- The bag and the party opened from a battle.

It is the bottom screen's own picture, 1:1 in the middle of the top screen,
with the game darkened in the 40 pixels at each side. Every one of those
screens is worked with the buttons, so the touch screen is never needed.
The 3D slider is ignored while it shows.

Battles keep the game's own menus, as on the GBA: FIGHT, BAG, POKéMON and
RUN beside the prompt in the text box, the moves with their PP and type, the
target chosen on the battlers. The bottom screen's battle buttons are not
used, and the text box is the GBA's throughout the battle (the HD box has no
art for the menu pages), stretched across the width with the menu kept
whole at the right edge.

## How it works

- `3ds_bottom_ui.c` (`MirrorWanted`) decides each frame from the bottom
  screen's mode, the X focus, the Fly map and whether the top screen is held
  for a hidden menu, and tells the compositor (`CtrVideo_SetMirror`).
- `3ds_video.c` (`MirrorReady`, `MirrorUpload`, `MirrorDraw`, `MirrorFrame`)
  reads the bottom framebuffer back inside the frame, after `FrameBegin` has
  waited for the last frame's GPU work and before this frame's
  `BottomTransfer`, tiles it into a 512x256 RGB565 texture in linear memory
  (`CtrVideo_ScreenToTexels`, 256 KiB, made on first use and given back
  three seconds after the last) and draws it over the top screen. The copy
  is redone only when the canvas was copied to the screen since
  (`CtrBottom_BlitCount`) or a game screen is in the bottom window.
- While the top screen is held or a game screen is on the bottom one, the
  top is not composed: the copy goes over the frame it kept, darkened once,
  and stays until the top screen is drawn again.
- Without linear memory for the texture it logs once and the bottom screen
  stays where it is; it retries later.
- Battles: `CtrBattleMenu_Active` (`3ds_bottom_ui.c`) is false with the
  option on, so `battle_controller_player.c` runs the game's own menus on
  BG0's pages. `3ds_video.c` draws the GBA's box for them (`BattleOwnMenus`)
  and `BattleBandRight` keeps a page's second box whole at the right edge
  while the first one stretches; on the message page that is the right cap,
  as before. Recorded battles, which always used the game's menus, get the
  same layout.

`make verify-video` checks the framebuffer-to-texture tiling.
