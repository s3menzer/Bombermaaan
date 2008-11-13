<?php

include_once( "inc/webpage.php" );


$webpage = new WebPage();
$webpage->setOption_AddFeatureJavascript();
$webpage->head();

$item_number = 0;

function _add( $headline, $additionaltext, $additionalhtml = "" ) {

	global $item_number;
	
	$item_number ++;
	
?>
<div class="group">

<div class="head" onclick="toggleFeatureItem( <?php echo $item_number; ?> );">
<?php // IE7 removes the space between the sign and the text if split to more than one line ?>
<code><span id="feature-sign-<?php echo $item_number; ?>">+</span>&nbsp;</code><?php echo $headline; ?>
</div>

<div id="feature-more-<?php echo $item_number; ?>" class="more" style="display: none;">
<?php echo "<div>$additionaltext</div>"; if ( !empty( $additionalhtml ) ) { echo "<div style=\"margin-top: 1em;\">$additionalhtml</div>"; } ?>
</div>

</div>

<?php

}

?>

<h1>Features</h1>

<div id="content">

<p>
Bombermaaan already provides these features (click on the feature text to see details):
</p>

<div id="featurelist">

<?php

_add( "Classic Bomberman game", "Bombermaaan is a classic Bomberman game. There's a similar game called Dynablaster." );

_add( "Original gameplay and fun with minor adjustments", "If you're familiar with other Bomberman games, you already know how to play Bombermaaan." );

_add( "Up to 5 players on the same computer", "You can play against four human players if you are using joysticks or USB keyboards." );

_add( "Various levels of computer AI", "No additional text available." );

_add( "Kick-Punch-Throw items mania", "Your bomber can kick bombs (bomb is moving through the arena), punch bombs (bomb flies three blocks far), or throw bombs (pick up a bomb and throw it in the direction your bomber is currently looking)." );

//_add( "Customizable arena levels with a level editor", "Rommel Santor's <a href=\"http://downloads.sourceforge.net/bombermaaan/LevelEditor_1.02.zip\">level editor</a> can be downloaded as a separate package from the <a href=\"https://sourceforge.net/project/showfiles.php?group_id=81520&amp;package_id=83430&amp;release_id=234468\">download section on SourceForge.net</a>." );

_add( "Keyboard and joystick support", "No additional text available." );

_add( "Full screen and windowed display modes", "<kbd>F1</kbd>: full screen mode, 320x240 pixels<br /><kbd>F2</kbd>: full screen mode, 512x384 pixels<br /><kbd>F3</kbd>: full screen mode, 640x480 pixels<br /><kbd>F4</kbd>: windowed mode, 240x234 pixels in 16-pixels version, 480x442 pixels in 32-pixels version<br />The <kbd>F1</kbd> and <kbd>F2</kbd> keys are only available in the 32-pixels version." );

_add( "Various power-ups", "At the beginning of a match your bomber can drop only one bomb with a flame size of two blocks. This can be changed during the game by collecting power-up icons. These icons are hidden in the soft walls. Destroy the soft walls to look for the power-ups. These items can improve your bomber skills: additional bomb, increase flame size, ability to kick bombs, increase speed, ability to throw bombs, ability to punch bombs.", 
"<img src=\"images/arena_item_0-bomb.png\" alt=\"Bomb item\" /> Additional bomb: you can put one more bomb<br />
<img src=\"images/arena_item_1-flame.png\" alt=\"Flame item\" /> Increase flame size: the flame of your bombs grows one block each time you pick up this item<br />
<img src=\"images/arena_item_3-speed.png\" alt=\"Skater item\" /> Increase speed: you can walk a little faster each time you pick up this item<br />
<img src=\"images/arena_item_2-kick.png\" alt=\"Bomb &amp; foot item\" /> Kick bombs: bombs start moving away when you walk towards them<br />
<img src=\"images/arena_item_4-skull.png\" alt=\"Skull item\" /> Contaminations: see the contaminations section below<br />
<img src=\"images/arena_item_5-throw.png\" alt=\"Hand item (blue)\" /> Throw bombs: pick up bombs, carry them and throw them away<br />
<img src=\"images/arena_item_6-punch.png\" alt=\"Glove item\" /> Punch bombs: punch a bomb that is lying in front of you<br />
<img src=\"images/arena_item_7-remote.png\" alt=\"Remote item\" /> Remote fuse: remote fuse a bomb<br />" );

_add( "Various contaminations after skull item is taken", "The skull item influences the health and skills of your bomber. It can result to: inverted controls, bombs with small flames, having no bombs to place, always dropping bombs, slow speed, fast speed, always move, bombs ticking shorter, bombs ticking longer than usual, invisibility of a bomber, and to a flameproof bomber." );

_add( "Item amount in walls as well as bomber skills on startup can be set on a per-level basis", "For this feature a new layout for level files has been designed to configure the amount of items and the skills at the start of a match." );

?>

</div>

<?php
    /*** Thibaut's list on his website:
		    *  Classic Bomberman game
		    * Original gameplay and fun with minor adjustments
		    * Up to 5 players on the same computer
		    * Various levels of computer AI
		    * Single and team battle
		    * Kick-Punch-Throw items mania
		    * Kangaroos to ride
		    * Possibility to throw bombs in the arena after you died
		    * Various arena closures
		    * Customizable arena levels with a level editor
		    * Many options to customize the game rules
		    * Keyboard and joystick support
		    * Full screen and windowed display modes
		    * Network support (will come after all of the above) 
	***/
?>

<p>
More list items may follow. A list of planned features can be seen in the <a href="./todo.php">To-Do list</a>.
</p>

</div>

<?php

$webpage->tail();

?>
