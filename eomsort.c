#include "eomsort.h"

#include <gmodule.h>
#include <gio/gio.h>

#include <eom/eom-debug.h>
#include <eom/eom-window.h>
#include <eom/eom-window-activatable.h>

static void eom_window_activatable_iface_init (EomWindowActivatableInterface *iface);

G_DEFINE_DYNAMIC_TYPE_EXTENDED (EomSortPlugin, eom_sort_plugin, PEAS_TYPE_EXTENSION_BASE, 0, G_IMPLEMENT_INTERFACE_DYNAMIC (EOM_TYPE_WINDOW_ACTIVATABLE, eom_window_activatable_iface_init))

enum {
  PROP_0,
  PROP_WINDOW
};

G_MODULE_EXPORT void
peas_register_types (PeasObjectModule *module)
{
  eom_sort_plugin_register_type (G_TYPE_MODULE (module));
  peas_object_module_register_extension_type (module, EOM_TYPE_WINDOW_ACTIVATABLE, EOM_TYPE_SORT_PLUGIN);
}


static void
eom_sort_plugin_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  EomSortPlugin *plugin = EOM_SORT_PLUGIN (object);

  switch (prop_id)
  {
  case PROP_WINDOW:
    plugin->window = EOM_WINDOW (g_value_dup_object (value));
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
eom_sort_plugin_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  EomSortPlugin *plugin = EOM_SORT_PLUGIN(plugin);

  switch (prop_id)
  {
  case PROP_WINDOW:
    g_value_set_object (value, plugin->window);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static guint
eom_sort_plugin_hash_func (gconstpointer *key) {
}

static void
eom_sort_plugin_init (EomSortPlugin *plugin)
{
  eom_debug_message (DEBUG_PLUGINS, "EomSortPlugin initializing");

  plugin->hash_table = g_hash_table_new (g_direct_hash, NULL);
}

static void
eom_sort_plugin_dispose (GObject *object)
{
  EomSortPlugin *plugin = EOM_SORT_PLUGIN (object);

  eom_debug_message (DEBUG_PLUGINS, "EomSortPlugin disposing");

  if (plugin->window != NULL) {
    g_object_unref (plugin->window);
    plugin->window = NULL;
  }

  g_hash_table_unref(plugin->hash_table);

  G_OBJECT_CLASS (eom_sort_plugin_parent_class)->dispose (object);
}

static void
eom_sort_plugin_class_init (EomSortPluginClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->dispose = eom_sort_plugin_dispose;
  object_class->set_property = eom_sort_plugin_set_property;
  object_class->get_property = eom_sort_plugin_get_property;

  g_object_class_override_property (object_class, PROP_WINDOW, "window");
}

static void
eom_sort_plugin_class_finalize (EomSortPluginClass *klass)
{
}

static gint
eom_sort_plugin_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  gint r_value;

  EomImage *image_a, *image_b;

  gtk_tree_model_get (model, a, EOM_LIST_STORE_EOM_IMAGE, &image_a, -1);
  gtk_tree_model_get (model, b, EOM_LIST_STORE_EOM_IMAGE, &image_b, -1);

  GDateTime *time_a, *time_b;

  EomSortPlugin *plugin = EOM_SORT_PLUGIN (user_data);

  time_a = g_hash_table_lookup (plugin->hash_table, image_a);
  time_b = g_hash_table_lookup (plugin->hash_table, image_b);

  if (!time_a) {
    GFile *file_a = eom_image_get_file(image_a);
    GFileInfo *info_a = g_file_query_info(G_FILE (file_a), G_FILE_ATTRIBUTE_TIME_MODIFIED, 0, NULL, NULL);
    if (info_a) {
      time_a = g_file_info_get_modification_date_time(G_FILE_INFO (info_a));
      g_object_unref(info_a);
      g_object_unref(file_a);
      g_hash_table_replace(plugin->hash_table, image_a, time_a);
    }
  }

  if (!time_b) {
    GFile *file_b = eom_image_get_file(image_b);
    GFileInfo *info_b = g_file_query_info(G_FILE (file_b), G_FILE_ATTRIBUTE_TIME_MODIFIED, 0, NULL, NULL);
    if (info_b) {
      time_b = g_file_info_get_modification_date_time(G_FILE_INFO (info_b));
      g_object_unref(info_b);
      g_object_unref(file_b);
      g_hash_table_replace(plugin->hash_table, image_b, time_b);
    }
  }

  if (time_a && time_b) {
    r_value = g_date_time_compare (time_a, time_b);
  }

  g_object_unref (G_OBJECT (image_a));
  g_object_unref (G_OBJECT (image_b));

  return r_value;
}

static gint
eom_list_store_compare_func (GtkTreeModel *model, GtkTreeIter *a, GtkTreeIter *b, gpointer user_data)
{
  gint r_value;

  EomImage *image_a, *image_b;

  gtk_tree_model_get (model, a, EOM_LIST_STORE_EOM_IMAGE, &image_a, -1);
  gtk_tree_model_get (model, b, EOM_LIST_STORE_EOM_IMAGE, &image_b, -1);

  r_value = strcmp (eom_image_get_collate_key (image_a), eom_image_get_collate_key (image_b));

  g_object_unref (G_OBJECT (image_a));
  g_object_unref (G_OBJECT (image_b));

  return r_value;
}

static void
eom_window_prepared (EomWindow *window, EomSortPlugin *plugin)
{
  EomListStore *store = eom_window_get_store(window);

  eom_debug_message (DEBUG_PLUGINS, "window is %x, store is %x, plugin is %x", window, store, plugin);

  if (store) {
    gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (store), eom_sort_plugin_compare_func, plugin, NULL);
  }
}

static void
eom_sort_plugin_activate (EomWindowActivatable *activatable)
{
  EomSortPlugin *plugin = EOM_SORT_PLUGIN (activatable);
  GtkWidget *view = eom_window_get_view (plugin->window);

  eom_debug (DEBUG_PLUGINS);

  plugin->signal_id = g_signal_connect (G_OBJECT (plugin->window), "prepared", G_CALLBACK (eom_window_prepared), plugin);

  EomListStore *store = eom_window_get_store(plugin->window);

  if (store) {
    gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (store), eom_sort_plugin_compare_func, plugin, NULL);
  }
}

static void
eom_sort_plugin_deactivate (EomWindowActivatable *activatable)
{
  EomSortPlugin *plugin = EOM_SORT_PLUGIN (activatable);
  GtkWidget *view = eom_window_get_view (plugin->window);

  g_signal_handler_disconnect (plugin->window, plugin->signal_id);

  EomListStore *store = eom_window_get_store(plugin->window);

  eom_debug_message (DEBUG_PLUGINS, "store is %x", store);

  if (store) {
    gtk_tree_sortable_set_default_sort_func (GTK_TREE_SORTABLE (store), eom_list_store_compare_func, NULL, NULL);
  }
}

static void
eom_window_activatable_iface_init (EomWindowActivatableInterface *iface)
{
  iface->activate = eom_sort_plugin_activate;
  iface->deactivate = eom_sort_plugin_deactivate;
}
