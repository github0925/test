// This code is auto-generated by Qul's qmltocpp tool from the file 'AnimatingItemLayer.qml'
// WARNING! All changes made in this file will be lost!
#include "AnimatingItemLayer.h"
#include <qul/private/graphicsdevice.h>
#include <qul/imageprovider.h>

extern const Qul::Private::FontEngine * const qul_font_default_18_50;
extern const Qul::Private::FontEngine * const qul_font_default_18_50;
extern const Qul::Private::RasterBuffer *qul_rasterBuffer_1_0_qt_2Dlogo_handle;

namespace {
struct TranslationIndexBindingFunctor {
   int operator()() const {
       Qul::Private::String id = Qul::Private::Builtins::GlobalQtObject::uiLanguage.value();
       if (id == Qul::Private::Latin1String("", 0)) return 0;
       return 0;
   }
};
static Qul::Private::Binding<int, TranslationIndexBindingFunctor, 0> translationIndexBinding((TranslationIndexBindingFunctor()), Qul::Private::BindingFunctorNoParameters());
static Qul::Property<int> translationIndex(&translationIndexBinding);
}



void AnimatingItemLayer::_onTChanged_bindingFunctor::operator()() const
{
    int r2_1;
    bool r2_2;
    QUL_DECL_UNUSED AnimatingItemLayer * r2;
    int r7__y;
    QUL_DECL_UNUSED AnimatingItemLayer * r7;
    int r8__y;
    QUL_DECL_UNUSED AnimatingItemLayer * r9_1;
    // itemLayer.y += 1
    // line 41  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self;
    r7 = r2;
    r2_1 = r2->y.value();
    r8__y = r2_1;
    r2_1 = 1;
    r2_1 = r8__y + r2_1;
    r7->y.setValue(r2_1);
    // if (itemLayer.y < -itemLayer.height || itemLayer.y >= 720)
    // line 42  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self;
    r2_1 = r2->y.value();
    r7__y = r2_1;
    r2 = self;
    r2_1 = r2->height.value();
    r2_1=-r2_1;
    r2_2 = r7__y < r2_1;
    if (r2_2) goto label_0;;
    r2 = self;
    r2_1 = r2->y.value();
    r8__y = r2_1;
    r2_1 = 720;
    r2_2 = r8__y >= r2_1;
    if (!r2_2) {
    goto label_1;};
    label_0:;
    // itemLayer.y = -itemLayer.height
    // line 43  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self;
    r9_1 = r2;
    r2 = self;
    r2_1 = r2->height.value();
    r2_1=-r2_1;
    r9_1->y.setValue(r2_1);
    {
    }
    label_1:;
    // }
    // line 44  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    return ;

}
Qul::Private::Items::LayerBase::ColorDepth AnimatingItemLayer::_depth_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2;
    bool r2_2;
    QUL_DECL_UNUSED Qul::Private::Items::LayerBase::ColorDepth r2__Bpp16Alpha;
    int r7;
    // depth: (colorDepth === 32) ? ItemLayer.Bpp32 : ItemLayer.Bpp16Alpha
    // line 31  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->colorDepth.value();
    r7 = r2;
    r2 = 32;
    r2_2 = r7 == r2;
    if (!r2_2) goto label_0;;
    r2__Bpp16Alpha = Qul::Private::Items::LayerBase::Bpp32;
    goto label_1;;
    label_0:;
    r2__Bpp16Alpha = Qul::Private::Items::LayerBase::Bpp16Alpha;
    label_1:;
    return r2__Bpp16Alpha;

}
int AnimatingItemLayer::rectangle_width_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int(self->width.value());
};
int AnimatingItemLayer::rectangle_height_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int(self->height.value());
};
int AnimatingItemLayer::textBox_height_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2__height;
    Qul::qreal r2_1;
    QUL_DECL_UNUSED rectangle_Rectangle * r2;
    int r7__height;
    int r8;
    // height: parent.height * 2 / 3
    // line 53  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->textBox.parent<rectangle_Rectangle >();
    r2__height = r2->height.value();
    r7__height = r2__height;
    r2__height = 2;
    r2__height = r7__height * r2__height;
    r8 = r2__height;
    r2__height = 3;
    r2_1 = Qul::qreal(r8) / r2__height;
    return int(r2_1);

}
int AnimatingItemLayer::textBox_width_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2__width;
    QUL_DECL_UNUSED rectangle_Rectangle * r2;
    // width: parent.width
    // line 52  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->textBox.parent<rectangle_Rectangle >();
    r2__width = r2->width.value();
    return r2__width;

}
int AnimatingItemLayer::textColumn_x_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int((int(self->textBox.width.value()) + 1)/2 - ((self->textColumn.width.value())/2));
};
int AnimatingItemLayer::textColumn_y_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
    struct marginsFunctor {
        AnimatingItemLayer *self;
        int operator()() const {
            int r2__height;
            QUL_DECL_UNUSED textBox_Rectangle * r2;
            Qul::qreal r2_2;
            int r7__height;
            // anchors.margins: parent.height / 16
            // line 59  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
            r2 = self->textColumn.parent<textBox_Rectangle >();
            r2__height = r2->height.value();
            r7__height = r2__height;
            r2__height = 16;
            r2_2 = Qul::qreal(r7__height) / r2__height;
            return int(r2_2);

        }
    } margins = { self };
        return int(margins());
};
int AnimatingItemLayer::text__x_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int((int(self->textColumn.width.value()) + 1)/2 - ((self->text_.width.value())/2));
};
Qul::Private::String AnimatingItemLayer::text__text_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2;
    QUL_DECL_UNUSED Qul::Private::String r2_1;
    int r7;
    // text: colorDepth + " bpp item layer"
    // line 62  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->colorDepth.value();
    r7 = r2;
    r2_1 = Qul::Private::String(Qul::Private::Latin1String(" bpp item layer", 15));
    r2_1 = Qul::Private::Builtins::Number(r7).toString() + r2_1;
    return r2_1;

}
int AnimatingItemLayer::text__2_x_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int((int(self->textColumn.width.value()) + 1)/2 - ((self->text__2.width.value())/2));
};
Qul::Private::String AnimatingItemLayer::text__2_text_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2__refreshInterval;
    QUL_DECL_UNUSED AnimatingItemLayer * r2_1;
    QUL_DECL_UNUSED Qul::Private::String r2;
    QUL_DECL_UNUSED Qul::Private::String r7;
    // text: "Refresh interval: " + itemLayer.refreshInterval
    // line 67  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = Qul::Private::String(Qul::Private::Latin1String("Refresh interval: ", 18));
    r7 = r2;
    r2_1 = self;
    r2__refreshInterval = r2_1->refreshInterval.value();
    r2 = r7 + Qul::Private::Builtins::Number(r2__refreshInterval).toString();
    return r2;

}
int AnimatingItemLayer::image_x_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int((int(self->rectangle.width.value()) + 1)/2 - ((self->image.width.value())/2));
};
int AnimatingItemLayer::image_y_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int(self->textBox.y.value() + self->textBox.height.value() - ((self->image.height.value())/2));
};
int AnimatingItemLayer::rectangle_2_x_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2;
    Qul::qreal r2_1;
    QUL_DECL_UNUSED rectangle_Rectangle * r2_2;
    int r7;
    Qul::qreal r8;
    int r9__width;
    // x: -redBoxWidth + t * (parent.width + redBoxWidth)
    // line 88  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->rectangle_2.redBoxWidth.value();
    r2=-r2;
    r7 = r2;
    r2_1 = self->t.value();
    r8 = r2_1;
    r2_2 = self->rectangle_2.parent<rectangle_Rectangle >();
    r2 = r2_2->width.value();
    r9__width = r2;
    r2 = self->rectangle_2.redBoxWidth.value();
    r2 = r9__width + r2;
    r2_1 = r8 * r2;
    r2_1 = r7 + r2_1;
    return int(r2_1);

}
int AnimatingItemLayer::rectangle_2_y_anchor_bindingFunctor::operator()(AnimatingItemLayer *self) const {
        return int(self->rectangle.height.value() - (self->rectangle_2.height.value()));
};
int AnimatingItemLayer::rectangle_2_height_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2;
    // height: redBoxWidth
    // line 86  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->rectangle_2.redBoxWidth.value();
    return r2;

}
int AnimatingItemLayer::rectangle_2_width_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2;
    // width: redBoxWidth
    // line 85  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->rectangle_2.redBoxWidth.value();
    return r2;

}
int AnimatingItemLayer::rectangle_2_redBoxWidth_bindingFunctor::operator()(AnimatingItemLayer *self) const
{
    (void) self;
    int r2__height;
    Qul::qreal r2_2;
    QUL_DECL_UNUSED rectangle_Rectangle * r2;
    int r7__height;
    // property int redBoxWidth: parent.height / 4
    // line 84  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    r2 = self->rectangle_2.parent<rectangle_Rectangle >();
    r2__height = r2->height.value();
    r7__height = r2__height;
    r2__height = 4;
    r2_2 = Qul::qreal(r7__height) / r2__height;
    return int(r2_2);

}

