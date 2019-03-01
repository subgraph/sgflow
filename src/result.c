#include <glib/gprintf.h>
#include "result.h"

#define RESULT_GET_PRIVATE( obj )	  ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), TYPE_RESULT, ResultPrivate ) )

G_DEFINE_TYPE( Result, result, G_TYPE_OBJECT )

typedef struct _ResultPrivate ResultPrivate;

struct _ResultPrivate
{
	gchar *id;
	GdkPixbuf *icon;
	gchar *displayname;
	gchar *tag;
	gchar *category; // Should be an enum of: Application, Realm, Window
	gchar *keywords;
};

enum 
{
	PROP_0,

	PROP_ID,

	PROP_ICON,
	
	PROP_DISPLAYNAME,

	PROP_TAG,

	PROP_CATEGORY,

	PROP_KEYWORDS,

	N_PROPERTIES
};

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void result_finalize(GObject *result);

static void result_set_property( GObject *object, guint prop_id, 
		const GValue *value, GParamSpec *pspec);

static void result_get_property(GObject *object, guint prop_id, GValue *value, 
		GParamSpec *pspec);

static void result_class_init(ResultClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS(klass);

	gobject_class->finalize = result_finalize;

	g_type_class_add_private(klass, sizeof(ResultPrivate));

	gobject_class->get_property = result_get_property;
	gobject_class->set_property = result_set_property;

	obj_properties[PROP_ID] =
		g_param_spec_string("id", "id", "Result Id", "", G_PARAM_READWRITE);
	
	obj_properties[PROP_ICON] =
		g_param_spec_object("icon", "Icon", "Result Icon", GDK_TYPE_PIXBUF, 
				G_PARAM_READWRITE);

	obj_properties[PROP_DISPLAYNAME] =
		g_param_spec_string("displayname", "DisplayName", "Result Display Name", 
				"", G_PARAM_READWRITE);

	obj_properties[PROP_TAG] =
		g_param_spec_string("tag", "Tag", "Result Tag", "", G_PARAM_READWRITE);
	
	obj_properties[PROP_CATEGORY] =
		g_param_spec_string("category", "Category", "Result Category", "", 
				G_PARAM_READWRITE);

	obj_properties[PROP_KEYWORDS] =
		g_param_spec_string("keywords", "Keywords", "Result Keywords", "", 
				G_PARAM_READWRITE);

	g_object_class_install_properties(gobject_class, N_PROPERTIES, 
			obj_properties);
}

static void result_init(Result *result)
{
	ResultPrivate *priv = RESULT_GET_PRIVATE(result);

	priv->id = g_strdup("");
	priv->icon = NULL;
	priv->displayname = g_strdup ("");
	priv->tag = g_strdup("");
	priv->category = g_strdup ("APPLICATION");
	priv->keywords = g_strdup ("");
}

static void result_finalize(GObject *object)
{
	Result *result = RESULT(object);
	ResultPrivate *priv = RESULT_GET_PRIVATE(result);
	GObjectClass *parent_class = G_OBJECT_CLASS(result_parent_class);

	g_free(priv->id);
	g_free(priv->icon);
	g_free(priv->displayname);
	g_free(priv->tag);
	g_free(priv->category);
	g_free(priv->keywords);

	(*parent_class->finalize)(object);
}

Result *result_new(const gchar *id, GdkPixbuf *icon, const gchar *displayname,
		const gchar *tag, const gchar *category, const gchar *keywords)
{
	Result *result;

	result = RESULT(g_object_new( TYPE_RESULT, NULL));

	if( id != NULL ) {
		result_set_id(result, id);
	}

	if(icon != NULL) {
		result_set_icon(result, icon);
	}
	
	if(displayname != NULL) {
		result_set_displayname(result, displayname);
	}
	
	if(tag != NULL) {
		result_set_tag(result, tag);
	}

	if(category != NULL) {
		result_set_category(result, category);
	}

	if(keywords != NULL) {
		result_set_keywords(result, keywords);
	}

	return result;
}
 
