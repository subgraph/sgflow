#ifndef _RESULT_H_
#define _RESULT_H_

#include <glib-object.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

#define TYPE_RESULT ( result_get_type( ) )
#define RESULT( obj )   ( G_TYPE_CHECK_INSTANCE_CAST( (obj), TYPE_RESULT, Result ) )
#define IS_RESULT( obj ) ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), TYPE_RESULT ) )
#define RESULT_CLASS( klass )   ( G_TYPE_CHECK_CLASS_CAST( (klass), TYPE_RESULT, ResultClass ) )
#define IS_RESULT_CLASS( klass )    ( G_TYPE_CHECK_CLASS_TYPE( (klass), TYPE_RESULT ) )
#define RESULT_GET_CLASS( obj ) ( G_TYPE_INSTANCE_GET_CLASS( (obj), TYPE_RESULT, ResultClass ) )

typedef struct _Result      Result;
typedef struct _ResultClass ResultClass;

struct _Result {
	GObject parent;
};

struct _ResultClass {
	GObjectClass parent_class;
};

GType result_get_type(void) G_GNUC_CONST;

Result *result_new(const gchar *id, GdkPixbuf *icon, const gchar *displayname, 
        const gchar *tag, const gchar *category, const gchar *keywords);

gchar *result_get_id(Result *result);
void result_set_id(Result *result, const gchar *id);

GdkPixbuf *result_get_icon(Result *result);
void result_set_icon(Result *result, GdkPixbuf *icon);

gchar *result_get_displayname( Result *result );
void result_set_displayname(Result *result, const gchar *displayname);

gchar *result_get_tag(Result *result);
void result_set_tag(Result *result, const gchar *tag);

gchar *result_get_category(Result *result);
void result_set_category(Result *result, const gchar *category);

gchar *result_get_keywords(Result *result);
void result_set_keywords(Result *result, const gchar *keywords);

G_END_DECLS

#endif /* _RESULT_H */
