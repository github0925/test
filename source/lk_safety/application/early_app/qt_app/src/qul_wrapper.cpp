#include "layers.h"
#include <qul/application.h>
#include <qul/qul.h>
#include "container.h"
#include <event.h>
#include "myimageloader.h"

extern "C" void qul_wrapper(token_handle_t token);
namespace Qul {
namespace Private {
const size_t ImageCacheSize = 0;
} // namespace Private
} // namespace Qul

void qul_wrapper(token_handle_t token) {

    // Apple apple;
    // apple.GetColor();
    MyImageProvider myImageProvider;
    Qul::Application _qul_app;
    Qul::initPlatform();

    _qul_app.addImageProvider("fatlocation", &myImageProvider);

    static struct layers _qul_item;
    _qul_app.setRootItem(&_qul_item);

#ifdef APP_DEFAULT_UILANGUAGE
    _qul_app.setUiLanguage(APP_DEFAULT_UILANGUAGE);
#endif
     _qul_app.exec();
}