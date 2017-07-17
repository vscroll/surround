#ifndef __GST_GL_VIDEO_PANORAMA_H__
#define __GST_GL_VIDEO_PANORAMA_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/gl/gl.h>
#include "gstglvideodef.h"

G_BEGIN_DECLS

#define GST_TYPE_GL_VIDEO_PANORAMA (gst_gl_video_panorama_get_type())
#define GST_GL_VIDEO_PANORAMA(obj) \
        (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_GL_VIDEO_PANORAMA, GstGLVideoPanorama))
#define GST_GL_VIDEO_PANORAMA_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_GL_VIDEO_PANORAMA, GstGLVideoPanoramaClass))
#define GST_IS_GL_VIDEO_PANORAMA(obj) \
        (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_GL_VIDEO_PANORAMA))
#define GST_IS_GL_VIDEO_PANORAMA_CLASS(klass) \
        (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_GL_VIDEO_PANORAMA))
#define GST_GL_VIDEO_PANORAMA_GET_CLASS(obj) \
        (G_TYPE_INSTANCE_GET_CLASS((obj), GST_TYPE_GL_VIDEO_PANORAMA, GstGLVideoPanoramaClass))

typedef struct _GstGLVideoPanorama GstGLVideoPanorama;
typedef struct _GstGLVideoPanoramaClass GstGLVideoPanoramaClass;

struct _GstGLVideoPanorama
{
    GstElement parent;

    GstPad *sinkpad[VIDEO_CHN_NUM];
    GstPad *srcpad;

    GQueue *queue[VIDEO_CHN_NUM];

    GQueue *panorama_queue;
    GMutex panorama_queue_mutex;

    GstTask *task;
    GRecMutex task_mutex;
};

struct _GstGLVideoPanoramaClass
{
    GstElementClass   parent_class;
};

GType gst_gl_video_panorama_get_type(void);

G_END_DECLS
#endif /* __GST_GL_VIDEO_PANORAMA_H__ */
