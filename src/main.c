#include <gio/gdesktopappinfo.h>
#include <gio/gio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include "sgflow.h"
#include "result.h"

static struct sgflow_state sgflow_state;

// NOTE: Apps with the Terminal=true key only launch if a supported terminal
// app is installed (see gdesktopappinfo.c implementation in glib). Currently
// supported terminals are: gnome-terminal, nxterm, color-xterm, rxvt, dtterm,
// and xterm
static void launch_application(Result *result, struct sgflow_state *sgflow_state) {
	GError *error = NULL;
	const gchar *category = result_get_category(result);
	const gchar *app_id = result_get_id(result);
	const gchar *display_name = result_get_displayname(result);
	if (g_strcmp0("APPLICATION", category) == 0) {
		g_debug("Launching application: %s\n", display_name);
		GDesktopAppInfo *desktop_app_info = g_desktop_app_info_new(app_id);
		gboolean launched = g_desktop_app_info_launch_uris_as_manager(
				desktop_app_info, NULL, NULL, G_SPAWN_SEARCH_PATH | 
				G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_LEAVE_DESCRIPTORS_OPEN, 
				NULL, NULL, NULL, NULL, &error);

		if (error != NULL) {
			g_log(NULL, G_LOG_LEVEL_ERROR, "%s: %s", __func__, error->message);
			g_error_free(error);
		}	

		if (launched) {
			if (category != NULL) {
				g_free((gpointer*) category);
			}
			if (app_id != NULL) {
				g_free((gpointer*) app_id);
			}
			if (display_name != NULL) {
				g_free((gpointer*) display_name);
			}

			GtkWindow *active_window = 
				gtk_application_get_active_window(sgflow_state->application);

			// This is to make the window disappear and then give a moment to launch
			// an app before quitting (I'm talking to you, Nautilus)
			gtk_widget_hide(GTK_WIDGET(active_window));	
			gdk_window_process_all_updates();
			sleep(2);
			g_object_unref(desktop_app_info);
			g_application_quit(G_APPLICATION(sgflow_state->application));
		}
	} else {
		g_log(NULL, G_LOG_LEVEL_WARNING, 
				"%s: selected result is not an application: %s - %s", __func__,
				display_name, category);

		if (category != NULL) {
			g_free((gpointer*) category);
		}
		if (app_id != NULL) {
			g_free((gpointer*) app_id);
		}
		if (display_name != NULL) {
			g_free((gpointer*) display_name);
		}
	}
}

static void applications_child_activated(GtkFlowBox *flow_box, 
		GtkFlowBoxChild *child, gpointer user_data) {
	GListStore *collection = G_LIST_STORE(user_data);
	gint selected = gtk_flow_box_child_get_index(child);
    GtkFlowBoxChild *selected_child = gtk_flow_box_get_child_at_index(flow_box, 
			selected);
	Result *result = g_list_model_get_item(G_LIST_MODEL(collection), selected);
	if (result != NULL && IS_RESULT(result)) {
		launch_application(result, &sgflow_state);	
	}
};

static void select_first_child(struct sgflow_state *sgflow_state) {
	guint n_items = g_list_model_get_n_items(
	G_LIST_MODEL(sgflow_state->applications_collection));
	gtk_flow_box_unselect_all(sgflow_state->applications_flow_box);
	for (guint i = 0; i < n_items; i++) { 
		GtkFlowBoxChild *child = gtk_flow_box_get_child_at_index(
				sgflow_state->applications_flow_box, i);
		gboolean visible = gtk_widget_get_visible(GTK_WIDGET(child)) && gtk_widget_get_child_visible(GTK_WIDGET(child));
		Result *result = g_list_model_get_item(G_LIST_MODEL(sgflow_state->applications_collection), i);
		if (visible) {
			gtk_flow_box_select_child(sgflow_state->applications_flow_box, child);
			return;
		} 
	}
}

static void search_entry_changed(GtkSearchEntry *entry, gpointer user_data) {
	struct sgflow_state *sgflow_state = user_data;
	gtk_flow_box_invalidate_filter(sgflow_state->applications_flow_box);
	select_first_child(sgflow_state);
}

static void search_entry_activate(GtkEntry *entry, gpointer user_data) {
	struct sgflow_state *sgflow_state = user_data;
	gint selected;
	
	GList *selected_children = gtk_flow_box_get_selected_children(sgflow_state->applications_flow_box);
	GtkFlowBoxChild *selected_child = g_list_nth_data(selected_children, 0);
	selected = gtk_flow_box_child_get_index(selected_child);

	Result *result = g_list_model_get_item(G_LIST_MODEL(sgflow_state->applications_collection), selected);
	if (result != NULL && IS_RESULT(result)) {
		launch_application(result, sgflow_state);	
	}
}

