/******************************************************************************
**
** Copyright (C) 2020 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Ultralite module.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
******************************************************************************/
#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qul API. It exists purely as an
// implementation detail for purpose of auto-generated code.
// This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qul/private/flickable.h>
#include <qul/private/models.h>
#include <platform/mem.h>

namespace Qul {
namespace Private {
namespace Items {

static const int QUL_LISTVIEW_MINIMUM_DELEGATES = 4;

template<typename ModelType, typename Delegate, typename ParentScope>
class ListView : public Flickable
{
    struct ModelOrLayoutChangedEvent
    {
        ModelOrLayoutChangedEvent(ListView *listView)
            : listView(listView)
        {}
        ListView *listView;
        void operator()() { listView->refreshModel(); }
    };

    struct ContentPositionChangedEvent
    {
        ContentPositionChangedEvent(ListView *listView)
            : listView(listView)
        {}
        ListView *listView;
        void operator()() { listView->contentPositionChanged(); }
    };

public:
    Property<ModelType *> model;
    Property<int> spacing;
    Property<Builtins::GlobalQtObject::Orientation> orientation QUL_PROPERTY_DEBUG_INIT("ListView::orientation");

    explicit ListView(ParentScope *scope)
        : parentScope(scope)
        , onContentPositionChanged(ContentPositionChangedEvent(this), QUL_DEPENDENCIES(&contentX, &contentY).data())
        , onModelOrLayoutChanged(ModelOrLayoutChangedEvent(this),
                                 QUL_DEPENDENCIES(&width, &height, &spacing, &orientation, &model).data())
        , onModelReset(this, refresh)
        , onDataChanged(this, dataChanged)
        , visibleDelegates(0)
        , firstVisibleIndex(0)
        , delegateHeight(0)
        , delegateWidth(0)
        , delegates(NULL)
    {}

    ~ListView() { cleanDelegates(); }

    Delegate *itemAtIndex(const int index)
    {
        int visibleIdx = index - firstVisibleIndex;
        if (visibleIdx < 0 || visibleIdx >= visibleDelegates)
            return nullptr;
        visibleIdx = (visibleIdx + firstVisibleItemOffset) % visibleDelegates;

        return delegateAt(visibleIdx);
    }

private:
    ParentScope *parentScope;

    // neither can be static: the properties already have static dependencies
    // except onModelOrLayoutChanged could maybe be split in two, if it's
    // fine that the dirty function may be called several times
    DirtyEvent<ContentPositionChangedEvent, 2> onContentPositionChanged;
    DirtyEvent<ModelOrLayoutChangedEvent, 5> onModelOrLayoutChanged;

    Slot<ListView, void()> onModelReset;
    Slot<ListView, void(int)> onDataChanged;

    static void refresh(ListView *view) { view->refreshModel(); };
    static void dataChanged(ListView *view, int idx)
    {
        int visibleIdx = idx - view->firstVisibleIndex;
        if (visibleIdx < 0 || visibleIdx >= view->visibleDelegates)
            return;
        visibleIdx = (visibleIdx + view->firstVisibleItemOffset) % view->visibleDelegates;
        Delegate *delegate = view->delegateAt(visibleIdx);
        setOptionalModelHelper(delegate, view->model.value()->get(idx));
    };

    inline int lastVisibleIndex() const { return firstVisibleIndex + visibleDelegates - 1; }

    void recreateDelegate(Delegate *delegate)
    {
        delegate->~Delegate();
        new (delegate) Delegate(parentScope);
    }

    void refreshDelegate(Delegate *delegate,
                         Builtins::GlobalQtObject::Orientation orientation,
                         ModelType *m,
                         int index,
                         int delegateDelta)
    {
        setOptionalModelHelper(delegate, m->get(index));
        setOptionalIndexHelper(delegate, index);
        if (orientation == Builtins::GlobalQtObject::Horizontal)
            delegate->x.setValue(delegateDelta * index);
        else
            delegate->y.setValue(delegateDelta * index);
        appendChild(&actualContentItem, delegate);
    }

    inline int contentPosition(Builtins::GlobalQtObject::Orientation orientation) const
    {
        return orientation == Builtins::GlobalQtObject::Horizontal ? contentX.value() : contentY.value();
    }

    inline int viewSize(Builtins::GlobalQtObject::Orientation orientation) const
    {
        return orientation == Builtins::GlobalQtObject::Horizontal ? width.value() : height.value();
    }

    inline int delegateSize(Builtins::GlobalQtObject::Orientation orientation) const
    {
        return orientation == Builtins::GlobalQtObject::Horizontal ? delegateWidth : delegateHeight;
    }

