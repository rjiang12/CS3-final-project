Game!!

# Project Log

## Tuesday, 5/17, 4:15pm
**WHO:** Emily

**WHAT:** ported EVERYTHING

**BUGS:** none yet!

**RESOURCES USED:** none

## Wednesday, 5/18, 12:30am - 2:30am
**WHO:** Kaushik

**WHAT:** began setting up airhockey.c demo file

**BUGS:** so far, so good

**RESOURCES USED:** none

## Wednesday, 5/18 6:30pm - 8:00pm
**WHO:** Roy

**WHAT:** started planning and writing powerup functions

**BUGS:** none yet. Not really a bug but for whatever reason I wrote it as an h file. This file will disappear when I move it into the demo. DW. 

**RESOURCES USED:** none

## Wednesday, 5/18 - Thursday, 5/19 11:00pm - 2:15am
**WHO:** Avi and Kaushik

**WHAT:** Set up display for table, puck, players, and walls. 

**BUGS:** Walls aren't being displayed, and the puck is off-center at initialization. Also we need to do keyboard stuff. 

**RESOURCES USED:** none

## Thursday, 5/18 1:30pm - 2:15pm
**WHO:** Emily

**WHAT:** Set up powerup in state, trying to display walls. Fixed puck placement.

**BUGS:** Walls aren't being displayed, and the puck is off-center at initialization. Also we need to do keyboard stuff. 

**RESOURCES USED:** libraries: SDL_mixer 2.0 for sound, sdl ttf, sdl image

## Thursday, 5/18 11:00pm - 11:59pm
**WHO:** Avi and Kaushik

**WHAT:** Added goals, scoring, and checking win condition. 

**BUGS:**

**RESOURCES USED:** none

## Saturday, 5/21 11:00am - 1:15pm
**WHO:** Roy

**WHAT:** Attempted to make text rendering work. 

**BUGS:** Text rendering doesn't work - flags were added as appropriate but SDL functions are supposedly undefined despite being included. 

**RESOURCES USED:** SDL documentation. 

## Saturday, 5/21 11:00am - 1:15pm
**WHO:** Emily and Roy

**WHAT:** Attempted to make text rendering APPEAR. 

**BUGS:** SDL functions/libraries are recognized, but the text still does not appear in the demo.

**RESOURCES USED:** SDL documentation. 

## Sunday, 5/22 6:45pm - 8:15pm 
**WHO:** Roy

**WHAT:** Attempted to make freeze powerups spawn regularly

**BUGS:** emscripten is bugging tf out

**RESOURCES USED:** None

## Monday, 5/23 10:00am - 11:00am, 1:30pm - 2:00pm 
**WHO:** Roy

**WHAT:** Powerups spawn, freeze works. Ready to implement more powerups. 

**BUGS:** None!

**RESOURCES USED:** Stackoverflow - I forgot how to make function pointers. 

## Monday, 5/23 7:00pm - 8:30pm, 9:30pm - 10:30pm
**WHO:** Roy

**WHAT:** Powerups spawn. Randomized powerup spawning and implementation works! 

**BUGS:** Balancing needs some work... I think double accel and double velocity both enable bodies to go mach 10 or something... (9:30-10:30) no more mach 10! No more bugs currently. 

**RESOURCES USED:** None. 

## Tuesday, 5/24 9:00pm - 10:00pm
**WHO:** Kaushik

**WHAT:** Rewrote key handler using SDL_GetKeyboardState to accept multiple simultaneous key inputs and account for diagonal motion!

**BUGS:** The handler is a bit laggy... not sure how to resolve this

**RESOURCES USED:** SDL documentation

## Wednesday, 5/24 2:00am - 3:00am
**WHO:** Avi 

**WHAT:** Resolved the laggy handler. Added a cap on the amount of power-ups that appear on the screen at once. 

**BUGS:** Acceleration + min velocity might need tweaking. 

**RESOURCES USED:** 

## Thursday, 5/26 1:30pm - 3:30pm
**WHO:** Roy 

**WHAT:** Trying to implement text rendering. Lots of failed debugging. 

**BUGS:** The path for the font is not recognized, and "SDL not built with thread support, apparently". 

**RESOURCES USED:** StackOverFlow. 

## Thursday, 5/26 10pm - 11:30pm
**WHO:** Roy 

**WHAT:** Trying to implement text rendering. Lots of failed debugging. 

**BUGS:** The path for the font is not recognized, and "SDL not built with thread support, apparently". 

**RESOURCES USED:** None. No responses from TAs, either.  

## Friday, 5/27 8:30am - 10:45am
**WHO:** Roy 

**WHAT:** Implemented sound! 

**BUGS:** None. Our sound effects suck though. 

**RESOURCES USED:** SDL documentation, SDL_Mixer tutorials. 

## Friday, 5/27 8:00pm - 11:00pm
**WHO:** Roy 

**WHAT:** Made some balancing changes. Reset player positions and removed all powerup effects after a goal. Attempted to implement text and images, both to no avail. 

**BUGS:** Neither text nor images works. 

**RESOURCES USED:** SDL documentation and tutorials.

## Saturday, 5/28 9:00am - 11:15am
**WHO:** Roy 

**WHAT:** IMAGES WORK. And they stick to the body too. 

**BUGS:** No text. Oh well. 

