/*
* SEMIDRIVE Copyright Statement
* Copyright (c) SEMIDRIVE. All rights reserved
* This software and all rights therein are owned by SEMIDRIVE,
* and are protected by copyright law and other relevant laws, regulations and protection.
* Without SEMIDRIVE’s prior written consent and /or related rights,
* please do not use this software or any potion thereof in any form or by any means.
* You may not reproduce, modify or distribute this software except in compliance with the License.
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTIES OF ANY KIND, either express or implied.
* You should have received a copy of the License along with this program.
* If not, see <http://www.semidrive.com/licenses/>.
*/

#define LOG_TAG "G2D_TEST"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "DrmDisplay.h"
#include "debug.h"
#include "HwConverter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "tests/doctest.h"

HwConverter *g2d = nullptr;
HwBuffer *pic0, *pic0_out;



int setup()
{
    if (g2d)
        return 0;
    LOGI("setup first time");
    g2d = HwConverter::getHwConverter();

    pic0 = new HwBuffer(1280, 720, DRM_FORMAT_YUYV);
    pic0_out = new HwBuffer(1920, 720, DRM_FORMAT_ARGB8888);

    g2d->FillColor(pic0_out, 0x3ff, 0xff);

    g2d->Display(pic0_out);

    // fill picture with raw data
    FILE *fp = fopen("/data/yuyv422_1280_720.yuv", "rb");
    if (fp) {
        pic0->MapBo();
        fread(pic0->handle.mapped_vaddrs[0], 256, 1280 * 720 * 2 / 256, fp);
        fclose(fp);
        pic0->UnMapBo();
    }
    return 0;
}

void teardown()
{
    LOGI("press any keyt to continue...\n");
    getchar();
}

void test_alloc(void)
{
    LOGI("checking: alloc and free function...");
}

TEST_CASE("testing the memory copy speed") {
    setup();
    size_t sz = 1920 * 720 * 2;

    HwBuffer *p = new HwBuffer(1920, 720, DRM_FORMAT_YUYV);
    HwBuffer *q = new HwBuffer(1920, 720, DRM_FORMAT_YUYV);

    void *v1 = malloc(sz);
    void *v2 = malloc(sz);
    p->MapBo();
    q->MapBo();
    LOGI("v1 %p", v1);
    LOGI("v2 %p", v2);
    LOGI("p->handle.mapped_vaddrs[0] %p", p->handle.mapped_vaddrs[0]);
    LOGI("q->handle.mapped_vaddrs[0] %p", q->handle.mapped_vaddrs[0]);

    {
        CostTime c("memcpy hw to hw");
        memcpy(q->handle.mapped_vaddrs[0], p->handle.mapped_vaddrs[0], sz);
    }
    {
        CostTime c("memcpy sw to hw");
        memcpy(q->handle.mapped_vaddrs[0], v1, sz);
    }
    {
        CostTime c("memcpy hw to sw");
        memcpy(v1, p->handle.mapped_vaddrs[0], sz);
    }
    {
        CostTime c("memcpy sw to sw");
        memcpy(v1, v2, sz);
    }

    free(v1);
    free(v2);
    p->UnMapBo();
    q->UnMapBo();
    delete p;
    delete q;
    teardown();
}

TEST_CASE("testing the alloc and free function") {
    int v1 = 0, v2 = 0;
    setup();
    for(int i=0; i<1000; i++)
    {
        HwBuffer *pic = new HwBuffer(1920, 720, DRM_FORMAT_YUYV);
        if (i == 0) v1 =  pic->handle.fds[0];
        if (i == 100) v2 =  pic->handle.fds[0];
        delete pic;
    }
    teardown();
    CHECK(v1 == v2);
}

