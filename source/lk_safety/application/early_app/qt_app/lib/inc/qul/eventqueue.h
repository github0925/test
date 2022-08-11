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

#include <qul/qul.h>
#include <qul/property.h>
#include <qul/private/array.h>
#include <qul/private/application.h>
#include <qul/private/global.h>
#include <qul/private/circularbuffer.h>

namespace Qul {

namespace Private {

class EventQueueInterface : public LinkedListNode<EventQueueInterface>
{
public:
    virtual void process() = 0;
};

extern EventQueueInterface *eventQueues;

void processEventQueues();

} // namespace Private

enum EventQueueOverrunPolicy {
    EventQueueOverrunPolicy_Discard,
    EventQueueOverrunPolicy_OverwriteOldest,
};

template<typename EventType_,
         EventQueueOverrunPolicy overrunPolicy = EventQueueOverrunPolicy_OverwriteOldest,
         size_t queueSize = 5>
class EventQueue : private Private::EventQueueInterface
{
public:
    EventQueue();
    ~EventQueue();

    typedef EventType_ EventType;

    void postEvent(const EventType &event);

    virtual void onQueueOverrun();
    virtual void onEvent(const EventType &event) = 0;

private:
    // Non-copyable
    EventQueue(const EventQueue &);
    EventQueue &operator=(const EventQueue &);

    void process() QUL_DECL_OVERRIDE;

    struct DoubleQueue
    {
        struct DataSet
        {
            DataSet()
                : m_seenOverrun(false)
            {}

            Private::CircularBuffer<EventType, queueSize> m_buffer;
            bool m_seenOverrun;

            void clear()
            {
                m_buffer.clear();
                m_seenOverrun = false;
            }
        };

        DoubleQueue()
            : m_dataSetIdx(0)
        {}

        // returns true if the buffer has changed from 0 to 1 elements
        bool enqueueOrDiscard(const EventType &event)
        {
            DataSet &dataSet = m_data[!m_dataSetIdx];
            if (dataSet.m_buffer.isFull()) {
                dataSet.m_seenOverrun = true;
                return false;
            }
            const bool wasEmpty = dataSet.m_buffer.isEmpty();
            dataSet.m_buffer.pushBack(event);
            return wasEmpty;
        }

        // returns true if the buffer has changed from 0 to 1 elements
        bool enqueueOrOverwrite(const EventType &event)
        {
            DataSet &dataSet = m_data[!m_dataSetIdx];
            bool wasEmpty = false;
            if (dataSet.m_buffer.isFull()) {
                dataSet.m_seenOverrun = true;
            } else if (dataSet.m_buffer.isEmpty()) {
                wasEmpty = true;
            }
            dataSet.m_buffer.pushBack(event);
            return wasEmpty;
        }

        void flush() { m_dataSetIdx = !m_dataSetIdx; }

        DataSet &data() { return m_data[m_dataSetIdx]; }

        const DataSet &data() const { return m_data[m_dataSetIdx]; }

        void reset()
        {
            m_data[0].clear();
            m_data[1].clear();
            m_dataSetIdx = 0;
        }

    private:
        volatile Private::quint8 m_dataSetIdx;
        DataSet m_data[2];
    };
    DoubleQueue m_queue;
};

template<typename EventType_, EventQueueOverrunPolicy overrunPolicy, size_t queueSize>
EventQueue<EventType_, overrunPolicy, queueSize>::EventQueue()
{
    insertInto(&Private::eventQueues);
}

template<typename EventType_, EventQueueOverrunPolicy overrunPolicy, size_t queueSize>
EventQueue<EventType_, overrunPolicy, queueSize>::~EventQueue()
{
    // the LinkedListNode destructor will remove the instance from the list
}

template<typename EventType_, EventQueueOverrunPolicy overrunPolicy, size_t queueSize>
void EventQueue<EventType_, overrunPolicy, queueSize>::postEvent(const EventType &event)
{
    // There is a statement about interrupt-safetiness in the class documentation.
    bool firstQueueEntry = false;

    switch (overrunPolicy) {
    case EventQueueOverrunPolicy_Discard:
        firstQueueEntry = m_queue.enqueueOrDiscard(event);
        break;
    case EventQueueOverrunPolicy_OverwriteOldest:
        firstQueueEntry = m_queue.enqueueOrOverwrite(event);
        break;
    }

    if (firstQueueEntry) {
        // Request immediate update so events could be dispatched asap.
        //
        // This is essential for events posted while Qul is idling. Otherwise
        // it would not wake up to process these events.
        Qul::Private::Application *instance = Qul::Private::Application::instance();
        if (instance) {
            instance->requestEventProcessing();
        }
    }
}

template<typename EventType_, EventQueueOverrunPolicy overrunPolicy, size_t queueSize>
void EventQueue<EventType_, overrunPolicy, queueSize>::process()
{
    m_queue.flush();

    typename DoubleQueue::DataSet &data = m_queue.data();
    if (data.m_seenOverrun) {
        data.m_seenOverrun = false;
        onQueueOverrun();
    }

    for (int pending = data.m_buffer.size(); pending > 0; pending--) {
        onEvent(data.m_buffer.front());
        data.m_buffer.popFront();
    }
    data.m_buffer.clear();
}

template<typename EventType_, EventQueueOverrunPolicy overrunPolicy, size_t queueSize>
void EventQueue<EventType_, overrunPolicy, queueSize>::onQueueOverrun()
{}

} // namespace Qul
