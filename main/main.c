/*
 * @Descripttion :
 * @version      :
 * @Author       : Kevincoooool
 * @Date         : 2021-09-04 16:11:59
 * @LastEditors  : Kevincoooool
 * @LastEditTime : 2021-09-04 18:05:01
 * @FilePath     : \lvgl_v8\main\main.c
 */
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"
#include "lv_examples/src/lv_demo_music/lv_demo_music.h"
#include "lv_examples/src/lv_demo_benchmark/lv_demo_benchmark.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"

#include "lvgl/examples/lv_examples.h"

LV_IMG_DECLARE(astronauts0) // 包含图片资源文件
LV_IMG_DECLARE(astronauts1) // 包含图片资源文件
LV_IMG_DECLARE(astronauts2) // 包含图片资源文件

static const lv_img_dsc_t *astronauts_anim_imgs[3] = {
    &astronauts0,
    &astronauts1,
    &astronauts2,
};

void astronauts_animing(void)
{
   lv_obj_t *animimg1 = lv_animimg_create(lv_scr_act());                       // 创建动画对象
   lv_obj_center(animimg1);                                                    // 将对象置于屏幕中央
   lv_animimg_set_src(animimg1, (lv_img_decoder_t **)astronauts_anim_imgs, 3); // 加载动画资源
   lv_animimg_set_duration(animimg1, 500);                                     // 创建动画时间
   lv_animimg_set_repeat_count(animimg1, LV_ANIM_REPEAT_INFINITE);             // 设置一直重复时间
   lv_animimg_start(animimg1);
}

static void lv_tick_task(void *arg)
{
   (void)arg;
   lv_tick_inc(portTICK_PERIOD_MS);
}
SemaphoreHandle_t xGuiSemaphore;

static void gui_task(void *arg)
{
   xGuiSemaphore = xSemaphoreCreateMutex();
   lv_init();          // lvgl内核初始化
   lvgl_driver_init(); // lvgl显示接口初始化

   /* Example for 1) */
   static lv_disp_draw_buf_t draw_buf;
   lv_color_t *buf1 = heap_caps_malloc(DLV_HOR_RES_MAX * DLV_VER_RES_MAX * sizeof(lv_color_t), MALLOC_CAP_DMA);
   lv_color_t *buf2 = heap_caps_malloc(DLV_HOR_RES_MAX * DLV_VER_RES_MAX * sizeof(lv_color_t), MALLOC_CAP_DMA);

   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, DLV_HOR_RES_MAX * DLV_VER_RES_MAX); /*Initialize the display buffer*/

   static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
   lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
   disp_drv.draw_buf = &draw_buf;         /*Set an initialized buffer*/
   disp_drv.flush_cb = disp_driver_flush; /*Set a flush callback to draw to the display*/
   disp_drv.hor_res = 80;                 /*Set the horizontal resolution in pixels*/
   disp_drv.ver_res = 160;                /*Set the vertical resolution in pixels*/
   lv_disp_drv_register(&disp_drv);       /*Register the driver and save the created display objects*/

   esp_register_freertos_tick_hook(lv_tick_task);
   // lv_demo_widgets();
   // lv_demo_music();
   astronauts_animing();
   // lv_example_animimg_1();
   // lv_demo_benchmark();
   while (1)
   {
      /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
      vTaskDelay(pdMS_TO_TICKS(10));

      /* Try to take the semaphore, call lvgl related function on success */
      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {
         lv_timer_handler();
         xSemaphoreGive(xGuiSemaphore);
      }
   }
}
void app_main(void)
{

   xTaskCreatePinnedToCore(gui_task, "gui task", 1024 * 3, NULL, 1, NULL, 0);
}