static int applications_filter_func(GtkFlowBoxChild *child, gpointer user_data) {
	struct sgflow_state *sgflow_state = user_data;
	const char *query;
	GString *value;
	gboolean visible;

	GtkWidget *box = gtk_bin_get_child(GTK_BIN(child));
	GList *box_children = gtk_container_get_children(GTK_CONTAINER(box));
	GtkWidget *label = g_list_nth_data(box_children, 1);
	GTK_ENTRY(sgflow_state->search_entry);
	query = gtk_entry_get_text(GTK_ENTRY(sgflow_state->search_entry));

	gint index = gtk_flow_box_child_get_index(child);
	Result *result = g_list_model_get_item(G_LIST_MODEL(sgflow_state->applications_collection), index);
		
	const gchar *display_name = result_get_displayname(result);
	const gchar *category = result_get_category(result);
	const gchar *keywords = result_get_keywords(result);
	visible = g_str_match_string(query, display_name, FALSE) || 
				g_str_match_string(query, category, FALSE) ||
				g_str_match_string(query, keywords, FALSE);

	if (box_children != NULL) {
		g_list_free_1(box_children);
	}
	if (display_name != NULL) {
		g_free((gpointer*) display_name);
	}
	if (category != NULL) {
		g_free((gpointer*) category);
	}
	if (keywords != NULL) {
		g_free((gpointer*) keywords);
	}

	return visible;
}

GdkPixbuf *get_icon_from_app_info(GDesktopAppInfo *app_info) {
	GdkPixbuf *icon;

	const char *display_name = g_app_info_get_display_name(G_APP_INFO(app_info));
	char *default_icon = "applications-other";
	GtkIconTheme *default_icon_theme = gtk_icon_theme_get_default();
	GIcon *app_info_icon = g_app_info_get_icon(G_APP_INFO(app_info));
	GError *error = NULL;

	if G_IS_THEMED_ICON(app_info_icon) {
		const char *icon_name = g_themed_icon_get_names(G_THEMED_ICON(app_info_icon))[0]; 
		icon = gtk_icon_theme_load_icon(default_icon_theme, icon_name, 
				48, GTK_ICON_LOOKUP_FORCE_SIZE, &error);	
		if (error != NULL) {
			icon = NULL;
			g_log(NULL, G_LOG_LEVEL_WARNING, "%s: %s for: %s", __func__, 
					error->message, display_name);
		}
	} else if G_IS_FILE_ICON(app_info_icon) {
		GFile *file = g_file_icon_get_file(G_FILE_ICON(app_info_icon));
		GFileInfo *info = g_file_query_info (file, 
			G_FILE_ATTRIBUTE_STANDARD_SYMBOLIC_ICON, 
			G_FILE_QUERY_INFO_NONE, NULL, NULL);

		GIcon *themed_icon = g_file_info_get_symbolic_icon(info);
		if (themed_icon != NULL) {
			const gchar *icon_name;
			
			icon_name = g_themed_icon_get_names(G_THEMED_ICON(themed_icon))[0];
			
			if (icon_name != NULL) {
				icon = gtk_icon_theme_load_icon(default_icon_theme, icon_name,
				48, GTK_ICON_LOOKUP_FORCE_SIZE, &error);
				if (error != NULL) {
					icon = NULL;
					g_log(NULL, G_LOG_LEVEL_WARNING, "%s: %s (%s)", __func__,
						error->message, display_name);
				}
			} else {
				icon = NULL;
			}
		} else {
			icon = NULL;
		}
		
		if (file) {
			g_object_unref(file);
		}
		if (info) {
			g_object_unref(info);
		}
	} else {
		icon = NULL;
		g_log(NULL, G_LOG_LEVEL_WARNING, "%s: unhandled icon type for: %s\n",
				__func__, display_name);
	}
	
	if (icon == NULL) {	
		g_log(NULL, G_LOG_LEVEL_INFO, "Could not create icon for %s, using the default icon\n",
				display_name);
		icon = gtk_icon_theme_load_icon(default_icon_theme, default_icon, 
				48, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
	}

	if (error != NULL) {
		g_error_free(error);
	}

	return icon;
}

static GtkWidget *create_applications_flow_box(gpointer item, 
		gpointer user_data) {
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
	GdkPixbuf *icon = result_get_icon(item); 
	GtkWidget *image = gtk_image_new_from_pixbuf(icon);
	GtkWidget *label = gtk_label_new(result_get_displayname(item));
	gtk_label_set_max_width_chars(GTK_LABEL(label), 20);
	gtk_box_pack_start(GTK_BOX(box), image, false, false, 1);
	gtk_box_pack_start(GTK_BOX(box), label, false, false, 1);

	return box;
}

static void create_ui(struct sgflow_state *sgflow_state) {
	GtkBuilder *builder; 
    const gchar resource_path[] = "/com/subgraph/sgflow/sgflow.ui";
	GError *error = NULL;
	builder = gtk_builder_new();
	if (!gtk_builder_add_from_resource(builder, resource_path, &error)) {
		g_log(NULL, G_LOG_LEVEL_ERROR, "%s: %s\n", __func__, error->message);
		g_error_free(error);
		g_application_quit(G_APPLICATION(sgflow_state->application));
	}

	GtkWindow *window = GTK_WINDOW(gtk_builder_get_object(builder, 
				"flowWindow"));

	GListStore *collection = g_list_store_new(TYPE_RESULT);

	GtkFlowBox *applications_flow_box = GTK_FLOW_BOX(gtk_builder_get_object(
				builder, "applicationsFlowBox"));
	g_signal_connect(applications_flow_box, "child-activated", 
			G_CALLBACK(applications_child_activated), collection);
	sgflow_state->applications_collection = collection;

	GtkSearchEntry *search_entry = GTK_SEARCH_ENTRY(
			gtk_builder_get_object(builder, "searchEntry"));
	sgflow_state->search_entry = search_entry;
	sgflow_state->applications_flow_box = applications_flow_box;
	g_signal_connect(search_entry, "search-changed", G_CALLBACK(search_entry_changed), sgflow_state);
	g_signal_connect(search_entry, "activate", G_CALLBACK(search_entry_activate), sgflow_state);

	GList *applications = g_app_info_get_all();
	GList *item;
	for(item = applications; item; item = item->next) {
		if (g_app_info_should_show(item->data)) {
			const char *app_id = g_app_info_get_id(item->data);
			GdkPixbuf *icon = get_icon_from_app_info(item->data);
			const char *display_name = g_app_info_get_display_name(item->data);
			const char *tag = "";
			const char *category = "APPLICATION";
			const char *keywords = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(item->data));
			if (icon != NULL) {
				Result *result = result_new(app_id, icon, display_name, tag, 
						category, keywords);
				g_list_store_append(collection, result);
			}
		}
	}

	gtk_flow_box_bind_model(applications_flow_box, G_LIST_MODEL(collection), 
			create_applications_flow_box, NULL, NULL); 
	gtk_flow_box_set_filter_func(applications_flow_box, 
			applications_filter_func, sgflow_state, NULL);
	gtk_application_add_window(sgflow_state->application, window);

	if (applications) {
		g_list_free_1(applications);
	}
	if (item) {
		g_list_free_1(item);
	}
	if (builder) {
		g_object_unref(builder);
	}
	sgflow_state->window = GTK_WIDGET(window);	
}

