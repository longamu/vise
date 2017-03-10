#include <Python.h>
#include <numpy/arrayobject.h>

#include <cstdlib>
#include <vector>

#include "jp_jpeg.hpp"
#include "jp_draw.hpp"

using namespace std;

unsigned char*
load_image(const std::string& fname, int& width, int& height)
{
  unsigned char* data;
  if (fname.find(".jpeg")!=fname.npos || fname.find(".jpg")!=fname.npos) {
    data = jp_jpeg_read(fname.c_str(), width, height);
  }
  else {
    std::cerr << fname << " has an unknown file extension!" << std::endl;
    std::exit(-1);
  }
  return data;
}

unsigned char*
convert_rgb24_to_argb32(unsigned char* data, int size)
{
  unsigned char* ret = new unsigned char[size*4];
  uint32_t* p = (uint32_t*)ret;

  for (int i=0; i<size; ++i) {
    p[i] = (0xff << 24)|(data[3*i+0] << 16)|(data[3*i+1] << 8)|(data[3*i+2] << 0);
  }

  return ret;
}

unsigned long
hex2dec(const char* beg, const char* end)
{
  unsigned long ret = 0;
  while (beg != end) {
    ret *= 16;
    unsigned long val = 0;
    if (*beg >= '0' && *beg <= '9') {
      val = *beg - '0';
    }
    else if (*beg >= 'a' && *beg <= 'f') {
      val = (*beg - 'a') + 10;
    }
    else if (*beg >= 'A' && *beg <= 'F') {
      val = (*beg - 'A') + 10;
    }
    ret += val;
    beg++;
  }
  return ret;
}

void
decode_colour(const std::string& col, double& r, double& g, double& b, double& a)
{
  if (col[0] == '#') { // Colour code.
    r = hex2dec(col.c_str() + 1, col.c_str() + 3)/256.0;
    g = hex2dec(col.c_str() + 3, col.c_str() + 5)/256.0;
    b = hex2dec(col.c_str() + 5, col.c_str() + 7)/256.0;
    if (col.size()>7)
      a = hex2dec(col.c_str() + 7, col.c_str() + 9)/256.0;
    else
      a = 256.0;
  }
  else if (col=="none")    { r = 0.0;  g = 0.0;  b = 0.0;  a = 0.0; }
  else if (col=="black")   { r = 0.0;  g = 0.0;  b = 0.0;  a = 1.0; }
  else if (col=="silver")  { r = 0.75; g = 0.75; b = 0.75; a = 1.0; }
  else if (col=="gray")    { r = 0.5;  g = 0.5;  b = 0.5;  a = 1.0; }
  else if (col=="white")   { r = 1.0;  g = 1.0;  b = 1.0;  a = 1.0; }
  else if (col=="maroon")  { r = 0.5;  g = 0.0;  b = 0.0;  a = 1.0; }
  else if (col=="red")     { r = 1.0;  g = 0.0;  b = 0.0;  a = 1.0; }
  else if (col=="purple")  { r = 0.5;  g = 0.0;  b = 0.5;  a = 1.0; }
  else if (col=="fuchsia") { r = 1.0;  g = 0.0;  b = 1.0;  a = 1.0; }
  else if (col=="green")   { r = 0.0;  g = 0.5;  b = 0.0;  a = 1.0; }
  else if (col=="lime")    { r = 0.0;  g = 1.0;  b = 0.0;  a = 1.0; }
  else if (col=="olive")   { r = 0.5;  g = 0.5;  b = 0.0;  a = 1.0; }
  else if (col=="yellow")  { r = 1.0;  g = 1.0;  b = 0.0;  a = 1.0; }
  else if (col=="navy")    { r = 0.0;  g = 0.0;  b = 0.5;  a = 1.0; }
  else if (col=="blue")    { r = 0.0;  g = 0.0;  b = 1.0;  a = 1.0; }
  else if (col=="teal")    { r = 0.0;  g = 0.5;  b = 0.5;  a = 1.0; }
  else if (col=="aqua")    { r = 0.0;  g = 1.0;  b = 1.0;  a = 1.0; }
}

