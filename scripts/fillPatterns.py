# need ledmatrix module, provided by XPMaster (C++ side), and rgb24 for color
# class, provided in ledmatrixext/ directory.
import ledmatrix
from ledmatrixext.rgb24 import rgb24

# Also all standard Python libraries and system installed are available
import random

# stdout works
print('Now running fill patterns example')

# create matrix object, which internally only one matrix object ever exists
# so it is OK to create multiple ledmatrix.matrix instances from Python side.
matrix = ledmatrix.matrix()

# make sure display is in manual mode so that we directly control what graphics
# are displayed.
matrix.setMode(ledmatrix.DISPLAY_Manual)
matrix.gifStop()
matrix.setBrightness(255)

# clear the display (black fill) and swap buffers (will not wait for ack from display)
matrix.fillScreen(rgb24(0, 0, 0))
matrix.swapBuffers()

# disable all text scrollers
for i in range(4):
    matrix.getScroller(i).stopScrollText()


# prepare to draw with random functions
matrix.setFont(ledmatrix.FONT_gohufont11b)

w = h = 0
if(matrix.width == matrix.height):
    w = h = matrix.width / 2
elif(matrix.width > matrix.height):
    w = h = matrix.height
else:
    w = h = matrix.width


# main loop
while True:    
    # randomize variables
    n      = random.randrange(13)
    x      = random.randrange(matrix.width  -1)
    y      = random.randrange(matrix.height -1)
    r      = w
    rEdge  = 4
    color0 = rgb24(random.randrange(255),
                   random.randrange(255),
                   random.randrange(255))
    color1 = rgb24(random.randrange(255),
                   random.randrange(255),
                   random.randrange(255))
    
    # use a random draw function
    if n == 0:
        x1 = random.randrange(matrix.width  -1)
        y1 = random.randrange(matrix.width  -1)
        matrix.drawLine(x, y, x1, y1, color0)
    elif n == 1:
        matrix.drawCircle(x, y, r, color0)
    elif n == 2:
        matrix.fillCircle(x, y, r, color1, color0)
    elif n == 3:
        matrix.drawEllipse(x, y, w, h, color0)
    elif n == 4:
        matrix.drawTriangle(x, y, x + w, y + h, x + w, y, color0)
    elif n == 5:
        matrix.fillTriangle(x, y, x + w, y + h, x + w, y, color1, color0)
    elif n == 6:
        matrix.drawRectangle(x, y, x + w, y + h, color0)
    elif n == 7:
        matrix.fillRectangle(x, y, x + w, y + h, color1, color0)
    elif n == 8:
        matrix.drawRoundRectangle(x, y, x + w, y + h, rEdge, color0)
    elif n == 9:
        matrix.fillRoundRectangle(x, y, x + w, y + h, rEdge, color1, color0)
    elif n == 10:
        matrix.drawChar(x, y, color0, '?')
    elif n == 11:
        matrix.drawString(x, y, color0, "Hello!")
    else:
        matrix.drawString(x, y, color1, color0, "World!")
    
    # swap buffers and vsync
    matrix.waitForVSync()