TEST_CASE("testing blit function") {
    setup();
    // blit
    LOGD("blit on output image");

    SUBCASE("first case") {
        pic0->setDisplay(0, 0, 960, 360);
        pic0_out->setDisplay(0, 0, 1920, 360);
        LOGD("%d %d, %dx%d", pic0->display.left, pic0->display.top, pic0->display.right, pic0->display.bottom);
        g2d->BlitSingle(pic0, pic0_out);

        pic0->setDisplay(0, 0, 960, 360);
        pic0_out->setDisplay(960, 0, 960 + 960, 360);
        g2d->BlitSingle(pic0, pic0_out);
        CHECK(true);
    }

    SUBCASE("second case") {
        CostTime cost("g2d cells");
        int hstep = 6, vstep = 2;
        int bw = 1920 / hstep;
        int bh = 720 / vstep;
        for (int j = 0; j < vstep; j++) {
            for (int i = 0; i < hstep; i++) {

            #if 0
                HwBuffer inputs[3] = {*pic0, *pic0, *pic0};
                inputs[0].setDisplay(0, 0, bw, bh);
                inputs[1].setDisplay(bw, 0, 2 * bw, bh);
                inputs[2].setDisplay(2 * bw, 0, 3 * bw, bh);
                pic0_out->setDisplay(i * 3 * bw, j * bh, i * 3 * bw +  3 * bw, j * bh + bh);
                HwBuffer *z_inputs[3] = {&inputs[0], &inputs[1], &inputs[2]};
                g2d->Blit((const HwBuffer **)z_inputs, pic0_out, 3);
            #else
                pic0->setDisplay(0, 0, bw, bh);
                pic0_out->setDisplay(i * bw, j * bh, i * bw + bw, j * bh + bh);
                g2d->BlitSingle(pic0, pic0_out);
            #endif
            }
        }
        CHECK(true);
    }

    SUBCASE("third case") {
        int hstep = 1, vstep = 2;
        int bw = 1920 / hstep;
        int bh = 720 / vstep;

        pic0->setDisplay(0, 0, bw, bh);
        pic0_out->setDisplay(0, 0 * bh, bw, 1 * bh);
        g2d->ConvertSingle(pic0, pic0_out);

        pic0_out->setDisplay(0, 0, 1920, 720);
        g2d->Display(pic0_out);
        CHECK(true);
    }
    teardown();
}