jp_draw_img::jp_draw_img(int width, int height)
 : width_(width), height_(height)
{
  surf_ = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
  cr_ = cairo_create(surf_);
}

jp_draw_img::~jp_draw_img()
{
  cairo_destroy(cr_);
  cairo_surface_destroy(surf_);
}

std::pair<int, int>
jp_draw_img::image_res(const std::string& fn)
{
  int w, h;
  unsigned char* d = jp_jpeg_read(fn.c_str(), w, h);
  delete[] d;
  return std::make_pair(w, h);
}

void
jp_draw_img::image(const std::string& fn, double x, double y, double w, double h)
{
  int img_w, img_h;
  unsigned char* img_data = load_image(fn, img_w, img_h);
  unsigned char* img_data_b = convert_rgb24_to_argb32(img_data, img_w*img_h);

  cairo_surface_t *img_surf =
    cairo_image_surface_create_for_data(img_data_b, CAIRO_FORMAT_ARGB32, img_w, img_h, 4*img_w);

  cairo_save(cr_);
  cairo_translate(cr_, x, y);
  if (w!=-1.0 && h!=-1.0)
    cairo_scale(cr_, w/img_w, h/img_h);
  cairo_set_source_surface(cr_, img_surf, 0.0, 0.0);
  cairo_paint(cr_);
  cairo_restore(cr_);

  delete[] img_data;
  delete[] img_data_b;
  cairo_surface_destroy(img_surf);
}

void
jp_draw_img::text(double x, double y, const std::string& text, double size, const std::string& stroke)
{
  cairo_save(cr_);
  cairo_translate(cr_, x, y);

  double r, g, b, a;
  decode_colour(stroke, r, g, b, a);
  cairo_set_source_rgba(cr_, r, g, b, a);
  cairo_set_font_size(cr_, size);
  cairo_show_text(cr_, text.c_str());
  cairo_stroke(cr_);

  cairo_restore(cr_);
}

void
jp_draw_img::line(double x1, double y1, double x2, double y2,
                  double width,
                  const std::string& stroke)
{
  cairo_move_to(cr_, x1, y1);
  cairo_line_to(cr_, x2, y2);

  double r, g, b, a;
  decode_colour(stroke, r, g, b, a);
  cairo_set_source_rgba(cr_, r, g, b, a);
  cairo_set_line_width(cr_, width);
  cairo_stroke(cr_);
}

void
ellipse_params(double a, double b, double c,
               double& min_axis_len, double& maj_axis_len, double& angle_in_rad)
{
  b = 2.0*b;
  double det = b*b/4.0 - a*c;
  double D2 = -det;
  if (det<0)
    det = -det;
  double trace = a+c;
  double disc = sqrt(trace*trace - 4.0*D2);
  double cmaj = (trace+disc)*D2/(2.0*det);
  double cmin = (trace-disc)*D2/(2.0*det);
  maj_axis_len = 1.0/sqrt(cmin);
  min_axis_len = 1.0/sqrt(cmaj);
  double den = c-a;
  angle_in_rad = -0.5*atan2(b, den);
}

void
jp_draw_img::ellipse(double x, double y, double a, double b, double c, 
                     double width,
                     const std::string& stroke, const std::string& fill)
{
  double min_axis_len, maj_axis_len, angle_in_rad;
  ellipse_params(a, b, c, min_axis_len, maj_axis_len, angle_in_rad);

  cairo_save(cr_);
  cairo_translate(cr_, x, y);
  cairo_rotate(cr_, angle_in_rad);
  cairo_scale(cr_, maj_axis_len, min_axis_len);
  cairo_arc(cr_, 0.0, 0.0, 1.0, 0.0, 2*M_PI);
  cairo_restore(cr_);

  double sr, sg, sb, sa;
  double fr, fg, fb, fa;
  decode_colour(stroke, sr, sg, sb, sa);
  decode_colour(fill, fr, fg, fb, fa);

  if (fill != "none") {
    cairo_set_source_rgba(cr_, fr, fg, fb, fa);
    cairo_fill_preserve(cr_);
  }

  cairo_set_line_width(cr_, width);
  cairo_set_source_rgba(cr_, sr, sg, sb, sa);
  cairo_stroke(cr_);
}

