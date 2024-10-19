#include <gst/gst.h>


int gst_main (int argc, char *argv[]) {
    GstElement *src, *sink, *pipeline;
    GstMessage *msg;
    GstBus *bus;



    gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}


int main (int argc, char *argv[]) {
    gst_main(argc, argv);
    return 0;
}
