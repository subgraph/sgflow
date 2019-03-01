#include <assert.h>
#include <stddef.h>
#include <sys/mman.h>
#include <unistd.h>
#include "sgflow.h"
#include <glib.h>
#include <xkbcommon/xkbcommon.h>

static struct wl_compositor *compositor;
struct wl_registry *registry;
static struct wl_seat *seat;
static struct wl_keyboard *keyboard;
static struct wl_pointer *pointer;
struct wl_surface *cursor_surface, *input_surface;

struct xkb_context *xkb_context;
struct xkb_keymap *xkb_keymap;

static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface,
		wl_fixed_t surface_x, wl_fixed_t surface_y) {
	// Unimplemented
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, struct wl_surface *surface) {
	// Unimplemented
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
	// Unimplemented
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
		uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
	// Unimplemented
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis, wl_fixed_t value) {
	// Unimplemented
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
	// Unimplemented
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis_source) {
	// Unimplemented
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
		uint32_t time, uint32_t axis) {
	// Unimplemented
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
		uint32_t axis, int32_t discrete) {
	// Unimplemented
}

struct wl_pointer_listener pointer_listener = {
	.enter = wl_pointer_enter,
	.leave = wl_pointer_leave,
	.motion = wl_pointer_motion,
	.button = wl_pointer_button,
	.axis = wl_pointer_axis,
	.frame = wl_pointer_frame,
	.axis_source = wl_pointer_axis_source,
	.axis_stop = wl_pointer_axis_stop,
	.axis_discrete = wl_pointer_axis_discrete,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t format, int32_t fd, uint32_t size) {
	struct sgflow_state *sgflow_state = data;

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		// TODO: exit?
	}
	char *keymap_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (keymap_shm == MAP_FAILED) {
		close(fd);
		// TODO: exit?
	}
	struct xkb_keymap *keymap = xkb_keymap_new_from_string(xkb_context, 
			keymap_shm, XKB_KEYMAP_FORMAT_TEXT_V1, 0);

	munmap(keymap_shm, size);
	close(fd);
	assert(keymap);
	struct xkb_state *state = xkb_state_new(keymap);
	assert(state);

	xkb_keymap_unref(xkb_keymap);
	xkb_state_unref(sgflow_state->xkb_state);
	xkb_keymap = keymap;
	sgflow_state->xkb_state = state;
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
	// Unimplemented
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, struct wl_surface *surface) {
	// Unimplemented
}

static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
	struct sgflow_state *sgflow_state = data;

	enum wl_keyboard_key_state key_state = state;
	xkb_keysym_t sym = xkb_state_key_get_one_sym(sgflow_state->xkb_state, key + 8);
	uint32_t keycode = key_state == WL_KEYBOARD_KEY_STATE_PRESSED ?
		key + 8 : 0;
	uint32_t codepoint = xkb_state_key_get_utf32(sgflow_state->xkb_state, keycode);
	if (key_state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		switch (sym) {
			case XKB_KEY_Escape:
				g_application_quit(G_APPLICATION(sgflow_state->application));
				break;
			default:
				break;
		}
	}
}

static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
		uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
		uint32_t mods_locked, uint32_t group) {
	struct sgflow_state *sgflow_state = data;

	xkb_state_update_mask(sgflow_state->xkb_state, mods_depressed, 
			mods_latched, mods_locked, 0, 0, group);
	// Doing nothing with modifiers at the moment
}

static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
		int32_t rate, int32_t delay) {
	// Unimplemented
}

static struct wl_keyboard_listener keyboard_listener = {
	.keymap = wl_keyboard_keymap,
	.enter = wl_keyboard_enter,
	.leave = wl_keyboard_leave,
	.key = wl_keyboard_key,
	.modifiers = wl_keyboard_modifiers,
	.repeat_info = wl_keyboard_repeat_info,
};

static void seat_handle_capabilities(void *data, struct wl_seat *wl_seat,
		enum wl_seat_capability caps) {
	struct sgflow_state *sgflow_state = data;

	if ((caps & WL_SEAT_CAPABILITY_POINTER)) {
		pointer = wl_seat_get_pointer(wl_seat);
		wl_pointer_add_listener(pointer, &pointer_listener, sgflow_state);
	}
	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
		keyboard = wl_seat_get_keyboard(wl_seat);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, sgflow_state);
	}
}