**RESOURCES USED:** SDL documentation and tutorials.

## Saturday, 5/28 9:30pm - 10:00pm
**WHO:** Roy 

**WHAT:** SCOREBOARD

**BUGS:** No text. Haven't tried. 

**RESOURCES USED:** None.

## Sunday, 5/29 12:30pm - 2:30pm
**WHO:** Kaushik 

**WHAT:** Attempted to convert all info into enums to avoid confusion and potential memory leaks. Also
re-rewrote the key handler so that the resultant motion of the paddles is far more intuitive.

**BUGS:** Enums don't work due to inconsistencies between void * and enum types. I just resolved the
numerous warnings regarding our info types instead - now we no longer have warnings.

**RESOURCES USED:** None.

## Monday, 5/30 9:30am - 10:30am
**WHO:** Roy 

**WHAT:** Made text. 

**BUGS:** None!

**RESOURCES USED:** SDL documentation.

## Monday, 5/30 9:00pm - 10:30pm
**WHO:** Roy 

**WHAT:** Found and implemented powerup sprites. 

**BUGS:** None!

**RESOURCES USED:** None.

## Tuesday, 5/31 7:00pm - 8:30pm
**WHO:** Roy 

**WHAT:** Tried making some sort of powerup indicator. And tried freeing everything. The solution was to put in a grid that let us see where things were. 

**BUGS:** No matter what, indicator refuses to show, and the walls instead turn invisible (they aren't removed, they literally just disappear). These are two entities that have absolutely no correlation with one another. 

**RESOURCES USED:** None.

## Tuesday, 5/31 10:00pm - 11:30pm
**WHO:** Emily, Kaushik, and Avi

**WHAT:** PAUSE with a pause screen! Next, we will put in a menu!

**BUGS:** Nah

**RESOURCES USED:** None.

## Tuesday, 5/31, 11:30pm-12:00am
**WHO:** Kaushik

**WHAT:** Drew a table for the game (Canada-themed, of course)!

**BUGS:** None! everything looks good and the drawn walls appear to line up exactly
with the walls we created as bodies

**RESOURCES USED:** None.

## Wednesday, 6/01, 8:00am - 8:30am
**WHO:** Roy

**WHAT:** Put in background music, and freed everything in emcripten_free. 

**BUGS:** None! 

**RESOURCES USED:** None.

## Wednesday, 6/01, 9:30am - 10:30am
**WHO:** Roy

**WHAT:** Fixed the mach 10 puck. 

**BUGS:** None! 

**RESOURCES USED:** None.

## Wednesday, 6/01, 3:00pm - 4:00pm
**WHO:** Kaushik and Avi

**WHAT:** Designed instruction screen and menu screen as images that will be displayed when
instructions are requested.

**BUGS:** I don't think there are any, but let's hope it doesn't look ugly in-game.

**RESOURCES USED:** None.

## Wednesday, 6/01, 9:00pm - 10:30pm
**WHO:** Roy

**WHAT:** Made a powerup indicator in the form of a message. 

**BUGS:** None.

**RESOURCES USED:** None.

## Thursday, 6/02, 2:00am - 3:00am
**WHO:** Avi 

**WHAT:** Removed magic numbers and some pesky warnings. 

**BUGS:** None. 

**RESOURCES USED:** Stack Overflow to resolve warnings. 

## Thursday, 6/02, 2:30pm - 3:00pm
**WHO:** Kaushik 

**WHAT:** Cleaned up demo code a bit.

**BUGS:** None. 

**RESOURCES USED:** None 

## Thursday, 6/02, 5:30pm - 6:00pm
**WHO:** Kaushik 

**WHAT:** Set up enums for the current screen being displayed, and created color
selection screen and uploaded image to assets folder.

**BUGS:** None. 

**RESOURCES USED:** None

## Thursday, 6/02, 10:30pm - 11:30pm
**WHO:** Kaushik 

**WHAT:** Implemented navigation through menus.

**BUGS:** Color selection doesn't actually allow the player to select their color. Need to
think about how to do that. 

**RESOURCES USED:** None

## Thursday, 6/02, 11:30pm - 12:00am
**WHO:** Roy 

**WHAT:** Color selection works, put in free statements to prevent hella memory leaks

**BUGS:** None! We're... done??

**RESOURCES USED:** None

## Friday, 6/03, 9:00am - 9:30am
**WHO:** Roy 

**WHAT:** Code cleanup, magic numbers. 

**BUGS:** None! 

**RESOURCES USED:** None

## Thursday, 6/03, 11:00am - 12:30pm
**WHO:** Avi 

**WHAT:** Added option to exit to menu from the pause screen. Also, the game resets to the menu when it ends, instead of exiting. 

**BUGS:** The player win message no longer gets displayed... can someone figure this out please and thanks. 

**RESOURCES USED:** None

## Thursday, 6/03, 12:00pm - 1:15pm
**WHO:** Roy 

**WHAT:** Remade menu screens with better art

**BUGS:** None. Message is not displayed due to some sort of async issue. 

**RESOURCES USED:** None

## Thursday, 6/03, 1:30pm - 3:45pm
**WHO:** Roy and Kaushik
 
**WHAT:** Merge request + fixing up aftermath + done. 

**BUGS:** None. Dwai. 

**RESOURCES USED:** None