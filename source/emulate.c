/*******************************************************************************
 * NewOswan
 * emulate.c:
 *
 * Based on the original Oswan-unix
 * Copyright (c) 2014-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <sys/mman.h>
#include <sys/time.h>

#ifndef PRETENT_DISPLAY
#define GLFW_INCLUDE_GLEXT
#define GL_SILENCE_DEPRECATION

#include <GLFW/glfw3.h>
/* "Apple" fix */
#ifndef GL_TEXTURE_RECTANGLE
#define GL_TEXTURE_RECTANGLE GL_TEXTURE_RECTANGLE_EXT
#endif
#endif /* PRETENT_DISPLAY */

#include <log.h>
#include <io.h>
#include <ws.h>
#include <wsrom.h>
#include "nec.h"
#include "necintrf.h"
#include <gpu.h>
#include <audio.h>
#include <memory.h>

char app_window_title[256];
int app_terminate = 0;

int ws_key_esc = 0;

#ifndef PRETENT_DISPLAY
/* Open GL stuffs */
typedef struct GLWindow_t GLWindow;
struct KeyArray
{
    uint8_t lastState;
    uint8_t curState;
    uint8_t debounced;
    GLFWwindow *window;
};
struct GLWindow_t
{
    struct KeyArray keyArray[512];
    GLFWwindow *windows;
    uint8_t *videoMemory;
    GLuint videoTexture;
    int WIDTH;
    int HEIGHT;
};
static GLWindow mainWindow;
static int window_num = 0;

static void ShowScreen(GLWindow *g, int w, int h)
{
    glBindTexture(GL_TEXTURE_RECTANGLE, g->videoTexture);

    // glTexSubImage2D is faster when not using a texture range
    glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV, g->videoMemory);
    glBegin(GL_QUADS);

    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);

    glTexCoord2f(0.0f, h);
    glVertex2f(-1.0f, -1.0f);

    glTexCoord2f(w, h);
    glVertex2f(1.0f, -1.0f);

    glTexCoord2f(w, 0.0f);
    glVertex2f(1.0f, 1.0f);
    glEnd();

    glFlush();
}

static void GLWindowInitEx(GLWindow *g, int w, int h)
{
    g->WIDTH = w;
    g->HEIGHT = h;
    g->videoTexture = window_num++;
}

static void setupGL(GLWindow *g, int w, int h)
{
    g->videoMemory = (uint8_t *)malloc(w * h * sizeof(uint16_t));
    memset(g->videoMemory, 0, w * h * sizeof(uint16_t));
    //Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, w, h);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glClearColor(1.0f, 0.f, 1.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE);
    glBindTexture(GL_TEXTURE_RECTANGLE, g->videoTexture);

    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, g->videoMemory);

    glDisable(GL_DEPTH_TEST);
}

void restoreGL(GLWindow *g)
{
    //Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, g->WIDTH, g->HEIGHT);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glClearColor(1.0f, 0.f, 1.0f, 1.0f);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_RECTANGLE);
    glDisable(GL_DEPTH_TEST);
}

static void kbHandler(GLFWwindow *window, int key, int scan, int action, int mod)
{
    struct KeyArray *keyArray;

    keyArray = (struct KeyArray *)glfwGetWindowUserPointer(window);

    keyArray[key].lastState = keyArray[key].curState;
    if (action == GLFW_RELEASE)
    {
        keyArray[key].curState = GLFW_RELEASE;
    }
    else
    {
        keyArray[key].curState = GLFW_PRESS;
    }
    keyArray[key].debounced |= (keyArray[key].lastState == GLFW_RELEASE) && (keyArray[key].curState == GLFW_PRESS);
    keyArray[key].window = window;
    /*printf("key:%d, state:%d debounce:%d, laststate:%d\n", key, keyArray[key].curState,
           keyArray[key].debounced, keyArray[key].lastState);*/
}

static void sizeHandler(GLFWwindow *window, int xs, int ys)
{
    glfwMakeContextCurrent(window);
    glViewport(0, 0, xs, ys);
    ShowScreen(&mainWindow, 244, 144);
}

static void error_callback(int error, const char *description)
{
    puts(description);
}

static void initDisplay(GLWindow *g)
{
    int h = g->HEIGHT;
    int w = g->WIDTH;

    /// Initialize GLFW
    glfwInit();

    glfwSetErrorCallback(error_callback);

    // Open screen OpenGL window
    if (!(g->windows = glfwCreateWindow(g->WIDTH, g->HEIGHT, "Main", NULL, NULL)))
    {
        glfwTerminate();
        Log(TLOG_PANIC, "emulate", "Window creation error...");
        abort();
    }

    glfwSetWindowAspectRatio(g->windows, 244, 144);

    glfwMakeContextCurrent(g->windows);
    setupGL(g, g->WIDTH, g->HEIGHT);

    glfwSwapInterval(1);            // We need vsync

    glfwGetWindowSize(g->windows, &w, &h);

    glfwSetWindowUserPointer(g->windows, g->keyArray);

    glfwSetKeyCallback(g->windows, kbHandler);
    glfwSetWindowSizeCallback(g->windows, sizeHandler);
}

static void clearScreen(GLWindow *g)
{
    memset(g->videoMemory, 0, sizeof(uint16_t) * g->WIDTH * g->HEIGHT);
}

static void updateScreen(GLWindow *g)
{
    /* Update windows code */
    glfwMakeContextCurrent(g->windows);
    ShowScreen(g, g->WIDTH, g->HEIGHT);
    glfwSwapBuffers(g->windows);
    glfwPollEvents();
}

