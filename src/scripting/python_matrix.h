#include "rpc.h"
#include "matrix.h"

/*
 * SmartMatrix wrapper library exposure to Python environment.
 * 
 * Note: rgb24 class is purely python script implemented for simplicity.
 */


//=============================================================================
// Graphics and Python related utility functions
//=============================================================================

static bool parseRGB24(rgb24 &dest, PyObject *src)
{
	PyObject *obj = PyObject_GetAttrString(src, "value");
	if(!obj)
		return false;

	size_t	i;
	bool	result = false;

	PyObject *pvalues[3] = {NULL, NULL, NULL};
	uint8_t   values[3];

	for(i=0; i<3; i++)
	{
		if( !(pvalues[i] = PyObject_GetItem(obj, PyInt_FromSize_t(i))) ||
			!PyInt_CheckExact(pvalues[i]))
			goto Abort;

		values[i] = (uint8_t)PyInt_AS_LONG(pvalues[i]);
	}

	dest.red	= values[0];
	dest.green	= values[1];
	dest.blue	= values[2];
	result		= true;

Abort:
	for(i=0; i<3; i++)
	{
		if(pvalues[i])
			Py_DECREF(pvalues[i]);
	}

	Py_DECREF(obj);
	return result;
}



//=============================================================================
// Text Scroller hooks
//=============================================================================

struct tTextScrollerObject
{
	PyObject_HEAD
	TextScroller	*scroller;
};


static PyObject *TextScroller_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
	tTextScrollerObject *self = NULL;

	// allocate ourselves
	if((self = (tTextScrollerObject *)type->tp_alloc(type, 0)))
	{
		Py_INCREF(self);
	}

	return (PyObject *)self;
}

static int TextScroller_init(tTextScrollerObject *self, PyObject *args, PyObject *kwds)
{
	static const char *kwlist[] = {"scroller", NULL};
	int scroller;


	if(!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char **)kwlist, 
										&scroller)
		|| (scroller < 0) || (scroller >= MATRIX_SCROLLERS))
		return -1;

	// initialization success
	self->scroller = &::matrix.getScroller(scroller);
	return 0;
}

static void TextScroller_dealloc(tTextScrollerObject *self)
{
	// deallocate ourselves
	self->ob_type->tp_free((PyObject *)self);
}

static PyObject* TextScroller_getter(tTextScrollerObject *self, void *closure)
{
	switch((ssize_t)closure)
	{
		case 0:	return Py_BuildValue("i", self->scroller->lineCount);
		case 1:	return Py_BuildValue("i", self->scroller->charCount);
		default:
			break;
	}

	return NULL;
}


//-----------------------------------------------------------------------------
// scroll text
//-----------------------------------------------------------------------------

