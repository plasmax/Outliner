// gl.h
// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

/*! \file gl.h
   This header file allows a plugin to include the OpenGL header file
   in a portable way, even though MicroSoft tried to break it. It also
   defines some OpenGL convienence functions.
 */

#ifndef DDImage_gl_H
#define DDImage_gl_H

#include "DDWindows.h"
#include "ViewerContext.h"

#if USE_GLEW
  #include <GL/glew.h>
#elif defined(__GLEW_H__)
/* do nothing if they included glew.h */
#else
  #define GL_GLEXT_PROTOTYPES 1
  #if defined(__APPLE__)
    #include "OpenGL/gl.h"
  #else
    #include <GL/gl.h>
  #endif
#endif

#include "DDImage_API.h"

/*////////////////////////////////////////////////////////////// */

#include "Box.h"
#include "Box3.h"
#include "Matrix4.h"

// Utility macro for checking OpenGL errors after calls
#ifdef FN_DEBUG
#define mFnGlCall(fun) fun; DD::Image::glGetErrors();
#else
#define mFnGlCall(fun) fun
#endif //FN_DEBUG

namespace DD
{

namespace Image
{

/*! Set the current OpenGL color to a color specified by the bytes of
    the unsigned value. The highest byte is red, the next green, then
    blue, then alpha. For instance 0xff00ff00 is purple (red+blue).
    This is the same form the Nuke and fltk use to identify GUI colors.
    Currently the alpha value is ignored.
 */
DDImage_API void glColor(unsigned);
DDImage_API void glColor4(unsigned);
DDImage_API void glColor4fvLUT(const float*);

/*! Draw the null-terminated string, appending it to the previous string
    or at the last call to glRasterPos. */
DDImage_API void gl_text(const char* str);

/*! Draw the first n bytes of the string, appending them to the previous
    string or last call to glRasterPos. */
DDImage_API void gl_text(const char* str, int n);

/*! Draw the null-terminated string, at position x,y,z.
    The font is controlled by the current glListBase(), Nuke will
    have preset this and it is probably best to leave it alone.
 */
DDImage_API void gl_text(const char* str, float x, float y, float z = 0);

/*! Draw the first n bytes of the string at position x,y,z */
DDImage_API void gl_text(const char* str, int n, float x, float y, float z = 0);

/*! Draw the null-terminated string, at position x,y,z.
    The font is controlled by the current glListBase(), Nuke will
    have preset this and it is probably best to leave it alone.
 */
DDImage_API void gl_text(const char* str, int x, int y, int z = 0);

/*! Draw the first n bytes of the string at position x,y,z */
DDImage_API void gl_text(const char* str, int n, int x, int y, int z = 0);
DDImage_API void gl_text(const char* str, double x, double y, double z = 0);
DDImage_API void gl_text(const char* str, int n, double x, double y, double z = 0);

DDImage_API void gl_enable_line_smoothing(float width);
DDImage_API void gl_disable_line_smoothing();

/*! gl_circle: Draw a circle or disc.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of circle
    diam    = diameter of circle
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_circlef(float x, float y, float z, float diam, int plane, bool filled = false, float normal = 1.0f);

/*! gl_circlefAsTris: Draw a circle or disc using a tristrip.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of circle
    diam    = diameter of circle
    lineWidth = width of the circle border
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_circlefAsTris(float x, float y, float z, float diam, float lineWidth, int plane, bool filled = false, float normal = 1.0f);

/*! gl_cylinder: Draw two connected discs forming a cylinder.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of center of cylinder
    diam1   = diameter of endcap 1
    diam2   = diameter of endcap 2
    plane   = which plane to draw in: XY, YZ, XZ - Provided for convenience
    filled  = whether to make them polygons
 */
DDImage_API void gl_cylinderf(float x, float y, float z, float width,
                              float diam1, float diam2, int plane,
                              bool capped = true, bool filled = false);

/*! gl_cone: Draw a open or closed cone.
    Uses a precalculated cosine table of 51 entries encompassing 360degs.
    x, y, z = position of tip
    length  = length of cone
    diam    = diameter of cone
    plane   = which plane to draw on: XY, YZ, XZ - Provided for convenience
 */
DDImage_API void gl_conef(float x, float y, float z, float length, float diam, int plane);

/*! Draw a wireframe sphere around 0,0,0 */
DDImage_API void gl_sphere(float radius = 0.5);

enum { XY, XZ, YZ, YX, ZX, ZY };

/*! Draw a flat plane in any of the three basic planes */
DDImage_API void gl_planef(float x, float y, float z, float width, float height, int plane);
DDImage_API void gl_gridf(float x, float y, float z, int numW, int numH, int plane);

DDImage_API void gl_quadi(int x, int y, int r, int t); // filled
DDImage_API void gl_rectangle(float x, float y, float r, float t); // outlined
DDImage_API void gl_rectangle(int x, int y, int r, int t);
DDImage_API void gl_bracket(float x, float y, float r, float t, float sx, float sy);
DDImage_API void gl_crossf(float x, float y, float z, float size, int plane);
DDImage_API void gl_crossi(int x, int y, int z, int size, int plane);

/*! Draw a 3-D T with each beam 1 unit long */
DDImage_API void gl_axis();

/*! Draw labels at the ends of the T drawn by gl_axis() */
DDImage_API void gl_axis_label();
DDImage_API void gl_boxf(float x, float y, float n, float r, float t, float f, bool filled = false);
DDImage_API void gl_cubef(float x, float y, float z, float width);

/*! Call glVertex2f with points on a 2-D bezier curve. x,y are
    the initial point, which is not drawn (this is done so several curves
    join together, you should draw the point yourself for the first curve
    in the sequence. The curve goes to x3,y3. x1,y1 and x2,y2 are the
    bezier handles.

    Enough points are drawn so that the error of the curve is less than
    one pixel anywhere and the straight edges should not be visible. To
    to this it needs to know how big one unit is in pixels, this can be
    passed as \a scale.

    This should not be used for curves that are way bigger than the
    viewport. The algorithim would normally divide the line into millions
    of pieces and waste a huge amount of time drawing the clipped pieces.
    This version does a maximum of 100 segments, so the machine does not
    lock up, but the result is then inaccurate.
 */
DDImage_API void gl_bezierf(float x, float y,
                            float x1, float y1,
                            float x2, float y2,
                            float x3, float y3,
                            float scale = 1);

//! draws text in the top left of the ViewerWindow tied to the ViewerContext. By default draws it in red text, if you
//! want another colour, set redColour to false and set the colour before calling this then pop the attribute after calling this.
DDImage_API void gl_drawTextTopLeftOrthogonal(const std::string& text, DD::Image::ViewerContext* ctx, bool redColour = true);

// Routines that emulate GLU calls:

/*! Print GL errors - reasonably emulates gluErrorString */
DDImage_API const char* gl_errorstring(int errorcode);

/*! Clear accumulated OpenGL errors. If FN_DEBUG is on, it prints them to
   stderr, prefixed with \a message. Returns number of errors found.
 */
DDImage_API int glGetErrors(const char* message = 0);

// Routines that are a *bit* like GLU calls:
DDImage_API bool GLProject(float x, float y, float z, const DD::Image::Matrix4& m, const DD::Image::Box& vp, float* winx, float* winy, float* winz);
DDImage_API bool GLUnproject(float wx, float wy, float wz, const DD::Image::Matrix4& m, const DD::Image::Box& vp, float* objx, float* objy, float* objz);
DDImage_API DD::Image::Box GLProjectCube(const DD::Image::Box3& cube, const DD::Image::Matrix4& m, const DD::Image::Box& vp);
DDImage_API void GLZOffsetProjection(float offsetAmount);

} // namespace Image

} // namespace DD

#endif

// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.