static inline int getKeyState(int key)
{
    return mainWindow.keyArray[key].curState;
}

static void read_keys()
{
#if 0
    ws_key_start = 0;
    ws_key_x4 = 0;
    ws_key_x2 = 0;
    ws_key_x1 = 0;
    ws_key_x3 = 0;
    ws_key_y4 = 0;
    ws_key_y2 = 0;
    ws_key_y1 = 0;
    ws_key_y3 = 0;
    ws_key_button_a = 0;
    ws_key_button_b = 0;

    if (getKeyState(GLFW_KEY_E))
    {
        dump_memory();
    }

    if (getKeyState(GLFW_KEY_R))
    {
        Log(TLOG_DEBUG, "emulate", "Boop reset");
        ws_reset();
    }

    if (getKeyState(GLFW_KEY_ESCAPE))
    {
        ws_key_esc = 1;
    }

    if (getKeyState(GLFW_KEY_UP))
    {
        ws_key_x1 = 1;
    }

    if (getKeyState(GLFW_KEY_DOWN))
    {
        ws_key_x3 = 1;
    }

    if (getKeyState(GLFW_KEY_RIGHT))
    {
        ws_key_x2 = 1;
    }

    if (getKeyState(GLFW_KEY_LEFT))
    {
        ws_key_x4 = 1;
    }

    if (getKeyState(GLFW_KEY_ENTER))
    {
        ws_key_start = 1;
    }

    if (getKeyState(GLFW_KEY_C))
    {
        ws_key_button_a = 1;
    }

    if (getKeyState(GLFW_KEY_X))
    {
        ws_key_button_b = 1;
    }

    if (getKeyState(GLFW_KEY_W))
    {
        ws_key_y1 = 1;
    }

    if (getKeyState(GLFW_KEY_A))
    {
        ws_key_y4 = 1;
    }

    if (getKeyState(GLFW_KEY_S))
    {
        ws_key_y3 = 1;
    }

    if (getKeyState(GLFW_KEY_D))
    {
        ws_key_y2 = 1;
    }

    if (getKeyState(GLFW_KEY_O))
    {
        ws_cyclesByLine += 10;
    }

    if (getKeyState(GLFW_KEY_L))
    {
        ws_cyclesByLine -= 10;
    }
#endif
}
#else
typedef struct PseudoWindow_t
{
    uint8_t *videoMemory;
    int WIDTH;
    int HEIGHT;
} PseudoWindow_t;

static PseudoWindow_t mainWindow;

static void GLWindowInitEx(PseudoWindow_t *g, int w, int h)
{
    g->WIDTH = w;
    g->HEIGHT = h;
}

static void initDisplay(PseudoWindow_t *g)
{
    int w = g->WIDTH;
    int h = g->HEIGHT;
    g->videoMemory = (uint8_t *)malloc(w * h * sizeof(uint16_t));
    memset(g->videoMemory, 0, w * h * sizeof(uint16_t));
}

static void clearScreen(PseudoWindow_t *g)
{
    memset(g->videoMemory, 0, sizeof(uint16_t) * g->WIDTH * g->HEIGHT);
}

static void updateScreen(PseudoWindow_t *g)
{
}

static void read_keys()
{
    static uint32_t i = 0;

    ws_key_start = 0;
    ws_key_x4 = 0;
    ws_key_x2 = 0;
    ws_key_x1 = 0;
    ws_key_x3 = 0;
    ws_key_y4 = 0;
    ws_key_y2 = 0;
    ws_key_y1 = 0;
    ws_key_y3 = 0;
    ws_key_button_a = 0;
    ws_key_button_b = 0;

    if (i > 1024)
    {
        ws_key_esc = 1;
    }

    i++;
}
#endif /* PRETENT_DISPLAY */

double getTicks()
{
    struct timeval curTime;
    double ticks;
    /* Get datetime */
    gettimeofday(&curTime, NULL);

    ticks = (curTime.tv_sec * 1000.) + curTime.tv_usec / 1000.;

    return ticks;
}

void ws_emulate(void)
{
    int32_t nCount = 0;
    int i = 0;

    double dTime = 0.0, dNormalLast = 0.0, dTemp;

    // 15 bits RGB555
    //Format format(16, 0x007c00, 0x00003e0, 0x0000001f);
    GLWindowInitEx(&mainWindow, 224, 144);
    initDisplay(&mainWindow);
    clearScreen(&mainWindow);
    updateScreen(&mainWindow);
    int16_t *backBuffer = (int16_t *)mainWindow.videoMemory;


    dNormalLast = getTicks();

#define HCLK (1000. / 12000.)

    while (1)
    {

        dTemp = getTicks();
        dTime = dTemp - dNormalLast;

        nCount = (int32_t)(dTime * HCLK); // does this calculation make sense?

        if (nCount <= 0)
        {
            /* Sleep for 500us */
            usleep(500);
        } // No need to do anything for a bit
        else
        {

            dNormalLast += nCount * (1 / HCLK);

            if (nCount > 10)
            {
                nCount = 10;
            }

            read_keys();

            if (ws_key_esc)
            {
                app_terminate = 1;

                if ((ws_rom_path != NULL) || (app_terminate))
                {
                    break;
                }
            }

            /* What is this mess? Frameskip? */
            for (i = 0 ; i < nCount - 1 ; i++)
            {
                while (!ws_executeLine(backBuffer, 1))
                {
                }
            }

            while (!ws_executeLine(backBuffer, 1))
            {
            }

            updateScreen(&mainWindow);
        }
    }
}
