class rgb24:
    '24bit pixel color value class for LEDMatrix.'

    def __init__(self, r=[0, 0, 0], g=None, b=None):
        if(isinstance(r, list) and g is None and b is None):
            self.value = r  # Red, Green, Blue list
        elif(isinstance(r, (int,long)) and isinstance(g, (int,long)) and isinstance(b, (int,long))):
            self.value = [r, g, b] # build Red, Green, Blue list from integers
        else:
            self.value = [0, 0, 0] # invalid arguments, failsafe initialize

    def __str__(self):
        return 'rgb24[R:%d, G:%d, B:%d]' % (self.value[0], self.value[1], self.value[2])

    def __eq__(self, other):
        assert isinstance(other, rgb24)
        return self.value == other.value

    def __add__(self, other):
        assert isinstance(other, rgb24)
        return self.value + other.value

    def __sub__(self, other):
        assert isinstance(other, rgb24)
        return self.value - other.value

    def setValue(self, value):
        assert isinstance(value, list)
        if(len(value) == 3):
            self.value = value

    def setRGB(self, r, g, b):
        self.value = [r, g, b]

    def setRed(self, r):
        self.value[0] = r

    def setGreen(self, g):
        self.value[1] = r

    def setBlue(self, b):
        self.value[2] = r

    def getRGB(self):
        return self.value

    def getRed(self):
        return self.value[0]

    def getGreen(self):
        return self.value[1]

    def getBlue(self):
        return self.value[2]


