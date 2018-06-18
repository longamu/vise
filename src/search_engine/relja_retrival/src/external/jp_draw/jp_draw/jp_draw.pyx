import numpy

cdef extern from "stdint.h":
  ctypedef unsigned int uint32_t
  ctypedef unsigned long size_t

cdef extern from "string.h":
  void* memcpy(void* dest, void* stc, size_t n)

cdef extern from "numpy/arrayobject.h":
  ctypedef int npy_intp

  cdef int NPY_CONTIGUOUS

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

  void import_array()

# Must have this
import_array()

cdef extern from "jp_draw.hpp":
  ctypedef struct int_pair "std::pair<int,int>":
    int first
    int second
  ctypedef struct c_jp_draw_img "jp_draw_img":
    void (* image "image")(char *fn, double x, double y, double w, double h)
    void (* ellipse "ellipse")(double x, double y, double a, double b, double c,
                               double width,
                               char *stroke, char *fill)
    void (* text "text")(double x, double y, char *text, double size, char *stroke)
    int_pair (* image_res "image_res")(char *fn)
    void (* line "line")(double x1, double y1, double x2, double y2, double width, char *stroke)
    void (* save "save")(char *fn)
  c_jp_draw_img* new_jp_draw_img "new jp_draw_img" (int width, int height)
  void del_jp_draw_img "delete"(c_jp_draw_img* ptr)

cdef class image:
  cdef c_jp_draw_img *ptr

  def __cinit__(self, int width, int height):
    self.ptr = new_jp_draw_img(width, height)

  def image(self, char* fn, double x, double y, double w, double h):
    self.ptr.image(fn, x, y, w, h)

  def ellipse(self, double x, double y, double a, double b, double c, 
                    double width, char *stroke, char *fill):
    self.ptr.ellipse(x, y, a, b, c, width, stroke, fill)

  def line(self, double x1, double y1, double x2, double y2, double width = 1.0, char *stroke = '#ffffff'):
    self.ptr.line(x1, y1, x2, y2, width, stroke)

  def text(self, double x, double y, char *text, double size, char *stroke = '#ffffff'):
    self.ptr.text(x, y, text, size, stroke)

  def save(self, char *fn):
    self.ptr.save(fn)