void
jp_draw_img::save(const std::string& fn)
{
  cairo_surface_write_to_png(surf_, fn.c_str());
}

void
jp_draw_img::push_transform(double dx, double dy, double scale)
{
  cairo_save(cr_);
  cairo_translate(cr_, dx, dy);
  cairo_scale(cr_, scale, scale);
}

void
jp_draw_img::pop_transform()
{
  cairo_restore(cr_);
}

void
jp_draw_img::rectangle(double x, double y, double w, double h, double width,
                       const std::string& stroke, const std::string& fill)
{
  double sr, sg, sb, sa;
  double fr, fg, fb, fa;
  decode_colour(stroke, sr, sg, sb, sa);
  decode_colour(fill, fr, fg, fb, fa);

  cairo_rectangle(cr_, x, y, w, h);

  if (fill != "none") {
    cairo_set_source_rgba(cr_, fr, fg, fb, fa);
    cairo_fill_preserve(cr_);
  }
  cairo_set_line_width(cr_, width);
  cairo_set_source_rgba(cr_, sr, sg, sb, sa);
  cairo_stroke(cr_);
}

template<class Float>
inline void
inv3x3(const Float* h, Float* hi)
{
  Float common = h[0]*h[4]*h[8] - h[0]*h[5]*h[7] - h[3]*h[1]*h[8] + h[3]*h[2]*h[7] + h[6]*h[1]*h[5] - h[6]*h[2]*h[4];
  common = Float(1)/common;

  hi[0] = (h[4]*h[8] - h[5]*h[7])*common;
  hi[1] = -(h[1]*h[8] - h[2]*h[7])*common;
  hi[2] = (h[1]*h[5] - h[2]*h[4])*common;
  //
  hi[3] = -(h[3]*h[8] - h[5]*h[6])*common;
  hi[4] = (h[0]*h[8] - h[2]*h[6])*common;
  hi[5] = -(h[0]*h[5] - h[2]*h[3])*common;
  //
  hi[6] = (h[3]*h[7] - h[4]*h[6])*common;
  hi[7] = -(h[0]*h[7] - h[1]*h[6])*common;
  hi[8] = (h[0]*h[4] - h[1]*h[3])*common;
}

void
jp_draw_img::splatter_ellipses(
  const std::string& fn,
  const double* x, const double* y,
  const double* a, const double* b, const double* c,
  size_t num,
  const double* H,
  double alpha)
{
  double Hi[9];
  inv3x3(H, Hi);

  // 1. Load the image.
  int img_w, img_h;
  unsigned char* img_data = load_image(fn, img_w, img_h);
  unsigned char* img_data_b = convert_rgb24_to_argb32(img_data, img_w*img_h);

  cairo_surface_t *img_surf =
    cairo_image_surface_create_for_data(img_data_b, CAIRO_FORMAT_ARGB32, img_w, img_h, 4*img_w);

  for (size_t i=0; i<num; ++i) {
    // 2. Get the clip
    double min_axis_len, maj_axis_len, angle_in_rad;
    ellipse_params(a[i], b[i], c[i], min_axis_len, maj_axis_len, angle_in_rad);

    // 3. Ellipse path.
    cairo_save(cr_);
    cairo_translate(cr_, x[i], y[i]);
    cairo_rotate(cr_, angle_in_rad);
    cairo_scale(cr_, maj_axis_len, min_axis_len);
    cairo_arc(cr_, 0.0, 0.0, 1.0, 0.0, 2*M_PI);
    cairo_restore(cr_);
    
    // 4. Clip the path.
    cairo_reset_clip(cr_);
    cairo_clip(cr_);

    // 5. Transform to img_surf.
    cairo_save(cr_);
    cairo_matrix_t mat;
    mat.xx = H[0];
    mat.xy = H[1];
    mat.x0 = H[2];
    mat.yx = H[3];
    mat.yy = H[4];
    mat.y0 = H[5];
    cairo_transform(cr_, &mat);

    // 6. Draw the pixels.
    cairo_set_source_surface(cr_, img_surf, 0, 0);
    cairo_paint_with_alpha(cr_, alpha);

    // 7. Restore the state.
    cairo_restore(cr_);
  }

  delete[] img_data_b;
  delete[] img_data;
}