gchar *result_get_id(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return g_strdup(priv->id);
}

void result_set_id(Result *result, const gchar *id)
{
	ResultPrivate *priv;

	g_return_if_fail(id);
	g_return_if_fail(IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if(priv->id != NULL) {
		g_free(priv->id);
	}

	priv->id = g_strdup(id);
}

GdkPixbuf *result_get_icon(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return priv->icon;
}
 
void result_set_icon(Result *result, GdkPixbuf *icon)
{
	ResultPrivate *priv;

	g_return_if_fail(icon);
	g_return_if_fail(IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if(priv->icon != NULL) {
		g_free(priv->icon);
	}

	priv->icon = icon;
}
 
gchar *result_get_displayname(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return g_strdup(priv->displayname);
}

void result_set_displayname(Result *result, const gchar *displayname)
{
	ResultPrivate *priv;

	g_return_if_fail(displayname);
	g_return_if_fail(IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if(priv->displayname != NULL) {
		g_free(priv->displayname);
	}

	priv->displayname = g_strdup(displayname);
}
 
gchar *result_get_tag(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return g_strdup( priv->tag );
}
 
void result_set_tag(Result *result, const gchar *tag)
{
	ResultPrivate *priv;
   
	g_return_if_fail(tag);
	g_return_if_fail(IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if( priv->tag != NULL ) {
		g_free(priv->tag);
	}

  priv->tag = g_strdup(tag);
}

gchar *result_get_category(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return g_strdup(priv->category);
}

void result_set_category(Result *result, const gchar *category)
{
	ResultPrivate *priv;

	g_return_if_fail(category);
	g_return_if_fail(IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if(priv->category != NULL) {
		g_free(priv->category);
	}

	priv->category = g_strdup(category);
}

gchar *result_get_keywords(Result *result)
{
	ResultPrivate *priv;

	g_return_val_if_fail(IS_RESULT(result), NULL);

	priv = RESULT_GET_PRIVATE(result);

	return g_strdup(priv->keywords);
}

void result_set_keywords(Result *result, const gchar *keywords)
{
	ResultPrivate *priv;

	g_return_if_fail(keywords);
	g_return_if_fail( IS_RESULT(result));

	priv = RESULT_GET_PRIVATE(result);

	if(priv->keywords != NULL) {
		g_free(priv->keywords);
	}

	priv->keywords = g_strdup(keywords);
}

static void result_set_property(GObject *object, guint prop_id, 
		const GValue *value, GParamSpec *pspec)
{
	Result *result = RESULT(object);

	switch(prop_id) {
	case PROP_ID:
		result_set_id(result, g_value_get_string(value));
		break;
	case PROP_ICON:
		result_set_id(result, g_value_get_object(value));
		break;
	case PROP_DISPLAYNAME:
		result_set_displayname(result, g_value_get_string(value));
		break;
	case PROP_TAG:
		result_set_tag(result, g_value_get_string(value));
		break;
	case PROP_CATEGORY:
		result_set_category(result, g_value_get_string(value));
		break;
	case PROP_KEYWORDS:
		result_set_keywords(result, g_value_get_string(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void result_get_property(GObject *object, guint prop_id, 
		GValue *value, GParamSpec *pspec)
{
	Result *result = RESULT(object);

	switch(prop_id) {
	case PROP_ID:
		g_value_set_string(value, result_get_id(result));
		break;
    case PROP_ICON:
		g_value_set_object(value, result_get_icon(result));
		break;
    case PROP_DISPLAYNAME:
		g_value_set_string(value, result_get_displayname(result));
		break;
    case PROP_TAG:
		g_value_set_string(value, result_get_tag(result));
		break;
    case PROP_CATEGORY:
		g_value_set_string(value, result_get_category(result));
		break;
	case PROP_KEYWORDS:
		g_value_set_string(value, result_get_keywords(result));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}
