#ifndef __JP_DRAW_HPP
#define __JP_DRAW_HPP

#include <map>
#include <string>

extern "C" {
  #include <cairo.h>
}

class
jp_draw_img
{
  cairo_surface_t *surf_;
  cairo_t *cr_;
  int width_;
  int height_;

public:
  jp_draw_img(int width, int height);
  ~jp_draw_img();

  static std::pair<int, int> image_res(const std::string& fn);
  void image(const std::string& fn, double x, double y, double w, double h);
  void text(double x, double y, const std::string& text, double size,
            const std::string& stroke);
  void ellipse(double x, double y, double a, double b, double c,
               double width,
               const std::string& stroke, const std::string& fill);
  void line(double x1, double y1, double x2, double y2, double width, const std::string& stroke);
  void rectangle(double x, double y, double w, double h, double width,
                 const std::string& stroke, const std::string& fill);
  void save(const std::string& fn);

  void push_transform(double dx, double dy, double scale);
  void pop_transform();

  void splatter_ellipses(const std::string& fn,
                         const double* x, const double* y,
                         const double* a, const double* b, const double* c,
                         size_t num,
                         const double* H,
                         double alpha);
};

#endif
