#include "gstglvideopanorama.h"

/* GStreamer license */
#define GST_LICENSE "LGPL"

/* package name in plugins */
#define GST_PACKAGE_NAME "GStreamer Plug-ins source release"

/* package origin */
#define GST_PACKAGE_ORIGIN "Unknown package origin"

/* Name of package */
#ifndef PACKAGE
#define PACKAGE "gst-plugins-gladas"
#endif

#define VERSION "1.4.5"

#define GST_CAT_DEFAULT gst_gl_gstgladas_debug
GST_DEBUG_CATEGORY_STATIC (GST_CAT_DEFAULT);

/* Register filters that make up the gstgladas plugin */
static gboolean
plugin_init (GstPlugin * plugin)
{
  GST_DEBUG_CATEGORY_INIT (gst_gl_gstgladas_debug, "gstgladas", 0, "gstgladas");

  if (!gst_element_register (plugin, "glvideopanorama",
          GST_RANK_NONE, GST_TYPE_GL_VIDEO_PANORAMA)) {
    return FALSE;/* GStreamer license */
  }

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    opengl-adas,
    "OpenGL ADAS plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
