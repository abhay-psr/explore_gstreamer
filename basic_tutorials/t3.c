#include "glib-object.h"
#include "glibconfig.h"
#include <gst/gst.h>
#include <math.h>

char PIPELINE_NAME[] = "basic-t3-pipeline";

typedef struct _PipelineElements {
    GstElement *pipeline;
    GstElement *src;
    GstElement *decoder;
    GstElement *audioconvert;
    GstElement *audioresample;
    GstElement *audiosink;
    GstElement *videoconvert;
    GstElement *videosink;
} PipelineElements;

static void pad_added_handler(GstElement *src, GstPad *new_pad, PipelineElements *eles) {
    GstPad *sinkpad = gst_element_get_static_pad(eles->audioconvert, "sink");
    g_print("new pad type: %s.%s\n", GST_ELEMENT_NAME(src), GST_PAD_NAME(new_pad));
    if (gst_pad_is_linked(sinkpad)) {
        goto exit;
    }

    GstCaps *new_pad_caps = gst_pad_get_current_caps(new_pad);
    GstStructure *pad_structure = gst_caps_get_structure(new_pad_caps, 0);
    const gchar *pad_type = gst_structure_get_name(pad_structure);
    if (!g_str_has_prefix(pad_type, "audio/x-raw")) {
        g_printerr("incorrect pad type. type = %s\n", pad_type);
        goto exit;
    }

    GstPadLinkReturn ret = gst_pad_link(new_pad, sinkpad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_printerr("dynamic pad linking failed. %s -> %s\n", pad_type, GST_PAD_NAME(sinkpad));
        goto exit;
    } else {
        g_print("dynamic pad linking done. %s -> %s\n", pad_type, GST_PAD_NAME(sinkpad));
    }

exit:
    if (new_pad_caps != NULL) {
        gst_caps_unref(new_pad_caps);
    }
    gst_object_unref(sinkpad);
}





int gst_main (int argc, char *argv[]) {
    gchar *input_source = argv[1];
    if (input_source == NULL) {
        g_printerr("Missing Argument: URI\n");
        return -1;
    }
    g_print("Input: %s\n", input_source);

    PipelineElements ppln_elements;
    GstMessage *msg;
    GstBus *bus;
    gboolean terminate = FALSE;

    // Init GST
    gst_init(&argc, &argv);

    // create gst elements
    ppln_elements.decoder = gst_element_factory_make("uridecodebin", "decoder");
    ppln_elements.audioconvert = gst_element_factory_make("audioconvert", "audioconvert");
    ppln_elements.audioresample = gst_element_factory_make("audioresample", "audioresample");
    ppln_elements.audiosink = gst_element_factory_make("autoaudiosink", "audiosink");
    ppln_elements.videoconvert = gst_element_factory_make("videoconvert", "videoconvert");
    ppln_elements.videosink = gst_element_factory_make("autovideosink", "videosink");

    // create pipeline
    ppln_elements.pipeline = gst_pipeline_new(PIPELINE_NAME);

    // check pipeline elements errors
    if (!ppln_elements.pipeline || !ppln_elements.decoder) {
        g_printerr("Elements pipeline, decoder count not be created.\n");
        return -1;
    }
    if (!ppln_elements.audioconvert || !ppln_elements.audioresample || !ppln_elements.audiosink) { 
        g_printerr("Elements audioconvert, audioresample, audiosink count not be created.\n");
        return -1;
    }
    if (!ppln_elements.videoconvert || !ppln_elements.videosink) {
        g_printerr("Elements videoconvert, videosink count not be created.\n");
        return -1;
    }

    // add all elements to same bin
    // gst_bin_add_many(GST_BIN(ppln_elements.pipeline), ppln_elements.decoder, NULL);
    // gst_bin_add_many(GST_BIN(ppln_elements.pipeline), ppln_elements.audioconvert, ppln_elements.audioresample, ppln_elements.audiosink, NULL);
    // gst_bin_add_many(GST_BIN(ppln_elements.pipeline), ppln_elements.videoconvert, ppln_elements.videosink, NULL);
    gst_bin_add_many(GST_BIN(ppln_elements.pipeline), ppln_elements.decoder, ppln_elements.audioconvert, ppln_elements.audioresample, ppln_elements.audiosink, ppln_elements.videoconvert, ppln_elements.videosink, NULL);

    // link elements
    // how is the first element in pipeline decided? does a source element get auto assigned as first element in pipeline? what if multiple source elements
    if (gst_element_link_many(ppln_elements.audioconvert, ppln_elements.audioresample, ppln_elements.audiosink, NULL) == FALSE) {
        g_printerr("Failed to link audio elements.\n");
        gst_object_unref(ppln_elements.pipeline);
        return -1;
    }

    if (gst_element_link_many(ppln_elements.videoconvert, ppln_elements.videosink, NULL) == FALSE) {
        g_printerr("Failed to link video elements.\n");
        gst_object_unref(ppln_elements.pipeline);
        return -1;
    }

    // set properties
    g_object_set(ppln_elements.decoder, "uri", input_source, NULL);

    g_signal_connect(ppln_elements.decoder, "pad-added", G_CALLBACK(pad_added_handler), &ppln_elements);

    // set pipeline playing
    GstStateChangeReturn ret = gst_element_set_state(ppln_elements.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr("Error playing pipeline.\n");
        gst_object_unref(ppln_elements.pipeline);
        return -1;
    }


    // bus work
    bus = gst_element_get_bus(ppln_elements.pipeline);

    do {
        msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
        if (msg != NULL) {
            GError *err;
            gchar *debug_info;

            switch (GST_MESSAGE_TYPE(msg)) {
                case GST_MESSAGE_ERROR:
                    gst_message_parse_error(msg, &err, &debug_info);
                    g_printerr("Message:%s:ERROR: %s\n", GST_OBJECT_NAME(msg->src), err->message);
                    g_printerr("Message:%s:DEBUG_INFO: %s\n", GST_OBJECT_NAME(msg->src), debug_info ? debug_info : "None");
                    g_clear_error(&err);
                    g_free(debug_info);
                    terminate = TRUE;
                    break;
                case GST_MESSAGE_EOS:
                    g_print("Message:%s:EOS: %s\n", GST_OBJECT_NAME(msg->src), "reached end of stream.");
                    terminate = TRUE;
                    break;
                case GST_MESSAGE_STATE_CHANGED:
                    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(ppln_elements.pipeline)) {
                        GstState old, new, pending;
                        gst_message_parse_state_changed(msg, &old, &new, &pending);
                        g_print("Message:%s:STATE_CHANGED: %s -> %s\n", GST_OBJECT_NAME(msg->src), gst_element_state_get_name(old), gst_element_state_get_name(new));
                    }
                    break;
                default:
                    g_printerr("Message:%s:? %s\n", GST_OBJECT_NAME(msg->src), "unknown message type");
                    break;
            }
            gst_message_unref(msg);
        }
    } while(terminate == FALSE);


    g_print("Ending.\n");

    gst_object_unref(bus);
    gst_element_set_state(ppln_elements.pipeline, GST_STATE_NULL);
    gst_object_unref(ppln_elements.pipeline);

    return 0;
}


int main (int argc, char *argv[]) {
    gst_main(argc, argv);
    return 0;
}
