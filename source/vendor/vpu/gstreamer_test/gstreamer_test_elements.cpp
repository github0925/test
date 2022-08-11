#include "gstreamer_test_elements.h"

#include <stdio.h>

#include <fstream>
#include <memory>
#include <mutex>

#include "gstreamer_test_utils.h"

#define MM_LOG_TAG "GstElements"
using namespace std;
using namespace Json;

#define GSTREAMER_TEST_ELEMENTS_JSON_PATH \
  "/data/gstreamer_test/gstreamer_elements.json"

static std::once_flag onceFlag;
static GstElements *gGstElementsInstance = nullptr;
static string gstreamer_test_elements_json(R"({
    "video_decode": [
        {   "mime": ["video/avc"],
            "element": "omxh264dec",
            "caps": "video/x-h264, stream-format=byte-stream, alignment=au"
        },
        {   "mime": ["video/hevc"],
            "element": "omxh265dec",
            "caps": "video/x-h265, stream-format=byte-stream, alignment=au"
        },
        {   "mime": ["video/mpegvideo", "video/mpeg2"],
            "element": "omxmpeg2dec",
            "caps": "video/mpeg, mpegversion=2"
        },
        {   "mime": ["video/mpeg4v-es"],
            "element": "omxmpeg4dec",
            "caps": "video/mpeg, mpegversion=4, systemstream=false"
        },
        {   "mime": ["video/h263", "video/3gpp"],
            "element": "omxh263dec",
            "caps": "video/x-h263, variant=itu"
        },
        {   "mime": ["video/mp4v-es"],
            "element": "omxmpeg4dec",
            "caps": "video/mpeg, mpegversion=4, systemstream=false"
        }
    ],
    "video_parse": {
        "video/avc":    "h264parse",
        "video/hevc":   "h265parse",
        "video/mpeg2":  "mpegvideoparse",
        "video/mp4v-es":"mpeg4videoparse"
    },
    "video_convert": "videoconvert",
    "video_render": "waylandsink",
    "video_encode": [
        {   "mime":     ["video/avc"],
            "element":  "omxh264enc",
            "caps":     "video/x-h264, stream-format=byte-stream, alignment=au" }
    ]
}
)");

GstElements *GstElements::shared() {
  std::call_once(onceFlag, []() {
    JSONCPP_STRING err;
    Json::Value root;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const jsonReader(
        readerBuilder.newCharReader());
    bool ret = jsonReader->parse(gstreamer_test_elements_json.c_str(),
                                 gstreamer_test_elements_json.c_str() +
                                     gstreamer_test_elements_json.length(),
                                 &root, &err);
    if (!ret || !err.empty()) {
      slog_err("json parse fail !!!");
      return nullptr;
    }
    gGstElementsInstance = new GstElements(root);
    slog_info("GstElements singleton created %p\n", gGstElementsInstance);
  });
  return gGstElementsInstance;
}

GstElements::GstElements(const Json::Value &value) : root(value) {}
GstElements::~GstElements() {}

static bool includeMimeType(const Value &mimeTypes, const char *mime) {
  string theMime = mime;
  for (int i = 0; i < mimeTypes.size(); ++i) {
    auto mimeType = mimeTypes[i].asString();
    if (mimeType == theMime) return true;
  }
  return false;
}

static bool getCodecElement(const Value &elements, const char *mime,
                            std::string &elementName, std::string &caps) {
  for (int i = 0; i < elements.size(); ++i) {
    Value element = elements[i];
    if (includeMimeType(element["mime"], mime)) {
      elementName = element["element"].asString();
      caps = element["caps"].asString();
      return true;
    }
  }
  return false;
}
bool GstElements::getVideoEncoder(const char *mime, std::string &element,
                                  std::string &caps) {
  return getCodecElement(root["video_encode"], mime, element, caps);
}
bool GstElements::getVideoDecoder(const char *mime, std::string &element,
                                  std::string &caps) {
  return getCodecElement(root["video_decode"], mime, element, caps);
}

std::string GstElements::videoParser(const char *mime) {
  return root["video_parse"][mime].asString();
}
std::string GstElements::videoConverter(const char *mime) {
  return root["video_convert"].asString();
}
std::string GstElements::videoRenderer(const char *mime) {
  return root["video_render"].asString();
}

std::string GstElements::videoTests() { return root["video_tests"].asString(); }
