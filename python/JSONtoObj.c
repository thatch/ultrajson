/*
Developed by ESN, an Electronic Arts Inc. studio. 
Copyright (c) 2014, Electronic Arts Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of ESN, Electronic Arts Inc. nor the
names of its contributors may be used to endorse or promote products
derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS INC. BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Portions of code from MODP_ASCII - Ascii transformations (upper/lower, etc)
http://code.google.com/p/stringencoders/
Copyright (c) 2007  Nick Galbreath -- nickg [at] modp [dot] com. All rights reserved.

Numeric decoder derived from from TCL library
http://www.opensource.apple.com/source/tcl/tcl-14/tcl/license.terms
 * Copyright (c) 1988-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
*/

#include "py_defines.h"
#include <ultrajson.h>

/*
 * The DecoderParams contains some Python-specific decoder parameters.
 * It is accessible via the JSONObjectDecoder->prv field.
 */
typedef struct __DecoderParams
{
  /*
   * These hooks are Python functions (callable objects) provided by the user
   * via keyword parameters of JSONToObj(). They allow to decode JSON entities
   * into specific Python class instances (instead of generic dictionaries
   * and strings).
   *
   * The objectHook is called when a JSON object is decoded. It takes the object
   * (as dictionary) and may transform it into an arbitrary object (instanciate
   * a class, etc).
   *
   * The stringHook is called for every decoded string. It may replace the string
   * with an arbitrary object. Useful for deserializing things like dates from
   * their textual representations into native Python types.
   */
  PyObject *objectHook;
  PyObject *stringHook;

} DecoderParams;

//#define PRINTMARK() fprintf(stderr, "%s: MARK(%d)\n", __FILE__, __LINE__)
#define PRINTMARK()

static void Object_objectAddKey(void *prv, JSOBJ obj, JSOBJ name, JSOBJ value)
{
  PyDict_SetItem (obj, name, value);
  Py_DECREF( (PyObject *) name);
  Py_DECREF( (PyObject *) value);
  return;
}

static void Object_arrayAddItem(void *prv, JSOBJ obj, JSOBJ value)
{
  PyList_Append(obj, value);
  Py_DECREF( (PyObject *) value);
  return;
}

static JSOBJ Object_newString(void *prv, wchar_t *start, wchar_t *end)
{
  PyObject *strobj, *newobj;
  DecoderParams *dp = prv;

  strobj = PyUnicode_FromWideChar (start, (end - start));

  if (!dp->stringHook)
    return strobj;

  newobj = PyObject_CallFunctionObjArgs(dp->stringHook, strobj, NULL);

  if (newobj != strobj)
    Py_DECREF(strobj);

  return newobj;
}

static JSOBJ Object_newTrue(void *prv)
{
  Py_RETURN_TRUE;
}

static JSOBJ Object_newFalse(void *prv)
{
  Py_RETURN_FALSE;
}

static JSOBJ Object_newNull(void *prv)
{
  Py_RETURN_NONE;
}

static JSOBJ Object_newObject(void *prv)
{
  return PyDict_New();
}

static JSOBJ Object_newArray(void *prv)
{
  return PyList_New(0);
}

static JSOBJ Object_newInteger(void *prv, JSINT32 value)
{
  return PyInt_FromLong( (long) value);
}

static JSOBJ Object_newLong(void *prv, JSINT64 value)
{
  return PyLong_FromLongLong (value);
}

static JSOBJ Object_newUnsignedLong(void *prv, JSUINT64 value)
{
  return PyLong_FromUnsignedLongLong (value);
}

static JSOBJ Object_newDouble(void *prv, double value)
{
  return PyFloat_FromDouble(value);
}

JSOBJ Object_callObjectHook(JSOBJ _obj, void *prv)
{
  PyObject *obj, *newobj;
  DecoderParams *dp = prv;

  obj = (PyObject *)_obj;
  newobj = PyObject_CallFunctionObjArgs(dp->objectHook, obj, NULL);

  if (obj != newobj)
    Py_DECREF(obj);

  return newobj;
}

