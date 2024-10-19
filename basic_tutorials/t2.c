#include <gst/gst.h>

int gst_main (int argc, char *argv[]) {
    GstElement *pipeline, *sink, *src;
    GstBus *bus;
    GstMessage *msg;
    
    gst_init(&argc, &argv);

    src = gst_element_factory_make("videotestsrc", "source");
    sink = gst_element_factory_make("autovideosink", "sink");


    pipeline = gst_pipeline_new("basic_t2_pipeline");

    if (!pipeline || !sink || !src) {
        g_printerr("failed to create elements.");
        return -1;
    }


    gst_bin_add_many(GST_BIN(pipeline), src, sink, NULL);
    if (gst_element_link(src, sink) == FALSE) {
        g_printerr("failed to link source and sink.");
        gst_object_unref(pipeline);
        return -1;
    }

    g_object_set(src, "pattern", 1, NULL);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to change pipeline state to playing.");
        gst_object_unref(pipeline);
        return -1;
    }


    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);


    if (msg != NULL) {
        GError *err;
        gchar *debug_info;

        switch (GST_MESSAGE_TYPE(msg)) {
            case GST_MESSAGE_ERROR:
                gst_message_parse_error(msg, &err, &debug_info);
                g_printerr("Element: %s, Error: %s", GST_OBJECT_NAME(msg->src), err->message);
                g_printerr("Debug Info: %s", debug_info ? debug_info : "none");
                break;
            case GST_MESSAGE_EOS:
                g_print("reached end of stream.");
                break;
            default:
                g_printerr("unknown message type.");
        }
        g_clear_error(&err);
        g_free(debug_info);
    }

    gst_object_unref(bus);
    gst_message_unref(msg);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}


int main (int argc, char *argv[]) {
    return gst_main(argc, argv);
}
