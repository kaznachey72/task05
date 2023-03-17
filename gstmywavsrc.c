#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>
#include "gstmywavsrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_mywavsrc_debug_category);
#define GST_CAT_DEFAULT gst_mywavsrc_debug_category

/* prototypes */

static void gst_mywavsrc_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec);
static void gst_mywavsrc_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec);
static void gst_mywavsrc_dispose (GObject *object);
static void gst_mywavsrc_finalize (GObject *object);

static GstCaps *gst_mywavsrc_get_caps (GstBaseSrc *src, GstCaps *filter);
static gboolean gst_mywavsrc_negotiate (GstBaseSrc *src);
static GstCaps *gst_mywavsrc_fixate (GstBaseSrc *src, GstCaps *caps);
static gboolean gst_mywavsrc_set_caps (GstBaseSrc *src, GstCaps *caps);
static gboolean gst_mywavsrc_decide_allocation (GstBaseSrc *src, GstQuery *query);
static gboolean gst_mywavsrc_start (GstBaseSrc *src);
static gboolean gst_mywavsrc_stop (GstBaseSrc *src);
static void gst_mywavsrc_get_times (GstBaseSrc *src, GstBuffer *buffer, GstClockTime *start, GstClockTime *end);
static gboolean gst_mywavsrc_get_size (GstBaseSrc *src, guint64 *size);
static gboolean gst_mywavsrc_is_seekable (GstBaseSrc *src);
static gboolean gst_mywavsrc_prepare_seek_segment (GstBaseSrc *src, GstEvent *seek, GstSegment *segment);
static gboolean gst_mywavsrc_do_seek (GstBaseSrc *src, GstSegment *segment);
static gboolean gst_mywavsrc_unlock (GstBaseSrc *src);
static gboolean gst_mywavsrc_unlock_stop (GstBaseSrc *src);
static gboolean gst_mywavsrc_query (GstBaseSrc *src, GstQuery *query);
static gboolean gst_mywavsrc_event (GstBaseSrc *src, GstEvent *event);
static GstFlowReturn gst_mywavsrc_create (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buf);
static GstFlowReturn gst_mywavsrc_alloc (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buf);
static GstFlowReturn gst_mywavsrc_fill (GstBaseSrc *src, guint64 offset, guint size, GstBuffer *buf);

enum
{
    PROP_0
};

/* pad templates */

static GstStaticPadTemplate gst_mywavsrc_src_template =
GST_STATIC_PAD_TEMPLATE (
    "src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
);

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (
    GstMyWavSrc, 
    gst_mywavsrc, 
    GST_TYPE_BASE_SRC,
    GST_DEBUG_CATEGORY_INIT (
        gst_mywavsrc_debug_category, 
        "mywavsrc", 
        0,
        "debug category for mywavsrc element"
    )
)

static void gst_mywavsrc_class_init (GstMyWavSrcClass * klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
    GstBaseSrcClass *base_src_class = GST_BASE_SRC_CLASS (klass);

    gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass), &gst_mywavsrc_src_template);
    gst_element_class_set_static_metadata (
        GST_ELEMENT_CLASS(klass), 
        "FIXME Long name", "Generic", "FIXME Description",
        "FIXME <fixme@example.com>"
    );

    gobject_class->set_property = gst_mywavsrc_set_property;
    gobject_class->get_property = gst_mywavsrc_get_property;
    gobject_class->dispose = gst_mywavsrc_dispose;
    gobject_class->finalize = gst_mywavsrc_finalize;
    base_src_class->get_caps = GST_DEBUG_FUNCPTR (gst_mywavsrc_get_caps);
    base_src_class->negotiate = GST_DEBUG_FUNCPTR (gst_mywavsrc_negotiate);
    base_src_class->fixate = GST_DEBUG_FUNCPTR (gst_mywavsrc_fixate);
    base_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_mywavsrc_set_caps);
    base_src_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_mywavsrc_decide_allocation);
    base_src_class->start = GST_DEBUG_FUNCPTR (gst_mywavsrc_start);
    base_src_class->stop = GST_DEBUG_FUNCPTR (gst_mywavsrc_stop);
    base_src_class->get_times = GST_DEBUG_FUNCPTR (gst_mywavsrc_get_times);
    base_src_class->get_size = GST_DEBUG_FUNCPTR (gst_mywavsrc_get_size);
    base_src_class->is_seekable = GST_DEBUG_FUNCPTR (gst_mywavsrc_is_seekable);
    base_src_class->prepare_seek_segment = GST_DEBUG_FUNCPTR (gst_mywavsrc_prepare_seek_segment);
    base_src_class->do_seek = GST_DEBUG_FUNCPTR (gst_mywavsrc_do_seek);
    base_src_class->unlock = GST_DEBUG_FUNCPTR (gst_mywavsrc_unlock);
    base_src_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_mywavsrc_unlock_stop);
    base_src_class->query = GST_DEBUG_FUNCPTR (gst_mywavsrc_query);
    base_src_class->event = GST_DEBUG_FUNCPTR (gst_mywavsrc_event);
    base_src_class->create = GST_DEBUG_FUNCPTR (gst_mywavsrc_create);
    base_src_class->alloc = GST_DEBUG_FUNCPTR (gst_mywavsrc_alloc);
    base_src_class->fill = GST_DEBUG_FUNCPTR (gst_mywavsrc_fill);
}

