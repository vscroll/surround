#ifndef __GST_GL_VIDEO_PANORAMA_SINK_H__
#define __GST_GL_VIDEO_PANORAMA_SINK_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/gl/gl.h>
#include "gstglvideodef.h"
#include "esUtil.h"

G_BEGIN_DECLS

#define GST_TYPE_GL_VIDEO_PANORAMA_SINK (gst_gl_video_panorama_sink_get_type())
#define GST_GL_VIDEO_PANORAMA_SINK(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GL_VIDEO_PANORAMA_SINK, GstGLVideoPanoramaSink))
#define GST_GL_VIDEO_PANORAMA_SINK_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_GL_VIDEO_PANORAMA_SINK, GstGLVideoPanoramaSinkClass))
#define GST_IS_GL_VIDEO_PANORAMA_SINK(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GL_VIDEO_PANORAMA_SINK))
#define GST_IS_GL_VIDEO_PANORAMA_SINK_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_GL_VIDEO_PANORAMA_SINK))
#define GST_GL_VIDEO_PANORAMA_SINK_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_GL_VIDEO_PANORAMA_SINK, GstGLVideoPanoramaSinkClass))

typedef struct _GstGLVideoPanoramaSink GstGLVideoPanoramaSink;
typedef struct _GstGLVideoPanoramaSinkClass GstGLVideoPanoramaSinkClass;

struct _GstGLVideoPanoramaSink
{
    GstElement parent;

    GstPad *sinkpad[VIDEO_CHN_NUM];
    GstPad *srcpad;

    GQueue *queue[VIDEO_CHN_NUM];
    GMutex queue_mutex;

    GQueue *panorama_queue;
    GMutex panorama_queue_mutex;

    GstTask *task;
    GRecMutex task_mutex;
    gboolean running;

    ESContext esContext;
};

struct _GstGLVideoPanoramaSinkClass
{
    GstElementClass   parent_class;
};

GType gst_gl_video_panorama_sink_get_type(void);

G_END_DECLS
#endif /* __GST_GL_VIDEO_PANORAMA_SINK_H__ */