static void activate(GtkApplication *app, gpointer user_data) {
	struct sgflow_state *sgflow_state = user_data;
	GdkDisplay *display;
	GtkWidget *window;
	
	display = gdk_display_get_default();
	assert(display);
	assert(GDK_IS_WAYLAND_DISPLAY(display));
	register_display(display, sgflow_state);
	
	create_ui(sgflow_state); 
	gtk_widget_realize(sgflow_state->window);
	GdkWindow *gdk_window = gtk_widget_get_window(sgflow_state->window);

	init_surface(gdk_window, sgflow_state->window, sgflow_state);
	gtk_window_set_default_size(GTK_WINDOW(sgflow_state->window), sgflow_state->width - 100,
		sgflow_state->height - 100);
	select_first_child(sgflow_state);
}

void sgflow_destroy(struct sgflow_state *sgflow_state) {
	// TODO: Better cleanup
	if (sgflow_state->layer_shell != NULL) {
    		zwlr_layer_shell_v1_destroy(sgflow_state->layer_shell);
	}

	if (sgflow_state->layer_surface != NULL) {
    		zwlr_layer_surface_v1_destroy(sgflow_state->layer_surface);
	}

	if (sgflow_state->xdg_output_manager != NULL) {
		zxdg_output_manager_v1_destroy(sgflow_state->xdg_output_manager);
	}
}

int main(int argc, char **argv) {
	g_log_set_writer_func (g_log_writer_journald, NULL, NULL);
	sgflow_state.application = gtk_application_new("com.subgraph.flow", 
			G_APPLICATION_FLAGS_NONE);
	g_signal_connect(G_APPLICATION(sgflow_state.application), "activate", 
			G_CALLBACK (activate), &sgflow_state);
	g_application_hold(G_APPLICATION(sgflow_state.application));
	g_application_run(G_APPLICATION(sgflow_state.application), 0, NULL);
	g_object_unref(sgflow_state.application);
	sgflow_destroy(&sgflow_state);
}

