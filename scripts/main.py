# need ledmatrix module, provided by XPMaster (C++ side), and rgb24 for color
# class, provided in ledmatrixext/ directory.
import ledmatrix
from ledmatrixext.rgb24 import rgb24

# Also all standard Python libraries and system installed are available
from datetime import datetime, date, time
import random

# stdout works
print('Showing off python script demo')

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

# acquire some text scrollers, in this case we're assuming there's at least two
scroller0 = matrix.getScroller(0)
scroller1 = matrix.getScroller(1)

# initialize them to known configurations
for scroller in [scroller0, scroller1]:
    scroller.setScrollFont(ledmatrix.FONT_font6x10)
    scroller.setScrollMode(ledmatrix.SCROLL_wrapForward)
    scroller.setScrollStartOffsetFromLeft(0)
    scroller.setScrollSpeed(255)
    scroller.stopScrollText()


# declare some helper functions

def sleep(secs):
    # because the current python glue implementation is still single-threaded
    # we have to make sure to keep servicing the RPC/USB routines to prevent
    # the teensy side from stalling out with a full TX FIFO. The only functions
    # that will implicitly service the RPC queue for us is matrix.safeSleep and
    # matrix.waitForVSync, so we have to call either of those every so often.
    if(matrix.safeSleep(secs) != True):
        # abort on RPC error
        exit(-1);


def drawFlushAndSleep(secs):
    # wait for swapBuffers acknowledgment from the display/teensy before sleeping
    matrix.waitForVSync()
    sleep(secs)


def centerText(font, color, text):
    # center align the text
    dims = matrix.getFontStringDims(font, text)
    x = (matrix.width  / 2) - (dims[0] / 2)
    y = (matrix.height / 2) - (dims[1] / 2)
    
    matrix.setFont(font)
    matrix.drawString(x, y, color, text)


def colorWheel(WheelPos):
    if WheelPos < 85:
        color_red   = WheelPos * 3
        color_green = 255 - WheelPos * 3
        color_blue  = 0
        return rgb24(color_red, color_green, color_blue)
    elif WheelPos < 170:
        WheelPos   -=  85
        color_red   = 255 - WheelPos * 3
        color_green = 0
        color_blue  = WheelPos * 3
        return rgb24(color_red, color_green, color_blue)
    else:
        WheelPos   -= 170
        color_red   = 0
        color_green = WheelPos * 3
        color_blue  = 255 - WheelPos * 3
        return rgb24(color_red, color_green, color_blue)


# main loop
while True:
    # display date and time for a few seconds
    secs = 4
    while secs > 0:
        # alternate between date and time
        ts = datetime.now().strftime("%m/%d/%Y" if(secs % 2 == 0) else "%I:%M:%S %p")
        
        # draw centered text
        matrix.fillScreen(rgb24(0, 0, 0))
        centerText(ledmatrix.FONT_font8x13, rgb24(255, 255, 0), ts)
        
        # display it for a second
        drawFlushAndSleep(1)
        secs -= 1
    
    # scroll boundary constrained text, top line
    scroller0.setScrollColor(rgb24(255,255,255))
    scroller0.setScrollFont(ledmatrix.FONT_gohufont11b)
    scroller0.setScrollBoundary(matrix.width - (matrix.width / 2), 0, matrix.width -1, matrix.height -1)
    scroller0.scrollText("This is scrolling text is constrained.", -1)
    
    # scroll unconstrained text, bottom line
    scroller1.setScrollColor(rgb24(192,192,192))
    scroller1.setScrollFont(ledmatrix.FONT_gohufont11)
    scroller1.setScrollBoundary(0, 0, matrix.width -1, matrix.height -1)
    scroller1.setScrollOffsetFromTop(matrix.height - matrix.getFontStringDims(ledmatrix.FONT_gohufont11, "This")[1])
    scroller1.scrollText("And this is free run scrolling text!", 2)
    
    # clear background from previous drawing
    matrix.fillScreen(rgb24(0, 0, 0))
    
    # draw color wheel while scrolling text
    wheelPos = 128
    while (scroller1.getScrollStatus() > 0):
        wheelPos = (wheelPos + 6) % 255
        cw = wheelPos
        for x in range(matrix.width / 2):
            cw = (cw + 4) % 255
            color = colorWheel(cw)
            matrix.drawFastVLine(x, 0, matrix.height -1, color);
        
        scroller0.setScrollColor(color)

        # swap buffers and vsync
        matrix.waitForVSync()
    
    # stop unlimited run text scroller
    scroller0.stopScrollText()
    
    # clear background from previous drawing
    matrix.fillScreen(rgb24(0, 0, 0))
    
    # prepare to use random draw functions
    matrix.setFont(ledmatrix.FONT_gohufont11b)
    
    w = h = 0
    if(matrix.width == matrix.height):
        w = h = matrix.width / 2
    elif(matrix.width > matrix.height):
        w = h = matrix.height
    else:
        w = h = matrix.width
    
    # draw with random functions for a limited number of times
    draws = 200
    while draws > 0:
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
        draws -= 1

    # continue


# the end
