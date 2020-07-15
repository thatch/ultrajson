UltraJSON (Internet Archive Fork)
=================================

.. image:: https://img.shields.io/pypi/v/ujson-ia.svg?style=flat
    :alt: PyPI version
    :target: https://pypi.python.org/pypi/ujson-ia

.. image:: https://img.shields.io/pypi/pyversions/ujson-ia.svg
    :alt: Supported Python versions
    :target: https://pypi.python.org/pypi/ujson-ia

.. image:: https://travis-ci.org/internetarchive/ultrajson.svg?branch=master
    :target: https://travis-ci.org/internetarchive/ultrajson

About this fork
~~~~~~~~~~~~~~~

We use this version at the Internet Archive. We have merged @vdmit11's changes from https://github.com/dignio/ultrajson and the latest from upstream master, and may continue to make other tweaks. 

To install:

.. code-block:: sh

    $ pip install ujson-ia

Back to your regularly scheduled readme
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

UltraJSON is an ultra fast JSON encoder and decoder written in pure C with bindings for Python 2.7 and 3.5+.

For a more painless day to day C/C++ JSON decoder experience please checkout ujson4c_, based on UltraJSON.

.. _ujson4c: https://github.com/esnme/ujson4c/

| Please checkout the rest of the projects in the Ultra series:
| https://github.com/esnme/ultramemcache
| https://github.com/esnme/ultramysql

============
Usage
============
May be used as a drop in replacement for most other JSON parsers for Python:

.. code-block:: python

    >>> import ujson
    >>> ujson.dumps([{"key": "value"}, 81, True])
    '[{"key":"value"},81,true]'
    >>> ujson.loads("""[{"key": "value"}, 81, true]""")
    [{u'key': u'value'}, 81, True]

~~~~~~~~~~~~~~~
Encoder options
~~~~~~~~~~~~~~~
encode_html_chars
-----------------
Used to enable special encoding of "unsafe" HTML characters into safer Unicode sequences. Default is ``False``:

.. code-block:: python

    >>> ujson.dumps("<script>John&Doe", encode_html_chars=True)
    '"\\u003cscript\\u003eJohn\\u0026Doe"'

ensure_ascii
-------------
Limits output to ASCII and escapes all extended characters above 127. Default is true. If your end format supports UTF-8 setting this option to false is highly recommended to save space:

.. code-block:: python

    >>> ujson.dumps(u"\xe5\xe4\xf6")
    '"\\u00e5\\u00e4\\u00f6"'
    >>> ujson.dumps(u"\xe5\xe4\xf6", ensure_ascii=False)
    '"\xc3\xa5\xc3\xa4\xc3\xb6"'

escape_forward_slashes
----------------------
Controls whether forward slashes (``/``) are escaped. Default is ``True``:

.. code-block:: python

    >>> ujson.dumps("http://esn.me")
    '"http:\/\/esn.me"'
    >>> ujson.dumps("http://esn.me", escape_forward_slashes=False)
    '"http://esn.me"'

indent
------
Controls whether indention ("pretty output") is enabled. Default is ``0`` (disabled):

.. code-block:: python

    >>> ujson.dumps({"foo": "bar"})
    '{"foo":"bar"}'
    >>> ujson.dumps({"foo": "bar"}, indent=4)
    {
        "foo":"bar"
    }

pre_encode_hook
---------------
Allows to provide a custom function which is called for every encoded Python object.

The hook function semantics is similar to the standard JSONEncoder.default() method,
but the pre_encode_hook() is called before any other serialization attempts, while
the default() is called when all other options didn't work.

That allows to override already exsiting behavior and define custom serialization
formats for things like dates. For example::

    # Default behavior: datetime is converted to timestamp
    >>> ujson.dumps({"a": "foo", "b": datetime.now()})
    '{"a":"foo","b":1454523657}'
    
    # Hook is involved: the datetime object is replaced with the .isoformat() string
    >>> def hook(obj):
            return obj.isoformat() if hasattr(obj, 'isoformat') else obj
    
    >>> ujson.dumps({"a": "foo", "b": datetime.now()}, pre_encode_hook=hook)
    '{"a":"foo","b":"2016-02-03T18:21:55.351081"}'

The hook may be used to replace any object with any other arbitrary object before
encoding it. However, it doesn't cancel all further encoding transformations.
For example, if you return a `datetime` object from the hook instead of a string,
it will be transformed to a timestamp.


pre_encode_primitive
--------------------
The boolean flag that indicates that pre_encode_hook() should also be called
for Python objects that serialized to primitive JSON types (Number, String,
Boolean, null).

Usually you don't need to define any special serialization format for these
types, so the flag is false by default.

Enabling this flag may produce huge amount of pre_encode_hook() calls (the
hook will be called for every single JSON value) and thus affect the performance.

double_precision
----------------

This option is ignored, and is allowed for compatibility with ujson 1.35

~~~~~~~~~~~~~~~~
Decoders options
~~~~~~~~~~~~~~~~
precise_float
-------------
This option is ignored, and is allowed for compatibility with ujson 1.35

object_hook
-----------
A custom Python function which is called after a JSON object is decoded.

The hook semantics is similar to the standard JSONDecoder.object_hook() behavior.
You may use it to transform a dictionary (the decoded JSON object) into a more
specific object.

