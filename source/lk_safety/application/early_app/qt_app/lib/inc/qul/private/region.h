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
#ifndef QUL_REGION_H
#define QUL_REGION_H

#include <qul/private/array.h>
#include <platforminterface/rect.h>

//#define REGION_DEBUG
//#define EMIT_DEBUG

#ifdef REGION_DEBUG
#include <cstdio>
#endif

namespace Qul {
namespace Private {

struct RectNode
{
    RectNode(const PlatformInterface::Rect &rect = PlatformInterface::Rect(),
             RectNode *next = NULL,
             RectNode *prev = NULL)
        : rect(rect)
        , next(next)
        , prev(prev)
    {}

    PlatformInterface::Rect rect;
    RectNode *next;
    RectNode *prev;

    inline int left() const { return rect.x(); }
    inline int top() const { return rect.y(); }
    inline int right() const { return rect.x() + rect.width(); }
    inline int bottom() const { return rect.y() + rect.height(); }
};

template<int MaxRectCount>
class Region
{
public:
    enum RegionMode {
        StrictMonotonic, // same as QRegion
        Monotonic,
        NonMonotonic
    };

    bool computeRegion(RectNode *rectNodes, int count, RegionMode mode = NonMonotonic);

    VarLengthArray<RectNode, MaxRectCount> rects;

private:
    RectNode *sortNodes(RectNode *rects, int count);

    bool emitSpan(int x, int y, int w, int h)
    {
        if (w <= 0 || h <= 0)
            return true;

#ifdef EMIT_DEBUG
        printf("emitting width %d y span %d %d\n", w, y, y + h);
#endif

        // new row?
        assert(!m_currentRowHead || m_currentRowHead->bottom() == m_currentY + m_currentHeight);
        if (!m_currentRowHead || m_currentY + m_currentHeight != y + h) {
            if (m_lastRowHead) {
                if (!undoExtensions())
                    return false;
            }

            if (m_currentRowHead && m_currentY + m_currentHeight == y)
                m_lastRowHead = m_currentRowHead;
            else
                m_lastRowHead = NULL;

            m_currentRowHead = NULL;
            m_currentRowTail = NULL;

            m_currentY = y;
            m_currentHeight = h;
        }

        RectNode *originalLastRowHead = m_lastRowHead;

        // make sure last row is on or ahead of current span
        while (m_lastRowHead && m_lastRowHead->left() < x)
            m_lastRowHead = m_lastRowHead->next;

        bool couldExtend = false;

        RectNode *node = NULL;
        if (m_lastRowHead) {
            // able to extend?
            if (!(m_mode != NonMonotonic && originalLastRowHead != m_lastRowHead) && m_lastRowHead->left() == x
                && m_lastRowHead->right() == x + w) {
#ifdef EMIT_DEBUG
                printf("extending width %d span from bottom %d to %d\n",
                       w,
                       m_lastRowHead->bottom(),
                       m_lastRowHead->bottom() + h);
#endif
                m_lastRowHead->rect.setHeight(m_lastRowHead->rect.height() + h);
                node = m_lastRowHead;
                m_lastRowHead = m_lastRowHead->next;

                node->prev = NULL;
                node->next = NULL;

                couldExtend = true;
            }
        }

        if ((m_mode != NonMonotonic && node != originalLastRowHead) || (m_mode == StrictMonotonic && !couldExtend)) {
            if (!undoExtensions())
                return false;
            m_lastRowHead = NULL;
        }

        if (!node) {
            if (rects.isFull())
                return false;

            rects.append(RectNode(PlatformInterface::Rect(x, y, w, h)));
            node = &rects.at(rects.size() - 1);
        }

        if (m_currentRowHead) {
            assert(m_currentRowTail);
            m_currentRowTail->next = node;
            node->prev = m_currentRowTail;
            m_currentRowTail = node;
        } else {
            m_currentRowHead = node;
            m_currentRowTail = node;
        }
        return true;
    }

    bool undoExtensions()
    {
        if (m_mode == NonMonotonic)
            return true;
        for (RectNode *node = m_currentRowHead; node; node = node->next) {
            if (node->top() < m_currentY) {
                if (rects.isFull())
                    return false;
#ifdef EMIT_DEBUG
                printf("span width %d, resetting bottom from %d to %d\n",
                       node->rect.width(),
                       node->bottom(),
                       node->top() + m_currentY - node->top());
#endif
                node->rect.setHeight(m_currentY - node->top());

                rects.append(RectNode(
                    PlatformInterface::Rect(node->left(), m_currentY, node->right() - node->left(), m_currentHeight)));
                RectNode *replacement = &rects.at(rects.size() - 1);
                if (node->prev) {
                    node->prev->next = replacement;
                    replacement->prev = node->prev;
                    assert(node != m_currentRowHead);
                } else {
                    m_currentRowHead = replacement;
                }
                if (node->next) {
                    node->next->prev = replacement;
                    replacement->next = node->next;
                    assert(node != m_currentRowTail);
                } else {
                    m_currentRowTail = replacement;
                }
            }
        }
        return true;
    }

    inline void sanityCheck(RectNode *head)
    {
#ifdef REGION_DEBUG
        RectNode *node = head;
        while (node) {
            if (node != head && node->prev)
                assert(node->prev->next == node);
            if (node->next)
                assert(node->next->prev == node);
            node = node->next;
        }
#else
        QUL_UNUSED(head);
#endif
    }

