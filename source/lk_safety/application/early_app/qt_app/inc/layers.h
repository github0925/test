// This code is auto-generated by Qul's qmltocpp tool from the file 'layers.qml'
// WARNING! All changes made in this file will be lost!
// NOTE: This generated API is to be considered implementation detail for Qul.
//       It may change from version to version and should not be relied upon.
#pragma once
#include "AnimatingItemLayer.h"
#include "MovingImageLayer.h"
#include <qul/image.h>
#include <qul/private/items.h>
#include <qul/private/layers/layers.h>
#include <qul/private/qulrcc_lookup.h>
#include <qul/private/unicodestring.h>
#include <qul/signal.h>

struct layers : Qul::Private::Items::Application , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::Application  > {
    layers();


    Qul::Property<Qul::Private::String> state;
    struct animatingitemlayer_AnimatingItemLayer : AnimatingItemLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<AnimatingItemLayer  > {
        animatingitemlayer_AnimatingItemLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<AnimatingItemLayer  >(this)
        {}
    } animatingitemlayer;
    struct move1_MovingImageLayer : MovingImageLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  > {
        move1_MovingImageLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  >(this)
        {}
    } move1;
    struct move2_MovingImageLayer : MovingImageLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  > {
        move2_MovingImageLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  >(this)
        {}
    } move2;
    struct spriteLayer32bpp_SpriteLayer : Qul::Private::Items::SpriteLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::SpriteLayer  > {
        spriteLayer32bpp_SpriteLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::SpriteLayer  >(this)
        {}
    } spriteLayer32bpp;
    struct imagelayer_ImageLayer : Qul::Private::Items::ImageLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::ImageLayer  > {
        imagelayer_ImageLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::ImageLayer  >(this)
        {}
    } imagelayer;
    struct animatingitemlayer_2_AnimatingItemLayer : AnimatingItemLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<AnimatingItemLayer  > {
        animatingitemlayer_2_AnimatingItemLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<AnimatingItemLayer  >(this)
        {}
    } animatingitemlayer_2;
    struct move3_MovingImageLayer : MovingImageLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  > {
        move3_MovingImageLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<MovingImageLayer  >(this)
        {}
    } move3;
    struct spriteLayer16bpp_SpriteLayer : Qul::Private::Items::SpriteLayer , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::SpriteLayer  > {
        spriteLayer16bpp_SpriteLayer()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::SpriteLayer  >(this)
        {}
    } spriteLayer16bpp;
    struct screen_Screen : Qul::Private::Items::Screen , public Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::Screen  > {
        screen_Screen()
            : Qul::Private::Items::ItemBase ::ExtraStorage_itemExtraProperties<Qul::Private::Items::Screen  >(this)
        {}
        Qul::Property<int> boxWidth;
        Qul::Property<int> boxHeight;
        Qul::Property<int> marginWidth;
    } screen;
    struct screen_marginWidth_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(screen.marginWidth), screen_marginWidth_bindingFunctor, 0> screen_marginWidth_binding;
    struct screen_boxHeight_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(screen.boxHeight), screen_boxHeight_bindingFunctor, 0> screen_boxHeight_binding;
    struct screen_boxWidth_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(screen.boxWidth), screen_boxWidth_bindingFunctor, 0> screen_boxWidth_binding;
    struct spriteLayer32bpp_width_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,spriteLayer32bpp_width_anchor_bindingFunctor, 1> spriteLayer32bpp_width_anchor_binding;
    struct spriteLayer32bpp_height_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,spriteLayer32bpp_height_anchor_bindingFunctor, 1> spriteLayer32bpp_height_anchor_binding;
    struct animatingitemlayer_x_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,animatingitemlayer_x_anchor_bindingFunctor, 2> animatingitemlayer_x_anchor_binding;
    struct animatingitemlayer_height_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(animatingitemlayer.height), animatingitemlayer_height_bindingFunctor, 0> animatingitemlayer_height_binding;
    struct animatingitemlayer_width_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(animatingitemlayer.width), animatingitemlayer_width_bindingFunctor, 0> animatingitemlayer_width_binding;
    struct move1_container_bindingFunctor {
        Qul::Private::Items::ItemBase * operator()(layers *self) const;
    };
    Qul::Private::Binding<Qul::Private::Items::ItemBase *, move1_container_bindingFunctor, 0> move1_container_binding;
    struct move2_container_bindingFunctor {
        Qul::Private::Items::ItemBase * operator()(layers *self) const;
    };
    Qul::Private::Binding<Qul::Private::Items::ItemBase *, move2_container_bindingFunctor, 0> move2_container_binding;
    struct spriteLayer16bpp_depth_bindingFunctor {
        Qul::Private::Items::LayerBase::ColorDepth operator()(layers *self) const;
    };
    Qul::Private::Binding<Qul::Private::Items::LayerBase::ColorDepth, spriteLayer16bpp_depth_bindingFunctor, 0> spriteLayer16bpp_depth_binding;
    struct spriteLayer16bpp_width_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,spriteLayer16bpp_width_anchor_bindingFunctor, 1> spriteLayer16bpp_width_anchor_binding;
    struct spriteLayer16bpp_height_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,spriteLayer16bpp_height_anchor_bindingFunctor, 1> spriteLayer16bpp_height_anchor_binding;
    struct animatingitemlayer_2_depth_bindingFunctor {
        Qul::Private::Items::LayerBase::ColorDepth operator()(layers *self) const;
    };
    Qul::Private::Binding<Qul::Private::Items::LayerBase::ColorDepth, animatingitemlayer_2_depth_bindingFunctor, 0> animatingitemlayer_2_depth_binding;
    struct animatingitemlayer_2_x_anchor_bindingFunctor {
    int operator()(layers *self) const;
};
    Qul::Private::Binding<int,animatingitemlayer_2_x_anchor_bindingFunctor, 0> animatingitemlayer_2_x_anchor_binding;
    struct animatingitemlayer_2_height_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(animatingitemlayer_2.height), animatingitemlayer_2_height_bindingFunctor, 0> animatingitemlayer_2_height_binding;
    struct animatingitemlayer_2_width_bindingFunctor {
        int operator()(layers *self) const;
    };
    Qul::Private::Binding<QUL_PROPERTY_TYPE(animatingitemlayer_2.width), animatingitemlayer_2_width_bindingFunctor, 0> animatingitemlayer_2_width_binding;
    struct move3_container_bindingFunctor {
        Qul::Private::Items::ItemBase * operator()(layers *self) const;
    };
    Qul::Private::Binding<Qul::Private::Items::ItemBase *, move3_container_bindingFunctor, 0> move3_container_binding;
};