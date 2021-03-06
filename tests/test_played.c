#include <gmpc/plugin.h>
#include "fixture_mpd.h"
#include "fixture_gmpc.h"
#include "../src/played.h"
#include "../src/database.h"
#include "../src/defaults.h"

static void test_set_limits_allowed()
{
	int set1 = 0;
	int set2 = 500;

	g_assert_cmpint(get_played_limit_song(), ==, -1);
	g_assert_cmpint(get_played_limit_artist(), ==, -1);

	set_played_limit_song(set1);
	set_played_limit_artist(set1);
	g_assert_cmpint(get_played_limit_song(), ==, set1);
	g_assert_cmpint(get_played_limit_artist(), ==, set1);

	set_played_limit_song(set2);
	set_played_limit_artist(set2);
	g_assert_cmpint(get_played_limit_song(), ==, set2);
	g_assert_cmpint(get_played_limit_artist(), ==, set2);
}

static void test_set_limits_assert_song()
{
	if(g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDERR))
	{
		set_played_limit_song(-1);
		exit(EXIT_SUCCESS);
	}
	g_test_trap_assert_failed();
}

static void test_set_limits_assert_artist()
{
	if(g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDERR))
	{
		set_played_limit_artist(-1);
		exit(EXIT_SUCCESS);
	}
	g_test_trap_assert_failed();
}

static gint test_fill(dbList** l_list)
{
	g_assert(l_list != NULL);
	g_assert(*l_list == NULL);

	gint count = 0;
	MpdData* data;
	for(data = mpd_database_get_complete(connection); data != NULL; data = mpd_data_get_next(data))
		if(data->song->artist != NULL && data->song->title != NULL)
			*l_list = database_get_songs(*l_list, data->song->artist, data->song->title, &count);
	g_assert(*l_list != NULL);
	g_assert_cmpint(count, >, 0);

	GList* iter;
	for(iter = *l_list; iter != NULL; iter = g_list_next(iter))
		add_played_song(iter->data);

	return count;
}

static void test_artist_zero()
{
	fake_gmpc_init();
	fake_mpd_init(CONFIG);

	set_played_limit_song(100);
	set_played_limit_artist(0);

	dbList* list = NULL;
	test_fill(&list);

	dbList* iter = NULL;
	for(iter = list; iter != NULL; iter = g_list_next(iter))
	{
		dbSong* song = (dbSong*) iter->data;
		g_assert(!is_played_artist(song->artist));
	}

	g_list_free(list);
	free_played_list();

	fake_mpd_free(CONFIG);
	fake_gmpc_free();
}

static void test_artist_all()
{
	fake_mpd_init(CONFIG);

	set_played_limit_song(50);
	set_played_limit_artist(500);

	dbList* list = NULL;
	test_fill(&list);

	dbList* iter = NULL;
	for(iter = list; iter != NULL; iter = g_list_next(iter))
	{
		dbSong* song = (dbSong*) iter->data;
		g_assert(is_played_artist(song->artist));
	}

	g_list_free(list);
	free_played_list();

	fake_mpd_free(CONFIG);
}

static void test_artist_some_check()
{
	dbSong* song1 = new_dbSong("Metallica", "Frantic", "");
	dbSong* song2 = new_dbSong("Metallica", "St. Anger", "");
	dbSong* song3 = new_dbSong("The Offspring", "Self Esteem", "");
	dbSong* song4 = new_dbSong("Die Ärzte", "Schrei nach Liebe", "");
	dbSong* song5 = new_dbSong("H-Blockx", "Revolution", "");
	dbSong* song6 = new_dbSong("Metallica", "Fuel", "");

	g_assert(!is_played_artist(song1->artist));
	add_played_song(song1);
	g_assert(is_played_artist(song1->artist));
	add_played_song(song2);
	g_assert(is_played_artist(song1->artist));
	g_assert(is_played_artist(song2->artist));
	add_played_song(song3);
	g_assert(is_played_artist(song1->artist));
	g_assert(is_played_artist(song2->artist));
	g_assert(is_played_artist(song3->artist));
	add_played_song(song4);
	g_assert(is_played_artist("Metallica"));
	g_assert(is_played_artist("The Offspring"));
	g_assert(is_played_artist("Die Ärzte"));
	g_assert(!is_played_artist("H-Blockx"));
	add_played_song(song5);
	g_assert(!is_played_artist("Metallica"));
	g_assert(is_played_artist("The Offspring"));
	g_assert(is_played_artist("Die Ärzte"));
	g_assert(is_played_artist("H-Blockx"));
	add_played_song(song6);
	g_assert(!is_played_artist("The Offspring"));
	g_assert(is_played_artist("Die Ärzte"));
	g_assert(is_played_artist("H-Blockx"));
	g_assert(is_played_artist("Metallica"));

	free_played_list();
}

