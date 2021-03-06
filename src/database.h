#ifndef _DYN_LIST_PLUGIN_DATABASE
#define _DYN_LIST_PLUGIN_DATABASE

#include "dbSong.h"

dbList* database_get_songs_comment(dbList* l_list, const gchar* l_comment, gint* l_out_count);
dbList* database_get_songs_genre(dbList* l_list, const gchar* l_genre, gint* l_out_count);
dbList* database_get_songs(dbList* l_list, const gchar* l_artist, const gchar* l_title, gint* l_out_count);
strList* database_get_artists(strList* l_list, const gchar* l_artist, const gchar* l_genre, gint* l_out_count);

gboolean database_tryToAdd_artist(const gchar* l_artist);
gboolean database_tryToAdd_artists(strList** l_out_list, gint l_count);

#endif

/* vim:set ts=4 sw=4: */
