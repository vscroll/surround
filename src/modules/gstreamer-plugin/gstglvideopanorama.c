#include <gst/gst.h>
#include <gst/base/gstcollectpads.h>
#include <gst/video/video.h>

#include "gstglvideopanorama.h"

#define TEST_ALL_FUNC 0

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

G_DEFINE_TYPE_WITH_CODE (GstGLVideoPanorama, gst_gl_video_panorama, GST_TYPE_ELEMENT,
    DEBUG_INIT);
//G_DEFINE_TYPE (GstGLVideoPanorama, gst_gl_video_panorama, GST_TYPE_ELEMENT);

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

static void gst_gl_video_panorama_constructed (GObject * object);
static void gst_gl_video_panorama_dispose (GObject * object);
static void gst_gl_video_panorama_finalize (GObject * object);

static void gst_gl_video_panorama_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_gl_video_panorama_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn
gst_gl_video_panorama_chain (GstPad * pad,
    GstObject * object, GstBuffer * buffer);

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


//implementation

static void
gst_gl_video_panorama_class_init (GstGLVideoPanoramaClass * klass)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_class_init\n");
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

#if TEST_ALL_FUNC
    gobject_class->constructed = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_constructed);
    gobject_class->dispose = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_dispose);
    gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_gl_video_panorama_finalize);
#endif

    gobject_class->get_property = gst_gl_video_panorama_get_property;
    gobject_class->set_property = gst_gl_video_panorama_set_property;

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
    int video_chn_num;
    GST_DEBUG(" ===> gst_gl_video_panorama_init\n");

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
}

static void
gst_gl_video_panorama_constructed (GObject * object)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_constructed\n");
}

static void
gst_gl_video_panorama_dispose (GObject * object)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_dispose\n");

    G_OBJECT_CLASS(object)->dispose (object);
}

static void
gst_gl_video_panorama_finalize (GObject * object)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_finalize\n");
    G_OBJECT_CLASS(object)->finalize (object);
}

static void
gst_gl_video_panorama_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_get_property\n");
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
    GST_DEBUG(" ===> gst_gl_video_panorama_set_property\n");
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static GstFlowReturn
gst_gl_video_panorama_chain (GstPad * pad, GstObject * parent, GstBuffer * buffer)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_chain\n");
    GstGLVideoPanorama *panorama = GST_GL_VIDEO_PANORAMA(parent);

    return gst_pad_push (panorama->srcpad, buffer);
    //return gst_pad_chain (pad, buffer);
}

static gboolean
gst_gl_video_panorama_set_caps (GstPad * pad, GstCaps * caps)
{
    GST_DEBUG("===> gst_gl_video_panorama_set_caps\n");
    GstGLVideoPanorama *panorama;
    GstPad *otherpad;

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
    GST_DEBUG(" ===> gst_gl_video_panorama_sink_event\n");
    GstGLVideoPanorama *panorama = GST_GL_VIDEO_PANORAMA(parent);
    gboolean ret = TRUE;

    switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
        GST_DEBUG(" ===> GST_EVENT_CAPS\n");
        GstCaps * caps;

        gst_event_parse_caps (event, &caps);
        /* do something with the caps */

        /* and forward */
        ret = gst_pad_event_default (pad, parent, event);
        break;
    }
    case GST_EVENT_EOS:
        /* end-of-stream, we should close down all stream leftovers here */
        GST_DEBUG(" ===> GST_EVENT_EOS\n");
        break;
    default:
        break;
    }

    return gst_pad_event_default (pad, parent, event);
}

static gboolean
gst_gl_video_panorama_sink_query (GstPad * pad, GstObject * parent, GstQuery * query)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_sink_query\n");

    return gst_pad_query_default (pad, parent, query);
}

static gboolean
gst_gl_video_panorama_sink_activate_mode (GstPad * pad,
    GstObject * parent, GstPadMode mode, gboolean active)
{
    GST_DEBUG(" ===> gst_gl_video_panorama_sink_activate_mode\n");

    return TRUE;
}
