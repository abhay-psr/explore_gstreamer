#include <gstreamer-1.0/gst/gst.h>

int gst_main(int argc, char *argv[] ) {
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    char pipeline_str[] = "playbin uri=https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm";
    pipeline = gst_parse_launch(pipeline_str, NULL);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        g_printerr("seeeerrrr\n");
    }


    gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}

int main(int argc, char *argv[]) {
    gst_main(argc, argv);
}

