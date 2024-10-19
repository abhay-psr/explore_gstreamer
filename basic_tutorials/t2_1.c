#include <gst/gst.h>

char PIPELINE_NAME[] = "basic_t2.1_pipeline";



int gst_main (int argc, char *argv[]) {
    GstElement *src, *sink, *caps_filter, *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    pipeline = gst_pipeline_new(PIPELINE_NAME);

    src = gst_element_factory_make("testvideosrc", "src");
    sink = gst_element_factory_make("autovideosink", "sink");
    caps_filter = gst_element_factory_make("video/x-raw, height=400, width=600", "caps_filter");

    g_object_set(src, "pattern", 2, NULL);

    gst_bin_add_many(GST_BIN(pipeline), src, caps_filter, sink, NULL);

    gst_element_link(src, caps_filter);
    gst_element_link(caps_filter, sink);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Failed to build pipeline");
        return -1;
    }

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
        g_printerr("Error.");
        gst_object_unref(bus);
        gst_message_unref(msg);
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        return -1;
    } else if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_EOS)
        g_print("End of stream.");
    else 
        g_print("Unknown message type.");
    

    gst_object_unref(bus);
    gst_message_unref(msg);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    return 0;
}


int main (int argc, char *argv[]) {
    gst_main(argc, argv);
    return 0;
}