static void Object_releaseObject(void *prv, JSOBJ obj)
{
  Py_DECREF( ((PyObject *)obj));
}

static char *g_kwlist[] = {"obj", "object_hook", "string_hook", "precise_float", NULL};

PyObject* JSONToObj(PyObject* self, PyObject *args, PyObject *kwargs)
{
  PyObject *ret;
  PyObject *sarg;
  PyObject *arg;
  PyObject *oobjectHook = NULL;
  PyObject *ostringHook = NULL;
  PyObject *opreciseFloat = NULL;

  JSONObjectDecoder decoder =
  {
    Object_newString,
    Object_objectAddKey,
    Object_arrayAddItem,
    Object_newTrue,
    Object_newFalse,
    Object_newNull,
    Object_newObject,
    Object_newArray,
    Object_newInteger,
    Object_newLong,
    Object_newUnsignedLong,
    Object_newDouble,
    NULL, //callObjectHook (optional, initialized below if passed as the keyword parameter)
    Object_releaseObject,
    PyObject_Malloc,
    PyObject_Free,
    PyObject_Realloc
  };

  DecoderParams dp = {
      NULL, //objectHook
      NULL, //stringHook
  };
  decoder.prv = &dp;

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|OOO", g_kwlist, &arg, &oobjectHook, &ostringHook, &opreciseFloat))
  {
      return NULL;
  }

  if (oobjectHook && PyCallable_Check(oobjectHook))
  {
    decoder.callObjectHook = Object_callObjectHook;
    dp.objectHook = oobjectHook;
  }

  if (ostringHook && PyCallable_Check(ostringHook))
  {
    dp.stringHook = ostringHook;
  }

  if (PyString_Check(arg))
  {
      sarg = arg;
  }
  else
  if (PyUnicode_Check(arg))
  {
    sarg = PyUnicode_AsUTF8String(arg);
    if (sarg == NULL)
    {
      //Exception raised above us by codec according to docs
      return NULL;
    }
  }
  else
  {
    PyErr_Format(PyExc_TypeError, "Expected String or Unicode");
    return NULL;
  }

  decoder.errorStr = NULL;
  decoder.errorOffset = NULL;

  dconv_s2d_init(DCONV_S2D_ALLOW_TRAILING_JUNK, 0.0, 0.0, "Infinity", "NaN");

  ret = JSON_DecodeObject(&decoder, PyString_AS_STRING(sarg), PyString_GET_SIZE(sarg));

  dconv_s2d_free();

  if (sarg != arg)
  {
    Py_DECREF(sarg);
  }

  if (decoder.errorStr)
  {
    /*
    FIXME: It's possible to give a much nicer error message here with actual failing element in input etc*/

    PyErr_Format (PyExc_ValueError, "%s", decoder.errorStr);

    if (ret)
    {
        Py_DECREF( (PyObject *) ret);
    }

    return NULL;
  }

  return ret;
}

PyObject* JSONFileToObj(PyObject* self, PyObject *args, PyObject *kwargs)
{
  PyObject *read;
  PyObject *string;
  PyObject *result;
  PyObject *file = NULL;
  PyObject *argtuple;

  if (!PyArg_ParseTuple (args, "O", &file))
  {
    return NULL;
  }

  if (!PyObject_HasAttrString (file, "read"))
  {
    PyErr_Format (PyExc_TypeError, "expected file");
    return NULL;
  }

  read = PyObject_GetAttrString (file, "read");

  if (!PyCallable_Check (read)) {
    Py_XDECREF(read);
    PyErr_Format (PyExc_TypeError, "expected file");
    return NULL;
  }

  string = PyObject_CallObject (read, NULL);
  Py_XDECREF(read);

  if (string == NULL)
  {
    return NULL;
  }

  argtuple = PyTuple_Pack(1, string);

  result = JSONToObj (self, argtuple, kwargs);

  Py_XDECREF(argtuple);
  Py_XDECREF(string);

  if (result == NULL) {
    return NULL;
  }

  return result;
}
