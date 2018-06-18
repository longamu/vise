### generate wrapper_ransac.cpp using:
### cython --cplus wrapper_ransac.pyx


import numpy

# Standard C functions.
cdef extern from "stdlib.h":
  ctypedef long size_t
  ctypedef unsigned uint32_t
  void *malloc(size_t size)
  void free(void *ptr)

cdef extern from "string.h":
  char *strchr(char *s, int c)
  char *strcpy(char *dest, char *src)
  char *strncpy(char *dest, char *src, size_t n)
  int strcmp(char *s1, char *s2)
  char *strdup(char *s)
  void *memcpy(void *dest, void *src, size_t n)

cdef extern from "time.h":
  ctypedef int time_t

# Some helper routines from the Python API
cdef extern from "Python.h":

  # special types
  ctypedef int Py_ssize_t

  # references
  void Py_INCREF(object)
  void Py_DECREF(object)

  # To release global interpreter lock (GIL) for threading
  void Py_BEGIN_ALLOW_THREADS()
  void Py_END_ALLOW_THREADS()

  # Functions for integers
  object PyInt_FromLong(long)
  long PyInt_AsLong(object)
  object PyLong_FromLongLong(long long)
  long long PyLong_AsLongLong(object)

  # Functions for floating points
  object PyFloat_FromDouble(double)

  # Functions for strings
  object PyString_FromStringAndSize(char *s, int len)
  char *PyString_AsString(object string)
  object PyString_FromString(char *)

  # Functions for lists
  int PyList_Append(object list, object item)

  # Functions for tuples
  object PyTuple_New(int)
  int PyTuple_SetItem(object, int, object)
  object PyTuple_GetItem(object, int)
  int PyTuple_Size(object tuple)

  # Functions for dicts
  int PyDict_Contains(object p, object key)
  object PyDict_GetItem(object p, object key)

  # Functions for objects
  object PyObject_GetItem(object o, object key)
  int PyObject_SetItem(object o, object key, object v)
  int PyObject_DelItem(object o, object key)
  long PyObject_Length(object o)
  int PyObject_Compare(object o1, object o2)
  int PyObject_AsReadBuffer(object obj, void **buffer, Py_ssize_t *buffer_len)



#-----------------------------------------------------------------------------

# API for NumPy objects
cdef extern from "numpy/arrayobject.h":
  cdef int NPY_CONTIGUOUS

  # Platform independent types
  ctypedef int npy_intp
  ctypedef signed int npy_int8
  ctypedef unsigned int npy_uint8
  ctypedef signed int npy_int16
  ctypedef unsigned int npy_uint16
  ctypedef signed int npy_int32
  ctypedef unsigned int npy_uint32
  ctypedef signed long long npy_int64
  ctypedef unsigned long long npy_uint64
  ctypedef float npy_float32
  ctypedef double npy_float64

  cdef enum NPY_TYPES:
    NPY_BOOL
    NPY_BYTE
    NPY_UBYTE
    NPY_SHORT
    NPY_USHORT
    NPY_INT
    NPY_UINT
    NPY_LONG
    NPY_ULONG
    NPY_LONGLONG
    NPY_ULONGLONG
    NPY_FLOAT
    NPY_DOUBLE
    NPY_LONGDOUBLE
    NPY_CFLOAT
    NPY_CDOUBLE
    NPY_CLONGDOUBLE
    NPY_OBJECT
    NPY_STRING
    NPY_UNICODE
    NPY_VOID
    NPY_NTYPES
    NPY_NOTYPE

  # Platform independent types
  cdef enum:
    NPY_INT8, NPY_INT16, NPY_INT32, NPY_INT64,
    NPY_UINT8, NPY_UINT16, NPY_UINT32, NPY_UINT64,
    NPY_FLOAT32, NPY_FLOAT64, NPY_COMPLEX64, NPY_COMPLEX128

  # Classes
  ctypedef extern class numpy.dtype [object PyArray_Descr]:
    cdef int type_num, elsize, alignment
    cdef char type, kind, byteorder, hasobject
    cdef object fields, typeobj

  ctypedef extern class numpy.ndarray [object PyArrayObject]:
    cdef char *data
    cdef int nd
    cdef npy_intp *dimensions
    cdef npy_intp *strides
    cdef object base
    cdef dtype descr
    cdef int flags

  # Functions
  object PyArray_GETITEM(object arr, void *itemptr)
  int PyArray_SETITEM(object arr, void *itemptr, object obj)
  dtype PyArray_DescrFromType(int type)
  object PyArray_Scalar(void *data, dtype descr, object base)

  # The NumPy initialization function
  void import_array()