TEST_CASE("test performance")
{
    struct res {
        int width;
        int height;
    } test_resolutions[] = {
        {1280, 960},
        {1280, 720},
        {320, 280},
    };
    int num = 3;

    setup();
    SUBCASE("single layer format transform") {
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);

            //single format change
            HwBuffer *pic2 = new HwBuffer(r->width, r->height, DRM_FORMAT_ARGB8888);
            {
                LOGI("test %d x %d format change", r->width, r->height);
                g2d->ConvertSingle(pic1, pic2);

                for (int j = 0; j < 10; j++) {
                    CostTime c("single format change:");
                    g2d->ConvertSingle(pic1, pic2);
                }
            }

            delete pic1, pic2;
        }
    }

    SUBCASE("joining the 4 layers into 1") {
        for(int m = 0; m < num; m++) {
            struct res *r = &test_resolutions[m];
            HwBuffer *pic_srcs[4];
            int max_num = 4;
            for (int i = 0; i < max_num; i++) {
                pic_srcs[i] = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
                g2d->ConvertSingle(pic0, pic_srcs[i]);
            }

            //joining the 4 layers
            {
                LOGI("test %d x %d 4 layers joining", r->width, r->height);
                int bw = r->width;
                int bh = r->height;
                for (int n = 0; n < 10; n++) {
                    HwBuffer *pic_out = new HwBuffer(r->width, r->height * max_num, DRM_FORMAT_YUYV);
                    {
                        CostTime c("joining 4 layers:");
                        for (int i = 0, j = 0; j < max_num; j++) {
                            pic0->setDisplay(0, 0, bw, bh);
                            pic_out->setDisplay(i * bw, j * bh, i * bw + bw, j * bh + bh);
                            g2d->BlitSingle(pic_srcs[j], pic_out);
                        }
                    }
                    delete pic_out;
                }
            }

            for (int i = 0; i < max_num; i++) {
                delete pic_srcs[i];
            }
        }
    }

    SUBCASE("fastcopy tests") {
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_ARGB8888);
            g2d->ConvertSingle(pic0, pic1);
            HwBuffer *pic_out = new HwBuffer(r->width, r->height, DRM_FORMAT_ARGB8888);

            LOGI("fastcopy (%d x %d) data size 0x%x\n", r->width, r->height, pic_out->handle.size);
            for (int j = 0; j < 10; j++) {
                CostTime c("fastcopy:");
                g2d->FastCopy(pic_out->handle.fds[0], pic1->handle.fds[0], pic_out->handle.size);
            }
            delete pic1;
            delete pic_out;
        }
    }

    SUBCASE("crop 52x52") {
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);
            HwBuffer *pic2 = new HwBuffer(52, 52, DRM_FORMAT_YUYV);

            LOGI("crop 52x52 in (%d x %d)\n", r->width, r->height);
            for (int j = 0; j < 10; j++) {
                CostTime c("fastcopy:");
                pic1->setCrop(0, 0, 52, 52);
                g2d->ConvertSingle(pic1, pic2);
            }

            delete pic1;
            delete pic2;
        }
    }

    SUBCASE("resize 240x280") {
        const int rw = 240;
        const int rh = 280;
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);
            HwBuffer *pic2 = new HwBuffer(rw, rh, DRM_FORMAT_YUYV);

            LOGI("resize (%d x %d) to (%d x %d)", r->width, r->height, rw, rh);
            for (int j = 0; j < 10; j++) {
                CostTime c("fastcopy:");
                pic1->setCrop(0, 0, r->width, r->height);
                pic1->setDisplay(0, 0, rw, rh);
                pic2->setDisplay(0, 0, rw, rh);
                g2d->ConvertSingle(pic1, pic2);
            }
            delete pic1;
            delete pic2;
        }
    }

    SUBCASE("resize 128x128") {
        const int rw = 128;
        const int rh = 128;
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);
            HwBuffer *pic2 = new HwBuffer(rw, rh, DRM_FORMAT_YUYV);

            LOGI("resize (%d x %d) to (%d x %d)", r->width, r->height, rw, rh);
            for (int j = 0; j < 10; j++) {
                CostTime c("fastcopy:");
                pic1->setCrop(0, 0, r->width, r->height);
                pic1->setDisplay(0, 0, rw, rh);
                pic2->setDisplay(0, 0, rw, rh);
                g2d->ConvertSingle(pic1, pic2);
            }
            delete pic1;
            delete pic2;
        }
    }

    SUBCASE("resize 256x224") {
        const int rw = 256;
        const int rh = 224;
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);
            HwBuffer *pic2 = new HwBuffer(rw, rh, DRM_FORMAT_YUYV);

            LOGI("resize (%d x %d) to (%d x %d)", r->width, r->height, rw, rh);
            for (int j = 0; j < 10; j++) {
                CostTime c("fastcopy:");
                pic1->setCrop(0, 0, r->width, r->height);
                pic1->setDisplay(0, 0, rw, rh);
                pic2->setDisplay(0, 0, rw, rh);
                g2d->ConvertSingle(pic1, pic2);
            }
            delete pic1;
            delete pic2;
        }
    }

    SUBCASE("memcpy testing") {
        for(int i = 0; i < num; i++) {
            struct res *r = &test_resolutions[i];
            HwBuffer *pic1 = new HwBuffer(r->width, r->height, DRM_FORMAT_YUYV);
            g2d->ConvertSingle(pic0, pic1);
            void *out = malloc(pic1->handle.size);
            pic1->MapBo();
            LOGI("(%d x %d) memcpy by cpu", r->width, r->height);
            for (int j = 0; j < 10; j++) {
                CostTime c("memcpy:");
                memcpy(out, pic1->handle.mapped_vaddrs[0], pic1->handle.size);
            }
            pic1->UnMapBo();
            free(out);
            delete pic1;
        }
    }
    teardown();
}


void test_blit(HwBuffer *pic0, HwBuffer *pic0_out)
{
    // blit
    LOGD("blit on output image");
#if 0
    pic0->setDisplay(0, 0, 960, 360);
    pic0_out->setDisplay(0, 0, 1920, 360);
    LOGD("%d %d, %dx%d", pic0->display.left, pic0->display.top, pic0->display.right, pic0->display.bottom);
    g2d->BlitSingle(pic0, pic0_out);

    pic0->setDisplay(0, 0, 960, 360);
    pic0_out->setDisplay(960, 0, 960 + 960, 360);
    g2d->BlitSingle(pic0, pic0_out);

#endif

#if 1
{
    CostTime cost("g2d cells");
    int hstep = 6, vstep = 2;
    int bw = 1920 / hstep;
    int bh = 720 / vstep;
    for (int j = 0; j < vstep; j++) {
        for (int i = 0; i < hstep; i++) {

        #if 0
            HwBuffer inputs[3] = {*pic0, *pic0, *pic0};
            inputs[0].setDisplay(0, 0, bw, bh);
            inputs[1].setDisplay(bw, 0, 2 * bw, bh);
            inputs[2].setDisplay(2 * bw, 0, 3 * bw, bh);
            pic0_out->setDisplay(i * 3 * bw, j * bh, i * 3 * bw +  3 * bw, j * bh + bh);
            HwBuffer *z_inputs[3] = {&inputs[0], &inputs[1], &inputs[2]};
            g2d->Blit((const HwBuffer **)z_inputs, pic0_out, 3);
        #else
            pic0->setDisplay(0, 0, bw, bh);
            pic0_out->setDisplay(i * bw, j * bh, i * bw + bw, j * bh + bh);
            g2d->BlitSingle(pic0, pic0_out);
        #endif

        }
    }
}

#endif

#if 0
    int hstep = 1, vstep = 2;
    int bw = 1920 / hstep;
    int bh = 720 / vstep;

    pic0->setDisplay(0, 0, bw, bh);
    pic0_out->setDisplay(0, 0 * bh, bw, 1 * bh);
    g2d->ConvertSingle(pic0, pic0_out);

    pic0_out->setDisplay(0, 0, 1920, 720);
    g2d->Display(pic0_out);
#endif

}

