#include <dbus/dbus-glib.h>
#include <stdio.h>
#include <stdlib.h>

static void lose (const char *fmt, ...) G_GNUC_NORETURN G_GNUC_PRINTF (1, 2);
static void lose_gerror (const char *prefix, GError *error) G_GNUC_NORETURN;

static void
lose (const char *str, ...)
{
	va_list args;
	va_start (args, str);
	vfprintf (stderr, str, args);
	fputc ('\n', stderr);
	va_end (args);
	exit (1);
}

static void
lose_gerror (const char *prefix, GError *error)
{
	lose ("%s: %s", prefix, error->message);
}

typedef struct BonnieObject BonnieObject;
typedef struct BonnieObjectClass BonnieObjectClass;

GType bonnie_get_type (void);

struct BonnieObject
{
	GObject parent;
};

struct BonnieObjectClass
{
	GObjectClass parent;
	char **hash_applets;
};

#define SOME_TYPE_OBJECT						(bonnie_get_type ())
#define SOME_OBJECT(obj)						(G_TYPE_CHECK_INSTANCE_CAST ((obj), SOME_TYPE_OBJECT, BonnieObject))
#define SOME_OBJECT_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), SOME_TYPE_OBJECT, BonnieObjectClass))
#define SOME_IS_OBJECT(obj)					(G_TYPE_CHECK_INSTANCE_TYPE ((obj), SOME_TYPE_OBJECT))
#define SOME_IS_OBJECT_CLASS(klass)	(G_TYPE_CHECK_CLASS_TYPE ((klass), SOME_TYPE_OBJECT))
#define SOME_OBJECT_GET_CLASS(obj)	(G_TYPE_INSTANCE_GET_CLASS ((obj), SOME_TYPE_OBJECT, BonnieObjectClass))

G_DEFINE_TYPE (BonnieObject, bonnie, G_TYPE_OBJECT)

gboolean bonnie_register_applet 	(BonnieObject *obj, char **hello_message, GError **error);


#include "bonnie-glue.h"


static void
bonnie_init (BonnieObject *obj)
{
}

static void
bonnie_class_init (BonnieObjectClass *klass)
{
}

gboolean
bonnie_register_applet (BonnieObject *obj, char **hello_message, GError **error)
{
	char **ptr_msg;
	g_print ( "Registering Applet..." );
	for ( ptr_msg = hello_message; *ptr_msg; ptr_msg++)
	{
		g_print (" %s\n", *ptr_msg );
	}
	return TRUE;
}

int
main (int argc, char **argv)
{
	DBusGConnection *connection;
	DBusGProxy			*connection_proxy;
	GError 					*error = NULL;
	BonnieObject			*obj;
	GMainLoop				*mainloop;
	guint						request_name_result;

	g_type_init ();

	{
		GLogLevelFlags fatal_mask;

		fatal_mask = g_log_set_always_fatal (G_LOG_FATAL_MASK);
		fatal_mask |= G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL;
		g_log_set_always_fatal (fatal_mask);
	}

	dbus_g_object_type_install_info (SOME_TYPE_OBJECT, &dbus_glib_bonnie_object_info);

	mainloop = g_main_loop_new (NULL, FALSE);

	connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
	if (!connection)
		lose_gerror ("Couldn't connect to session bus", error);

	connection_proxy = dbus_g_proxy_new_for_name (connection, "org.freedesktop.DBus",
																								"/org/freedesktop/DBus",
																								"org.freedesktop.DBus");

	if (!dbus_g_proxy_call (connection_proxy, "RequestName", &error,
													G_TYPE_STRING, "org.anized.bonnie",
													G_TYPE_UINT, 0,
													G_TYPE_INVALID,
													G_TYPE_UINT, &request_name_result,
													G_TYPE_INVALID))
		lose_gerror ("Failed to acquire org.anized.bonnie", error);

	obj = g_object_new (SOME_TYPE_OBJECT, NULL);

	dbus_g_connection_register_g_object (connection, "/org/anized/bonnie", G_OBJECT (obj));

	printf ("service running\n");

	g_main_loop_run (mainloop);

	exit(0);
}
