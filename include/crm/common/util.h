/*
 * Copyright 2004-2020 the Pacemaker project contributors
 *
 * The version control history for this file may have further details.
 *
 * This source code is licensed under the GNU Lesser General Public License
 * version 2.1 or later (LGPLv2.1+) WITHOUT ANY WARRANTY.
 */

#ifndef CRM_COMMON_UTIL__H
#  define CRM_COMMON_UTIL__H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file
 * \brief Utility functions
 * \ingroup core
 */

#  include <sys/types.h>    // gid_t, mode_t, size_t, time_t, uid_t
#  include <stdlib.h>
#  include <stdbool.h>
#  include <stdint.h>       // uint32_t
#  include <limits.h>
#  include <signal.h>
#  include <glib.h>

#  include <libxml/tree.h>

#  include <crm/lrmd.h>
#  include <crm/common/acl.h>
#  include <crm/common/agents.h>
#  include <crm/common/results.h>

#  define ONLINESTATUS  "online"  // Status of an online client
#  define OFFLINESTATUS "offline" // Status of an offline client

// public name/value pair functions (from nvpair.c)
int pcmk_scan_nvpair(const char *input, char **name, char **value);
char *pcmk_format_nvpair(const char *name, const char *value, const char *units);
char *pcmk_format_named_time(const char *name, time_t epoch_time);

/* public Pacemaker Remote functions (from remote.c) */
int crm_default_remote_port(void);

/* public string functions (from strings.c) */
char *crm_itoa_stack(int an_int, char *buf, size_t len);
gboolean crm_is_true(const char *s);
int crm_str_to_boolean(const char *s, int *ret);
long long crm_parse_ll(const char *text, const char *default_text);
int crm_parse_int(const char *text, const char *default_text);
long long crm_get_msec(const char *input);
char * crm_strip_trailing_newline(char *str);
gboolean crm_strcase_equal(gconstpointer a, gconstpointer b);
guint crm_strcase_hash(gconstpointer v);
guint g_str_hash_traditional(gconstpointer v);
char *crm_strdup_printf(char const *format, ...) __attribute__ ((__format__ (__printf__, 1, 2)));
int pcmk_numeric_strcasecmp(const char *s1, const char *s2);

#  define crm_str_hash g_str_hash_traditional

static inline char *
crm_itoa(int an_int)
{
    return crm_strdup_printf("%d", an_int);
}

static inline char *
crm_ftoa(double a_float)
{
    return crm_strdup_printf("%f", a_float);
}

static inline char *
crm_ttoa(time_t epoch_time)
{
    return crm_strdup_printf("%lld", (long long) epoch_time);
}

/*!
 * \brief Create hash table with dynamically allocated string keys/values
 *
 * \return Newly allocated hash table
 * \note It is the caller's responsibility to free the result, using
 *       g_hash_table_destroy().
 */
static inline GHashTable *
crm_str_table_new(void)
{
    return g_hash_table_new_full(crm_str_hash, g_str_equal, free, free);
}

/*!
 * \brief Create hash table with case-insensitive dynamically allocated string keys/values
 *
 * \return Newly allocated hash table
 * \note It is the caller's responsibility to free the result, using
 *       g_hash_table_destroy().
 */
static inline GHashTable *
crm_strcase_table_new(void)
{
    return g_hash_table_new_full(crm_strcase_hash, crm_strcase_equal, free, free);
}

GHashTable *crm_str_table_dup(GHashTable *old_table);

#  define crm_atoi(text, default_text) crm_parse_int(text, default_text)

/* public I/O functions (from io.c) */
void crm_build_path(const char *path_c, mode_t mode);

guint crm_parse_interval_spec(const char *input);
int char2score(const char *score);
char *score2char(int score);
char *score2char_stack(int score, char *buf, size_t len);

/* public operation functions (from operations.c) */
gboolean parse_op_key(const char *key, char **rsc_id, char **op_type,
                      guint *interval_ms);
gboolean decode_transition_key(const char *key, char **uuid, int *transition_id,
                               int *action_id, int *target_rc);