    void contentPositionChanged()
    {
        ModelType *m = model.value();
        if (!m || !visibleDelegates)
            return;

        const Builtins::GlobalQtObject::Orientation orientationValue = orientation.value();

        const int contentPos = contentPosition(orientationValue);
        const int bottom = contentPos + viewSize(orientationValue);
        const int sp = spacing.value();
        const int itemDelta = delegateSize(orientationValue) + sp;
        const int modelCount = m->count();

        // new items became visible at the top
        while (firstVisibleIndex > 0 && contentPos < ((firstVisibleIndex * itemDelta) - sp)) {
            firstVisibleItemOffset = (firstVisibleItemOffset + visibleDelegates - 1) % visibleDelegates;
            --firstVisibleIndex;
            Delegate *delegate = delegateAt(firstVisibleItemOffset);
            recreateDelegate(delegate);
            refreshDelegate(delegate, orientationValue, m, firstVisibleIndex, itemDelta);
        }

        // new items became visible at the bottom
        while (lastVisibleIndex() < modelCount - 1 && bottom > (lastVisibleIndex() + 1) * itemDelta) {
            const int offset = firstVisibleItemOffset;
            firstVisibleItemOffset = (firstVisibleItemOffset + 1) % visibleDelegates;
            ++firstVisibleIndex;
            Delegate *delegate = delegateAt(offset);
            recreateDelegate(delegate);
            refreshDelegate(delegate, orientationValue, m, lastVisibleIndex(), itemDelta);
        }
    }

    inline int computeMaxVisibleItems(int viewSize, int itemDelta, int spacing)
    {
        if (itemDelta <= 0)
            return 1;

        // adding (itemDelta - 1) is for rounding up
        return qMax(1, 1 + (viewSize - spacing + (itemDelta - 1)) / itemDelta);
    }

    void refreshModel()
    {
        ModelType *m = model.value();
        if (m) {
            onModelReset.connect(&m->modelReset);
            onDataChanged.connect(&m->dataChanged);
        }

        const int modelCount = m ? m->count() : 0;
        if (modelCount <= 0 || !parentItem()) {
            cleanDelegates();
            return;
        }

        const Builtins::GlobalQtObject::Orientation orientationValue = orientation.value();
        const int sp = spacing.value();

        // Always allocate one Delegate on the stack, to get the delegate size
        // in order to compute how many delegates are visible and thus need to be
        // dynamically allocated
        if (!visibleDelegates) {
            Delegate *firstDelegate = delegateAt(0);
            firstDelegate = new (firstDelegate) Delegate(parentScope);
            appendChild(&actualContentItem,
                        firstDelegate); // in case height or width depends on parent

            // Assume size is the same for all delegates
            delegateHeight = firstDelegate->height.value();
            delegateWidth = firstDelegate->width.value();

            firstDelegate->~Delegate();
        }

        const int itemDelta = delegateSize(orientationValue) + sp;

        ScopedValueRollback<bool> sc(allBehaviorDisabled, true);
        const int newVisibleDelegates = qMin(modelCount,
                                             computeMaxVisibleItems(viewSize(orientationValue), itemDelta, sp));
        if (visibleDelegates != newVisibleDelegates) {
            if (visibleDelegates)
                cleanDelegates();

            if (newVisibleDelegates > QUL_LISTVIEW_MINIMUM_DELEGATES) {
                const size_t size = (newVisibleDelegates - QUL_LISTVIEW_MINIMUM_DELEGATES) * sizeof(Delegate);
                delegates = reinterpret_cast<Delegate *>(Platform::qul_malloc(size));
            }

            if (delegates == NULL)
                visibleDelegates = qMin(QUL_LISTVIEW_MINIMUM_DELEGATES, newVisibleDelegates);
            else
                visibleDelegates = newVisibleDelegates;
        } else {
            destructDelegates();
        }

        firstVisibleIndex = itemDelta <= 0 ? 0
                                           : qBound(0,
                                                    contentPosition(orientationValue) / itemDelta,
                                                    modelCount - visibleDelegates);
        firstVisibleItemOffset = 0;

        const int contentSize = qMax(0, itemDelta * modelCount - sp);
        if (orientationValue == Builtins::GlobalQtObject::Horizontal) {
            actualContentItem.height.setImplicitly(delegateHeight);
            actualContentItem.width.setImplicitly(contentSize);
        } else {
            actualContentItem.height.setImplicitly(contentSize);
            actualContentItem.width.setImplicitly(delegateWidth);
        }

        for (int i = 0; i < visibleDelegates; ++i) {
            const int index = i + firstVisibleIndex;
            Delegate *delegate = delegateAt(i);
            new (delegate) Delegate(parentScope);
            refreshDelegate(delegate, orientationValue, m, index, itemDelta);
        }
    }

    inline Delegate *delegateAt(int pos)
    {
        return pos < QUL_LISTVIEW_MINIMUM_DELEGATES ? &reinterpret_cast<Delegate *>(stackDelegates)[pos]
                                                    : &delegates[pos - QUL_LISTVIEW_MINIMUM_DELEGATES];
    }

    void destructDelegates()
    {
        if (visibleDelegates > 0) {
            for (int i = visibleDelegates - 1; i >= 0; --i)
                delegateAt(i)->~Delegate();
        }
    }

    void cleanDelegates()
    {
        destructDelegates();
        Platform::qul_free(delegates);
        delegates = NULL;
        visibleDelegates = 0;
        firstVisibleIndex = 0;
        firstVisibleItemOffset = 0;
    }

    int visibleDelegates;
    int firstVisibleIndex;
    int firstVisibleItemOffset;

    int delegateHeight;
    int delegateWidth;

    QUL_DECL_ALIGN(QUL_ALIGNOF(Delegate))
    char stackDelegates[QUL_LISTVIEW_MINIMUM_DELEGATES][sizeof(Delegate)];
    Delegate *delegates;
};

} // namespace Items
} // namespace Private
} // namespace Qul