static void gst_mywavsrc_init (GstMyWavSrc *mywavsrc)
{
    (void) mywavsrc;
}

void gst_mywavsrc_set_property (GObject *object, guint property_id, const GValue *value, GParamSpec *pspec)
{
    (void) value;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (object);
    GST_DEBUG_OBJECT (mywavsrc, "set_property");

    switch (property_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_mywavsrc_get_property (GObject *object, guint property_id, GValue *value, GParamSpec *pspec)
{
    (void) value;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (object);
    GST_DEBUG_OBJECT (mywavsrc, "get_property");

    switch (property_id) {
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
            break;
    }
}

void gst_mywavsrc_dispose (GObject *object)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (object);
    GST_DEBUG_OBJECT (mywavsrc, "dispose");
    G_OBJECT_CLASS (gst_mywavsrc_parent_class)->dispose (object);
}

void gst_mywavsrc_finalize (GObject *object)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (object);
    GST_DEBUG_OBJECT (mywavsrc, "finalize");
    G_OBJECT_CLASS (gst_mywavsrc_parent_class)->finalize (object);
}

static GstCaps *gst_mywavsrc_get_caps (GstBaseSrc *src, GstCaps *filter)
{
    (void) filter;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "get_caps");
    return NULL;
}

static gboolean gst_mywavsrc_negotiate (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "negotiate");
    return TRUE;
}

static GstCaps *gst_mywavsrc_fixate (GstBaseSrc *src, GstCaps *caps)
{
    (void) caps;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "fixate");
    return NULL;
}

static gboolean gst_mywavsrc_set_caps (GstBaseSrc *src, GstCaps *caps)
{
    (void) caps;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "set_caps");
    return TRUE;
}

static gboolean gst_mywavsrc_decide_allocation (GstBaseSrc *src, GstQuery *query)
{
    (void) query;
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "decide_allocation");
    return TRUE;
}

static gboolean gst_mywavsrc_start (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "start");
    return TRUE;
}

static gboolean gst_mywavsrc_stop (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "stop");
    return TRUE;
}

static void gst_mywavsrc_get_times (GstBaseSrc *src, GstBuffer *buffer, GstClockTime *start, GstClockTime *end)
{
    (void) buffer;
    (void) start;
    (void) end;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "get_times");
}

static gboolean gst_mywavsrc_get_size (GstBaseSrc *src, guint64 *size)
{
    (void) size;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "get_size");
    return TRUE;
}

static gboolean gst_mywavsrc_is_seekable (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "is_seekable");
    return TRUE;
}

static gboolean gst_mywavsrc_prepare_seek_segment (GstBaseSrc *src, GstEvent *seek, GstSegment *segment)
{
    (void) seek;
    (void) segment;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "prepare_seek_segment");
    return TRUE;
}

static gboolean gst_mywavsrc_do_seek (GstBaseSrc *src, GstSegment *segment)
{
    (void) segment;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "do_seek");
    return TRUE;
}

static gboolean gst_mywavsrc_unlock (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "unlock");
    return TRUE;
}

static gboolean gst_mywavsrc_unlock_stop (GstBaseSrc *src)
{
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "unlock_stop");
    return TRUE;
}

static gboolean gst_mywavsrc_query (GstBaseSrc *src, GstQuery *query)
{
    (void) query;
    
    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "query");
    return TRUE;
}

static gboolean gst_mywavsrc_event (GstBaseSrc *src, GstEvent *event)
{
    (void) event;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "event");
    return TRUE;
}

static GstFlowReturn gst_mywavsrc_create (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buf)
{
    (void) offset;
    (void) size;
    (void) buf;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "create");
    return GST_FLOW_OK;
}

static GstFlowReturn gst_mywavsrc_alloc (GstBaseSrc *src, guint64 offset, guint size, GstBuffer **buf)
{
    (void) offset;
    (void) size;
    (void) buf;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "alloc");
    return GST_FLOW_OK;
}

static GstFlowReturn gst_mywavsrc_fill (GstBaseSrc *src, guint64 offset, guint size, GstBuffer *buf)
{
    (void) offset;
    (void) size;
    (void) buf;

    GstMyWavSrc *mywavsrc = GST_MYWAVSRC (src);
    GST_DEBUG_OBJECT (mywavsrc, "fill");
    return GST_FLOW_OK;
}

static gboolean plugin_init (GstPlugin *plugin)
{
    return gst_element_register (plugin, "mywavsrc", GST_RANK_NONE, GST_TYPE_MYWAVSRC);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "my_wav_src"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "mywavsrc"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "http://mywavsrc.org/"
#endif

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    mywavsrc,
    "mywavsrc plugin description",
    plugin_init, 
    VERSION, 
    "LGPL", 
    PACKAGE_NAME, 
    GST_PACKAGE_ORIGIN
)
