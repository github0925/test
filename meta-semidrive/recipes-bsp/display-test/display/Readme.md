[toc]
# tasks 2020-12-11 18:40:25 星期五

 下面的例子运行环境都在mini_console下.为了方便控制drm, 我们建立了一个DrmDisplay的类来
 管理drm相关的资源。
```cpp
class DrmFrame;
// 描述可供显示的framebuffer，我们把数据和参数放到这个buffer里传给drm
class DrmPlane;
// framebuffer需要绑定的plane
class DrmResoures;
// 描述drm的所有资源， 包括crtc, connector, planes等等
class DrmBackend;
// 进程里打开drm的句柄
```
## 例子
### 1. 双路显示，分辨率可配置。
参考[`example01`](example01.cpp)
### 2. 双路拼显，也就是一路绘制两路输出。
这里有2个方案， 一个是硬件方案， 一个是利用drm来切分buffer
硬件方案更佳。
#### 2.1 硬件方案

利用dp的combine模式， 驱动会负责切分buffer来直接送到2个屏幕上显示。
这样对外只抽象一个crtc， 所有的软件部分都不需要修改。
修改方法:

> * dts修改

```dts

&display {
	sdriv,crtc = <&crtc0>;
	status = "okay";
};

&crtc0 {
	// display a single buffer
	dpc-master = <&dp1 &dp2>;
	status = "okay";
};

```
> + 加载drm_staging里的ko或者等待ptg3.5更新


#### 2.2 软件方案
使用两个crtc来分别显示buffer的左右部分。参考实例[`example02`](example02.cpp)

`big_frame`是一个大的buffer， 通过设置crop功能来把左半边通过crtc0显示，右半边通过crtc1来显示。

### 3. 多图层叠加，支持叠加的图层来源于绘制，也支持叠加图层来源于MIPI输入。
x9芯片是支持图层叠加(blending).
dp支持最多4层合成，但是要注意的是每个硬件层的能力是不一样的。
每层的能力都可以通过drm的property来获取。
如果没有`缩放`和`旋转`特殊要求， 可以简单的通过`格式`来查找合适的图层。
```cpp
/* Semidrive planes sorted by z.
	DP
	├── plane3  -- support rgb
	├── plane2	-- support rgb
	├── plane1	-- support yuv,rgb
	└── plane0	-- support yuv,rgb
*/
```
多进程访问同一个drm是允许的， 只要 **提前规划** 好z-order就可以。使用drm property的`zpos`来设置。
`example03` 可以显示一些图层，并设置了一个固定物理地址的pool，来模拟某些硬件（比如camera)输入。
多进程访问，需要更新最新的drm ko， 版本号要大于ptg3.4.
`drm_staging`目录临时驱动模块，可以供测试用。

该用例模拟了3个进程同时访问同一个drm的情形：thread 0, thread 1都是普通层， thread 2的数据来源于某个物理地址。

### 4. 使用OPENGL绘制camera的预览数据
使用OPENGL ES1来绘制的例子，支持9路同时输出到屏幕。

### 5. 使用EGL打开双屏 （暂未支持， 正在开发中）
由于GPU厂商没有支持双屏EGL， 我们需要一些时间加紧开发。