import_array()


cdef extern from "wrapper_ransac_engine3.h":
  ctypedef struct c_pair_u4_u4 "std::pair<uint32_t, uint32_t>":
    uint32_t first
    uint32_t second

  ctypedef struct c_vector_u4_u4 "std::vector< std::pair<uint32_t, uint32_t> >":
    size_t (* size)()
    c_pair_u4_u4 (* index "operator[]")(size_t ind)


  #void (* c_ransac "ransac")(
  uint32_t (* c_ransac "ransac")(
              void* data1, uint32_t size1,
              void* data2, uint32_t size2,
              int stride,
              int id_off,
              int x_off, int y_off,
              int a_off, int b_off, int c_off,
              int theta_off,
              
              int max_cor,
              float error_thresh,
              float low_area_change,
              float high_area_change,
              int max_reest,
              
              float* best_h,
              c_vector_u4_u4 best_cor
              )


  uint32_t (* c_ransac_desc_uchar "ransac_desc<unsigned char>")(
              char* data1,
              uint32_t size1,

              char* data2,
              uint32_t size2,

              int stride,

              int x_off,
              int y_off,
              int a_off,
              int b_off,
              int c_off,
              int theta_off,

              unsigned char* desc1,
              unsigned char* desc2,
              uint32_t ndims,

              float error_thresh,
              float low_area_change,
              float high_area_change,
              int max_reest,
              float epsilon,
              float delta,
              int use_lowe,

              float* best_h,
              c_vector_u4_u4 best_cor,
              float weighted_inliers,

              c_vector_u4_u4* all_cor
              )
  
  uint32_t (* c_ransac_desc_float "ransac_desc<float>")(
              char* data1,
              uint32_t size1,

              char* data2,
              uint32_t size2,

              int stride,

              int x_off,
              int y_off,
              int a_off,
              int b_off,
              int c_off,
              int theta_off,

              float* desc1,
              float* desc2,
              uint32_t ndims,

              float error_thresh,
              float low_area_change,
              float high_area_change,
              int max_reest,
              float epsilon,
              float delta,
              int use_lowe,

              float* best_h,
              c_vector_u4_u4 best_cor,
              float weighted_inliers,

              c_vector_u4_u4* all_cor
              )
              