For example::

    >>> def hook(obj):
            if '__complex__' in obj:
                return complex(obj['real'], obj['imag'])
            return obj
    
    >>> ujson.loads('{"__complex__": true, "real": 1, "imag": 2}', object_hook=hook)
    (1+2j)

string_hook
-----------
Similar to `object_hook`, but called for every decoded string.

Useful for deserializing objects like dates from their textual representations, e.g.::

    >>> def hook(s):
            if s.startswith('__DATE'):
                return datetime.strptime(s, '__DATE: %Y-%m-%d')
            return s
    
    >>> ujson.loads('{"a": "foo", "b": "__DATE: 2016-01-01"}', string_hook=hook)
    {'a': 'foo', 'b': datetime.datetime(2016, 1, 1, 0, 0)}

============		
Benchmarks		
============		
*UltraJSON* calls/sec compared to three other popular JSON parsers with performance gain specified below each.

~~~~~~~~~~~~~
Test machine:
~~~~~~~~~~~~~

Linux 3.13.0-66-generic x86_64 #108-Ubuntu SMP Wed Oct 7 15:20:27 UTC 2015

~~~~~~~~~
Versions:
~~~~~~~~~

- CPython 2.7.6 (default, Jun 22 2015, 17:58:13) [GCC 4.8.2]
- blist     : 1.3.6
- simplejson: 3.8.1
- ujson     : 1.34 (0c52200eb4e2d97e548a765d5f089858c41967b0)
- yajl      : 0.3.5

+-------------------------------------------------------------------------------+------------+------------+------------+------------+
|                                                                               | ujson      | yajl       | simplejson | json       |
+===============================================================================+============+============+============+============+
| Array with 256 doubles                                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |    3508.19 |    5742.00 |    3232.38 |    3309.09 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   25103.37 |   11257.83 |   11696.26 |   11871.04 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 UTF-8 strings                                                  |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |    3189.71 |    2717.14 |    2006.38 |    2961.72 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |    1354.94 |     630.54 |     356.35 |     344.05 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 strings                                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   18127.47 |   12537.39 |   12541.23 |   20001.00 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   23264.70 |   12788.85 |   25427.88 |    9352.36 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Medium complex object                                                         |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   10519.38 |    5021.29 |    3686.86 |    4643.47 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |    9676.53 |    5326.79 |    8515.77 |    3017.30 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 True values                                                    |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |  105998.03 |  102067.28 |   44758.51 |   60424.80 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |  163869.96 |   78341.57 |  110859.36 |  115013.90 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 dict{string, int} pairs                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   13471.32 |   12109.09 |    3876.40 |    8833.92 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   16890.63 |    8946.07 |   12218.55 |    3350.72 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Dict with 256 arrays with 256 dict{string, int} pairs                         |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |      50.25 |      46.45 |      13.82 |      29.28 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |      33.27 |      22.10 |      27.91 |      10.43 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Dict with 256 arrays with 256 dict{string, int} pairs, outputting sorted keys |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |      27.19 |            |       7.75 |       2.39 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Complex object                                                                |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |     577.98 |            |     387.81 |     470.02 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |     496.73 |     234.44 |     151.00 |     145.16 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+

~~~~~~~~~
Versions:
~~~~~~~~~

- CPython 3.4.3 (default, Oct 14 2015, 20:28:29) [GCC 4.8.4]
- blist     : 1.3.6
- simplejson: 3.8.1
- ujson     : 1.34 (0c52200eb4e2d97e548a765d5f089858c41967b0)
- yajl      : 0.3.5

+-------------------------------------------------------------------------------+------------+------------+------------+------------+
|                                                                               | ujson      | yajl       | simplejson | json       |
+===============================================================================+============+============+============+============+
| Array with 256 doubles                                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |    3477.15 |    5732.24 |    3016.76 |    3071.99 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   23625.20 |    9731.45 |    9501.57 |    9901.92 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 UTF-8 strings                                                  |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |    1995.89 |    2151.61 |    1771.98 |    1817.20 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |    1425.04 |     625.38 |     327.14 |     305.95 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 strings                                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   25461.75 |   12188.64 |   13054.76 |   14429.81 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   21981.31 |   17014.22 |   23869.48 |   22483.58 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Medium complex object                                                         |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   10821.46 |    4837.04 |    3114.04 |    4254.46 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |    7887.77 |    5126.67 |    4934.60 |    6204.97 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 True values                                                    |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |  100452.86 |   94639.42 |   46657.63 |   60358.63 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |  148312.69 |   75485.90 |   88434.91 |  116395.51 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Array with 256 dict{string, int} pairs                                        |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |   11698.13 |    8886.96 |    3043.69 |    6302.35 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |   10686.40 |    7061.77 |    5646.80 |    7702.29 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Dict with 256 arrays with 256 dict{string, int} pairs                         |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |      44.26 |      34.43 |      10.40 |      21.97 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |      28.46 |      23.95 |      18.70 |      22.83 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Dict with 256 arrays with 256 dict{string, int} pairs, outputting sorted keys |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |      33.60 |            |       6.94 |      22.34 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| Complex object                                                                |            |            |            |            |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| encode                                                                        |     432.30 |            |     351.47 |     379.34 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
| decode                                                                        |     434.40 |     221.97 |     149.57 |     147.79 |
+-------------------------------------------------------------------------------+------------+------------+------------+------------+
