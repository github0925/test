#include "early_app_cfg.h"
#include <dcf_common.h>
#include "str.h"

#ifdef ENABLE_CLUSTER
void lv_cluster_init(void *token);
void lv_cluster_start(void *token);
void lv_cluster_stop(void *token);
void lv_cluster_deinit(void *token);
#endif
#ifdef ENABLE_CONTROLPANEL
void controlpanel_init(void *token);
void controlpanel_entry(void *token);
#endif

#ifdef ENABLE_TLVGL
void tlvgl_entry(void *token);
#endif

#ifdef ENABLE_QT_APP
void qul_wrapper(void* token);
#endif

void avm_init(void* token);
void avm_start(void* token);
void animation_entry(void* token);

#ifdef ENABLE_BOOTANIMATION
uso_t animation = {
    .name = "animation",
    .init = NULL,
    .entry = animation_entry,
    .property_id = DMP_ID_BA_STATUS,
};
#endif

#ifdef ENABLE_FASMAVM
uso_t fastavm = {
    .name = "fastavm",
    .init = avm_init,
    .entry = avm_start,
    .property_id = DMP_ID_AVM_STATUS,
};
#endif

#ifdef ENABLE_CLUSTER
uso_t cluster = {
    .name = "cluster",
    .init = lv_cluster_init,
    .entry = lv_cluster_start,
    .property_id = DMP_ID_CLUSTER_STATUS,
};
#endif
#ifdef ENABLE_CONTROLPANEL
uso_t control_panel = {
    .name = "control_panel",
    .init = controlpanel_init,
    .entry = controlpanel_entry,
    .property_id = DMP_ID_CP_STATUS,
};
#endif
#ifdef ENABLE_TLVGL
uso_t tlvgl = {
    .name = "tlvgl",
    .init = NULL,
    .entry = tlvgl_entry,
};
#endif

#ifdef ENABLE_QT_APP
uso_t qt_app =
{
    .name = "qt_app",
    .init = NULL,
    .entry = qul_wrapper,
};
#endif

void unified_servcie_load(void)
{
// #ifdef ENABLE_CLUSTER
//     unified_servcie_deps_chain_append(&cluster,NULL,0,0);
// #endif
// #ifdef ENABLE_CONTROLPANEL
//     unified_servcie_deps_chain_append(&control_panel,NULL,0,0);
// #endif
    // unified_servcie_deps_chain_append(&test1,NULL,0,0);
    // unified_servcie_deps_chain_append(&test3,NULL,0,0);
    // unified_servcie_deps_chain_append(&test2,&test1,ussReady,0);
    // unified_servcie_deps_chain_append(&test2,&test3,ussTerminated,0);
    // unified_servcie_deps_chain_append(&test2,&test1,ussTerminated,1);
    // unified_service_subscribe(6,ussRun,test2_observer,NULL,0);
#ifdef ENABLE_BOOTANIMATION
    if (!is_str_resume(STR_AP1))
        unified_servcie_deps_chain_append(&animation, NULL, 0, 0);

#endif
#ifdef ENABLE_FASMAVM
#if defined(ENABLE_BOOTANIMATION) && !defined(ENABLE_AVM_FIRST)

    if (!is_str_resume(STR_AP1))
        unified_servcie_deps_chain_append(&fastavm, &animation, ussTerminated, 1);
    else
        unified_servcie_deps_chain_append(&fastavm, NULL, 0, 0);

#else
    unified_servcie_deps_chain_append(&fastavm, NULL, 0, 0);
#endif
#endif

#ifdef ENABLE_CLUSTER
    unified_servcie_deps_chain_append(&cluster,NULL,0,0);
#endif

#ifdef ENABLE_CONTROLPANEL
#ifdef ENABLE_BOOTANIMATION

    if (!is_str_resume(STR_AP1))
        unified_servcie_deps_chain_append(&control_panel, &animation,
                                          ussTerminated, 1);
    else
        unified_servcie_deps_chain_append(&control_panel, NULL, 0, 0);

#else
    unified_servcie_deps_chain_append(&control_panel, NULL, 0, 0);
#endif
#endif
#ifdef ENABLE_TLVGL
    unified_servcie_deps_chain_append(&tlvgl, NULL, 0, 0);
#endif

#ifdef ENABLE_QT_APP
    unified_servcie_deps_chain_append(&qt_app,NULL,0,0);
#endif
}

