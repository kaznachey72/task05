#ifndef GST_MYWAVSRC_H
#define GST_MYWAVSRC_H

#include <gst/base/gstbasesrc.h>
#include <stdio.h>

G_BEGIN_DECLS

#define GST_TYPE_MYWAVSRC          (gst_mywavsrc_get_type())
#define GST_MYWAVSRC(obj)          (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MYWAVSRC,GstMyWavSrc))
#define GST_MYWAVSRC_CLASS(klass)  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MYWAVSRC,GstMyWavSrcClass))
#define GST_IS_MYWAVSRC(obj)       (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MYWAVSRC))
#define GST_IS_MYWAVSRC_CLASS(obj) (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MYWAVSRC))

typedef struct _GstMyWavSrc GstMyWavSrc;
typedef struct _GstMyWavSrcClass GstMyWavSrcClass;

struct _GstMyWavSrc
{
  GstBaseSrc base_mywavsrc;
  gchar *filename;
  FILE *fd;
};

struct _GstMyWavSrcClass
{
  GstBaseSrcClass base_mywav_class;
};

GType gst_mywavsrc_get_type (void);

G_END_DECLS

#endif
