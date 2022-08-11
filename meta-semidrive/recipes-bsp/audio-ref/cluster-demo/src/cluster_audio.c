#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "alsa/asoundlib.h"
// #define LOG_ON
#ifdef LOG_ON
#define AALOGV(fmt, ...)                                                       \
    printf("V %d %s: " fmt " .\n", __LINE__, __func__, ##__VA_ARGS__)
#else
#define AALOGV(fmt, ...) ;
#endif

#define AALOGI(fmt, ...)                                                       \
    printf("I %d %s: " fmt " .\n", __LINE__, __func__, ##__VA_ARGS__)
#define AALOGE(fmt, ...)                                                       \
    printf("E %d %s: " fmt " .\n", __LINE__, __func__, ##__VA_ARGS__)

typedef struct am_rpc {
    snd_mixer_t *handle;
    char card[128];
} am_rpc_t;
am_rpc_t audiomanager;
static int parse_simple_id(const char *str, snd_mixer_selem_id_t *sid)
{
    int c, size;
    char buf[128];
    char *ptr = buf;

    while (*str == ' ' || *str == '\t')
        str++;
    if (!(*str))
        return -EINVAL;
    size = 1; /* for '\0' */
    if (*str != '"' && *str != '\'') {
        while (*str && *str != ',') {
            if (size < (int)sizeof(buf)) {
                *ptr++ = *str;
                size++;
            }
            str++;
        }
    } else {
        c = *str++;
        while (*str && *str != c) {
            if (size < (int)sizeof(buf)) {
                *ptr++ = *str;
                size++;
            }
            str++;
        }
        if (*str == c)
            str++;
    }
    if (*str == '\0') {
        snd_mixer_selem_id_set_index(sid, 0);
        *ptr = 0;
        goto _set;
    }
    if (*str != ',')
        return -EINVAL;
    *ptr = 0; /* terminate the string */
    str++;
    if (!isdigit(*str))
        return -EINVAL;
    snd_mixer_selem_id_set_index(sid, atoi(str));
_set:
    snd_mixer_selem_id_set_name(sid, buf);
    return 0;
}
static int get_enum_item_index(snd_mixer_elem_t *elem, char **ptrp)
{
    char *ptr = *ptrp;
    int items, i, len;

    /* See snd_ctl_elem_init_enum_names() in sound/core/control.c. */
    char name[64];

    items = snd_mixer_selem_get_enum_items(elem);
    if (items <= 0)
        return -1;

    for (i = 0; i < items; i++) {
        if (snd_mixer_selem_get_enum_item_name(elem, i, sizeof(name) - 1,
                                               name) < 0)
            continue;

        len = strlen(name);
        if (!strncmp(name, ptr, len)) {
            if (!ptr[len] || ptr[len] == ',' || ptr[len] == '\n') {
                ptr += len;
                *ptrp = ptr;
                return i;
            }
        }
    }
    return -1;
}

static int sset_enum(snd_mixer_elem_t *elem, char *value)
{
    unsigned int item = 0;
    int check_flag = -1;

    char *ptr = value;
    while (*ptr) {
        int ival = get_enum_item_index(elem, &ptr);
        if (ival < 0)
            return check_flag;
        if (snd_mixer_selem_set_enum_item(elem, item++, ival) >= 0)
            check_flag = 1;
        /* skip separators */
        while (*ptr == ',' || isspace(*ptr))
            ptr++;
    }
    return check_flag;
}
static int sset(char *ctl, char *value, int keep_handle)
{
    int err = 0;
    am_rpc_t *rpc = &audiomanager;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);

    AALOGI("ctl: %s, value: %s", ctl, value);
    if (parse_simple_id(ctl, sid)) {
        AALOGE("Wrong scontrol identifier: %s", ctl);
        return 1;
    }

    if (rpc->handle == NULL) {
        if ((err = snd_mixer_open(&rpc->handle, 0)) < 0) {
            AALOGE("Mixer %s open error: %s", rpc->card, snd_strerror(err));
            return err;
        }
        if ((err = snd_mixer_attach(rpc->handle, rpc->card)) < 0) {
            AALOGE("Mixer attach %s error: %s", rpc->card, snd_strerror(err));
            snd_mixer_close(rpc->handle);
            rpc->handle = NULL;
            return err;
        }
        if ((err = snd_mixer_selem_register(rpc->handle, NULL, NULL)) < 0) {
            AALOGE("Mixer register error: %s", snd_strerror(err));
            snd_mixer_close(rpc->handle);
            rpc->handle = NULL;
            return err;
        }
        err = snd_mixer_load(rpc->handle);
        if (err < 0) {
            AALOGE("Mixer %s load error: %s", rpc->card, snd_strerror(err));
            snd_mixer_close(rpc->handle);
            rpc->handle = NULL;
            return err;
        }
    }
    elem = snd_mixer_find_selem(rpc->handle, sid);
    if (!elem) {

        AALOGE("Unable to find simple control '%s',%i\n",
               snd_mixer_selem_id_get_name(sid),
               snd_mixer_selem_id_get_index(sid));
        snd_mixer_close(rpc->handle);
        rpc->handle = NULL;
        return -ENOENT;
    }
    /* enum control */
    if (snd_mixer_selem_is_enumerated(elem)) {
        AALOGI("Set enum");
        err = sset_enum(elem, value);
    } else {
        if (snd_mixer_selem_has_playback_switch(elem)) {
            int mute = atoi(value);
            AALOGI("Set bool: %d\n", mute);
            snd_mixer_selem_set_playback_switch(elem, SND_MIXER_SCHN_FRONT_LEFT,
                                                mute);
        } else {
            int val = atoi(value);
            AALOGI("Set integer: %d\n", val);
            err = snd_mixer_selem_set_playback_volume(
                elem, SND_MIXER_SCHN_FRONT_LEFT, val);
        }
    }
    if (!err)
        goto done;
    if (err < 0) {
        AALOGE("Invalid command!");
        goto done;
    }

done:
    if (!keep_handle) {
        snd_mixer_close(rpc->handle);
        rpc->handle = NULL;
    }
    return err < 0 ? 1 : 0;
}
static void usage(const char *cmd)
{
    printf("This is audiomanager client demo\n");
    printf("command options:\n");
    printf("%s -h\t\t\t:Show this help info\n", cmd);
    printf("%s -l [list]\t\t\t:Show all amixer ctrl\n", cmd);
    printf("%s \"Ctrl Name\" \"Value\"       :Set ctrl-value\n\n", cmd);
    printf("example:\n %s \"Start Path\" \"hifip\" \n", cmd);
    printf("%s \"Hifi volume\" \"74\" \n", cmd);
    exit(0);
}
int main(int argc, char *argv[])
{
    char ctl[128];
    int opt;
    char value[128];
    if (1 == argc)
        usage("cluster_audio");
    while ((opt = getopt(argc, argv, "h:l")) != -1) {
        switch (opt) {
        case 'h':
            usage("cluster_audio");
            break;
        case 'l':
            system("amixer -D am_rpc");
            return 0;
        default:
            usage("cluster_audio");
            break;
        }
    }
    if (3 == argc) {
        strcpy(audiomanager.card, "am_rpc"); // define by asound.conf
        strcpy(ctl, argv[1]);
        strcpy(value, argv[2]);
        if (sset(ctl, value, 0))
            return -1;
    }
    return 0;
}
