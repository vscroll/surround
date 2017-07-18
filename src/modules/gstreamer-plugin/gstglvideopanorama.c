#include <gst/gst.h>
#include <gst/base/gstcollectpads.h>
#include <gst/video/video.h>

#include "gstglvideopanorama.h"

#define TEST_ALL_FUNC 1

GST_DEBUG_CATEGORY_STATIC (gst_gl_video_panorama_debug);
#define GST_CAT_DEFAULT gst_gl_video_panorama_debug

/* GLVideoPanorama signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0
};

#define DEBUG_INIT \
    GST_DEBUG_CATEGORY_INIT (gst_gl_video_panorama_debug, "glvideopanorama", 0, "glvideopanorama element");

#define gst_gl_video_panorama_parent_class parent_class
G_DEFINE_TYPE_WITH_CODE (GstGLVideoPanorama, gst_gl_video_panorama, GST_TYPE_ELEMENT,
    DEBUG_INIT);
//G_DEFINE_TYPE (GstGLVideoPanorama, gst_gl_video_panorama, GST_TYPE_ELEMENT);

#define VIDEOPANORAMA_QUEUE(self, i) (((GstGLVideoPanorama*)self)->queue[i])

#define QUEUE_PUSH(self, i, buf) G_STMT_START {                                         \
  GST_LOG ("=> Pushing to QUEUE[%d] in thread %p, length:%d",                         \
      i, g_thread_self(), g_queue_get_length (VIDEOPANORAMA_QUEUE (self, i)));        \
  g_queue_push_tail (VIDEOPANORAMA_QUEUE (self, i), buf);                              \
} G_STMT_END

#define QUEUE_POP(self, i, pointer) G_STMT_START {                                      \
  GST_LOG ("=> Waiting on QUEUE[%d] in thread %p, length:%d",                         \
        i, g_thread_self(), g_queue_get_length (VIDEOPANORAMA_QUEUE (self, i)));      \
  pointer = g_queue_pop_tail (VIDEOPANORAMA_QUEUE (self, i));                          \
  GST_LOG ("=> Waited on QUEUE[%d] in thread %p",                                     \
         i, g_thread_self());                                                           \
 } G_STMT_END

#define VIDEOPANORAMA_PANORAMA_QUEUE(self) (((GstGLVideoPanorama*)self)->panorama_queue)

#define PANORAMA_QUEUE_PUSH(self, buf) G_STMT_START {                                   \
  GST_LOG ("=> Pushing to PANORAMA_QUEUE in thread %p, length:%d",                    \
      g_thread_self(), g_queue_get_length (VIDEOPANORAMA_PANORAMA_QUEUE (self)));     \
  g_mutex_lock(&(((GstGLVideoPanorama*)self)->panorama_queue_mutex));                   \
  g_queue_push_tail (VIDEOPANORAMA_PANORAMA_QUEUE (self), buf);                        \
  g_mutex_unlock(&(((GstGLVideoPanorama*)self)->panorama_queue_mutex));                 \
} G_STMT_END

#define PANORAMA_QUEUE_POP(self, pointer) G_STMT_START {                                \
  GST_LOG ("=> Waiting on PANORAMA_QUEUE in thread %p, length:%d",                    \
        g_thread_self(), g_queue_get_length (VIDEOPANORAMA_PANORAMA_QUEUE (self)));   \
  g_mutex_lock(&(((GstGLVideoPanorama*)self)->panorama_queue_mutex));                   \
  pointer = g_queue_pop_tail (VIDEOPANORAMA_PANORAMA_QUEUE (self));                    \
  g_mutex_unlock(&(((GstGLVideoPanorama*)self)->panorama_queue_mutex));                 \
  GST_LOG ("=> Waited on PANORAMA_QUEUE in thread %p",                                \
         g_thread_self());                                                              \
 } G_STMT_END

static GstStaticPadTemplate sink_factory_0 = GST_STATIC_PAD_TEMPLATE ("sink_0",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_GL_COLOR_CONVERT_FORMATS))
    );

static GstStaticPadTemplate sink_factory_1 = GST_STATIC_PAD_TEMPLATE ("sink_1",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_GL_COLOR_CONVERT_FORMATS))
    );

static GstStaticPadTemplate sink_factory_2 = GST_STATIC_PAD_TEMPLATE ("sink_2",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_GL_COLOR_CONVERT_FORMATS))
    );

static GstStaticPadTemplate sink_factory_3 = GST_STATIC_PAD_TEMPLATE ("sink_3",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_GL_COLOR_CONVERT_FORMATS))
    );

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE_WITH_FEATURES
        (GST_CAPS_FEATURE_MEMORY_GL_MEMORY,
            "RGBA") "; "
        GST_VIDEO_CAPS_MAKE_WITH_FEATURES
        (GST_CAPS_FEATURE_META_GST_VIDEO_GL_TEXTURE_UPLOAD_META,
            "RGBA")
        "; " GST_VIDEO_CAPS_MAKE (GST_GL_COLOR_CONVERT_FORMATS))
    );

//object
static void gst_gl_video_panorama_constructed (GObject * object);
static void gst_gl_video_panorama_dispose (GObject * object);
static void gst_gl_video_panorama_finalize (GObject * object);

static void gst_gl_video_panorama_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_video_panorama_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

//element
static GstStateChangeReturn gst_gl_video_panorama_change_state (GstElement * element,
    GstStateChange transition);

//pad

static GstFlowReturn
gst_gl_video_panorama_chain (GstPad * pad,
    GstObject * object, GstBuffer * buffer);

//sink
static gboolean
gst_gl_video_panorama_set_caps (GstPad * pad, GstCaps * caps);

static gboolean
gst_gl_video_panorama_sink_event (GstPad *pad,
    GstObject *parent, GstEvent *event);

static gboolean
gst_gl_video_panorama_sink_query (GstPad * pad,
    GstObject * parent, GstQuery * query);

static gboolean
gst_gl_video_panorama_sink_activate_mode (GstPad * pad,
    GstObject * parent, GstPadMode mode, gboolean active);

//src
static gboolean
gst_gl_video_panorama_src_activate_mode (GstPad * pad,
    GstObject * parent, GstPadMode mode, gboolean active);


//priv
static gboolean
start_render_task (GstGLVideoPanorama * panorama);

static gboolean
stop_render_task (GstGLVideoPanorama * panorama);

static void
render_task (GstGLVideoPanorama * panorama);

//implementation

static void
gst_gl_video_panorama_class_init (GstGLVideoPanoramaClass * klass)
{
    GST_DEBUG("=> gst_gl_video_panorama_class_init\n");
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

#if TEST_ALL_FUNC
    gobject_class->constructed = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_constructed);
    gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_dispose);
    gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_finalize);
#endif

    gobject_class->get_property = gst_gl_video_panorama_get_property;
    gobject_class->set_property = gst_gl_video_panorama_set_property;

    element_class->change_state = gst_gl_video_panorama_change_state;

    gst_element_class_set_metadata (element_class, "OpenGL video_panorama",
        "Filter/Effect/Video/Compositor", "OpenGL video_panorama",
        "mingliang.wu <mingliang.wu@westalgo.com>");

    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&src_factory));
    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&sink_factory_0));
    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&sink_factory_1));
    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&sink_factory_2));
    gst_element_class_add_pad_template (element_class,
        gst_static_pad_template_get (&sink_factory_3));
}

static void
gst_gl_video_panorama_init (GstGLVideoPanorama * panorama)
{
    int i;
    GST_DEBUG("=> gst_gl_video_panorama_init\n");

    //sink
    panorama->sinkpad[VIDEO_CHN_FRONT] = gst_pad_new_from_static_template(&sink_factory_0, "sink_0");
    panorama->sinkpad[VIDEO_CHN_REAR] = gst_pad_new_from_static_template(&sink_factory_1, "sink_1");
    panorama->sinkpad[VIDEO_CHN_LEFT] = gst_pad_new_from_static_template(&sink_factory_2, "sink_2");
    panorama->sinkpad[VIDEO_CHN_RIGHT] = gst_pad_new_from_static_template(&sink_factory_3, "sink_3");

    for (i = 0; i < VIDEO_CHN_NUM; ++i) {
        gst_element_add_pad(GST_ELEMENT(panorama), panorama->sinkpad[i]);
    }

    for (i = 0; i < VIDEO_CHN_NUM; ++i) {
        gst_pad_set_event_function (panorama->sinkpad[i],
            GST_DEBUG_FUNCPTR(gst_gl_video_panorama_sink_event));

        gst_pad_set_chain_function (panorama->sinkpad[i],
            GST_DEBUG_FUNCPTR(gst_gl_video_panorama_chain));

        GST_PAD_SET_PROXY_CAPS (panorama->sinkpad[i]);

#if TEST_ALL_FUNC
        gst_pad_set_query_function (panorama->sinkpad[i],
            GST_DEBUG_FUNCPTR (gst_gl_video_panorama_sink_query));

        gst_pad_set_activatemode_function (panorama->sinkpad[i],
            GST_DEBUG_FUNCPTR (gst_gl_video_panorama_sink_activate_mode));
#endif
    }

    //source
    panorama->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
    gst_element_add_pad(GST_ELEMENT(panorama), panorama->srcpad);
    GST_PAD_SET_PROXY_CAPS (panorama->srcpad);
#if TEST_ALL_FUNC
    gst_pad_set_activatemode_function (panorama->srcpad,
        GST_DEBUG_FUNCPTR (gst_gl_video_panorama_src_activate_mode));
#endif

    for (i = 0; i < VIDEO_CHN_NUM; ++i) {
        panorama->queue[i] = g_queue_new ();
    }

    panorama->panorama_queue = g_queue_new ();
    g_mutex_init (&panorama->panorama_queue_mutex);

    panorama->task = NULL;
}

static void
gst_gl_video_panorama_constructed (GObject * object)
{
    GST_DEBUG("=> gst_gl_video_panorama_constructed\n");
}

static void
gst_gl_video_panorama_dispose (GObject * object)
{
    GST_DEBUG("=> gst_gl_video_panorama_dispose\n");

    G_OBJECT_CLASS(parent_class)->dispose (object);
}

static void
gst_gl_video_panorama_finalize (GObject * object)
{
    GST_DEBUG("=> gst_gl_video_panorama_finalize\n");
    G_OBJECT_CLASS(parent_class)->finalize (object);
}

static void
gst_gl_video_panorama_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
    GST_DEBUG("=> gst_gl_video_panorama_get_property\n");
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_gl_video_panorama_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
    GST_DEBUG("=> gst_gl_video_panorama_set_property\n");
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static GstStateChangeReturn gst_gl_video_panorama_change_state (GstElement * element,
    GstStateChange transition)
{
    GST_DEBUG("=> gst_gl_video_panorama_change_state\n");
    GstGLVideoPanorama *panorama = GST_GL_VIDEO_PANORAMA(element);

    GstStateChangeReturn ret;
    switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
        if (!start_render_task (panorama)) {
            goto start_failed;
        }
        break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        break;
    default:
        break;
    }

    ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        goto done;
    }

    switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
        ret = GST_STATE_CHANGE_SUCCESS;
        break;
    case GST_STATE_CHANGE_READY_TO_PAUSED:
        ret = GST_STATE_CHANGE_NO_PREROLL;
        break;
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        ret = GST_STATE_CHANGE_SUCCESS;
        break;
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
        ret = GST_STATE_CHANGE_NO_PREROLL;
        break;
    case GST_STATE_CHANGE_PAUSED_TO_READY:
        ret = GST_STATE_CHANGE_SUCCESS;
        break;
    case GST_STATE_CHANGE_READY_TO_NULL:
        stop_render_task (panorama);
        ret = GST_STATE_CHANGE_SUCCESS;
        break;
    default:
        break;
    }

done:
    return ret;

start_failed:
    {
        return GST_STATE_CHANGE_FAILURE;
    }
}

static GstFlowReturn
gst_gl_video_panorama_chain (GstPad * pad, GstObject * parent, GstBuffer * buffer)
{
    gchar *name = gst_pad_get_name (pad);
    GST_DEBUG("=> gst_gl_video_panorama_chain:%s\n", name);
    GstGLVideoPanorama *panorama = GST_GL_VIDEO_PANORAMA(parent);
    if (GST_PAD_IS_SINK(pad)) {
        
        int chn = -1;
        if (strcmp(name, "sink_0") == 0) {
            chn = VIDEO_CHN_FRONT;            
        } else if (strcmp(name, "sink_1") == 0) {
            chn = VIDEO_CHN_REAR;
        } else if (strcmp(name, "sink_2") == 0) {
            chn = VIDEO_CHN_LEFT;
        } else if (strcmp(name, "sink_3") == 0) {
            chn = VIDEO_CHN_RIGHT;
        } else {
        }
    
        if (chn >= 0) {
            QUEUE_PUSH (panorama, chn, buffer);
        }
    }


    GstBuffer* buf = NULL;
    PANORAMA_QUEUE_POP(panorama, buf);
    if (NULL == buf) {
        return GST_FLOW_OK;
    }
    
    return gst_pad_push (panorama->srcpad, buf);
    //return gst_pad_push (panorama->srcpad, buffer);
}

static gboolean
gst_gl_video_panorama_set_caps (GstPad * pad, GstCaps * caps)
{
    GST_DEBUG("=> gst_gl_video_panorama_set_caps\n");
    GstGLVideoPanorama *panorama;

    panorama = GST_GL_VIDEO_PANORAMA(gst_pad_get_parent (pad));
    if (pad == panorama->srcpad) {
        int i = 0;
        for (; i < VIDEO_CHN_NUM; ++i) {
            gst_pad_set_caps (panorama->sinkpad[i], caps);
        }
    }else {
        gst_pad_set_caps (panorama->srcpad, caps);
    }
    gst_object_unref (panorama);

    return TRUE;
}

static gboolean
gst_gl_video_panorama_sink_event (GstPad *pad, GstObject *parent, GstEvent *event)
{
    GST_DEBUG("=> gst_gl_video_panorama_sink_event\n");
    //GstGLVideoPanorama *panorama = GST_GL_VIDEO_PANORAMA(parent);

    switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
        GST_DEBUG("=> GST_EVENT_CAPS\n");
        GstCaps * caps;

        gst_event_parse_caps (event, &caps);
        /* do something with the caps */

        /* and forward */
        gst_pad_event_default (pad, parent, event);
        break;
    }
    case GST_EVENT_EOS:
        /* end-of-stream, we should close down all stream leftovers here */
        GST_DEBUG("=> GST_EVENT_EOS\n");
        break;
    default:
        break;
    }

    return gst_pad_event_default (pad, parent, event);
}