static void test_artist_some_song()
{
	fake_gmpc_init();
	set_played_limit_song(0);
	set_played_limit_artist(3);
	test_artist_some_check();
	fake_gmpc_free();
}

static void test_artist_some_nosong()
{
	fake_gmpc_init();
	set_played_limit_song(100);
	set_played_limit_artist(3);
	test_artist_some_check();
	fake_gmpc_free();
}

static void test_artist_some_assert_less()
{
	set_played_limit_song(g_test_rand_int_range(0, 666));
	set_played_limit_artist(g_test_rand_int_range(0, 3));
	if(g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDERR))
	{
		test_artist_some_check();
		exit(EXIT_SUCCESS);
	}
	g_test_trap_assert_failed();
}

static void test_artist_some_assert_more()
{
	set_played_limit_song(g_test_rand_int_range(0, 666));
	set_played_limit_artist(g_test_rand_int_range(4, 666));
	if(g_test_trap_fork(0, G_TEST_TRAP_SILENCE_STDERR))
	{
		test_artist_some_check();
		exit(EXIT_SUCCESS);
	}
	g_test_trap_assert_failed();
}

static void test_song_zero()
{
	fake_mpd_init(CONFIG);

	set_played_limit_song(0);
	set_played_limit_artist(0);

	dbList* list = NULL;
	gint count_test = test_fill(&list);

	/* get data twice because 'played list' will flush Queue */
	gint count = 0;
	dbList* searchList = NULL;
	MpdData* data;
	for(data = mpd_database_get_complete(connection); data != NULL; data = mpd_data_get_next(data))
		if(data->song->artist != NULL && data->song->title != NULL)
			searchList = database_get_songs(searchList, data->song->artist, data->song->title, &count);
	g_assert_cmpint(count, ==, count_test);

	dbList* iter;
	for(iter = searchList; iter != NULL; iter = g_list_next(iter))
	{
		dbSong* song = (dbSong*) iter->data;
		g_assert(!is_played_song(song->artist, song->title));
	}

	free_dbList(searchList);
	g_list_free(list);
	free_played_list();

	fake_mpd_free(CONFIG);
}

static void test_song_all()
{
	fake_mpd_init(CONFIG);

	set_played_limit_song(100);
	set_played_limit_artist(0);

	dbList* list = NULL;
	test_fill(&list);

	dbList* iter = NULL;
	for(iter = list; iter != NULL; iter = g_list_next(iter))
	{
		dbSong* song = (dbSong*) iter->data;
		g_assert(is_played_song(song->artist, song->title));
	}

	g_list_free(list);
	free_played_list();

	fake_mpd_free(CONFIG);
}

int main(int argc, char** argv)
{
	gtk_test_init(&argc, &argv, NULL);

	g_test_add_func("/played/set_limits/allowed", test_set_limits_allowed);
	g_test_add_func("/played/set_limits/assert/song", test_set_limits_assert_song);
	g_test_add_func("/played/set_limits/assert/artist", test_set_limits_assert_artist);

	g_test_add_func("/played/artist/zero", test_artist_zero);
	g_test_add_func("/played/artist/all", test_artist_all);
	g_test_add_func("/played/artist/some/song", test_artist_some_song);
	g_test_add_func("/played/artist/some/nosong", test_artist_some_nosong);
	g_test_add_func("/played/artist/some/assert/less", test_artist_some_assert_less);
	g_test_add_func("/played/artist/some/assert/more", test_artist_some_assert_more);

	g_test_add_func("/played/song/zero", test_song_zero);
	g_test_add_func("/played/song/all", test_song_all);

	/* mute standard debug output from plugin */
	g_log_set_handler("dynlist", G_LOG_LEVEL_DEBUG, redirect_log, NULL);

	return g_test_run();
}

/* vim:set ts=4 sw=4: */
