# Import from PDF
- Open helveticons.pdf in Photoshop, select the icons you like or all of them, leave the default options alone and all of the vectors will be opened in their own documents.
- Resize them down to a 512 x 512 square image with the glyph vector's larger dimension fully extended at 512 pixels. May have to do a combination of manual and automated approaches here.
- Save the PSDs as helvetiblox-XXX.psd, but keep them open in Photoshop.

# Exporting the Images 
- Copy layer style from "Helveticons-FX-State"
- Run the actions list from steps 1 to 3 using the Automation > Batch Processing command on the open Files.
- They'll end up on Desktop.
- Make an extra copy of the exported set. Using the mass rename tool, make them say "key-helvetiblox-XXX.png".

# Updating the SpriteSheet
- Locate and open the Zwoptex files helvetiblox-blocks.zwd and keylabels-helvetiblox.zwd
- Update the contents of the spritesheets. Relayout if needed, then republish.
- x1, x2, and x4 folders near the zwoptex files will produce updated ccz and plist files.
- Overwrite the contents of Typing Genius/resources/assets/spriteatlases with these files, with the following mappings: 
	- phone: x1
	- phonex2, tablet: x2
	- tabletx2: x4