AnimatingItemLayer::AnimatingItemLayer()

    : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::ItemLayer  >(this)
    , _onTChanged_binding(_onTChanged_bindingFunctor(this), &t)
    , _depth_binding(_depth_bindingFunctor())
    , rectangle_width_anchor_binding(rectangle_width_anchor_bindingFunctor())
    , rectangle_height_anchor_binding(rectangle_height_anchor_bindingFunctor())
    , textBox_height_binding(textBox_height_bindingFunctor())
    , textBox_width_binding(textBox_width_bindingFunctor())
    , textColumn_x_anchor_binding(textColumn_x_anchor_bindingFunctor())
    , textColumn_y_anchor_binding(textColumn_y_anchor_bindingFunctor())
    , text__x_anchor_binding(text__x_anchor_bindingFunctor())
    , text__text_binding(text__text_bindingFunctor())
    , text__2_x_anchor_binding(text__2_x_anchor_bindingFunctor())
    , text__2_text_binding(text__2_text_bindingFunctor())
    , image_x_anchor_binding(image_x_anchor_bindingFunctor())
    , image_y_anchor_binding(image_y_anchor_bindingFunctor())
    , rectangle_2_x_binding(rectangle_2_x_bindingFunctor())
    , rectangle_2_y_anchor_binding(rectangle_2_y_anchor_bindingFunctor())
    , rectangle_2_height_binding(rectangle_2_height_bindingFunctor())
    , rectangle_2_width_binding(rectangle_2_width_bindingFunctor())
    , rectangle_2_redBoxWidth_binding(rectangle_2_redBoxWidth_bindingFunctor())
{
    // duration: 4000
    // line 38  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.duration.setValue(4000);
    // to: 1
    // line 37  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.to.setValue(1);
    // from: 0
    // line 36  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.from.setValue(0);
    // loops: Animation.Infinite
    // line 35  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.animationExtraProperties->loops.setValue(2147483647);
    // running: true
    // line 34  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.running.setValue(true);
    _depth_binding.init<AnimatingItemLayer, &AnimatingItemLayer::_depth_binding>();
    // depth: (colorDepth === 32) ? ItemLayer.Bpp32 : ItemLayer.Bpp16Alpha
    // line 31  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    depth.setBinding(&_depth_binding);
    // property real t: 0
    // line 29  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    t.setValue(0);
    // property int colorDepth: 32
    // line 28  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    colorDepth.setValue(32);
    // Rectangle {
    // line 45  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(this, &rectangle);
    // color: "white"
    // line 47  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle.color.setValue(0xffffffff);
    rectangle.x.setValue(int());
    rectangle_width_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_width_anchor_binding>();
    rectangle.width.setBinding(&rectangle_width_anchor_binding);
    rectangle.y.setValue(int());
    rectangle_height_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_height_anchor_binding>();
    rectangle.height.setBinding(&rectangle_height_anchor_binding);
    // Rectangle {
    // line 49  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->rectangle, &textBox);
    textBox_height_binding.init<AnimatingItemLayer, &AnimatingItemLayer::textBox_height_binding>();
    // height: parent.height * 2 / 3
    // line 53  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    textBox.height.setBinding(&textBox_height_binding);
    textBox_width_binding.init<AnimatingItemLayer, &AnimatingItemLayer::textBox_width_binding>();
    // width: parent.width
    // line 52  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    textBox.width.setBinding(&textBox_width_binding);
    textBox.y.setValue(int());
    // Column {
    // line 55  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->textBox, &textColumn);
    textColumn_x_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::textColumn_x_anchor_binding>();
    textColumn.x.setBinding(&textColumn_x_anchor_binding);
    textColumn_y_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::textColumn_y_anchor_binding>();
    textColumn.y.setBinding(&textColumn_y_anchor_binding);
    // Text {
    // line 61  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->textColumn, &text_);
    text__x_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::text__x_anchor_binding>();
    text_.x.setBinding(&text__x_anchor_binding);
    // font.pixelSize: 18
    // line 63  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    text_.font.setValue(Qul::Private::FontPointer(qul_font_default_18_50));
    text__text_binding.init<AnimatingItemLayer, &AnimatingItemLayer::text__text_binding>();
    // text: colorDepth + " bpp item layer"
    // line 62  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    text_.text.setBinding(&text__text_binding);
    // Text {
    // line 66  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->textColumn, &text__2);
    text__2_x_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::text__2_x_anchor_binding>();
    text__2.x.setBinding(&text__2_x_anchor_binding);
    // font.pixelSize: 18
    // line 68  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    text__2.font.setValue(Qul::Private::FontPointer(qul_font_default_18_50));
    text__2_text_binding.init<AnimatingItemLayer, &AnimatingItemLayer::text__2_text_binding>();
    // text: "Refresh interval: " + itemLayer.refreshInterval
    // line 67  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    text__2.text.setBinding(&text__2_text_binding);
    // Image {
    // line 74  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->rectangle, &image);
    // source: "qt-logo.png"
    // line 80  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    //image.source.setValue(Qul::Private::ImagePrivate::fromRasterBuffer(qul_rasterBuffer_1_0_qt_2Dlogo_handle));
    image.source.setValue(Qul::Private::dispatchToImageProvider("fatlocation", 11, "qt-logo.png", 11));
    // opacity: 0.8
    // line 79  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    image.opacity.setValue(0.8);
    image_x_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::image_x_anchor_binding>();
    image.x.setBinding(&image_x_anchor_binding);
    image_y_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::image_y_anchor_binding>();
    image.y.setBinding(&image_y_anchor_binding);
    // Rectangle {
    // line 83  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    appendChild(&this->rectangle, &rectangle_2);
    // color: "red"
    // line 89  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle_2.color.setValue(0xffff0000);
    rectangle_2_x_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_2_x_binding>();
    // x: -redBoxWidth + t * (parent.width + redBoxWidth)
    // line 88  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle_2.x.setBinding(&rectangle_2_x_binding);
    rectangle_2_y_anchor_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_2_y_anchor_binding>();
    rectangle_2.y.setBinding(&rectangle_2_y_anchor_binding);
    rectangle_2_height_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_2_height_binding>();
    // height: redBoxWidth
    // line 86  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle_2.height.setBinding(&rectangle_2_height_binding);
    rectangle_2_width_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_2_width_binding>();
    // width: redBoxWidth
    // line 85  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle_2.width.setBinding(&rectangle_2_width_binding);
    rectangle_2_redBoxWidth_binding.init<AnimatingItemLayer, &AnimatingItemLayer::rectangle_2_redBoxWidth_binding>();
    // property int redBoxWidth: parent.height / 4
    // line 84  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    rectangle_2.redBoxWidth.setBinding(&rectangle_2_redBoxWidth_binding);
    // NumberAnimation on t {
    // line 33  "D:/0_install/Qul1.7/QtMCUs/1.7.0/examples/layers/AnimatingItemLayer.qml"
    numberanimation.on(&t);
}