gboolean decode_transition_magic(const char *magic, char **uuid,
                                 int *transition_id, int *action_id,
                                 int *op_status, int *op_rc, int *target_rc);
int rsc_op_expected_rc(lrmd_event_data_t *event);
gboolean did_rsc_op_fail(lrmd_event_data_t *event, int target_rc);
bool crm_op_needs_metadata(const char *rsc_class, const char *op);
xmlNode *crm_create_op_xml(xmlNode *parent, const char *prefix,
                           const char *task, const char *interval_spec,
                           const char *timeout);
#define CRM_DEFAULT_OP_TIMEOUT_S "20s"

int compare_version(const char *version1, const char *version2);

/* coverity[+kill] */
void crm_abort(const char *file, const char *function, int line,
               const char *condition, gboolean do_core, gboolean do_fork);

/*!
 * \brief Check whether any of specified flags are set in a flag group
 *
 * \param[in] flag_group        The flag group being examined
 * \param[in] flags_to_check    Which flags in flag_group should be checked
 *
 * \return true if \p flags_to_check is nonzero and any of its flags are set in
 *         \p flag_group, or false otherwise
 */
static inline bool
pcmk_any_flags_set(uint64_t flag_group, uint64_t flags_to_check)
{
    return (flag_group & flags_to_check) != 0;
}

/*!
 * \brief Check whether all of specified flags are set in a flag group
 *
 * \param[in] flag_group        The flag group being examined
 * \param[in] flags_to_check    Which flags in flag_group should be checked
 *
 * \return true if \p flags_to_check is zero or all of its flags are set in
 *         \p flag_group, or false otherwise
 */
static inline bool
pcmk_all_flags_set(uint64_t flag_group, uint64_t flags_to_check)
{
    return (flag_group & flags_to_check) == flags_to_check;
}

/*!
 * \brief Convenience alias for pcmk_all_flags_set(), to check single flag
 */
#define pcmk_is_set(g, f)   pcmk_all_flags_set((g), (f))

static inline guint
crm_hash_table_size(GHashTable * hashtable)
{
    if (hashtable == NULL) {
        return 0;
    }
    return g_hash_table_size(hashtable);
}

char *crm_meta_name(const char *field);
const char *crm_meta_value(GHashTable * hash, const char *field);

char *crm_md5sum(const char *buffer);

char *crm_generate_uuid(void);

// This belongs in ipc.h but is here for backward compatibility
bool crm_is_daemon_name(const char *name);

int crm_user_lookup(const char *name, uid_t * uid, gid_t * gid);
int pcmk_daemon_user(uid_t *uid, gid_t *gid);

#ifdef HAVE_GNUTLS_GNUTLS_H
void crm_gnutls_global_init(void);
#endif

char *pcmk_hostname(void);

bool pcmk_str_is_infinity(const char *s);
bool pcmk_str_is_minus_infinity(const char *s);

#ifndef PCMK__NO_COMPAT
/* Everything here is deprecated and kept only for public API backward
 * compatibility. It will be moved to compatibility.h in a future release.
 */

//! \deprecated Use crm_parse_interval_spec() instead
#define crm_get_interval crm_parse_interval_spec

//! \deprecated Use !pcmk_is_set() or !pcmk_all_flags_set() instead
static inline gboolean
is_not_set(long long word, long long bit)
{
    return ((word & bit) == 0);
}

//! \deprecated Use pcmk_is_set() or pcmk_all_flags_set() instead
static inline gboolean
is_set(long long word, long long bit)
{
    return ((word & bit) == bit);
}

//! \deprecated Use pcmk_any_flags_set() instead
static inline gboolean
is_set_any(long long word, long long bit)
{
    return ((word & bit) != 0);
}

//! \deprecated Use strcmp or strcasecmp instead
gboolean crm_str_eq(const char *a, const char *b, gboolean use_case);

//! \deprecated Use strcmp instead
gboolean safe_str_neq(const char *a, const char *b);

//! \deprecated Use strcasecmp instead
#define safe_str_eq(a, b) crm_str_eq(a, b, FALSE)

#endif // PCMK__NO_COMPAT

#ifdef __cplusplus
}
#endif

#endif