void test_map(void)
{

    HwBuffer *pic0_out = new HwBuffer(1920, 720, DRM_FORMAT_NV21);

    int n = 10;
    while (n--) {
        pic0_out->MapBo();
        uint8_t *vaddr = (uint8_t *)pic0_out->handle.mapped_vaddrs[0];
        // memset((void *)(vaddr + 4 * n), 0xff, 4);
        LOGE("vaddr = %p", vaddr);
        if (vaddr == -1) {
            LOGE("mmap failed: %s", strerror(errno));
        } else {
            pic0_out->UnMapBo();
        }
    }

    delete pic0_out;
}

void test_scale_rotation(HwBuffer *pic0, HwBuffer *pic0_out)
{
    #if 0
        // scale
    LOGD("format scaling");
    HwBuffer *pic1 = new HwBuffer(*pic0);
    pic1->setDisplay(sdm::Rect(100, 100, 600, 600));
    pic0_out->setDisplay(sdm::Rect(100, 100, 600, 600));
    g2d->ConvertSingle(pic1, pic0_out);
    g2d->Display(pic0_out);
    delete pic1;
    // rotate
    LOGD("format and rotation changed");

    int rotate_cases[] = {
        HW_ROTATION_TYPE_NONE,
        HW_ROTATION_TYPE_ROT_90,
        HW_ROTATION_TYPE_HFLIP,
        HW_ROTATION_TYPE_VFLIP,
        HW_ROTATION_TYPE_ROT_180,
        HW_ROTATION_TYPE_ROT_270,
        HW_ROTATION_TYPE_VF_90 ,
        HW_ROTATION_TYPE_HF_90 ,
    };

    for (int i = 0; i < 8; i++) {
        HwBuffer out(1920, 720, DRM_FORMAT_ARGB8888);
        HwBuffer *pic2 = new HwBuffer(*pic0);
        // input rotation not supported?
        pic2->rotation = rotate_cases[i];
        out.rotation = rotate_cases[i];
        switch (out.rotation) {
            case HW_ROTATION_TYPE_ROT_90:
            case HW_ROTATION_TYPE_ROT_270:
            case HW_ROTATION_TYPE_VF_90:
            case HW_ROTATION_TYPE_HF_90:
                out.setDisplay(sdm::Rect(0, 0, 640, 720));
            break;
            default:
            break;
        }
        g2d->ConvertSingle(pic2, &out);

        g2d->Display(&out);
    }

    #endif
}

int fake_main(int argc, char const *argv[])
{

    g2d = HwConverter::getHwConverter();

    HwBuffer *pic0 = new HwBuffer(1280, 720, DRM_FORMAT_YUYV);
    HwBuffer *pic0_out = new HwBuffer(1920, 720, DRM_FORMAT_ARGB8888);

    g2d->FillColor(pic0_out, 0x3ff, 0xff);

    for (int i = 0; i < 100; i++) {
        g2d->Display(pic0_out);
    }

    // fill picture with raw data
    FILE *fp = fopen("/data/yuyv422_1280_720.yuv", "rb");
    if (fp) {
        pic0->MapBo();
        fread(pic0->handle.mapped_vaddrs[0], 256, 1280 * 720 * 2 / 256, fp);
        fclose(fp);
        pic0->UnMapBo();
    }
    test_alloc();

    // test_blit(pic0, pic0_out);
    // pause();
    test_map();

    return 0;
}

