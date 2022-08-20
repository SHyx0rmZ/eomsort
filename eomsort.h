#ifndef __EOMSORT_H__
#define __EOMSORT_H__

#include <glib.h>
#include <glib-object.h>
#include <libpeas/peas-extension-base.h>
#include <libpeas/peas-object-module.h>

#include <eom/eom-window.h>

G_BEGIN_DECLS

#define EOM_TYPE_SORT_PLUGIN \
  (eom_sort_plugin_get_type())
#define EOM_SORT_PLUGIN(o) \
  (G_TYPE_CHECK_INSTANCE_CAST((o), EOM_TYPE_SORT_PLUGIN, EomSortPlugin))

typedef struct _EomSortPlugin EomSortPlugin;

struct _EomSortPlugin {
  PeasExtensionBase parent_instance;

  EomWindow *window;
  gulong signal_id;
  gulong signal_id2;

  GHashTable *hash_table;
};

typedef struct _EomSortPluginClass EomSortPluginClass;

struct _EomSortPluginClass {
  PeasExtensionBaseClass parent_class;
};

GType eom_sort_plugin_get_type (void) G_GNUC_CONST;

G_MODULE_EXPORT void peas_register_types (PeasObjectModule *module);

G_END_DECLS

#endif