def ransac_desc(ndarray data1, ndarray data2,\
                ndarray desc1, ndarray desc2,\
                float error_thresh,\
                float low_area_change, float high_area_change,\
                int max_reest,\
                float epsilon, float delta, int use_lowe,
                return_all_cor = False):
  """
  Performs RANSAC between the points in data1 and data2.
  This version looks at the actual descriptors, not the
  quantized visual words.

  Returns (num_inliers, best_h, [best_cor]).

  if return_all_cor == True, returns
      (num_inliers, best_h, [best_cor], [all_cor])
  """
  cdef int stride
  cdef int x_off
  cdef int y_off
  cdef int a_off
  cdef int b_off
  cdef int c_off
  cdef int theta_off
  cdef ndarray best_h
  cdef ndarray best_cor
  cdef float weighted_inliers
  cdef c_vector_u4_u4 best_cor_v
  cdef c_vector_u4_u4 all_cor
  cdef c_vector_u4_u4* all_cor_p
  cdef size_t i
  cdef uint32_t score

  assert data1.dtype == data2.dtype
  assert desc1.dtype == desc2.dtype
  assert desc1.flags & NPY_CONTIGUOUS
  assert desc2.flags & NPY_CONTIGUOUS

  assert data1.shape[0] == desc1.shape[0]
  assert data2.shape[0] == desc2.shape[0]

  best_h = numpy.zeros((3,3), dtype='f4')

  theta_off = -1

  stride = data1.dtype.itemsize

  x_off = data1.dtype.fields['x'][1]
  y_off = data1.dtype.fields['y'][1]
  a_off = data1.dtype.fields['a'][1]
  b_off = data1.dtype.fields['b'][1]
  c_off = data1.dtype.fields['c'][1]

  if 'theta' in data1.dtype.fields:
    theta_off = data1.dtype.fields['theta'][1]

  all_cor_p = <c_vector_u4_u4*>0
  if return_all_cor:
      all_cor_p = &all_cor

  if desc1.dtype == 'u1':
      score= \
      c_ransac_desc_uchar(data1.data, data1.shape[0],
                          data2.data, data2.shape[0],
                          stride,
                          x_off, y_off, a_off, b_off, c_off, theta_off,
                          <unsigned char*>desc1.data,
                          <unsigned char*>desc2.data,
                          desc1.shape[1],
                          error_thresh, low_area_change, high_area_change,
                          max_reest, epsilon, delta, use_lowe,
                          <float*>best_h.data,
                          best_cor_v,
                          weighted_inliers,
                          all_cor_p)
  elif desc1.dtype == 'f4':
      score= \
      c_ransac_desc_float(data1.data, data1.shape[0],
                          data2.data, data2.shape[0],
                          stride,
                          x_off, y_off, a_off, b_off, c_off, theta_off,
                          <float*>desc1.data,
                          <float*>desc2.data,
                          desc1.shape[1],
                          error_thresh, low_area_change, high_area_change,
                          max_reest, epsilon, delta, use_lowe,
                          <float*>best_h.data,
                          best_cor_v,
                          weighted_inliers,
                          all_cor_p)
  else:
      raise TypeError, 'descs must be f4 or u1'
  best_cor = numpy.empty((best_cor_v.size(), 2), dtype='u4')
  for i from 0 <= i < best_cor_v.size():
      best_cor[i,0] = best_cor_v.index(i).first
      best_cor[i,1] = best_cor_v.index(i).second

  if return_all_cor:
      all_cor_arr = numpy.empty((all_cor.size(), 2), dtype='u4')
      for i from 0 <= i < all_cor.size():
          all_cor_arr[i,0] = all_cor.index(i).first
          all_cor_arr[i,1] = all_cor.index(i).second

      return (score, best_h, best_cor, all_cor_arr)
  
  else:
      return (score, best_h, best_cor)

          

def ransac(ndarray data1, ndarray data2,\
           int max_cor, float error_thresh,\
           float low_area_change,float high_area_change,\
           int max_reest):
  """
  Performs RANSAC between the points stored in data1 and data2.

  Returns (num_inliers, best_h, [best_cor]).
  """
  cdef int stride
  cdef int id_off
  cdef int x_off
  cdef int y_off
  cdef int a_off
  cdef int b_off
  cdef int c_off
  cdef int theta_off
  cdef ndarray best_h
  cdef ndarray best_cor
  cdef float weighted_inliers
  cdef c_vector_u4_u4 best_cor_v
  cdef size_t i
  #HACK!!
  cdef uint32_t numIter_debug

  # Check that everything is in its right place.
  assert data1.dtype == data2.dtype

  best_h = numpy.zeros((3,3), dtype='f4')
  
  theta_off = -1

  stride = data1.dtype.itemsize

  id_off = data1.dtype.fields['id'][1]
  x_off = data1.dtype.fields['x'][1]
  y_off = data1.dtype.fields['y'][1]
  a_off = data1.dtype.fields['a'][1]
  b_off = data1.dtype.fields['b'][1]
  c_off = data1.dtype.fields['c'][1]
  
  if 'theta' in data1.dtype.fields:
    theta_off = data1.dtype.fields['theta'][1]

  score= \
  c_ransac(
         data1.data, data1.shape[0],
         data2.data, data2.shape[0],
         stride,
         id_off, x_off, y_off,
         a_off, b_off, c_off, theta_off,
         max_cor,
         error_thresh,
         low_area_change,
         high_area_change,
         max_reest,
         <float*>best_h.data,
         best_cor_v
        )
  best_cor = numpy.empty((best_cor_v.size(), 2), dtype='u4')
  for i from 0 <= i < best_cor_v.size():
    best_cor[i,0] = best_cor_v.index(i).first
    best_cor[i,1] = best_cor_v.index(i).second

  return (score, best_h, best_cor)