    RectNode *m_lastRowHead;

    RectNode *m_currentRowHead;
    RectNode *m_currentRowTail;

    int m_currentY;
    int m_currentHeight;

    RegionMode m_mode;
};

inline bool compareBounds(const PlatformInterface::Rect &a, const PlatformInterface::Rect &b)
{
    if (a.y() < b.y())
        return true;
    if (a.y() == b.y())
        return a.x() < b.x();
    return false;
}

template<int MaxRectCount>
RectNode *Region<MaxRectCount>::sortNodes(RectNode *rects, int count)
{
    if (!count)
        return rects;

    // does insertion sort currently, assumes count is relatively low
    RectNode *head = &rects[0];
    head->next = NULL;
    head->prev = NULL;
    for (int i = 1; i < count; ++i) {
        RectNode **n = &head;
        RectNode *prev = NULL;
        while (*n && compareBounds((*n)->rect, rects[i].rect)) {
            prev = *n;
            n = &(*n)->next;
        }

        if (*n) {
            rects[i].next = *n;
            (*n)->prev = &rects[i];
        } else {
            rects[i].next = NULL;
        }
        rects[i].prev = prev;
        *n = &rects[i];
    }

    return head;
}

template<int MaxRectCount>
bool Region<MaxRectCount>::computeRegion(RectNode *rectNodes, int count, RegionMode mode)
{
    rects.clear();
    m_currentRowHead = NULL;
    m_currentRowTail = NULL;
    m_lastRowHead = NULL;
    m_mode = mode;

    if (count == 0)
        return false;

#if defined(REGION_DEBUG)
    printf("Sorting %d nodes\n", count);
    for (int i = 0; i < count; ++i) {
        printf("(%d,%d) (%d, %d)\n",
               rectNodes[i].left(),
               rectNodes[i].top(),
               rectNodes[i].right(),
               rectNodes[i].bottom());
    }
#endif

    RectNode *head = sortNodes(rectNodes, count);

    while (head) {
        int y1 = head->top();
        int y2 = head->bottom();

        RectNode *tail = head->next;
        while (tail && tail->top() == y1) {
            if (tail->bottom() < y2)
                y2 = tail->bottom();
            tail = tail->next;
        }

        if (tail && tail->top() < y2)
            y2 = tail->top();

#if defined(REGION_DEBUG)
        if (tail)
            printf("New row: %d to %d, tail: (%d, %d)-(%d, %d)\n",
                   y1,
                   y2,
                   tail->left(),
                   tail->top(),
                   tail->right(),
                   tail->bottom());
        else
            printf("New row: %d to %d, no tail\n", y1, y2);
#endif

        // now we can make spans of equal height from [head, tail)
        while (head != tail) {
            int x1 = head->left();
            int x2 = head->right();

            int nextY2 = head->bottom();

#if defined(REGION_DEBUG)
            printf("Loop: head: (%d, %d)-(%d, %d)\n", head->left(), head->top(), head->right(), head->bottom());
#endif

            RectNode *next = head->next;
            if (head->bottom() == y2) {
                head = head->next;
            }

            for (RectNode *node = next; node != tail; node = node->next) {
#if defined(REGION_DEBUG)
                printf("Span, x1: %d, x2: %d\n", node->left(), node->right());
#endif
                if (node->left() > x2) {
                    if (!emitSpan(x1, y1, x2 - x1, y2 - y1))
                        return false;
                    x1 = node->left();
                }

                if (node->right() > x2)
                    x2 = node->right();

                // done with this node?
                assert(node->bottom() >= y2);
                if (node->bottom() == y2) {
                    node->prev->next = node->next;
                    if (node->next)
                        node->next->prev = node->prev;
                    if (node == head)
                        head = node->next;

                    sanityCheck(head);
#if defined(REGION_DEBUG)
                    printf("Done with node (%d, %d)-(%d, %d)\n",
                           node->left(),
                           node->top(),
                           node->right(),
                           node->bottom());
#endif
                } else if (node->bottom() < nextY2) {
                    nextY2 = node->bottom();
                }
            }

            if (!emitSpan(x1, y1, x2 - x1, y2 - y1))
                return false;

            y1 = y2;
            y2 = nextY2;

            RectNode *node = tail;
            assert(!tail || tail->top() >= y1);
            while (tail && tail->top() == y1) {
                if (tail->bottom() < y2)
                    y2 = tail->bottom();
                tail = tail->next;
            }

            RectNode **n = &head;
            RectNode *prev = NULL;

            // reinsert new nodes in x-sorted order
            while (node != tail) {
                RectNode *next = node->next;
                while (*n != node && (*n)->left() < node->left()) {
                    prev = *n;
                    n = &(*n)->next;
                    assert(prev == (*n)->prev);
                }

                if (*n == node) {
                    // all sorted
                    break;
                } else {
                    if (next)
                        next->prev = node->prev;
                    node->prev->next = next;
                    node->next = *n;
                    (*n)->prev = node;
                    node->prev = prev;
                    *n = node;
                }

                n = &node->next;
                prev = node;
                node = next;

                sanityCheck(head);
            }

            if (tail && tail->top() < y2)
                y2 = tail->top();
        }
    }

    if (m_mode != NonMonotonic && m_lastRowHead) {
        if (!undoExtensions())
            return false;
    }

    return true;
}

} // namespace Private
} // namespace Qul

#endif // QUL_REGION_H