static void seat_handle_name(void *data, struct wl_seat *wl_seat,
		const char *name) {
	// Unimplemented 
}

const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void handle_global(void *data, struct wl_registry *registry,
	uint32_t name, const char *interface, uint32_t version) {
	struct sgflow_state *sgflow_state = data;
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		compositor = wl_registry_bind(registry, name,
			&wl_compositor_interface, 1);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		seat = wl_registry_bind(registry, name,
				&wl_seat_interface, 1);
		wl_seat_add_listener(seat, &seat_listener, sgflow_state);
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		sgflow_state->layer_shell = wl_registry_bind(
			registry, name, &zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, zwlr_input_inhibit_manager_v1_interface.name) == 0) {
		sgflow_state->input_inhibit_manager = wl_registry_bind(
				registry, name, &zwlr_input_inhibit_manager_v1_interface, 1);
	} else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0
    	&& version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION) {
	sgflow_state->xdg_output_manager = wl_registry_bind(registry, name,
		&zxdg_output_manager_v1_interface, ZXDG_OUTPUT_V1_NAME_SINCE_VERSION);
	}	
}

static void handle_global_remove(void *data, struct wl_registry *registry,
		uint32_t name) {
	// Unimplemented 
}

static void layer_surface_configure(void *data,
         struct zwlr_layer_surface_v1 *surface,
         uint32_t serial, uint32_t width, uint32_t height) {
	struct sgflow_state *sgflow_state = data;

	sgflow_state->width = width;
	sgflow_state->height = height;
	zwlr_layer_surface_v1_ack_configure(surface, serial);
	// TODO: Maybe move this
	zwlr_input_inhibit_manager_v1_get_inhibitor(sgflow_state->input_inhibit_manager);
	wl_display_roundtrip(sgflow_state->wl_display);
	gtk_widget_show_all(sgflow_state->window);
}

static void layer_surface_closed(void *_output,
         struct zwlr_layer_surface_v1 *surface) {
	// Unimplemented
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

int register_display(GdkDisplay *display, 
		struct sgflow_state *sgflow_state) {
	// TODO: Move xkb_context init somewhere else
	xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	sgflow_state->wl_display = gdk_wayland_display_get_wl_display(display);
	registry = wl_display_get_registry(sgflow_state->wl_display);
	assert(registry);
	static const struct wl_registry_listener registry_listener = {
		.global = handle_global,
		.global_remove = handle_global_remove,
  	};
	wl_registry_add_listener(registry, &registry_listener, sgflow_state);
	wl_display_roundtrip(sgflow_state->wl_display);
	return 0;
}

void init_surface(GdkWindow *window, GtkWidget *gtk_window, 
		struct sgflow_state *sgflow_state) { 
	gdk_wayland_window_set_use_custom_surface(window);
	
	sgflow_state->surface = gdk_wayland_window_get_wl_surface(window);
	assert(sgflow_state->surface &&  sgflow_state->layer_shell && 
			sgflow_state->xdg_output_manager);
	
	sgflow_state->layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		sgflow_state->layer_shell, sgflow_state->surface, sgflow_state->wl_output, 
			ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "menu");
	assert(sgflow_state->layer_surface);
		
	zwlr_layer_surface_v1_add_listener(sgflow_state->layer_surface,
                 &layer_surface_listener, sgflow_state);

	zwlr_layer_surface_v1_set_size(sgflow_state->layer_surface, 0, 0);
	uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP | ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |  
		ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT | ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
 	zwlr_layer_surface_v1_set_anchor(sgflow_state->layer_surface, anchor);
 	zwlr_layer_surface_v1_set_margin(sgflow_state->layer_surface, 0, 0, 0, 100);
   	zwlr_layer_surface_v1_set_exclusive_zone(sgflow_state->layer_surface, -1);
	zwlr_layer_surface_v1_set_keyboard_interactivity(sgflow_state->layer_surface, TRUE);

	wl_surface_commit(sgflow_state->surface);
	wl_display_roundtrip(sgflow_state->wl_display);
}

