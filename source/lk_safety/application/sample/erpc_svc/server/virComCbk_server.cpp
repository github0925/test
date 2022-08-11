/*
 * Generated by erpcgen 1.7.0 on Sun Feb  2 16:49:53 2020.
 *
 * AUTOGENERATED - DO NOT EDIT
 */


#include "virComCbk_server.h"
#include <new>
#include "erpc_port.h"
#include "erpc_manually_constructed.h"

#if 10700 != ERPC_VERSION_NUMBER
#error "The generated shim code version is different to the rest of eRPC code."
#endif

using namespace erpc;
using namespace std;

#if ERPC_NESTED_CALLS_DETECTION
extern bool nestingDetection;
#endif

static ManuallyConstructed<virComCbk_service> s_virComCbk_service;



// Call the correct server shim based on method unique ID.
erpc_status_t virComCbk_service::handleInvocation(uint32_t methodId, uint32_t sequence, Codec * codec, MessageBufferFactory *messageFactory)
{
    switch (methodId)
    {
        case kvirComCbk_virCan_ControllerBusOff_id:
            return virCan_ControllerBusOff_shim(codec, messageFactory, sequence);

        case kvirComCbk_virCan_SetWakeupEvent_id:
            return virCan_SetWakeupEvent_shim(codec, messageFactory, sequence);

        case kvirComCbk_virCan_RxIndication_id:
            return virCan_RxIndication_shim(codec, messageFactory, sequence);

        case kvirComCbk_virCan_TxConfirmation_id:
            return virCan_TxConfirmation_shim(codec, messageFactory, sequence);

        default:
            return kErpcStatus_InvalidArgument;
    }
}

// Server shim for virCan_ControllerBusOff of virComCbk interface.
erpc_status_t virComCbk_service::virCan_ControllerBusOff_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint8_t Controller;

    // startReadMessage() was already called before this shim was invoked.

    codec->read(&Controller);

    err = codec->getStatus();
    if (!err)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        virCan_ControllerBusOff(Controller);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (!err)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kvirComCbk_service_id, kvirComCbk_virCan_ControllerBusOff_id, sequence);

        err = codec->getStatus();
    }

    return err;
}

// Server shim for virCan_SetWakeupEvent of virComCbk interface.
erpc_status_t virComCbk_service::virCan_SetWakeupEvent_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint8_t Controller;

    // startReadMessage() was already called before this shim was invoked.

    codec->read(&Controller);

    err = codec->getStatus();
    if (!err)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        virCan_SetWakeupEvent(Controller);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (!err)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kvirComCbk_service_id, kvirComCbk_virCan_SetWakeupEvent_id, sequence);

        err = codec->getStatus();
    }

    return err;
}

// Server shim for virCan_RxIndication of virComCbk interface.
erpc_status_t virComCbk_service::virCan_RxIndication_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint16_t Hrh;
    uint32_t CanId;
    uint8_t CanDlc;
    uint8_t * CanSduPtr = NULL;

    // startReadMessage() was already called before this shim was invoked.

    codec->read(&Hrh);

    codec->read(&CanId);

    uint32_t lengthTemp_0;
    codec->startReadList(&lengthTemp_0);
    CanDlc = lengthTemp_0;
    CanSduPtr = (uint8_t *) erpc_malloc(lengthTemp_0 * sizeof(uint8_t));
    if (CanSduPtr == NULL)
    {
        codec->updateStatus(kErpcStatus_MemoryError);
    }
    for (uint32_t listCount0 = 0; listCount0 < lengthTemp_0; ++listCount0)
    {
        codec->read(&CanSduPtr[listCount0]);
    }

    err = codec->getStatus();
    if (!err)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        virCan_RxIndication(Hrh, CanId, CanDlc, CanSduPtr);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (!err)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kvirComCbk_service_id, kvirComCbk_virCan_RxIndication_id, sequence);

        err = codec->getStatus();
    }

    if (CanSduPtr)
    {
        erpc_free(CanSduPtr);
    }

    return err;
}

// Server shim for virCan_TxConfirmation of virComCbk interface.
erpc_status_t virComCbk_service::virCan_TxConfirmation_shim(Codec * codec, MessageBufferFactory *messageFactory, uint32_t sequence)
{
    erpc_status_t err = kErpcStatus_Success;

    uint16_t canTxPduId;

    // startReadMessage() was already called before this shim was invoked.

    codec->read(&canTxPduId);

    err = codec->getStatus();
    if (!err)
    {
        // Invoke the actual served function.
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = true;
#endif
        virCan_TxConfirmation(canTxPduId);
#if ERPC_NESTED_CALLS_DETECTION
        nestingDetection = false;
#endif

        // preparing MessageBuffer for serializing data
        err = messageFactory->prepareServerBufferForSend(codec->getBuffer());
    }

    if (!err)
    {
        // preparing codec for serializing data
        codec->reset();

        // Build response message.
        codec->startWriteMessage(kReplyMessage, kvirComCbk_service_id, kvirComCbk_virCan_TxConfirmation_id, sequence);

        err = codec->getStatus();
    }

    return err;
}

erpc_service_t create_virComCbk_service()
{
    s_virComCbk_service.construct();
    return s_virComCbk_service.get();
}