static gboolean
gst_gl_video_panorama_sink_query (GstPad * pad, GstObject * parent, GstQuery * query)
{
    GST_DEBUG("=> gst_gl_video_panorama_sink_query\n");

    return gst_pad_query_default (pad, parent, query);
}

static gboolean
gst_gl_video_panorama_sink_activate_mode (GstPad * pad,
    GstObject * parent, GstPadMode mode, gboolean active)
{
    GST_DEBUG("=> gst_gl_video_panorama_sink_activate_mode:active(%d), mode(%d)\n", active, mode);

    return TRUE;
}

static gboolean
gst_gl_video_panorama_src_activate_mode (GstPad * pad,
    GstObject * parent, GstPadMode mode, gboolean active)
{
    GST_DEBUG("=> gst_gl_video_panorama_src_activate_mode:active(%d), mode(%d)\n", active, mode);

    if (active == TRUE) {
        switch (mode) {
        case GST_PAD_MODE_PUSH:
        {
            GST_DEBUG("=> activate push pad");
            return TRUE;
        }
        default:
            GST_DEBUG("=> Only supported mode is PUSH");
            return FALSE;
        }      
    }

    return TRUE;
}

static gboolean
start_render_task (GstGLVideoPanorama * panorama)
{
    GST_DEBUG("=> start_render_task\n");

    if (panorama->task == NULL) {
        panorama->task = gst_task_new ((GstTaskFunction) render_task, panorama, NULL);
        if (panorama->task == NULL)
        {
            goto task_error;
        }

        g_rec_mutex_init (&panorama->task_mutex);
        gst_task_set_lock (panorama->task, &panorama->task_mutex);
    }

    panorama->running = TRUE;
    gst_task_start (panorama->task);
    return TRUE;
/* ERRORS */
task_error:
    {
        GST_DEBUG("=> start_render_task error\n");
        return FALSE;
    }
}

static gboolean
stop_render_task (GstGLVideoPanorama * panorama)
{
    GST_DEBUG("=> stop_render_task\n");

    GstTask *task;
    if (NULL != (task = panorama->task)) {

        panorama->running = FALSE;
        panorama->task = NULL;

        gst_task_stop (task);
        gst_task_join (task);

        /* and free the task */
        gst_object_unref (GST_OBJECT (task));

        g_rec_mutex_clear (&panorama->task_mutex);
    }

    return TRUE;
}

static void
render_task (GstGLVideoPanorama * panorama)
{
    GST_DEBUG("=> render_task start\n");

    int i = 0;
    while (panorama->running) {
        for (i = 0; i < VIDEO_CHN_NUM; ++i) {
            GstBuffer *buf = NULL;
            QUEUE_POP(panorama, i, buf);
            if (buf != NULL && i == VIDEO_CHN_FRONT) {
                PANORAMA_QUEUE_PUSH(panorama, buf);
            }
        }

        usleep(10);
    }

    GST_DEBUG("=> render_task end\n");
}
