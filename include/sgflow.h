#ifndef __SGBAR_H__
#define __SGBAR_H__

#include <linux/input-event-codes.h>
#include <assert.h>
#include <GLES2/gl2.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-cursor.h>
#include <wayland-egl.h>
#include <wlr/render/egl.h>
#include <wlr/util/log.h>
#include <gdk/gdk.h>
#include <gdk/gdkwayland.h>
#include <gtk/gtk.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "wlr-input-inhibitor-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

struct sgflow_state {
	GtkApplication *application;
	GtkSearchEntry *search_entry;
	GtkFlowBox *applications_flow_box;
	GListStore *applications_collection;
	GtkWidget *window;
	struct wl_display *wl_display;
	struct wl_output *wl_output;
	struct zwlr_layer_shell_v1 *layer_shell;
	struct zwlr_layer_surface_v1 *layer_surface;
	struct wl_surface *surface;
	struct zxdg_output_manager_v1 *xdg_output_manager;
	struct zwlr_input_inhibit_manager_v1 *input_inhibit_manager;
	struct xkb_state *xkb_state;
	uint32_t width;
	uint32_t height;
};

int register_display(GdkDisplay *display, struct sgflow_state *sgflow_state);

void init_surface(GdkWindow *window, GtkWidget *gtk_window,
		struct sgflow_state *sgflow_state); 


#endif /* __SGBAR_H__ */