static PyObject *TextScroller_scrollText(tTextScrollerObject *self, PyObject *args)
{
	const char	*text;
	int			num;


	if(!PyArg_ParseTuple(args, "si:scrollText", &text, &num))
		return NULL;

	self->scroller->scrollText(text, num);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollMode(tTextScrollerObject *self, PyObject *args)
{
	int			mode;


	if(!PyArg_ParseTuple(args, "i:setScrollMode", &mode))
		return NULL;

	self->scroller->setScrollMode((ScrollMode)mode);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollSpeed(tTextScrollerObject *self, PyObject *args)
{
	int			speed;


	if(!PyArg_ParseTuple(args, "i:setScrollSpeed", &speed))
		return NULL;

	self->scroller->setScrollSpeed((uint8_t)speed);
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollFont(tTextScrollerObject *self, PyObject *args)
{
	int			font;

	
	if(!PyArg_ParseTuple(args, "i:setScrollFont", &font))
		return NULL;

	self->scroller->setScrollFont((fontChoices)font);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollColor(tTextScrollerObject *self, PyObject *args)
{
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "O:setScrollColor", &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->scroller->setScrollColor(color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollOffsetFromTop(tTextScrollerObject *self, PyObject *args)
{
	int			offset;


	if(!PyArg_ParseTuple(args, "i:setScrollOffsetFromTop", &offset))
		return NULL;

	self->scroller->setScrollOffsetFromTop(offset);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollStartOffsetFromLeft(tTextScrollerObject *self, PyObject *args)
{
	int			offset;


	if(!PyArg_ParseTuple(args, "i:setScrollStartOffsetFromLeft", &offset))
		return NULL;

	self->scroller->setScrollStartOffsetFromLeft(offset);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_stopScrollText(tTextScrollerObject *self)
{
	self->scroller->stopScrollText();

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_setScrollBoundary(tTextScrollerObject *self, PyObject *args)
{
	int			x0, y0, x1, y1;


	if(!PyArg_ParseTuple(args, "iiii:setScrollBoundary", &x0, &y0, &x1, &y1))
		return NULL;

	self->scroller->setScrollBoundary(x0, y0, x1, y1);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *TextScroller_appendText(tTextScrollerObject *self, PyObject *args)
{
	const char	*text;


	if(!PyArg_ParseTuple(args, "s:appendText", &text))
		return NULL;

	self->scroller->appendText(text);

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject *TextScroller_getScrollStatus(tTextScrollerObject *self)
{
	return Py_BuildValue("i", self->scroller->getScrollStatus());
}


//-----------------------------------------------------------------------------


static PyMethodDef TextScroller_methods[] =
{
	// scroll text
	{ "scrollText",						(PyCFunction)TextScroller_scrollText,					METH_VARARGS, "Set and start the scrolling text." },
	{ "setScrollMode",					(PyCFunction)TextScroller_setScrollMode,				METH_VARARGS, "Start, pause, or stop the scrolling text." },
	{ "setScrollSpeed",					(PyCFunction)TextScroller_setScrollSpeed,				METH_VARARGS, "Change scrolling speed." },
	{ "setScrollFont",					(PyCFunction)TextScroller_setScrollFont,				METH_VARARGS, "Change text font." },
	{ "setScrollColor",					(PyCFunction)TextScroller_setScrollColor,				METH_VARARGS, "Change text color." },
	{ "setScrollOffsetFromTop",			(PyCFunction)TextScroller_setScrollOffsetFromTop,		METH_VARARGS, "Set vertical offset position." },
	{ "setScrollStartOffsetFromLeft",	(PyCFunction)TextScroller_setScrollStartOffsetFromLeft,	METH_VARARGS, "Set starting horizontal offset position." },
	{ "stopScrollText",					(PyCFunction)TextScroller_stopScrollText,				METH_NOARGS,  "Stop the scrolling text." },
	{ "setScrollBoundary",				(PyCFunction)TextScroller_setScrollBoundary,			METH_VARARGS, "Set rectangular scroll boundary." },
	{ "appendText",						(PyCFunction)TextScroller_appendText,					METH_VARARGS, "Add a text line to continuous text scroll." },

	{ "getScrollStatus",				(PyCFunction)TextScroller_getScrollStatus,				METH_NOARGS,  "Get current scroller status." },

	{ NULL, NULL, 0, NULL }
};

static PyGetSetDef TextScroller_getset[] =
{
	{ const_cast<char *>("lineCount"),	(getter)TextScroller_getter, NULL, const_cast<char *>("Number of lines in text scroller"), (void *)0 },
	{ const_cast<char *>("charCount"),	(getter)TextScroller_getter, NULL, const_cast<char *>("Number of characters in text scroller"), (void *)1 },

	{ NULL, 0, 0, 0, NULL }
};

static PyTypeObject tTextScrollerObjectType =
{
	PyObject_HEAD_INIT(NULL)
	0,                              // ob_size (not used, always set to 0)
	"ledmatrix.textscroller",       // tp_name (module name, object name)
	sizeof(tTextScrollerObject),    // tp_basicsize
	0,                              // tp_itemsize
	(destructor)TextScroller_dealloc, // tp_dealloc
	0,                              // tp_print
	0,                              // tp_getattr
	0,                              // tp_setattr
	0,                              // tp_compare
	0,                              // tp_repr
	0,                              // tp_as_number
	0,                              // tp_as_sequence
	0,                              // tp_as_mapping
	0,                              // tp_hash
	0,                              // tp_call
	0,                              // tp_str
	0,                              // tp_getattro
	0,                              // tp_setattro
	0,                              // tp_as_buffer
	Py_TPFLAGS_DEFAULT,             // tp_flags
	0,                              // tp_doc
	0,                              // tp_traverse
	0,                              // tp_clear
	0,                              // tp_richcompare
	0,                              // tp_weaklistoffset
	0,                              // tp_iter
	0,                              // tp_iternext
	TextScroller_methods,           // tp_methods
	0,                              // tp_members
	TextScroller_getset,            // tp_getset
	0,                              // tp_base
	0,                              // tp_dict
	0,                              // tp_descr_get
	0,                              // tp_descr_set
	0,                              // tp_dictoffset
	(initproc)TextScroller_init,    // tp_init
	0,                              // tp_alloc
	TextScroller_new,               // tp_new
	0,                              // tp_free
};



//=============================================================================
// Matrix hooks
//=============================================================================

struct tMatrixObject
{
	PyObject_HEAD
	LEDMatrix		*matrix;
	PyObject		*scrollers[MATRIX_SCROLLERS];
	uint16_t		width, height;
};

static PyObject *Matrix_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	tMatrixObject *self = NULL;

	// allocate ourselves
	if((self = (tMatrixObject *)type->tp_alloc(type, 0)))
	{
		self->matrix = &::matrix;
		self->width  = ::matrix.width;
		self->height = ::matrix.height;

		for(int i=0; i<MATRIX_SCROLLERS; i++)
		{
			PyObject *argList = Py_BuildValue("(i)", i);
			self->scrollers[i] = PyObject_CallObject((PyObject *)&tTextScrollerObjectType, argList);
			Py_DECREF(argList);
		}
		
		Py_INCREF(self);
	}

	return (PyObject *)self;
}

static int Matrix_init(tMatrixObject *self, PyObject *args)
{
	return 0; // do nothing
}

static void Matrix_dealloc(tMatrixObject *self)
{
	// deallocate ourselves
	self->ob_type->tp_free((PyObject *)self);
}


//-----------------------------------------------------------------------------
// utility
//-----------------------------------------------------------------------------

static PyObject *Matrix_getScroller(tMatrixObject *self, PyObject *args)
{
	int			scroller;


	if(!PyArg_ParseTuple(args, "i:getScroller", &scroller) ||
		(scroller < 0) || (scroller >= MATRIX_SCROLLERS))
		return NULL;

	return self->scrollers[scroller];
}


//-----------------------------------------------------------------------------
// display control
//-----------------------------------------------------------------------------

static PyObject *Matrix_setBrightness(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	uint8_t		fg, bg;

	
	if(argc == 1)
	{
		if(!PyArg_ParseTuple(args, "B:setBrightness", &fg))
			return NULL;
		bg = fg;
	} else
	if(argc == 2)
	{
		if(!PyArg_ParseTuple(args, "BB:setBrightness", &fg, &bg))
			return NULL;
	} else
		goto Abort;

	self->matrix->setBrightness(fg, bg);

Abort:
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_swapBuffers(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);

	
	if(argc == 1)
	{
		int copy;
		if(!PyArg_ParseTuple(args, "i:swapBuffers", &copy))
			return NULL;
		
		self->matrix->swapBuffers((bool)copy);
	} else
	{
		self->matrix->swapBuffers();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_waitForVSync(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	size_t		times;
	int			copy;

	
	if(argc == 1)
	{
		if(!PyArg_ParseTuple(args, "k:waitForVSync", &times))
			return NULL;

		self->matrix->waitForVSync(times);
	} else
	if(argc == 2)
	{
		if(!PyArg_ParseTuple(args, "ki:waitForVSync", &times, &copy))
			return NULL;
		self->matrix->waitForVSync(times, (bool)copy);
	} else
	{
		self->matrix->waitForVSync();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_safeSleep(tMatrixObject *self, PyObject *args)
{
	float		secs;


	if(!PyArg_ParseTuple(args, "f:safeSleep", &secs))
		return NULL;

	bool result = self->matrix->safeSleep((size_t)abs(secs * 1000.0f + 0.5f));

	if(gm_Exit)
		PyErr_SetInterrupt();

	return Py_BuildValue("N", PyBool_FromLong(result));
}

static PyObject *Matrix_setMode(tMatrixObject *self, PyObject *args)
{
	int			mode;

	
	if(!PyArg_ParseTuple(args, "i:setMode", &mode))
		return NULL;

	self->matrix->setMode((eDisplayState)mode);

	Py_INCREF(Py_None);
	return Py_None;
}


//-----------------------------------------------------------------------------
// drawing functions
//-----------------------------------------------------------------------------

static PyObject *Matrix_drawPixel(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y;
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "hhO:drawPixel", &x, &y, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawPixel(x, y, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawLine(tMatrixObject *self, PyObject *args)
{
	int16_t		x0, y0, x1, y1;
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "hhhhO:drawLine", &x0, &y0, &x1, &y1, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawLine(x0, y0, x1, y1, color);
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawFastVLine(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y0, y1;
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "hhhO:drawFastVLine", &x, &y0, &y1, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawFastVLine(x, y0, y1, color);
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawFastHLine(tMatrixObject *self, PyObject *args)
{
	int16_t		x0, x1, y;
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "hhhO:drawFastHLine", &x0, &x1, &y, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawFastHLine(x0, x1, y, color);
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawCircle(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y, radius;
	PyObject	*rgb;


	if(!PyArg_ParseTuple(args, "hhhO:drawCircle", &x, &y, &radius, &rgb))
		return NULL;
	
	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawCircle(x, y, radius, color);
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_fillCircle(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		x, y, radius;
	PyObject	*rgb1, *rgb2;


	if(argc < 5)
	{
		if(!PyArg_ParseTuple(args, "hhhO:fillCircle", &x, &y, &radius, &rgb1))
			return NULL;
		
		rgb24 color;
		if(parseRGB24(color, rgb1))
			self->matrix->fillCircle(x, y, radius, color);
	} else
	{
		if(!PyArg_ParseTuple(args, "hhhOO:fillCircle", &x, &y, &radius, &rgb1, &rgb2))
			return NULL;

		rgb24 colorFG, colorBG;
		if(parseRGB24(colorFG, rgb1) && parseRGB24(colorBG, rgb2))
			self->matrix->fillCircle(x, y, radius, colorFG, colorBG);
	}
	
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawEllipse(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y, radiusX, radiusY;
	PyObject	*rgb;


	if(!PyArg_ParseTuple(args, "hhhhO:drawEllipse", &x, &y, &radiusX, &radiusY, &rgb))
		return NULL;
	
	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawEllipse(x, y, radiusX, radiusY, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawTriangle(tMatrixObject *self, PyObject *args)
{
	int16_t		x0, y0, x1, y1, x2, y2;
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "hhhhhhO:drawTriangle", &x0, &y0, &x1, &y1, &x2, &y2, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawTriangle(x0, y0, x1, y1, x2, y2, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_fillTriangle(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		x0, y0, x1, y1, x2, y2;
	PyObject	*rgb1, *rgb2;

	
	if(argc < 8)
	{
		if(!PyArg_ParseTuple(args, "hhhhhhO:fillTriangle", &x0, &y0, &x1, &y1, &x2, &y2, &rgb1))
			return NULL;

		rgb24 color;
		if(parseRGB24(color, rgb1))
			self->matrix->fillTriangle(x0, y0, x1, y1, x2, y2, color);
	} else
	{
		if(!PyArg_ParseTuple(args, "hhhhhhOO:fillTriangle", &x0, &y0, &x1, &y1, &x2, &y2, &rgb1, &rgb2))
			return NULL;

		rgb24 colorFG, colorBG;
		if(parseRGB24(colorFG, rgb1) && parseRGB24(colorBG, rgb2))
			self->matrix->fillTriangle(x0, y0, x1, y1, x2, y2, colorFG, colorBG);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawRectangle(tMatrixObject *self, PyObject *args)
{
	int16_t		x0, y0, x1, y1;
	PyObject	*rgb;


	if(!PyArg_ParseTuple(args, "hhhhO:drawRectangle", &x0, &y0, &x1, &y1, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawRectangle(x0, y0, x1, y1, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_fillRectangle(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		x0, y0, x1, y1;
	PyObject	*rgb1, *rgb2;

	
	if(argc < 6)
	{
		if(!PyArg_ParseTuple(args, "hhhhO:fillRectangle", &x0, &y0, &x1, &y1, &rgb1))
			return NULL;

		rgb24 color;
		if(parseRGB24(color, rgb1))
			self->matrix->fillRectangle(x0, y0, x1, y1, color);
	} else
	{
		if(!PyArg_ParseTuple(args, "hhhhOO:fillRectangle", &x0, &y0, &x1, &y1, &rgb1, &rgb2))
			return NULL;

		rgb24 colorFG, colorBG;
		if(parseRGB24(colorFG, rgb1) && parseRGB24(colorBG, rgb2))
			self->matrix->fillRectangle(x0, y0, x1, y1, colorFG, colorBG);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawRoundRectangle(tMatrixObject *self, PyObject *args)
{
	int16_t		x0, y0, x1, y1, radius;
	PyObject	*rgb;


	if(!PyArg_ParseTuple(args, "hhhhhO:drawRoundRectangle", &x0, &y0, &x1, &y1, &radius, &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawRoundRectangle(x0, y0, x1, y1, radius, color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_fillRoundRectangle(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		x0, y0, x1, y1, radius;
	PyObject	*rgb1, *rgb2;

	
	if(argc < 7)
	{
		if(!PyArg_ParseTuple(args, "hhhhO:fillRoundRectangle", &x0, &y0, &x1, &y1, &radius, &rgb1))
			return NULL;

		rgb24 color;
		if(parseRGB24(color, rgb1))
			self->matrix->fillRoundRectangle(x0, y0, x1, y1, radius, color);
	} else
	{
		if(!PyArg_ParseTuple(args, "hhhhhOO:fillRoundRectangle", &x0, &y0, &x1, &y1, &radius, &rgb1, &rgb2))
			return NULL;

		rgb24 colorFG, colorBG;
		if(parseRGB24(colorFG, rgb1) && parseRGB24(colorBG, rgb2))
			self->matrix->fillRoundRectangle(x0, y0, x1, y1, radius, colorFG, colorBG);
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_fillScreen(tMatrixObject *self, PyObject *args)
{
	PyObject	*rgb;

	
	if(!PyArg_ParseTuple(args, "O:fillScreen", &rgb))
		return NULL;

	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->fillScreen(color);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawChar(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y;
	PyObject	*rgb;
	char		character;

	
	if(!PyArg_ParseTuple(args, "hhOc:drawChar", &x, &y, &rgb, &character))
		return NULL;
	
	rgb24 color;
	if(parseRGB24(color, rgb))
		self->matrix->drawChar(x, y, color, character);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_drawString(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		x, y;
	PyObject	*rgb1, *rgb2;
	const char	*text;


	if(argc < 5)
	{
		if(!PyArg_ParseTuple(args, "hhOs:drawString", &x, &y, &rgb1, &text))
			return NULL;
		
		rgb24 color;
		if(parseRGB24(color, rgb1))
			self->matrix->drawString(x, y, color, text);
	} else
	{
		if(!PyArg_ParseTuple(args, "hhOOs:drawString", &x, &y, &rgb1, &rgb2, &text))
			return NULL;

		rgb24 colorFG, colorBG;
		if(parseRGB24(colorFG, rgb1) && parseRGB24(colorBG, rgb2))
			self->matrix->drawString(x, y, colorFG, colorBG, text);
	}

	Py_INCREF(Py_None);
	return Py_None;
}


//-----------------------------------------------------------------------------
// fonts
//-----------------------------------------------------------------------------

static PyObject *Matrix_setFont(tMatrixObject *self, PyObject *args)
{
	int			font;

	
	if(!PyArg_ParseTuple(args, "i:setFont", &font))
		return NULL;

	self->matrix->setFont((fontChoices)font);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_getFontStringDims(tMatrixObject *self, PyObject *args)
{
	int			font;
	const char	*text;

	
	if(!PyArg_ParseTuple(args, "is:getFontStringDims", &font, &text))
		return NULL;

	Point2I dims = self->matrix->getFontStringDims((fontChoices)font, text);

	return Py_BuildValue("[i,i]", dims.x, dims.y);
}

static PyObject *Matrix_getFontCharsInRect(tMatrixObject *self, PyObject *args)
{
	int			font, w, h;

	
	if(!PyArg_ParseTuple(args, "iii:getFontCharsInRect", &font, &w, &h))
		return NULL;

	Point2I dims = self->matrix->getFontCharsInRect((fontChoices)font, Point2I(w, h));

	return Py_BuildValue("[i,i]", dims.x, dims.y);
}


//-----------------------------------------------------------------------------
// GIF player control
//-----------------------------------------------------------------------------

static PyObject *Matrix_gifLoad(tMatrixObject *self, PyObject *args)
{
	const char	*filepath;

	
	if(!PyArg_ParseTuple(args, "s:gifLoad", &filepath))
		return NULL;
	
	self->matrix->gifLoad(filepath);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_gifPlay(tMatrixObject *self, PyObject *args)
{
	Py_ssize_t	argc = PyTuple_Size(args);
	int16_t		interval;


	if(argc)
	{
		if(!PyArg_ParseTuple(args, "h:gifPlay", &interval))
			return NULL;
		
		self->matrix->gifPlay((uint16_t)interval);
	} else
	{
		self->matrix->gifPlay();
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_gifStop(tMatrixObject *self)
{
	self->matrix->gifStop();

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *Matrix_gifPosition(tMatrixObject *self, PyObject *args)
{
	int16_t		x, y;


	if(!PyArg_ParseTuple(args, "hh:gifPosition", &x, &y))
		return NULL;

	self->matrix->gifPosition(x, y);

	Py_INCREF(Py_None);
	return Py_None;
}



//-----------------------------------------------------------------------------


static PyMethodDef Matrix_methods[] =
{
	// utility
	{ "getScroller",		(PyCFunction)Matrix_getScroller,		METH_VARARGS, "Retrieve specified text scroller object." },
	
	// display control
	{ "setBrightness",		(PyCFunction)Matrix_setBrightness,		METH_VARARGS, "Set display brightness level for foreground and background graphics layers." },
	{ "swapBuffers",		(PyCFunction)Matrix_swapBuffers,		METH_VARARGS, "Swap both drawing and displayed framebuffers." },
	{ "waitForVSync",		(PyCFunction)Matrix_waitForVSync,		METH_VARARGS, "Block call until display raster vertical retrace." },
	{ "safeSleep",			(PyCFunction)Matrix_safeSleep,			METH_VARARGS, "Safely delay code execution and still service matrix operations." },
	{ "setMode",			(PyCFunction)Matrix_setMode,			METH_VARARGS, "Set display operation mode state." },

	// drawing functions
	{ "drawPixel",			(PyCFunction)Matrix_drawPixel,			METH_VARARGS, "Set a pixel value." },
	{ "drawLine",			(PyCFunction)Matrix_drawLine,			METH_VARARGS, "Draw a line." },
	{ "drawFastVLine",		(PyCFunction)Matrix_drawFastVLine,		METH_VARARGS, "Draw a fast vertical line." },
	{ "drawFastHLine",		(PyCFunction)Matrix_drawFastHLine,		METH_VARARGS, "Draw a fast horizontal line." },
	{ "drawCircle",			(PyCFunction)Matrix_drawCircle,			METH_VARARGS, "Draw a circle." },
	{ "fillCircle",			(PyCFunction)Matrix_fillCircle,			METH_VARARGS, "Draw a filled circle." },
	{ "drawEllipse",		(PyCFunction)Matrix_drawEllipse,		METH_VARARGS, "Draw an ellipse." },
	{ "drawTriangle",		(PyCFunction)Matrix_drawTriangle,		METH_VARARGS, "Draw a triangle." },
	{ "fillTriangle",		(PyCFunction)Matrix_fillTriangle,		METH_VARARGS, "Draw a filled triangle." },
	{ "drawRectangle",		(PyCFunction)Matrix_drawRectangle,		METH_VARARGS, "Draw a rectangle." },
	{ "fillRectangle",		(PyCFunction)Matrix_fillRectangle,		METH_VARARGS, "Draw a filled rectangle." },
	{ "drawRoundRectangle",	(PyCFunction)Matrix_drawRoundRectangle,	METH_VARARGS, "Draw a rounded edged rectangle." },
	{ "fillRoundRectangle",	(PyCFunction)Matrix_fillRoundRectangle,	METH_VARARGS, "Draw a rounded edged filled rectangle." },
	{ "fillScreen",			(PyCFunction)Matrix_fillScreen,			METH_VARARGS, "Fill entire screen." },
	{ "drawChar",			(PyCFunction)Matrix_drawChar,			METH_VARARGS, "Draw a single character." },
	{ "drawString",			(PyCFunction)Matrix_drawString,			METH_VARARGS, "Draw a string of characters." },

	// fonts
	{ "setFont",			(PyCFunction)Matrix_setFont,			METH_VARARGS, "Change font to draw text strings with." },
	{ "getFontStringDims",	(PyCFunction)Matrix_getFontStringDims,	METH_VARARGS, "Get pixel dimensions of the passed font and text string." },
	{ "getFontCharsInRect",	(PyCFunction)Matrix_getFontCharsInRect,	METH_VARARGS, "Get count of font's characters and lines that'll be visible in the passed pixel dimensions." },

	// GIF player control
	{ "gifLoad",			(PyCFunction)Matrix_gifLoad,			METH_VARARGS, "Load GIF file from specified filepath." },
	{ "gifPlay",			(PyCFunction)Matrix_gifPlay,			METH_VARARGS, "Start GIF animation." },
	{ "gifStop",			(PyCFunction)Matrix_gifStop,			METH_NOARGS,  "Stop GIF animation." },
	{ "gifPosition",		(PyCFunction)Matrix_gifPosition,		METH_VARARGS, "Change GIF animation offset position." },

	{ NULL, NULL, 0, NULL }
};

static PyMemberDef Matrix_members[] =
{
	{ const_cast<char *>("width"),  T_USHORT, offsetof(tMatrixObject, width),  READONLY, const_cast<char *>("Display width in pixels/LEDs.") },
	{ const_cast<char *>("height"), T_USHORT, offsetof(tMatrixObject, height), READONLY, const_cast<char *>("Display height in pixels/LEDs.") },

	{ NULL, 0, 0, 0, NULL }
};

static PyTypeObject tMatrixObjectType =
{
	PyObject_HEAD_INIT(NULL)
	0,                              // ob_size (not used, always set to 0)
	"ledmatrix.matrix",             // tp_name (module name, object name)
	sizeof(tMatrixObject),          // tp_basicsize
	0,                              // tp_itemsize
	(destructor)Matrix_dealloc,     // tp_dealloc
	0,                              // tp_print
	0,                              // tp_getattr
	0,                              // tp_setattr
	0,                              // tp_compare
	0,                              // tp_repr
	0,                              // tp_as_number
	0,                              // tp_as_sequence
	0,                              // tp_as_mapping
	0,                              // tp_hash
	0,                              // tp_call
	0,                              // tp_str
	0,                              // tp_getattro
	0,                              // tp_setattro
	0,                              // tp_as_buffer
	Py_TPFLAGS_DEFAULT,             // tp_flags
	0,                              // tp_doc
	0,                              // tp_traverse
	0,                              // tp_clear
	0,                              // tp_richcompare
	0,                              // tp_weaklistoffset
	0,                              // tp_iter
	0,                              // tp_iternext
	Matrix_methods,                 // tp_methods
	Matrix_members,                 // tp_members
	0,                              // tp_getset
	0,                              // tp_base
	0,                              // tp_dict
	0,                              // tp_descr_get
	0,                              // tp_descr_set
	0,                              // tp_dictoffset
	(initproc)Matrix_init,          // tp_init
	0,                              // tp_alloc
	Matrix_new,                     // tp_new
	0,                              // tp_free
};



static bool PythonHook_Matrix()
{
	PyObject* m;


	m = Py_InitModule("ledmatrix", NULL);
	if(!m ||
		(PyType_Ready(&tTextScrollerObjectType)	< 0) ||
		(PyType_Ready(&tMatrixObjectType)		< 0))
		return false;
	
	Py_INCREF(&tMatrixObjectType);
	PyModule_AddObject(m, "matrix", (PyObject *)&tMatrixObjectType);
	
	Py_INCREF(&tTextScrollerObjectType);
	PyModule_AddObject(m, "textscroller", (PyObject *)&tTextScrollerObjectType);


	// fonts
	PyModule_AddIntConstant(m, "FONT_font3x5",		font3x5);
	PyModule_AddIntConstant(m, "FONT_font5x7",		font5x7);
	PyModule_AddIntConstant(m, "FONT_font6x10",		font6x10);
	PyModule_AddIntConstant(m, "FONT_font8x13",		font8x13);
	PyModule_AddIntConstant(m, "FONT_gohufont11",	gohufont11);
	PyModule_AddIntConstant(m, "FONT_gohufont11b",	gohufont11b);

	// scroll mode
	PyModule_AddIntConstant(m, "SCROLL_wrapForward",			wrapForward);
	PyModule_AddIntConstant(m, "SCROLL_bounceForward",			bounceForward);
	PyModule_AddIntConstant(m, "SCROLL_bounceReverse",			bounceReverse);
	PyModule_AddIntConstant(m, "SCROLL_stopped",				stopped);
	PyModule_AddIntConstant(m, "SCROLL_off",					off);
	PyModule_AddIntConstant(m, "SCROLL_wrapForwardFromLeft",	wrapForwardFromLeft);

	// Display mode states
	PyModule_AddIntConstant(m, "DISPLAY_None",			DisplayState_None);
	PyModule_AddIntConstant(m, "DISPLAY_Settings",		DisplayState_Settings);
	PyModule_AddIntConstant(m, "DISPLAY_DateTime",		DisplayState_DateTime);
	PyModule_AddIntConstant(m, "DISPLAY_Alarm",			DisplayState_Alarm);
	PyModule_AddIntConstant(m, "DISPLAY_Timer",			DisplayState_Timer);
	PyModule_AddIntConstant(m, "DISPLAY_Messages",		DisplayState_Messages);
	PyModule_AddIntConstant(m, "DISPLAY_Manual",		DisplayState_Manual);
	
	return true;
}