/*

double
conv_np_float(PyObject* obj)
{
  if (!obj) {
    PyErr_SetString(PyExc_RuntimeError, "Error 1!");
    throw_error_already_set();
  }

  PyObject* nobj = PyNumber_Float(obj);
  if (!nobj) {
    PyErr_SetString(PyExc_RuntimeError, "Error 2!");
    throw_error_already_set();
  }

  double d = PyFloat_AsDouble(nobj);

  Py_DECREF(nobj);
  Py_DECREF(obj);

  return d;
}

void
jpd_splatter_ellipses(jp_draw_img* img, const std::string& fn,
                      object xo, object yo,
                      object ao, object bo, object co,
                      object Ho,
                      double alpha)
{
  vector<double> x, y, a, b, c;
  vector<double> H;

  if (!PySequence_Check(xo.ptr()) ||
      !PySequence_Check(yo.ptr()) ||
      !PySequence_Check(ao.ptr()) ||
      !PySequence_Check(bo.ptr()) ||
      !PySequence_Check(co.ptr()) ||
      !PySequence_Check(Ho.ptr()))
  {
    PyErr_SetString(PyExc_RuntimeError, "x,y,a,b,c,H must be sequences");
    throw_error_already_set();
  }

  if (
       (PySequence_Size(xo.ptr()) != PySequence_Size(yo.ptr())) ||
       (PySequence_Size(xo.ptr()) != PySequence_Size(ao.ptr())) ||
       (PySequence_Size(xo.ptr()) != PySequence_Size(bo.ptr())) ||
       (PySequence_Size(xo.ptr()) != PySequence_Size(co.ptr()))
     )
  {
    PyErr_SetString(PyExc_RuntimeError, "x,y,a,b,c must be the same size");
    throw_error_already_set();
  }

  if (PySequence_Size(Ho.ptr())!=9) {
    PyErr_SetString(PyExc_RuntimeError, "H must be size 9");
    throw_error_already_set();
  }

  for (int i=0; i<PySequence_Size(Ho.ptr()); ++i) {
    H.push_back(conv_np_float(PySequence_GetItem(Ho.ptr(), i)));
  }

  for (int i=0; i<PySequence_Size(xo.ptr()); ++i) {
    x.push_back(conv_np_float(PySequence_GetItem(xo.ptr(), i)));
    y.push_back(conv_np_float(PySequence_GetItem(yo.ptr(), i)));
    a.push_back(conv_np_float(PySequence_GetItem(ao.ptr(), i)));
    b.push_back(conv_np_float(PySequence_GetItem(bo.ptr(), i)));
    c.push_back(conv_np_float(PySequence_GetItem(co.ptr(), i)));
  }

  img->splatter_ellipses(fn, &x[0], &y[0], &a[0], &b[0], &c[0], x.size(), &H[0], alpha);
}

*/

/**
 * Python specific stuff.
 **/

/*

template<class T1, class T2>
struct
pair_to_pytuple
  : to_python_converter<std::pair<T1,T2>, pair_to_pytuple<T1,T2> >
{
  static PyObject* convert(const std::pair<T1,T2>& p)
  {
    return incref(make_tuple(p.first, p.second).ptr());
  }
};

BOOST_PYTHON_MODULE(jp_draw_)
{
  import_array();
  pair_to_pytuple<int, int>();
  class_<jp_draw_img>("img", init<int, int>())
    .def("image", &jp_draw_img::image)
    .def("image_res", &jp_draw_img::image_res)
    .staticmethod("image_res")
    .def("ellipse", &jp_draw_img::ellipse)
    .def("save", &jp_draw_img::save)
    .def("push_transform", &jp_draw_img::push_transform)
    .def("pop_transform", &jp_draw_img::pop_transform)
    .def("line", &jp_draw_img::line)
    .def("rectangle", &jp_draw_img::rectangle)
    .def("splatter_ellipses", &jpd_splatter_ellipses)
    ;
}

*/
