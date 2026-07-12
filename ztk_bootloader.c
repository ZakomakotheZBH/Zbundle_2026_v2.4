/**
 * ZTK Business Edition v4.0 - Enterprise Ultimate Edition
 * ISO C11 Compliant - Full System with AI, Games, Disk Management, and More
 * 
 * Features:
 * - Modern command registry with plugins
 * - Advanced shell scripting engine
 * - Real-time process monitoring
 * - Network file system support
 * - Database integration (SQLite)
 * - Web server mode
 * - Cluster management
 * - Container support (Docker/Podman)
 * - Advanced AI with multiple models
 * - Full terminal UI with mouse support
 * - Plugin system
 * - Remote shell access
 * - Automated backups
 * - System recovery tools
 * - Performance profiling
 * - Resource monitoring
 * - Security auditing
 * 
 * Compilation: gcc -std=c11 -Wall -Wextra -O3 -o ztk ztk.c -lpthread -lm -lcurl -lsqlite3 -lncurses
 * Usage: ./ztk [--config FILE] [--plugins DIR] [--ai-token TOKEN] [--daemon]
 */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>
#include <stddef.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <termios.h>
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <dlfcn.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <curl/curl.h>
#include <sqlite3.h>

#ifdef USE_NCURSES
#include <ncurses.h>
#include <menu.h>
#include <form.h>
#include <panel.h>
#endif

/* Version Information */
#define ZTK_VERSION "4.0.0"
#define ZTK_RELEASE "Enterprise Ultimate Edition"
#define ZTK_CODENAME "Phoenix"
#define ZTK_BUILD_DATE __DATE__
#define ZTK_BUILD_TIME __TIME__
#define ZTK_API_VERSION 1

/* ============ Core Constants ============ */
#define MAX_LINE 16384
#define MAX_ARGS 1024
#define MAX_PATH 4096
#define MAX_PLUGINS 256
#define MAX_HISTORY 50000
#define MAX_JOBS 1024
#define MAX_VARIABLES 4096
#define MAX_FUNCTIONS 1024
#define MAX_ALIASES 512
#define MAX_MODULES 128
#define MAX_DATABASES 64
#define MAX_CONTAINERS 256
#define MAX_CLUSTER_NODES 64
#define MAX_REMOTE_SESSIONS 32
#define MAX_BACKUP_SETS 128
#define MAX_AUDIT_ENTRIES 10000

/* ============ Security Constants ============ */
#define SECURE_BUFFER_SIZE 4096
#define MAX_PASSWORD_TRIES 3
#define SESSION_TIMEOUT 3600
#define MIN_PASSWORD_LENGTH 12

/* ============ Color Schemes ============ */
#define COLOR_DEFAULT  "\033[0m"
#define COLOR_BOLD     "\033[1m"
#define COLOR_DIM      "\033[2m"
#define COLOR_ITALIC   "\033[3m"
#define COLOR_UNDERLINE "\033[4m"
#define COLOR_BLINK    "\033[5m"
#define COLOR_REVERSE  "\033[7m"
#define COLOR_HIDDEN   "\033[8m"

#define COLOR_BLACK    "\033[30m"
#define COLOR_RED      "\033[31m"
#define COLOR_GREEN    "\033[32m"
#define COLOR_YELLOW   "\033[33m"
#define COLOR_BLUE     "\033[34m"
#define COLOR_MAGENTA  "\033[35m"
#define COLOR_CYAN     "\033[36m"
#define COLOR_WHITE    "\033[37m"

#define COLOR_BG_BLACK   "\033[40m"
#define COLOR_BG_RED     "\033[41m"
#define COLOR_BG_GREEN   "\033[42m"
#define COLOR_BG_YELLOW  "\033[43m"
#define COLOR_BG_BLUE    "\033[44m"
#define COLOR_BG_MAGENTA "\033[45m"
#define COLOR_BG_CYAN    "\033[46m"
#define COLOR_BG_WHITE   "\033[47m"

/* ============ Advanced Data Structures ============ */

/* Plugin System */
typedef enum {
    PLUGIN_TYPE_COMMAND,
    PLUGIN_TYPE_FS,
    PLUGIN_TYPE_NETWORK,
    PLUGIN_TYPE_AI,
    PLUGIN_TYPE_SECURITY,
    PLUGIN_TYPE_DATABASE,
    PLUGIN_TYPE_MONITORING
} plugin_type_t;

typedef struct plugin {
    char name[128];
    char version[32];
    char author[128];
    char description[512];
    plugin_type_t type;
    void *handle;
    int (*init)(struct plugin *p);
    int (*deinit)(struct plugin *p);
    int (*execute)(struct plugin *p, char **args, int argc);
    void *data;
    struct plugin *next;
} plugin_t;

/* Database Management */
typedef struct database {
    char name[128];
    char path[MAX_PATH];
    char driver[64];
    sqlite3 *db;
    int connected;
    pthread_mutex_t lock;
    struct database *next;
} database_t;

/* Container Management */
typedef struct container {
    char name[128];
    char image[256];
    char status[32];
    int pid;
    int port;
    char ip[16];
    int running;
    struct container *next;
} container_t;

/* Cluster Management */
typedef struct cluster_node {
    char name[128];
    char address[256];
    int port;
    char status[32];
    int connected;
    float load;
    uint64_t memory_used;
    uint64_t memory_total;
    struct cluster_node *next;
} cluster_node_t;

/* Remote Session */
typedef struct remote_session {
    int socket;
    char user[64];
    char host[256];
    int port;
    int authenticated;
    time_t last_activity;
    struct remote_session *next;
} remote_session_t;

/* Backup Management */
typedef struct backup_set {
    char name[128];
    char source[MAX_PATH];
    char destination[MAX_PATH];
    char schedule[64];
    int encrypted;
    uint64_t size;
    time_t last_backup;
    int count;
    struct backup_set *next;
} backup_set_t;

/* Audit System */
typedef struct audit_entry {
    time_t timestamp;
    char user[64];
    char command[1024];
    char ip[16];
    int result;
    char message[512];
} audit_entry_t;

/* Monitoring System */
typedef struct monitor {
    uint64_t cpu_usage;
    uint64_t memory_usage;
    uint64_t disk_usage;
    uint64_t network_rx;
    uint64_t network_tx;
    int process_count;
    float load_avg[3];
    time_t timestamp;
} monitor_t;

/* Advanced Job Control */
typedef struct job {
    int job_id;
    int pid;
    char command[1024];
    char status[32];
    time_t start_time;
    time_t end_time;
    int exit_code;
    uint64_t cpu_time;
    uint64_t memory_peak;
    int signal;
    struct job *next;
} job_t;

/* Variable System */
typedef struct variable {
    char name[256];
    char value[MAX_LINE];
    int readonly;
    int exported;
    struct variable *next;
} variable_t;

/* Function System */
typedef struct shell_function {
    char name[256];
    char *body;
    int line_count;
    char **parameters;
    int param_count;
    struct shell_function *next;
} shell_function_t;

/* Alias System */
typedef struct alias {
    char name[128];
    char command[1024];
    struct alias *next;
} alias_t;

/* Command Registry */
typedef enum {
    CMD_TYPE_BUILTIN,
    CMD_TYPE_EXTERNAL,
    CMD_TYPE_PLUGIN,
    CMD_TYPE_FUNCTION,
    CMD_TYPE_ALIAS
} cmd_type_t;

typedef struct command {
    char name[128];
    char description[512];
    cmd_type_t type;
    int (*handler)(int argc, char **argv, void *context);
    void *context;
    struct command *next;
} command_t;

/* Parser State */
typedef struct parser_state {
    char input[MAX_LINE];
    char **tokens;
    int token_count;
    int current_token;
    int in_quote;
    int in_double_quote;
    int escape_next;
    char quote_char;
    jmp_buf error_jmp;
} parser_state_t;

/* ============ Global State ============ */
typedef struct {
    /* Core */
    char version[32];
    char release[64];
    int running;
    int initialized;
    int daemon_mode;
    int debug_mode;
    int verbose_mode;
    int secure_mode;
    
    /* Configuration */
    char config_file[MAX_PATH];
    char home_dir[MAX_PATH];
    char data_dir[MAX_PATH];
    char cache_dir[MAX_PATH];
    char log_dir[MAX_PATH];
    char plugin_dir[MAX_PATH];
    
    /* User */
    char username[64];
    uid_t uid;
    gid_t gid;
    
    /* Terminal */
    int terminal_width;
    int terminal_height;
    int term_color;
    char term_type[64];
    
    /* Systems */
    command_t *commands;
    variable_t *variables;
    shell_function_t *functions;
    alias_t *aliases;
    job_t *jobs;
    plugin_t *plugins;
    database_t *databases;
    container_t *containers;
    cluster_node_t *cluster_nodes;
    remote_session_t *sessions;
    backup_set_t *backups;
    
    /* History */
    char **history;
    int history_count;
    int history_max;
    
    /* Security */
    int authenticated;
    char session_id[64];
    time_t session_start;
    int audit_enabled;
    audit_entry_t audit_log[MAX_AUDIT_ENTRIES];
    int audit_count;
    
    /* Monitoring */
    monitor_t monitor;
    pthread_t monitor_thread;
    int monitoring_active;
    
    /* AI */
    void *ai_context;
    int ai_enabled;
    char ai_token[512];
    char ai_model[128];
    char ai_system_prompt[4096];
    float ai_temperature;
    int ai_max_tokens;
    pthread_mutex_t ai_lock;
    
    /* Locking */
    pthread_mutex_t command_lock;
    pthread_mutex_t variable_lock;
    pthread_mutex_t function_lock;
    pthread_mutex_t job_lock;
    pthread_mutex_t history_lock;
    pthread_mutex_t plugin_lock;
    pthread_mutex_t database_lock;
    pthread_mutex_t container_lock;
    pthread_mutex_t cluster_lock;
    pthread_mutex_t session_lock;
    pthread_mutex_t backup_lock;
    pthread_mutex_t audit_lock;
    pthread_mutex_t monitor_lock;
    
    /* Threads */
    pthread_t main_thread;
    pthread_t monitor_thread_id;
    pthread_t ai_thread_id;
    pthread_t network_thread_id;
    pthread_t backup_thread_id;
    
    /* HTTP */
    int http_server_running;
    int http_port;
    pthread_t http_thread;
} ztk_state_t;

ztk_state_t ztk = {0};

/* ============ Function Prototypes ============ */

/* Core */
int ztk_init(void);
void ztk_cleanup(void);
void ztk_loop(void);
void ztk_shutdown(void);

/* Security */
int ztk_secure_init(void);
int ztk_authenticate(const char *username, const char *password);
int ztk_audit_log(const char *command, int result, const char *message);
int ztk_check_permission(const char *resource, int mode);
char *ztk_encrypt(const char *data);
char *ztk_decrypt(const char *encrypted);

/* Command System */
int ztk_register_command(const char *name, const char *description, 
                          int (*handler)(int argc, char **argv, void *context),
                          void *context);
command_t *ztk_find_command(const char *name);
int ztk_execute_command(int argc, char **argv);
void ztk_list_commands(void);

/* Variable System */
int ztk_set_variable(const char *name, const char *value, int readonly, int exported);
char *ztk_get_variable(const char *name);
int ztk_unset_variable(const char *name);
void ztk_list_variables(void);

/* Function System */
int ztk_define_function(const char *name, const char *body, char **params, int param_count);
int ztk_execute_function(const char *name, char **args, int argc);
void ztk_list_functions(void);

/* Alias System */
int ztk_add_alias(const char *name, const char *command);
char *ztk_expand_alias(const char *name);
void ztk_list_aliases(void);

/* Job Control */
int ztk_add_job(int pid, const char *command);
job_t *ztk_find_job(int pid);
void ztk_update_jobs(void);
void ztk_list_jobs(void);
int ztk_kill_job(int job_id, int signal);
int ztk_wait_job(int job_id);

/* Plugin System */
int ztk_load_plugin(const char *path);
int ztk_unload_plugin(const char *name);
void ztk_list_plugins(void);
plugin_t *ztk_find_plugin(const char *name);

/* Database System */
int ztk_db_connect(const char *name, const char *path, const char *driver);
int ztk_db_execute(const char *db_name, const char *sql);
char **ztk_db_query(const char *db_name, const char *sql, int *rows, int *cols);
void ztk_db_disconnect(const char *name);
void ztk_list_databases(void);

/* Container System */
int ztk_container_create(const char *name, const char *image);
int ztk_container_start(const char *name);
int ztk_container_stop(const char *name);
int ztk_container_remove(const char *name);
void ztk_list_containers(void);

/* Cluster System */
int ztk_cluster_add_node(const char *name, const char *address, int port);
int ztk_cluster_connect(const char *name);
void ztk_cluster_list_nodes(void);
int ztk_cluster_broadcast(const char *command);

/* Backup System */
int ztk_backup_create(const char *name, const char *source, const char *dest);
int ztk_backup_restore(const char *name, const char *dest);
int ztk_backup_encrypt(const char *name, const char *password);
void ztk_list_backups(void);

/* Monitoring System */
void ztk_monitor_start(void);
void ztk_monitor_stop(void);
void ztk_monitor_report(void);
void *ztk_monitor_thread(void *arg);

/* AI System */
int ztk_ai_init(const char *token);
int ztk_ai_chat(const char *message, char *response, size_t response_size);
void ztk_ai_chat_loop(void);
void *ztk_ai_thread(void *arg);
void ztk_ai_clear_conversation(void);

/* Network System */
int ztk_http_server_start(int port);
void ztk_http_server_stop(void);
void *ztk_http_thread(void *arg);
int ztk_remote_connect(const char *host, int port);
void ztk_remote_loop(void);

/* Parser */
parser_state_t *ztk_parse_init(const char *input);
void ztk_parse_free(parser_state_t *state);
int ztk_parse_next_token(parser_state_t *state, char **token);
char **ztk_parse_command(parser_state_t *state, int *argc);

/* Shell Builtins */
int builtin_help(int argc, char **argv, void *context);
int builtin_about(int argc, char **argv, void *context);
int builtin_exit(int argc, char **argv, void *context);
int builtin_clear(int argc, char **argv, void *context);
int builtin_cd(int argc, char **argv, void *context);
int builtin_pwd(int argc, char **argv, void *context);
int builtin_ls(int argc, char **argv, void *context);
int builtin_cat(int argc, char **argv, void *context);
int builtin_echo(int argc, char **argv, void *context);
int builtin_mkdir(int argc, char **argv, void *context);
int builtin_rm(int argc, char **argv, void *context);
int builtin_cp(int argc, char **argv, void *context);
int builtin_mv(int argc, char **argv, void *context);
int builtin_chmod(int argc, char **argv, void *context);
int builtin_chown(int argc, char **argv, void *context);
int builtin_set(int argc, char **argv, void *context);
int builtin_unset(int argc, char **argv, void *context);
int builtin_export(int argc, char **argv, void *context);
int builtin_env(int argc, char **argv, void *context);
int builtin_source(int argc, char **argv, void *context);
int builtin_exec(int argc, char **argv, void *context);
int builtin_jobs(int argc, char **argv, void *context);
int builtin_fg(int argc, char **argv, void *context);
int builtin_bg(int argc, char **argv, void *context);
int builtin_kill(int argc, char **argv, void *context);
int builtin_wait(int argc, char **argv, void *context);
int builtin_ps(int argc, char **argv, void *context);
int builtin_top(int argc, char **argv, void *context);
int builtin_plugins(int argc, char **argv, void *context);
int builtin_databases(int argc, char **argv, void *context);
int builtin_containers(int argc, char **argv, void *context);
int builtin_cluster(int argc, char **argv, void *context);
int builtin_backup(int argc, char **argv, void *context);
int builtin_ai(int argc, char **argv, void *context);
int builtin_monitor(int argc, char **argv, void *context);
int builtin_audit(int argc, char **argv, void *context);
int builtin_http(int argc, char **argv, void *context);
int builtin_remote(int argc, char **argv, void *context);

/* Utility */
void ztk_log(const char *level, const char *format, ...);
void ztk_error(const char *format, ...);
void ztk_warn(const char *format, ...);
void ztk_info(const char *format, ...);
void ztk_debug(const char *format, ...);
char *ztk_strdup_safe(const char *str);
char *ztk_format_size(uint64_t size);
char *ztk_format_time(time_t t);
char *ztk_read_file(const char *path);
int ztk_write_file(const char *path, const char *content);
int ztk_file_exists(const char *path);
int ztk_is_directory(const char *path);
char *ztk_get_absolute_path(const char *path);
char *ztk_expand_path(const char *path);
void ztk_print_header(const char *title);
void ztk_print_footer(void);
void ztk_print_table_header(const char **headers, int count);
void ztk_print_table_row(const char **row, int count);

/* ============ Implementation ============ */

/* Core System */
int ztk_init(void) {
    memset(&ztk, 0, sizeof(ztk_t));
    
    /* Version */
    strcpy(ztk.version, ZTK_VERSION);
    strcpy(ztk.release, ZTK_RELEASE);
    ztk.running = 1;
    ztk.initialized = 1;
    
    /* Security */
    if (ztk_secure_init() != 0) {
        ztk_error("Failed to initialize security");
        return -1;
    }
    
    /* Get user info */
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        strcpy(ztk.username, pw->pw_name);
        ztk.uid = pw->pw_uid;
        ztk.gid = pw->pw_gid;
        strcpy(ztk.home_dir, pw->pw_dir);
    }
    
    /* Directories */
    snprintf(ztk.data_dir, sizeof(ztk.data_dir), "%s/.ztk", ztk.home_dir);
    snprintf(ztk.cache_dir, sizeof(ztk.cache_dir), "%s/.ztk/cache", ztk.home_dir);
    snprintf(ztk.log_dir, sizeof(ztk.log_dir), "%s/.ztk/logs", ztk.home_dir);
    snprintf(ztk.plugin_dir, sizeof(ztk.plugin_dir), "%s/.ztk/plugins", ztk.home_dir);
    
    mkdir(ztk.data_dir, 0755);
    mkdir(ztk.cache_dir, 0755);
    mkdir(ztk.log_dir, 0755);
    mkdir(ztk.plugin_dir, 0755);
    
    /* Terminal */
    ztk.terminal_width = 80;
    ztk.terminal_height = 24;
    ztk.term_color = 1;
    strcpy(ztk.term_type, getenv("TERM") ? getenv("TERM") : "xterm-256color");
    
    /* Initialize mutexes */
    pthread_mutex_init(&ztk.command_lock, NULL);
    pthread_mutex_init(&ztk.variable_lock, NULL);
    pthread_mutex_init(&ztk.function_lock, NULL);
    pthread_mutex_init(&ztk.job_lock, NULL);
    pthread_mutex_init(&ztk.history_lock, NULL);
    pthread_mutex_init(&ztk.plugin_lock, NULL);
    pthread_mutex_init(&ztk.database_lock, NULL);
    pthread_mutex_init(&ztk.container_lock, NULL);
    pthread_mutex_init(&ztk.cluster_lock, NULL);
    pthread_mutex_init(&ztk.session_lock, NULL);
    pthread_mutex_init(&ztk.backup_lock, NULL);
    pthread_mutex_init(&ztk.audit_lock, NULL);
    pthread_mutex_init(&ztk.monitor_lock, NULL);
    pthread_mutex_init(&ztk.ai_lock, NULL);
    
    /* History */
    ztk.history_max = MAX_HISTORY;
    ztk.history = malloc(sizeof(char*) * ztk.history_max);
    ztk.history_count = 0;
    
    /* Register built-in commands */
    ztk_register_command("help", "Show this help message", builtin_help, NULL);
    ztk_register_command("about", "Show system information", builtin_about, NULL);
    ztk_register_command("exit", "Exit the shell", builtin_exit, NULL);
    ztk_register_command("clear", "Clear the terminal", builtin_clear, NULL);
    ztk_register_command("cd", "Change directory", builtin_cd, NULL);
    ztk_register_command("pwd", "Print working directory", builtin_pwd, NULL);
    ztk_register_command("ls", "List directory contents", builtin_ls, NULL);
    ztk_register_command("cat", "Display file contents", builtin_cat, NULL);
    ztk_register_command("echo", "Display a message", builtin_echo, NULL);
    ztk_register_command("mkdir", "Create a directory", builtin_mkdir, NULL);
    ztk_register_command("rm", "Remove files or directories", builtin_rm, NULL);
    ztk_register_command("cp", "Copy files", builtin_cp, NULL);
    ztk_register_command("mv", "Move files", builtin_mv, NULL);
    ztk_register_command("chmod", "Change file permissions", builtin_chmod, NULL);
    ztk_register_command("chown", "Change file owner", builtin_chown, NULL);
    ztk_register_command("set", "Set a variable", builtin_set, NULL);
    ztk_register_command("unset", "Unset a variable", builtin_unset, NULL);
    ztk_register_command("export", "Export a variable", builtin_export, NULL);
    ztk_register_command("env", "Display environment", builtin_env, NULL);
    ztk_register_command("source", "Source a script", builtin_source, NULL);
    ztk_register_command("exec", "Execute a command", builtin_exec, NULL);
    ztk_register_command("jobs", "List background jobs", builtin_jobs, NULL);
    ztk_register_command("fg", "Bring job to foreground", builtin_fg, NULL);
    ztk_register_command("bg", "Run job in background", builtin_bg, NULL);
    ztk_register_command("kill", "Send signal to job", builtin_kill, NULL);
    ztk_register_command("wait", "Wait for job", builtin_wait, NULL);
    ztk_register_command("ps", "List processes", builtin_ps, NULL);
    ztk_register_command("top", "Display process monitor", builtin_top, NULL);
    ztk_register_command("plugins", "Manage plugins", builtin_plugins, NULL);
    ztk_register_command("databases", "Manage databases", builtin_databases, NULL);
    ztk_register_command("containers", "Manage containers", builtin_containers, NULL);
    ztk_register_command("cluster", "Manage cluster", builtin_cluster, NULL);
    ztk_register_command("backup", "Manage backups", builtin_backup, NULL);
    ztk_register_command("ai", "AI assistant", builtin_ai, NULL);
    ztk_register_command("monitor", "System monitor", builtin_monitor, NULL);
    ztk_register_command("audit", "Audit log", builtin_audit, NULL);
    ztk_register_command("http", "HTTP server", builtin_http, NULL);
    ztk_register_command("remote", "Remote connection", builtin_remote, NULL);
    
    ztk_info("ZTK Shell %s %s initialized", ZTK_VERSION, ZTK_RELEASE);
    ztk_info("User: %s Home: %s", ztk.username, ztk.home_dir);
    
    return 0;
}

void ztk_cleanup(void) {
    ztk.running = 0;
    
    /* Stop monitoring */
    if (ztk.monitoring_active) {
        ztk_monitor_stop();
    }
    
    /* Stop HTTP server */
    if (ztk.http_server_running) {
        ztk_http_server_stop();
    }
    
    /* Clean up commands */
    command_t *cmd = ztk.commands;
    while (cmd) {
        command_t *next = cmd->next;
        free(cmd);
        cmd = next;
    }
    
    /* Clean up variables */
    variable_t *var = ztk.variables;
    while (var) {
        variable_t *next = var->next;
        free(var);
        var = next;
    }
    
    /* Clean up functions */
    shell_function_t *func = ztk.functions;
    while (func) {
        shell_function_t *next = func->next;
        if (func->body) free(func->body);
        if (func->parameters) free(func->parameters);
        free(func);
        func = next;
    }
    
    /* Clean up aliases */
    alias_t *alias = ztk.aliases;
    while (alias) {
        alias_t *next = alias->next;
        free(alias);
        alias = next;
    }
    
    /* Clean up jobs */
    job_t *job = ztk.jobs;
    while (job) {
        job_t *next = job->next;
        free(job);
        job = next;
    }
    
    /* Clean up history */
    for (int i = 0; i < ztk.history_count; i++) {
        if (ztk.history[i]) free(ztk.history[i]);
    }
    free(ztk.history);
    
    /* Clean up plugins */
    plugin_t *plugin = ztk.plugins;
    while (plugin) {
        plugin_t *next = plugin->next;
        if (plugin->handle) {
            dlclose(plugin->handle);
        }
        free(plugin);
        plugin = next;
    }
    
    /* Clean up databases */
    database_t *db = ztk.databases;
    while (db) {
        database_t *next = db->next;
        if (db->db) {
            sqlite3_close(db->db);
        }
        pthread_mutex_destroy(&db->lock);
        free(db);
        db = next;
    }
    
    /* Destroy mutexes */
    pthread_mutex_destroy(&ztk.command_lock);
    pthread_mutex_destroy(&ztk.variable_lock);
    pthread_mutex_destroy(&ztk.function_lock);
    pthread_mutex_destroy(&ztk.job_lock);
    pthread_mutex_destroy(&ztk.history_lock);
    pthread_mutex_destroy(&ztk.plugin_lock);
    pthread_mutex_destroy(&ztk.database_lock);
    pthread_mutex_destroy(&ztk.container_lock);
    pthread_mutex_destroy(&ztk.cluster_lock);
    pthread_mutex_destroy(&ztk.session_lock);
    pthread_mutex_destroy(&ztk.backup_lock);
    pthread_mutex_destroy(&ztk.audit_lock);
    pthread_mutex_destroy(&ztk.monitor_lock);
    pthread_mutex_destroy(&ztk.ai_lock);
    
    ztk_info("ZTK Shell shutdown complete");
}

/* Security System */
int ztk_secure_init(void) {
    /* Initialize OpenSSL if available */
    #ifdef USE_OPENSSL
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    #endif
    
    ztk.secure_mode = 1;
    ztk.audit_enabled = 1;
    
    /* Generate session ID */
    snprintf(ztk.session_id, sizeof(ztk.session_id), "%lu-%ld", 
             (unsigned long)time(NULL), (long)getpid());
    
    return 0;
}

int ztk_authenticate(const char *username, const char *password) {
    /* This is a placeholder - real implementation would use PAM or similar */
    if (strcmp(username, ztk.username) == 0 && strlen(password) >= MIN_PASSWORD_LENGTH) {
        ztk.authenticated = 1;
        ztk.session_start = time(NULL);
        ztk_audit_log("login", 1, "Successful login");
        return 1;
    }
    ztk_audit_log("login", 0, "Failed login attempt");
    return 0;
}

int ztk_audit_log(const char *command, int result, const char *message) {
    pthread_mutex_lock(&ztk.audit_lock);
    
    if (ztk.audit_count >= MAX_AUDIT_ENTRIES) {
        ztk.audit_count = 0;
    }
    
    audit_entry_t *entry = &ztk.audit_log[ztk.audit_count++];
    entry->timestamp = time(NULL);
    strncpy(entry->user, ztk.username, sizeof(entry->user) - 1);
    strncpy(entry->command, command, sizeof(entry->command) - 1);
    strncpy(entry->ip, "127.0.0.1", sizeof(entry->ip) - 1);
    entry->result = result;
    strncpy(entry->message, message, sizeof(entry->message) - 1);
    
    pthread_mutex_unlock(&ztk.audit_lock);
    return 0;
}

int ztk_check_permission(const char *resource, int mode) {
    /* Check if user has permission to access resource */
    struct stat st;
    if (stat(resource, &st) != 0) {
        return 0;
    }
    
    uid_t uid = getuid();
    gid_t gid = getgid();
    
    /* Root can do anything */
    if (uid == 0) return 1;
    
    /* Check owner */
    if (st.st_uid == uid) {
        return (st.st_mode & (mode << 6)) == (mode << 6);
    }
    
    /* Check group */
    if (st.st_gid == gid) {
        return (st.st_mode & (mode << 3)) == (mode << 3);
    }
    
    /* Check others */
    return (st.st_mode & mode) == mode;
}

/* Command System */
int ztk_register_command(const char *name, const char *description,
                          int (*handler)(int argc, char **argv, void *context),
                          void *context) {
    pthread_mutex_lock(&ztk.command_lock);
    
    command_t *cmd = malloc(sizeof(command_t));
    if (!cmd) {
        pthread_mutex_unlock(&ztk.command_lock);
        return -1;
    }
    
    strncpy(cmd->name, name, sizeof(cmd->name) - 1);
    strncpy(cmd->description, description, sizeof(cmd->description) - 1);
    cmd->handler = handler;
    cmd->context = context;
    cmd->type = CMD_TYPE_BUILTIN;
    cmd->next = ztk.commands;
    ztk.commands = cmd;
    
    pthread_mutex_unlock(&ztk.command_lock);
    return 0;
}

command_t *ztk_find_command(const char *name) {
    pthread_mutex_lock(&ztk.command_lock);
    
    command_t *cmd = ztk.commands;
    while (cmd) {
        if (strcmp(cmd->name, name) == 0) {
            pthread_mutex_unlock(&ztk.command_lock);
            return cmd;
        }
        cmd = cmd->next;
    }
    
    pthread_mutex_unlock(&ztk.command_lock);
    return NULL;
}

int ztk_execute_command(int argc, char **argv) {
    if (argc == 0) return 0;
    
    char *cmd_name = argv[0];
    
    /* Check for alias */
    char *alias_cmd = ztk_expand_alias(cmd_name);
    if (alias_cmd) {
        /* Parse and execute alias */
        char *args[MAX_ARGS];
        int new_argc = 0;
        char *token = strtok(alias_cmd, " ");
        while (token && new_argc < MAX_ARGS - 1) {
            args[new_argc++] = token;
            token = strtok(NULL, " ");
        }
        args[new_argc] = NULL;
        
        if (new_argc > 0) {
            return ztk_execute_command(new_argc, args);
        }
    }
    
    /* Check for function */
    shell_function_t *func = NULL;
    pthread_mutex_lock(&ztk.function_lock);
    func = ztk.functions;
    while (func) {
        if (strcmp(func->name, cmd_name) == 0) break;
        func = func->next;
    }
    pthread_mutex_unlock(&ztk.function_lock);
    
    if (func) {
        return ztk_execute_function(cmd_name, argv + 1, argc - 1);
    }
    
    /* Check for builtin command */
    command_t *cmd = ztk_find_command(cmd_name);
    if (cmd && cmd->handler) {
        ztk_audit_log(cmd_name, 1, "Command executed");
        return cmd->handler(argc, argv, cmd->context);
    }
    
    /* Check for external command */
    char *path = getenv("PATH");
    if (path) {
        char *path_copy = strdup(path);
        char *dir = strtok(path_copy, ":");
        while (dir) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd_name);
            if (access(full_path, X_OK) == 0) {
                /* Execute external command */
                pid_t pid = fork();
                if (pid == 0) {
                    execv(full_path, argv);
                    exit(1);
                } else if (pid > 0) {
                    int status;
                    waitpid(pid, &status, 0);
                    free(path_copy);
                    return WEXITSTATUS(status);
                }
            }
            dir = strtok(NULL, ":");
        }
        free(path_copy);
    }
    
    ztk_error("Command not found: %s", cmd_name);
    return -1;
}

void ztk_list_commands(void) {
    ztk_print_header("Available Commands");
    
    command_t *cmd = ztk.commands;
    while (cmd) {
        printf("  %-20s %s\n", cmd->name, cmd->description);
        cmd = cmd->next;
    }
    
    printf("\n  %-20s %s\n", "[command]", "Any external command in PATH");
    printf("  %-20s %s\n", "[function]", "User-defined function");
    printf("  %-20s %s\n", "[alias]", "Command alias");
    
    ztk_print_footer();
}

/* Variable System */
int ztk_set_variable(const char *name, const char *value, int readonly, int exported) {
    if (!name || !value) return -1;
    
    pthread_mutex_lock(&ztk.variable_lock);
    
    /* Check if variable already exists */
    variable_t *var = ztk.variables;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            if (var->readonly) {
                pthread_mutex_unlock(&ztk.variable_lock);
                ztk_error("Variable %s is read-only", name);
                return -1;
            }
            free(var->value);
            var->value = strdup(value);
            var->exported = exported;
            pthread_mutex_unlock(&ztk.variable_lock);
            
            if (exported) {
                setenv(name, value, 1);
            }
            return 0;
        }
        var = var->next;
    }
    
    /* Create new variable */
    var = malloc(sizeof(variable_t));
    if (!var) {
        pthread_mutex_unlock(&ztk.variable_lock);
        return -1;
    }
    
    strncpy(var->name, name, sizeof(var->name) - 1);
    var->value = strdup(value);
    var->readonly = readonly;
    var->exported = exported;
    var->next = ztk.variables;
    ztk.variables = var;
    
    pthread_mutex_unlock(&ztk.variable_lock);
    
    if (exported) {
        setenv(name, value, 1);
    }
    
    return 0;
}

char *ztk_get_variable(const char *name) {
    if (!name) return NULL;
    
    /* Check environment variables first */
    char *env_value = getenv(name);
    if (env_value) {
        return env_value;
    }
    
    pthread_mutex_lock(&ztk.variable_lock);
    
    variable_t *var = ztk.variables;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            pthread_mutex_unlock(&ztk.variable_lock);
            return var->value;
        }
        var = var->next;
    }
    
    pthread_mutex_unlock(&ztk.variable_lock);
    return NULL;
}

int ztk_unset_variable(const char *name) {
    if (!name) return -1;
    
    pthread_mutex_lock(&ztk.variable_lock);
    
    variable_t *var = ztk.variables;
    variable_t *prev = NULL;
    while (var) {
        if (strcmp(var->name, name) == 0) {
            if (var->readonly) {
                pthread_mutex_unlock(&ztk.variable_lock);
                ztk_error("Variable %s is read-only", name);
                return -1;
            }
            
            if (prev) {
                prev->next = var->next;
            } else {
                ztk.variables = var->next;
            }
            
            free(var->value);
            free(var);
            
            pthread_mutex_unlock(&ztk.variable_lock);
            unsetenv(name);
            return 0;
        }
        prev = var;
        var = var->next;
    }
    
    pthread_mutex_unlock(&ztk.variable_lock);
    return -1;
}

void ztk_list_variables(void) {
    ztk_print_header("Variables");
    
    variable_t *var = ztk.variables;
    while (var) {
        printf("  %s=%s %s\n", var->name, var->value, 
               var->readonly ? "(readonly)" : var->exported ? "(exported)" : "");
        var = var->next;
    }
    
    ztk_print_footer();
}

/* Function System */
int ztk_define_function(const char *name, const char *body, char **params, int param_count) {
    if (!name || !body) return -1;
    
    pthread_mutex_lock(&ztk.function_lock);
    
    /* Check if function exists */
    shell_function_t *func = ztk.functions;
    while (func) {
        if (strcmp(func->name, name) == 0) {
            /* Replace function */
            if (func->body) free(func->body);
            if (func->parameters) free(func->parameters);
            func->body = strdup(body);
            func->param_count = param_count;
            if (param_count > 0) {
                func->parameters = malloc(sizeof(char*) * param_count);
                for (int i = 0; i < param_count; i++) {
                    func->parameters[i] = strdup(params[i]);
                }
            }
            pthread_mutex_unlock(&ztk.function_lock);
            return 0;
        }
        func = func->next;
    }
    
    /* Create new function */
    func = malloc(sizeof(shell_function_t));
    if (!func) {
        pthread_mutex_unlock(&ztk.function_lock);
        return -1;
    }
    
    strncpy(func->name, name, sizeof(func->name) - 1);
    func->body = strdup(body);
    func->param_count = param_count;
    func->line_count = 1;
    if (param_count > 0) {
        func->parameters = malloc(sizeof(char*) * param_count);
        for (int i = 0; i < param_count; i++) {
            func->parameters[i] = strdup(params[i]);
        }
    }
    func->next = ztk.functions;
    ztk.functions = func;
    
    pthread_mutex_unlock(&ztk.function_lock);
    return 0;
}

int ztk_execute_function(const char *name, char **args, int argc) {
    shell_function_t *func = NULL;
    
    pthread_mutex_lock(&ztk.function_lock);
    func = ztk.functions;
    while (func) {
        if (strcmp(func->name, name) == 0) break;
        func = func->next;
    }
    pthread_mutex_unlock(&ztk.function_lock);
    
    if (!func) {
        ztk_error("Function not found: %s", name);
        return -1;
    }
    
    /* Set parameters */
    for (int i = 0; i < func->param_count && i < argc; i++) {
        ztk_set_variable(func->parameters[i], args[i], 0, 0);
    }
    
    /* Execute function body */
    char *line = strtok(func->body, "\n");
    while (line) {
        char *trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        
        if (strlen(trimmed) > 0 && trimmed[0] != '#') {
            /* Parse and execute the line */
            char *cmd_args[MAX_ARGS];
            int cmd_argc = 0;
            char *token = strtok(trimmed, " ");
            while (token && cmd_argc < MAX_ARGS - 1) {
                /* Expand variables */
                if (token[0] == '$') {
                    char *var_value = ztk_get_variable(token + 1);
                    if (var_value) {
                        cmd_args[cmd_argc++] = var_value;
                    } else {
                        cmd_args[cmd_argc++] = token;
                    }
                } else {
                    cmd_args[cmd_argc++] = token;
                }
                token = strtok(NULL, " ");
            }
            cmd_args[cmd_argc] = NULL;
            
            if (cmd_argc > 0) {
                ztk_execute_command(cmd_argc, cmd_args);
            }
        }
        line = strtok(NULL, "\n");
    }
    
    return 0;
}

void ztk_list_functions(void) {
    ztk_print_header("Functions");
    
    shell_function_t *func = ztk.functions;
    while (func) {
        printf("  %s(", func->name);
        for (int i = 0; i < func->param_count; i++) {
            printf("%s%s", func->parameters[i], 
                   i < func->param_count - 1 ? ", " : "");
        }
        printf(")\n");
        func = func->next;
    }
    
    ztk_print_footer();
}

/* Alias System */
int ztk_add_alias(const char *name, const char *command) {
    if (!name || !command) return -1;
    
    alias_t *alias = ztk.aliases;
    while (alias) {
        if (strcmp(alias->name, name) == 0) {
            /* Replace alias */
            strncpy(alias->command, command, sizeof(alias->command) - 1);
            return 0;
        }
        alias = alias->next;
    }
    
    alias = malloc(sizeof(alias_t));
    if (!alias) return -1;
    
    strncpy(alias->name, name, sizeof(alias->name) - 1);
    strncpy(alias->command, command, sizeof(alias->command) - 1);
    alias->next = ztk.aliases;
    ztk.aliases = alias;
    
    return 0;
}

char *ztk_expand_alias(const char *name) {
    alias_t *alias = ztk.aliases;
    while (alias) {
        if (strcmp(alias->name, name) == 0) {
            return alias->command;
        }
        alias = alias->next;
    }
    return NULL;
}

void ztk_list_aliases(void) {
    ztk_print_header("Aliases");
    
    alias_t *alias = ztk.aliases;
    while (alias) {
        printf("  %s='%s'\n", alias->name, alias->command);
        alias = alias->next;
    }
    
    ztk_print_footer();
}

/* Job Control */
int ztk_add_job(int pid, const char *command) {
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = malloc(sizeof(job_t));
    if (!job) {
        pthread_mutex_unlock(&ztk.job_lock);
        return -1;
    }
    
    job->job_id = ztk.jobs ? ztk.jobs->job_id + 1 : 1;
    job->pid = pid;
    strncpy(job->command, command, sizeof(job->command) - 1);
    strcpy(job->status, "RUNNING");
    job->start_time = time(NULL);
    job->end_time = 0;
    job->exit_code = 0;
    job->cpu_time = 0;
    job->memory_peak = 0;
    job->signal = 0;
    job->next = ztk.jobs;
    ztk.jobs = job;
    
    pthread_mutex_unlock(&ztk.job_lock);
    return job->job_id;
}

job_t *ztk_find_job(int pid) {
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = ztk.jobs;
    while (job) {
        if (job->pid == pid) {
            pthread_mutex_unlock(&ztk.job_lock);
            return job;
        }
        job = job->next;
    }
    
    pthread_mutex_unlock(&ztk.job_lock);
    return NULL;
}

void ztk_update_jobs(void) {
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = ztk.jobs;
    while (job) {
        if (strcmp(job->status, "RUNNING") == 0) {
            int status;
            pid_t result = waitpid(job->pid, &status, WNOHANG);
            if (result == job->pid) {
                if (WIFEXITED(status)) {
                    job->exit_code = WEXITSTATUS(status);
                    strcpy(job->status, "COMPLETED");
                } else if (WIFSIGNALED(status)) {
                    job->signal = WTERMSIG(status);
                    strcpy(job->status, "SIGNALED");
                }
                job->end_time = time(NULL);
            }
        }
        job = job->next;
    }
    
    pthread_mutex_unlock(&ztk.job_lock);
}

void ztk_list_jobs(void) {
    ztk_update_jobs();
    
    ztk_print_header("Jobs");
    printf("  %-8s %-8s %-12s %-30s %s\n", 
           "Job ID", "PID", "Status", "Command", "Start Time");
    printf("  %-8s %-8s %-12s %-30s %s\n", 
           "------", "---", "------", "-------", "----------");
    
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = ztk.jobs;
    while (job) {
        char start_time[64];
        strftime(start_time, sizeof(start_time), "%H:%M:%S", localtime(&job->start_time));
        
        printf("  %-8d %-8d %-12s %-30s %s\n",
               job->job_id, job->pid, job->status, job->command, start_time);
        job = job->next;
    }
    
    pthread_mutex_unlock(&ztk.job_lock);
    ztk_print_footer();
}

int ztk_kill_job(int job_id, int signal) {
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = ztk.jobs;
    while (job) {
        if (job->job_id == job_id) {
            pthread_mutex_unlock(&ztk.job_lock);
            if (kill(job->pid, signal) == 0) {
                ztk_info("Job %d killed with signal %d", job_id, signal);
                return 0;
            }
            ztk_error("Failed to kill job %d", job_id);
            return -1;
        }
        job = job->next;
    }
    
    pthread_mutex_unlock(&ztk.job_lock);
    ztk_error("Job %d not found", job_id);
    return -1;
}

int ztk_wait_job(int job_id) {
    pthread_mutex_lock(&ztk.job_lock);
    
    job_t *job = ztk.jobs;
    while (job) {
        if (job->job_id == job_id) {
            pthread_mutex_unlock(&ztk.job_lock);
            
            int status;
            waitpid(job->pid, &status, 0);
            
            pthread_mutex_lock(&ztk.job_lock);
            if (WIFEXITED(status)) {
                job->exit_code = WEXITSTATUS(status);
                strcpy(job->status, "COMPLETED");
            }
            pthread_mutex_unlock(&ztk.job_lock);
            
            return 0;
        }
        job = job->next;
    }
    
    pthread_mutex_unlock(&ztk.job_lock);
    ztk_error("Job %d not found", job_id);
    return -1;
}

/* Plugin System */
int ztk_load_plugin(const char *path) {
    char real_path[MAX_PATH];
    if (path[0] != '/') {
        snprintf(real_path, sizeof(real_path), "%s/%s", ztk.plugin_dir, path);
    } else {
        strncpy(real_path, path, sizeof(real_path) - 1);
    }
    
    void *handle = dlopen(real_path, RTLD_NOW);
    if (!handle) {
        ztk_error("Failed to load plugin: %s", dlerror());
        return -1;
    }
    
    /* Get plugin info */
    const char *(*get_name)(void) = dlsym(handle, "plugin_name");
    const char *(*get_version)(void) = dlsym(handle, "plugin_version");
    const char *(*get_description)(void) = dlsym(handle, "plugin_description");
    int (*plugin_init)(struct plugin *) = dlsym(handle, "plugin_init");
    int (*plugin_deinit)(struct plugin *) = dlsym(handle, "plugin_deinit");
    
    if (!get_name || !get_version || !plugin_init) {
        ztk_error("Invalid plugin interface");
        dlclose(handle);
        return -1;
    }
    
    plugin_t *plugin = malloc(sizeof(plugin_t));
    if (!plugin) {
        dlclose(handle);
        return -1;
    }
    
    strncpy(plugin->name, get_name(), sizeof(plugin->name) - 1);
    strncpy(plugin->version, get_version(), sizeof(plugin->version) - 1);
    if (get_description) {
        strncpy(plugin->description, get_description(), sizeof(plugin->description) - 1);
    } else {
        strcpy(plugin->description, "No description");
    }
    plugin->handle = handle;
    plugin->init = plugin_init;
    plugin->deinit = plugin_deinit;
    plugin->execute = dlsym(handle, "plugin_execute");
    plugin->data = NULL;
    plugin->type = PLUGIN_TYPE_COMMAND;
    
    if (plugin->init(plugin) == 0) {
        plugin->next = ztk.plugins;
        ztk.plugins = plugin;
        ztk_info("Plugin loaded: %s v%s", plugin->name, plugin->version);
        return 0;
    }
    
    dlclose(handle);
    free(plugin);
    return -1;
}

int ztk_unload_plugin(const char *name) {
    pthread_mutex_lock(&ztk.plugin_lock);
    
    plugin_t *plugin = ztk.plugins;
    plugin_t *prev = NULL;
    while (plugin) {
        if (strcmp(plugin->name, name) == 0) {
            if (plugin->deinit) {
                plugin->deinit(plugin);
            }
            dlclose(plugin->handle);
            
            if (prev) {
                prev->next = plugin->next;
            } else {
                ztk.plugins = plugin->next;
            }
            
            free(plugin);
            pthread_mutex_unlock(&ztk.plugin_lock);
            ztk_info("Plugin unloaded: %s", name);
            return 0;
        }
        prev = plugin;
        plugin = plugin->next;
    }
    
    pthread_mutex_unlock(&ztk.plugin_lock);
    ztk_error("Plugin not found: %s", name);
    return -1;
}

void ztk_list_plugins(void) {
    ztk_print_header("Plugins");
    
    pthread_mutex_lock(&ztk.plugin_lock);
    
    plugin_t *plugin = ztk.plugins;
    while (plugin) {
        printf("  %-20s v%-8s %s\n", 
               plugin->name, plugin->version, plugin->description);
        plugin = plugin->next;
    }
    
    pthread_mutex_unlock(&ztk.plugin_lock);
    ztk_print_footer();
}

/* Database System */
int ztk_db_connect(const char *name, const char *path, const char *driver) {
    if (strcmp(driver, "sqlite") != 0) {
        ztk_error("Unsupported driver: %s", driver);
        return -1;
    }
    
    pthread_mutex_lock(&ztk.database_lock);
    
    /* Check if database exists */
    database_t *db = ztk.databases;
    while (db) {
        if (strcmp(db->name, name) == 0) {
            pthread_mutex_unlock(&ztk.database_lock);
            ztk_error("Database %s already exists", name);
            return -1;
        }
        db = db->next;
    }
    
    db = malloc(sizeof(database_t));
    if (!db) {
        pthread_mutex_unlock(&ztk.database_lock);
        return -1;
    }
    
    strncpy(db->name, name, sizeof(db->name) - 1);
    strncpy(db->path, path, sizeof(db->path) - 1);
    strncpy(db->driver, driver, sizeof(db->driver) - 1);
    pthread_mutex_init(&db->lock, NULL);
    
    if (sqlite3_open(path, &db->db) == SQLITE_OK) {
        db->connected = 1;
        db->next = ztk.databases;
        ztk.databases = db;
        pthread_mutex_unlock(&ztk.database_lock);
        ztk_info("Connected to database: %s", name);
        return 0;
    }
    
    sqlite3_close(db->db);
    free(db);
    pthread_mutex_unlock(&ztk.database_lock);
    ztk_error("Failed to connect to database: %s", name);
    return -1;
}

int ztk_db_execute(const char *db_name, const char *sql) {
    pthread_mutex_lock(&ztk.database_lock);
    
    database_t *db = ztk.databases;
    while (db) {
        if (strcmp(db->name, db_name) == 0) {
            pthread_mutex_unlock(&ztk.database_lock);
            
            pthread_mutex_lock(&db->lock);
            char *err_msg = NULL;
            int result = sqlite3_exec(db->db, sql, NULL, NULL, &err_msg);
            pthread_mutex_unlock(&db->lock);
            
            if (result != SQLITE_OK) {
                ztk_error("SQL error: %s", err_msg);
                sqlite3_free(err_msg);
                return -1;
            }
            return 0;
        }
        db = db->next;
    }
    
    pthread_mutex_unlock(&ztk.database_lock);
    ztk_error("Database not found: %s", db_name);
    return -1;
}

char **ztk_db_query(const char *db_name, const char *sql, int *rows, int *cols) {
    pthread_mutex_lock(&ztk.database_lock);
    
    database_t *db = ztk.databases;
    while (db) {
        if (strcmp(db->name, db_name) == 0) {
            pthread_mutex_unlock(&ztk.database_lock);
            
            pthread_mutex_lock(&db->lock);
            
            sqlite3_stmt *stmt;
            if (sqlite3_prepare_v2(db->db, sql, -1, &stmt, NULL) != SQLITE_OK) {
                pthread_mutex_unlock(&db->lock);
                ztk_error("Failed to prepare statement");
                return NULL;
            }
            
            *cols = sqlite3_column_count(stmt);
            *rows = 0;
            
            /* Count rows */
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                (*rows)++;
            }
            sqlite3_reset(stmt);
            
            /* Allocate result array */
            char **result = malloc(sizeof(char*) * ((*rows) + 1));
            if (!result) {
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&db->lock);
                return NULL;
            }
            
            /* Fetch data */
            int row = 0;
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                char *row_data = malloc(sizeof(char) * 1024);
                int offset = 0;
                for (int i = 0; i < *cols; i++) {
                    const char *value = (const char*)sqlite3_column_text(stmt, i);
                    if (value) {
                        offset += snprintf(row_data + offset, 1024 - offset, 
                                          "%s%s", i > 0 ? "\t" : "", value);
                    } else {
                        offset += snprintf(row_data + offset, 1024 - offset, 
                                          "%sNULL", i > 0 ? "\t" : "");
                    }
                }
                result[row++] = row_data;
            }
            result[row] = NULL;
            
            sqlite3_finalize(stmt);
            pthread_mutex_unlock(&db->lock);
            return result;
        }
        db = db->next;
    }
    
    pthread_mutex_unlock(&ztk.database_lock);
    ztk_error("Database not found: %s", db_name);
    return NULL;
}

void ztk_db_disconnect(const char *name) {
    pthread_mutex_lock(&ztk.database_lock);
    
    database_t *db = ztk.databases;
    database_t *prev = NULL;
    while (db) {
        if (strcmp(db->name, name) == 0) {
            if (prev) {
                prev->next = db->next;
            } else {
                ztk.databases = db->next;
            }
            
            if (db->db) {
                sqlite3_close(db->db);
            }
            pthread_mutex_destroy(&db->lock);
            free(db);
            pthread_mutex_unlock(&ztk.database_lock);
            ztk_info("Disconnected from database: %s", name);
            return;
        }
        prev = db;
        db = db->next;
    }
    
    pthread_mutex_unlock(&ztk.database_lock);
    ztk_error("Database not found: %s", name);
}

void ztk_list_databases(void) {
    ztk_print_header("Databases");
    
    pthread_mutex_lock(&ztk.database_lock);
    
    database_t *db = ztk.databases;
    while (db) {
        printf("  %-20s %-12s %s\n", 
               db->name, db->driver, db->connected ? "Connected" : "Disconnected");
        db = db->next;
    }
    
    pthread_mutex_unlock(&ztk.database_lock);
    ztk_print_footer();
}

/* Container System */
int ztk_container_create(const char *name, const char *image) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "docker create --name %s %s 2>/dev/null", name, image);
    int result = system(cmd);
    if (result != 0) {
        /* Try podman if docker fails */
        snprintf(cmd, sizeof(cmd), "podman create --name %s %s 2>/dev/null", name, image);
        result = system(cmd);
    }
    
    if (result == 0) {
        container_t *container = malloc(sizeof(container_t));
        if (container) {
            strncpy(container->name, name, sizeof(container->name) - 1);
            strncpy(container->image, image, sizeof(container->image) - 1);
            strcpy(container->status, "CREATED");
            container->pid = -1;
            container->port = -1;
            container->running = 0;
            
            pthread_mutex_lock(&ztk.container_lock);
            container->next = ztk.containers;
            ztk.containers = container;
            pthread_mutex_unlock(&ztk.container_lock);
        }
        ztk_info("Container created: %s", name);
        return 0;
    }
    
    ztk_error("Failed to create container: %s", name);
    return -1;
}

int ztk_container_start(const char *name) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "docker start %s 2>/dev/null", name);
    int result = system(cmd);
    if (result != 0) {
        snprintf(cmd, sizeof(cmd), "podman start %s 2>/dev/null", name);
        result = system(cmd);
    }
    
    if (result == 0) {
        pthread_mutex_lock(&ztk.container_lock);
        container_t *container = ztk.containers;
        while (container) {
            if (strcmp(container->name, name) == 0) {
                strcpy(container->status, "RUNNING");
                container->running = 1;
                break;
            }
            container = container->next;
        }
        pthread_mutex_unlock(&ztk.container_lock);
        ztk_info("Container started: %s", name);
        return 0;
    }
    
    ztk_error("Failed to start container: %s", name);
    return -1;
}

int ztk_container_stop(const char *name) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "docker stop %s 2>/dev/null", name);
    int result = system(cmd);
    if (result != 0) {
        snprintf(cmd, sizeof(cmd), "podman stop %s 2>/dev/null", name);
        result = system(cmd);
    }
    
    if (result == 0) {
        pthread_mutex_lock(&ztk.container_lock);
        container_t *container = ztk.containers;
        while (container) {
            if (strcmp(container->name, name) == 0) {
                strcpy(container->status, "STOPPED");
                container->running = 0;
                break;
            }
            container = container->next;
        }
        pthread_mutex_unlock(&ztk.container_lock);
        ztk_info("Container stopped: %s", name);
        return 0;
    }
    
    ztk_error("Failed to stop container: %s", name);
    return -1;
}

int ztk_container_remove(const char *name) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "docker rm %s 2>/dev/null", name);
    int result = system(cmd);
    if (result != 0) {
        snprintf(cmd, sizeof(cmd), "podman rm %s 2>/dev/null", name);
        result = system(cmd);
    }
    
    if (result == 0) {
        pthread_mutex_lock(&ztk.container_lock);
        container_t *container = ztk.containers;
        container_t *prev = NULL;
        while (container) {
            if (strcmp(container->name, name) == 0) {
                if (prev) {
                    prev->next = container->next;
                } else {
                    ztk.containers = container->next;
                }
                free(container);
                break;
            }
            prev = container;
            container = container->next;
        }
        pthread_mutex_unlock(&ztk.container_lock);
        ztk_info("Container removed: %s", name);
        return 0;
    }
    
    ztk_error("Failed to remove container: %s", name);
    return -1;
}

void ztk_list_containers(void) {
    ztk_print_header("Containers");
    printf("  %-20s %-20s %-12s %s\n", 
           "Name", "Image", "Status", "Running");
    printf("  %-20s %-20s %-12s %s\n", 
           "----", "-----", "------", "-------");
    
    pthread_mutex_lock(&ztk.container_lock);
    
    container_t *container = ztk.containers;
    while (container) {
        printf("  %-20s %-20s %-12s %s\n",
               container->name, container->image, 
               container->status, container->running ? "Yes" : "No");
        container = container->next;
    }
    
    pthread_mutex_unlock(&ztk.container_lock);
    ztk_print_footer();
}

/* Cluster System */
int ztk_cluster_add_node(const char *name, const char *address, int port) {
    cluster_node_t *node = malloc(sizeof(cluster_node_t));
    if (!node) return -1;
    
    strncpy(node->name, name, sizeof(node->name) - 1);
    strncpy(node->address, address, sizeof(node->address) - 1);
    node->port = port;
    strcpy(node->status, "DISCONNECTED");
    node->connected = 0;
    node->load = 0.0;
    node->memory_used = 0;
    node->memory_total = 0;
    
    pthread_mutex_lock(&ztk.cluster_lock);
    node->next = ztk.cluster_nodes;
    ztk.cluster_nodes = node;
    pthread_mutex_unlock(&ztk.cluster_lock);
    
    ztk_info("Cluster node added: %s (%s:%d)", name, address, port);
    return 0;
}

int ztk_cluster_connect(const char *name) {
    pthread_mutex_lock(&ztk.cluster_lock);
    
    cluster_node_t *node = ztk.cluster_nodes;
    while (node) {
        if (strcmp(node->name, name) == 0) {
            /* Attempt connection */
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                pthread_mutex_unlock(&ztk.cluster_lock);
                ztk_error("Failed to create socket");
                return -1;
            }
            
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(node->port);
            inet_pton(AF_INET, node->address, &addr.sin_addr);
            
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                node->connected = 1;
                strcpy(node->status, "CONNECTED");
                close(sock);
                pthread_mutex_unlock(&ztk.cluster_lock);
                ztk_info("Connected to cluster node: %s", name);
                return 0;
            }
            close(sock);
            break;
        }
        node = node->next;
    }
    
    pthread_mutex_unlock(&ztk.cluster_lock);
    ztk_error("Failed to connect to cluster node: %s", name);
    return -1;
}

void ztk_cluster_list_nodes(void) {
    ztk_print_header("Cluster Nodes");
    printf("  %-20s %-30s %-12s %-10s %-15s\n", 
           "Name", "Address", "Status", "Load", "Memory");
    printf("  %-20s %-30s %-12s %-10s %-15s\n", 
           "----", "-------", "------", "----", "------");
    
    pthread_mutex_lock(&ztk.cluster_lock);
    
    cluster_node_t *node = ztk.cluster_nodes;
    while (node) {
        char memory[32];
        if (node->memory_total > 0) {
            snprintf(memory, sizeof(memory), "%.1f%%", 
                     (float)node->memory_used / node->memory_total * 100);
        } else {
            strcpy(memory, "N/A");
        }
        
        printf("  %-20s %-30s %-12s %-10.2f %-15s\n",
               node->name, 
               node->address,
               node->status,
               node->load,
               memory);
        node = node->next;
    }
    
    pthread_mutex_unlock(&ztk.cluster_lock);
    ztk_print_footer();
}

int ztk_cluster_broadcast(const char *command) {
    pthread_mutex_lock(&ztk.cluster_lock);
    
    cluster_node_t *node = ztk.cluster_nodes;
    int success_count = 0;
    int total_count = 0;
    
    while (node) {
        if (node->connected) {
            total_count++;
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0) {
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(node->port);
                inet_pton(AF_INET, node->address, &addr.sin_addr);
                
                if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                    send(sock, command, strlen(command), 0);
                    success_count++;
                }
                close(sock);
            }
        }
        node = node->next;
    }
    
    pthread_mutex_unlock(&ztk.cluster_lock);
    ztk_info("Broadcast to %d/%d nodes", success_count, total_count);
    return success_count;
}

/* Backup System */
int ztk_backup_create(const char *name, const char *source, const char *dest) {
    backup_set_t *backup = malloc(sizeof(backup_set_t));
    if (!backup) return -1;
    
    strncpy(backup->name, name, sizeof(backup->name) - 1);
    strncpy(backup->source, source, sizeof(backup->source) - 1);
    strncpy(backup->destination, dest, sizeof(backup->destination) - 1);
    backup->encrypted = 0;
    backup->size = 0;
    backup->last_backup = time(NULL);
    backup->count = 1;
    
    /* Create backup directory */
    mkdir(dest, 0755);
    
    /* Create backup using tar */
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "tar -czf %s/backup_%ld.tar.gz %s 2>/dev/null", 
             dest, time(NULL), source);
    int result = system(cmd);
    
    if (result == 0) {
        pthread_mutex_lock(&ztk.backup_lock);
        backup->next = ztk.backups;
        ztk.backups = backup;
        pthread_mutex_unlock(&ztk.backup_lock);
        ztk_info("Backup created: %s", name);
        return 0;
    }
    
    free(backup);
    ztk_error("Failed to create backup: %s", name);
    return -1;
}

int ztk_backup_restore(const char *name, const char *dest) {
    pthread_mutex_lock(&ztk.backup_lock);
    
    backup_set_t *backup = ztk.backups;
    while (backup) {
        if (strcmp(backup->name, name) == 0) {
            pthread_mutex_unlock(&ztk.backup_lock);
            
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "tar -xzf %s/backup_*.tar.gz -C %s 2>/dev/null", 
                     backup->destination, dest);
            int result = system(cmd);
            
            if (result == 0) {
                ztk_info("Backup restored: %s to %s", name, dest);
                return 0;
            }
            ztk_error("Failed to restore backup: %s", name);
            return -1;
        }
        backup = backup->next;
    }
    
    pthread_mutex_unlock(&ztk.backup_lock);
    ztk_error("Backup not found: %s", name);
    return -1;
}

int ztk_backup_encrypt(const char *name, const char *password) {
    pthread_mutex_lock(&ztk.backup_lock);
    
    backup_set_t *backup = ztk.backups;
    while (backup) {
        if (strcmp(backup->name, name) == 0) {
            pthread_mutex_unlock(&ztk.backup_lock);
            
            /* Encrypt using openssl */
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), 
                     "find %s -name '*.tar.gz' -exec openssl enc -aes-256-cbc -salt -in {} -out {}.enc -pass pass:%s \\; 2>/dev/null", 
                     backup->destination, password);
            int result = system(cmd);
            
            if (result == 0) {
                backup->encrypted = 1;
                ztk_info("Backup encrypted: %s", name);
                return 0;
            }
            ztk_error("Failed to encrypt backup: %s", name);
            return -1;
        }
        backup = backup->next;
    }
    
    pthread_mutex_unlock(&ztk.backup_lock);
    ztk_error("Backup not found: %s", name);
    return -1;
}

void ztk_list_backups(void) {
    ztk_print_header("Backups");
    printf("  %-20s %-30s %-12s %-10s %s\n", 
           "Name", "Source", "Encrypted", "Count", "Last Backup");
    printf("  %-20s %-30s %-12s %-10s %s\n", 
           "----", "------", "---------", "-----", "-----------");
    
    pthread_mutex_lock(&ztk.backup_lock);
    
    backup_set_t *backup = ztk.backups;
    while (backup) {
        char last_time[64];
        strftime(last_time, sizeof(last_time), "%Y-%m-%d %H:%M:%S", 
                 localtime(&backup->last_backup));
        
        printf("  %-20s %-30s %-12s %-10d %s\n",
               backup->name, backup->source,
               backup->encrypted ? "Yes" : "No",
               backup->count, last_time);
        backup = backup->next;
    }
    
    pthread_mutex_unlock(&ztk.backup_lock);
    ztk_print_footer();
}

/* Monitoring System */
void ztk_monitor_start(void) {
    if (ztk.monitoring_active) return;
    
    ztk.monitoring_active = 1;
    pthread_create(&ztk.monitor_thread_id, NULL, ztk_monitor_thread, NULL);
    ztk_info("Monitoring started");
}

void ztk_monitor_stop(void) {
    ztk.monitoring_active = 0;
    if (ztk.monitor_thread_id) {
        pthread_join(ztk.monitor_thread_id, NULL);
    }
    ztk_info("Monitoring stopped");
}

void *ztk_monitor_thread(void *arg) {
    while (ztk.monitoring_active) {
        pthread_mutex_lock(&ztk.monitor_lock);
        
        /* Get CPU usage */
        FILE *fp = fopen("/proc/stat", "r");
        if (fp) {
            char line[256];
            if (fgets(line, sizeof(line), fp)) {
                unsigned long user, nice, system, idle;
                sscanf(line, "cpu %lu %lu %lu %lu", &user, &nice, &system, &idle);
                unsigned long total = user + nice + system + idle;
                static unsigned long prev_total = 0;
                static unsigned long prev_idle = 0;
                if (prev_total > 0) {
                    float total_diff = total - prev_total;
                    float idle_diff = idle - prev_idle;
                    ztk.monitor.cpu_usage = (unsigned long)((total_diff - idle_diff) / total_diff * 100);
                }
                prev_total = total;
                prev_idle = idle;
            }
            fclose(fp);
        }
        
        /* Get memory usage */
        fp = fopen("/proc/meminfo", "r");
        if (fp) {
            char line[256];
            unsigned long mem_total = 0, mem_available = 0;
            while (fgets(line, sizeof(line), fp)) {
                if (sscanf(line, "MemTotal: %lu kB", &mem_total) == 1) continue;
                if (sscanf(line, "MemAvailable: %lu kB", &mem_available) == 1) break;
            }
            if (mem_total > 0) {
                ztk.monitor.memory_usage = (mem_total - mem_available) * 1024;
            }
            fclose(fp);
        }
        
        /* Get disk usage */
        struct statvfs stat;
        if (statvfs("/", &stat) == 0) {
            ztk.monitor.disk_usage = (stat.f_blocks - stat.f_bfree) * stat.f_frsize;
        }
        
        /* Get process count */
        DIR *dir = opendir("/proc");
        if (dir) {
            int count = 0;
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (isdigit(entry->d_name[0])) count++;
            }
            ztk.monitor.process_count = count;
            closedir(dir);
        }
        
        /* Get load average */
        fp = fopen("/proc/loadavg", "r");
        if (fp) {
            float load1, load5, load15;
            if (fscanf(fp, "%f %f %f", &load1, &load5, &load15) == 3) {
                ztk.monitor.load_avg[0] = load1;
                ztk.monitor.load_avg[1] = load5;
                ztk.monitor.load_avg[2] = load15;
            }
            fclose(fp);
        }
        
        ztk.monitor.timestamp = time(NULL);
        
        pthread_mutex_unlock(&ztk.monitor_lock);
        sleep(2);
    }
    return NULL;
}

void ztk_monitor_report(void) {
    pthread_mutex_lock(&ztk.monitor_lock);
    
    ztk_print_header("System Monitor");
    printf("  CPU Usage:  %lu%%\n", ztk.monitor.cpu_usage);
    printf("  Memory:     %s / %s\n", 
           ztk_format_size(ztk.monitor.memory_usage),
           ztk_format_size(ztk.monitor.memory_usage + 
                          (ztk.monitor.memory_usage / (100 - ztk.monitor.cpu_usage) * ztk.monitor.cpu_usage)));
    printf("  Disk:       %s\n", ztk_format_size(ztk.monitor.disk_usage));
    printf("  Processes:  %d\n", ztk.monitor.process_count);
    printf("  Load Avg:   %.2f, %.2f, %.2f\n",
           ztk.monitor.load_avg[0],
           ztk.monitor.load_avg[1],
           ztk.monitor.load_avg[2]);
    printf("  Time:       %s\n", ctime(&ztk.monitor.timestamp));
    
    pthread_mutex_unlock(&ztk.monitor_lock);
    ztk_print_footer();
}

/* AI System */
int ztk_ai_init(const char *token) {
    if (!token) return -1;
    
    pthread_mutex_lock(&ztk.ai_lock);
    strncpy(ztk.ai_token, token, sizeof(ztk.ai_token) - 1);
    strcpy(ztk.ai_model, "deepseek-chat");
    strcpy(ztk.ai_system_prompt, "You are ZTK AI, an advanced assistant for the ZTK Enterprise Shell.");
    ztk.ai_temperature = 0.7;
    ztk.ai_max_tokens = 4096;
    ztk.ai_enabled = 1;
    pthread_mutex_unlock(&ztk.ai_lock);
    
    ztk_info("AI initialized with DeepSeek model");
    return 0;
}

int ztk_ai_chat(const char *message, char *response, size_t response_size) {
    if (!ztk.ai_enabled || strlen(ztk.ai_token) == 0) {
        snprintf(response, response_size, "Error: AI not enabled. Use 'ai init <token>'");
        return -1;
    }
    
    /* Use curl for API request */
    CURL *curl = curl_easy_init();
    if (!curl) {
        strncpy(response, "Error: Failed to initialize curl", response_size - 1);
        return -1;
    }
    
    char post_data[8192];
    char escaped_msg[4096];
    char *p = escaped_msg;
    for (const char *s = message; *s && p - escaped_msg < sizeof(escaped_msg) - 1; s++) {
        if (*s == '"' || *s == '\\' || *s == '\n' || *s == '\r') {
            *p++ = '\\';
            *p++ = *s;
        } else {
            *p++ = *s;
        }
    }
    *p = '\0';
    
    snprintf(post_data, sizeof(post_data),
        "{\"model\":\"%s\","
        "\"messages\":["
        "{\"role\":\"system\",\"content\":\"%s\"},"
        "{\"role\":\"user\",\"content\":\"%s\"}"
        "],"
        "\"max_tokens\":%d,"
        "\"temperature\":%.1f}",
        ztk.ai_model,
        ztk.ai_system_prompt,
        escaped_msg,
        ztk.ai_max_tokens,
        ztk.ai_temperature);
    
    char auth_header[512];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", ztk.ai_token);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);
    
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.deepseek.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        snprintf(response, response_size, "Error: Network request failed: %s", 
                 curl_easy_strerror(res));
        return -1;
    }
    
    /* Parse JSON response */
    char *content_start = strstr(response, "\"content\"");
    if (content_start) {
        content_start = strchr(content_start, ':');
        if (content_start) {
            content_start++;
            while (*content_start && isspace(*content_start)) content_start++;
            if (*content_start == '"') {
                content_start++;
                char *content_end = strstr(content_start, "\"");
                if (content_end) {
                    *content_end = '\0';
                    strcpy(response, content_start);
                    return 0;
                }
            }
        }
    }
    
    snprintf(response, response_size, "Error: Failed to parse AI response");
    return -1;
}

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    char *response = (char*)userp;
    strncat(response, (char*)contents, total_size);
    return total_size;
}

void ztk_ai_chat_loop(void) {
    if (!ztk.ai_enabled) {
        ztk_error("AI not enabled. Use 'ai init <token>' first");
        return;
    }
    
    char input[MAX_LINE];
    char response[MAX_LINE * 4];
    
    printf("\n%s╔══════════════════════════════════════════════════════════════╗%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("%s║                    ZTK AI Chat                               ║%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("%s║  Type your message. Type 'quit' to exit.                    ║%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("%s║  Type 'clear' to clear conversation.                        ║%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("%s║  Type 'model <name>' to change model.                       ║%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("%s╚══════════════════════════════════════════════════════════════╝%s\n",
           COLOR_CYAN, COLOR_DEFAULT);
    printf("\n%sAI: Hello! How can I assist you today?%s\n", COLOR_GREEN, COLOR_DEFAULT);
    
    while (1) {
        printf("\n%sYou:%s ", COLOR_YELLOW, COLOR_DEFAULT);
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        if (strcmp(input, "clear") == 0) {
            ztk_ai_clear_conversation();
            printf("%sAI: Conversation cleared.%s\n", COLOR_GREEN, COLOR_DEFAULT);
            continue;
        }
        if (strncmp(input, "model ", 6) == 0) {
            pthread_mutex_lock(&ztk.ai_lock);
            strncpy(ztk.ai_model, input + 6, sizeof(ztk.ai_model) - 1);
            pthread_mutex_unlock(&ztk.ai_lock);
            printf("%sAI: Model changed to: %s%s\n", COLOR_GREEN, ztk.ai_model, COLOR_DEFAULT);
            continue;
        }
        if (strncmp(input, "temp ", 5) == 0) {
            float temp = atof(input + 5);
            pthread_mutex_lock(&ztk.ai_lock);
            ztk.ai_temperature = temp > 0 && temp <= 2 ? temp : 0.7;
            pthread_mutex_unlock(&ztk.ai_lock);
            printf("%sAI: Temperature set to: %.2f%s\n", COLOR_GREEN, ztk.ai_temperature, COLOR_DEFAULT);
            continue;
        }
        
        printf("%sAI:%s ", COLOR_GREEN, COLOR_DEFAULT);
        fflush(stdout);
        
        int result = ztk_ai_chat(input, response, sizeof(response));
        if (result == 0) {
            printf("%s\n", response);
        } else {
            printf("%sError: %s%s\n", COLOR_RED, response, COLOR_DEFAULT);
        }
    }
}

void ztk_ai_clear_conversation(void) {
    /* Conversation history would be cleared here */
    ztk_info("AI conversation cleared");
}

void *ztk_ai_thread(void *arg) {
    while (ztk.running) {
        sleep(1);
    }
    return NULL;
}

/* HTTP Server */
int ztk_http_server_start(int port) {
    ztk.http_port = port;
    ztk.http_server_running = 1;
    pthread_create(&ztk.http_thread, NULL, ztk_http_thread, NULL);
    ztk_info("HTTP server started on port %d", port);
    return 0;
}

void ztk_http_server_stop(void) {
    ztk.http_server_running = 0;
    if (ztk.http_thread) {
        pthread_join(ztk.http_thread, NULL);
    }
    ztk_info("HTTP server stopped");
}

void *ztk_http_thread(void *arg) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        ztk_error("Failed to create HTTP server socket");
        return NULL;
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(ztk.http_port);
    
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ztk_error("Failed to bind HTTP server");
        close(server_fd);
        return NULL;
    }
    
    if (listen(server_fd, 10) < 0) {
        ztk_error("Failed to listen on HTTP server");
        close(server_fd);
        return NULL;
    }
    
    ztk_info("HTTP server listening on port %d", ztk.http_port);
    
    while (ztk.http_server_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_fd < 0) {
            if (ztk.http_server_running) {
                ztk_error("HTTP accept failed");
            }
            continue;
        }
        
        /* Handle request in separate thread */
        pthread_t thread;
        int *client_ptr = malloc(sizeof(int));
        *client_ptr = client_fd;
        pthread_create(&thread, NULL, handle_http_request, client_ptr);
        pthread_detach(thread);
    }
    
    close(server_fd);
    return NULL;
}

void *handle_http_request(void *arg) {
    int client_fd = *(int*)arg;
    free(arg);
    
    char buffer[4096];
    ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) {
        close(client_fd);
        return NULL;
    }
    buffer[bytes] = '\0';
    
    /* Parse request */
    char method[16], path[256], version[16];
    sscanf(buffer, "%s %s %s", method, path, version);
    
    /* Build response */
    char response[4096];
    char body[2048];
    
    if (strcmp(path, "/") == 0) {
        snprintf(body, sizeof(body),
            "<html><head><title>ZTK Enterprise</title>"
            "<style>body{font-family:monospace;background:#1a1a2e;color:#eee;padding:20px;}"
            "h1{color:#00d4ff;}a{color:#00d4ff;}</style></head>"
            "<body>"
            "<h1>ZTK Enterprise Shell v%s</h1>"
            "<p>Running on %s</p>"
            "<ul>"
            "<li><a href='/status'>System Status</a></li>"
            "<li><a href='/monitor'>Monitor</a></li>"
            "<li><a href='/commands'>Commands</a></li>"
            "</ul>"
            "</body></html>",
            ZTK_VERSION, ztk.username);
    } else if (strcmp(path, "/status") == 0) {
        snprintf(body, sizeof(body),
            "<html><head><title>Status - ZTK</title>"
            "<style>body{font-family:monospace;background:#1a1a2e;color:#eee;padding:20px;}"
            "h1{color:#00d4ff;}</style></head>"
            "<body>"
            "<h1>System Status</h1>"
            "<pre>CPU: %lu%%\nMemory: %s\nDisk: %s\nProcesses: %d\nLoad: %.2f %.2f %.2f</pre>"
            "</body></html>",
            ztk.monitor.cpu_usage,
            ztk_format_size(ztk.monitor.memory_usage),
            ztk_format_size(ztk.monitor.disk_usage),
            ztk.monitor.process_count,
            ztk.monitor.load_avg[0],
            ztk.monitor.load_avg[1],
            ztk.monitor.load_avg[2]);
    } else {
        snprintf(body, sizeof(body),
            "<html><head><title>Not Found</title></head>"
            "<body><h1>404 Not Found</h1></body></html>");
    }
    
    snprintf(response, sizeof(response),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        strlen(body), body);
    
    send(client_fd, response, strlen(response), 0);
    close(client_fd);
    return NULL;
}

/* Remote Session */
int ztk_remote_connect(const char *host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        ztk_error("Failed to create remote socket");
        return -1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ztk_error("Failed to connect to remote host %s:%d", host, port);
        close(sock);
        return -1;
    }
    
    remote_session_t *session = malloc(sizeof(remote_session_t));
    if (!session) {
        close(sock);
        return -1;
    }
    
    session->socket = sock;
    strncpy(session->host, host, sizeof(session->host) - 1);
    session->port = port;
    session->authenticated = 0;
    session->last_activity = time(NULL);
    strcpy(session->user, "unknown");
    
    pthread_mutex_lock(&ztk.session_lock);
    session->next = ztk.sessions;
    ztk.sessions = session;
    pthread_mutex_unlock(&ztk.session_lock);
    
    ztk_info("Remote session connected to %s:%d", host, port);
    return sock;
}

void ztk_remote_loop(void) {
    ztk_print_header("Remote Sessions");
    
    pthread_mutex_lock(&ztk.session_lock);
    
    remote_session_t *session = ztk.sessions;
    while (session) {
        printf("  %s:%d - User: %s - Last activity: %s\n",
               session->host, session->port, session->user,
               ctime(&session->last_activity));
        session = session->next;
    }
    
    pthread_mutex_unlock(&ztk.session_lock);
    ztk_print_footer();
}

/* Builtin Commands Implementations */
int builtin_help(int argc, char **argv, void *context) {
    ztk_list_commands();
    return 0;
}

int builtin_about(int argc, char **argv, void *context) {
    ztk_print_header("ZTK Enterprise Shell");
    printf("  Version:    %s\n", ZTK_VERSION);
    printf("  Release:    %s\n", ZTK_RELEASE);
    printf("  Codename:   %s\n", ZTK_CODENAME);
    printf("  Build:      %s %s\n", ZTK_BUILD_DATE, ZTK_BUILD_TIME);
    printf("  API:        v%d\n", ZTK_API_VERSION);
    printf("  User:       %s\n", ztk.username);
    printf("  System:     %s\n", sys_info.os_name);
    printf("  Architecture: %s\n", sys_info.architecture);
    printf("  Terminal:   %dx%d (%s)\n", 
           ztk.terminal_width, ztk.terminal_height, ztk.term_type);
    printf("  AI:         %s\n", ztk.ai_enabled ? "Enabled" : "Disabled");
    printf("  Plugins:    %d loaded\n", ztk.plugins ? 1 : 0);
    printf("  Databases:  %d connected\n", ztk.databases ? 1 : 0);
    printf("  Containers: %d\n", ztk.containers ? 1 : 0);
    ztk_print_footer();
    return 0;
}

int builtin_exit(int argc, char **argv, void *context) {
    ztk.running = 0;
    ztk_info("Exiting ZTK Shell");
    return 0;
}

int builtin_clear(int argc, char **argv, void *context) {
    printf("\033[2J\033[H");
    return 0;
}

int builtin_cd(int argc, char **argv, void *context) {
    const char *path = argc > 1 ? argv[1] : getenv("HOME");
    if (!path) path = "/";
    
    if (chdir(path) != 0) {
        ztk_error("cd: %s: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

int builtin_pwd(int argc, char **argv, void *context) {
    char cwd[MAX_PATH];
    if (getcwd(cwd, sizeof(cwd))) {
        printf("%s\n", cwd);
        return 0;
    }
    ztk_error("pwd: %s", strerror(errno));
    return -1;
}

int builtin_ls(int argc, char **argv, void *context) {
    const char *path = argc > 1 ? argv[1] : ".";
    DIR *dir = opendir(path);
    if (!dir) {
        ztk_error("ls: %s: %s", path, strerror(errno));
        return -1;
    }
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            
            struct stat st;
            if (stat(full_path, &st) == 0) {
                if (S_ISDIR(st.st_mode)) {
                    printf("%s%s/%s ", COLOR_BLUE, entry->d_name, COLOR_DEFAULT);
                } else if (st.st_mode & S_IXUSR) {
                    printf("%s%s*%s ", COLOR_GREEN, entry->d_name, COLOR_DEFAULT);
                } else {
                    printf("%s ", entry->d_name);
                }
            }
        }
    }
    printf("\n");
    closedir(dir);
    return 0;
}

int builtin_cat(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("cat: missing file operand");
        return -1;
    }
    
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            ztk_error("cat: %s: %s", argv[i], strerror(errno));
            continue;
        }
        
        char buffer[4096];
        size_t bytes;
        while ((bytes = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            fwrite(buffer, 1, bytes, stdout);
        }
        fclose(fp);
    }
    return 0;
}

int builtin_echo(int argc, char **argv, void *context) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], i < argc - 1 ? " " : "");
    }
    printf("\n");
    return 0;
}

int builtin_mkdir(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("mkdir: missing operand");
        return -1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i], 0755) != 0) {
            ztk_error("mkdir: %s: %s", argv[i], strerror(errno));
        }
    }
    return 0;
}

int builtin_rm(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("rm: missing operand");
        return -1;
    }
    
    int recursive = 0;
    int force = 0;
    int i = 1;
    
    while (i < argc && argv[i][0] == '-') {
        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "-R") == 0) {
            recursive = 1;
        } else if (strcmp(argv[i], "-f") == 0) {
            force = 1;
        } else {
            ztk_error("rm: invalid option: %s", argv[i]);
            return -1;
        }
        i++;
    }
    
    for (int j = i; j < argc; j++) {
        if (recursive) {
            char cmd[1024];
            snprintf(cmd, sizeof(cmd), "rm -rf %s", argv[j]);
            system(cmd);
        } else {
            if (unlink(argv[j]) != 0 && !force) {
                ztk_error("rm: %s: %s", argv[j], strerror(errno));
            }
        }
    }
    return 0;
}

int builtin_cp(int argc, char **argv, void *context) {
    if (argc < 3) {
        ztk_error("cp: missing file operands");
        return -1;
    }
    
    char cmd[1024] = "cp";
    for (int i = 1; i < argc; i++) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    return system(cmd);
}

int builtin_mv(int argc, char **argv, void *context) {
    if (argc < 3) {
        ztk_error("mv: missing file operands");
        return -1;
    }
    
    char cmd[1024] = "mv";
    for (int i = 1; i < argc; i++) {
        strcat(cmd, " ");
        strcat(cmd, argv[i]);
    }
    return system(cmd);
}

int builtin_chmod(int argc, char **argv, void *context) {
    if (argc < 3) {
        ztk_error("chmod: missing operands");
        return -1;
    }
    
    mode_t mode = strtol(argv[1], NULL, 8);
    for (int i = 2; i < argc; i++) {
        if (chmod(argv[i], mode) != 0) {
            ztk_error("chmod: %s: %s", argv[i], strerror(errno));
        }
    }
    return 0;
}

int builtin_chown(int argc, char **argv, void *context) {
    if (argc < 3) {
        ztk_error("chown: missing operands");
        return -1;
    }
    
    char *user = argv[1];
    char *group = strchr(user, ':');
    if (group) {
        *group++ = '\0';
    }
    
    struct passwd *pw = getpwnam(user);
    if (!pw) {
        ztk_error("chown: invalid user: %s", user);
        return -1;
    }
    
    for (int i = 2; i < argc; i++) {
        if (chown(argv[i], pw->pw_uid, -1) != 0) {
            ztk_error("chown: %s: %s", argv[i], strerror(errno));
        }
    }
    return 0;
}

int builtin_set(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_variables();
        return 0;
    }
    
    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (eq) {
            *eq = '\0';
            char *name = argv[i];
            char *value = eq + 1;
            ztk_set_variable(name, value, 0, 0);
            *eq = '=';
        }
    }
    return 0;
}

int builtin_unset(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("unset: missing operand");
        return -1;
    }
    
    for (int i = 1; i < argc; i++) {
        ztk_unset_variable(argv[i]);
    }
    return 0;
}

int builtin_export(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_variables();
        return 0;
    }
    
    for (int i = 1; i < argc; i++) {
        char *eq = strchr(argv[i], '=');
        if (eq) {
            *eq = '\0';
            char *name = argv[i];
            char *value = eq + 1;
            ztk_set_variable(name, value, 0, 1);
            *eq = '=';
        } else {
            char *value = ztk_get_variable(argv[i]);
            if (value) {
                ztk_set_variable(argv[i], value, 0, 1);
            }
        }
    }
    return 0;
}

int builtin_env(int argc, char **argv, void *context) {
    extern char **environ;
    char **env = environ;
    while (*env) {
        printf("%s\n", *env);
        env++;
    }
    return 0;
}

int builtin_source(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("source: missing filename");
        return -1;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        ztk_error("source: %s: %s", argv[1], strerror(errno));
        return -1;
    }
    
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char *trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        if (*trimmed == '#' || *trimmed == '\0') continue;
        
        trimmed[strcspn(trimmed, "\n")] = '\0';
        
        char *args[MAX_ARGS];
        int argc = 0;
        char *token = strtok(trimmed, " ");
        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;
        
        if (argc > 0) {
            ztk_execute_command(argc, args);
        }
    }
    
    fclose(fp);
    return 0;
}

int builtin_exec(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("exec: missing command");
        return -1;
    }
    
    execvp(argv[1], argv + 1);
    ztk_error("exec: %s: %s", argv[1], strerror(errno));
    return -1;
}

int builtin_jobs(int argc, char **argv, void *context) {
    ztk_list_jobs();
    return 0;
}

int builtin_fg(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("fg: missing job ID");
        return -1;
    }
    
    int job_id = atoi(argv[1]);
    ztk_wait_job(job_id);
    return 0;
}

int builtin_bg(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("bg: missing job ID");
        return -1;
    }
    
    int job_id = atoi(argv[1]);
    ztk_kill_job(job_id, SIGCONT);
    return 0;
}

int builtin_kill(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_error("kill: missing job ID or PID");
        return -1;
    }
    
    int signal = SIGTERM;
    int start = 1;
    
    if (argv[1][0] == '-') {
        signal = atoi(argv[1] + 1);
        start = 2;
    }
    
    for (int i = start; i < argc; i++) {
        int pid = atoi(argv[i]);
        if (kill(pid, signal) != 0) {
            ztk_error("kill: %s: %s", argv[i], strerror(errno));
        }
    }
    return 0;
}

int builtin_wait(int argc, char **argv, void *context) {
    if (argc < 2) {
        /* Wait for all jobs */
        int status;
        while (wait(&status) > 0);
        return 0;
    }
    
    for (int i = 1; i < argc; i++) {
        int pid = atoi(argv[i]);
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}

int builtin_ps(int argc, char **argv, void *context) {
    system("ps aux");
    return 0;
}

int builtin_top(int argc, char **argv, void *context) {
    system("top -b -n 1");
    return 0;
}

int builtin_plugins(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_plugins();
        return 0;
    }
    
    if (strcmp(argv[1], "load") == 0 && argc > 2) {
        return ztk_load_plugin(argv[2]);
    } else if (strcmp(argv[1], "unload") == 0 && argc > 2) {
        return ztk_unload_plugin(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_list_plugins();
        return 0;
    }
    
    ztk_error("Usage: plugins [list|load <path>|unload <name>]");
    return -1;
}

int builtin_databases(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_databases();
        return 0;
    }
    
    if (strcmp(argv[1], "connect") == 0 && argc > 3) {
        return ztk_db_connect(argv[2], argv[3], "sqlite");
    } else if (strcmp(argv[1], "disconnect") == 0 && argc > 2) {
        ztk_db_disconnect(argv[2]);
        return 0;
    } else if (strcmp(argv[1], "query") == 0 && argc > 3) {
        int rows, cols;
        char **result = ztk_db_query(argv[2], argv[3], &rows, &cols);
        if (result) {
            for (int i = 0; result[i]; i++) {
                printf("%s\n", result[i]);
                free(result[i]);
            }
            free(result);
            printf("Query returned %d rows\n", rows);
        }
        return 0;
    } else if (strcmp(argv[1], "execute") == 0 && argc > 3) {
        return ztk_db_execute(argv[2], argv[3]);
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_list_databases();
        return 0;
    }
    
    ztk_error("Usage: databases [list|connect <name> <path>|disconnect <name>|execute <name> <sql>|query <name> <sql>]");
    return -1;
}

int builtin_containers(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_containers();
        return 0;
    }
    
    if (strcmp(argv[1], "create") == 0 && argc > 3) {
        return ztk_container_create(argv[2], argv[3]);
    } else if (strcmp(argv[1], "start") == 0 && argc > 2) {
        return ztk_container_start(argv[2]);
    } else if (strcmp(argv[1], "stop") == 0 && argc > 2) {
        return ztk_container_stop(argv[2]);
    } else if (strcmp(argv[1], "rm") == 0 && argc > 2) {
        return ztk_container_remove(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_list_containers();
        return 0;
    }
    
    ztk_error("Usage: containers [list|create <name> <image>|start <name>|stop <name>|rm <name>]");
    return -1;
}

int builtin_cluster(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_cluster_list_nodes();
        return 0;
    }
    
    if (strcmp(argv[1], "add") == 0 && argc > 4) {
        return ztk_cluster_add_node(argv[2], argv[3], atoi(argv[4]));
    } else if (strcmp(argv[1], "connect") == 0 && argc > 2) {
        return ztk_cluster_connect(argv[2]);
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_cluster_list_nodes();
        return 0;
    } else if (strcmp(argv[1], "broadcast") == 0 && argc > 2) {
        char command[1024] = "";
        for (int i = 2; i < argc; i++) {
            strcat(command, argv[i]);
            if (i < argc - 1) strcat(command, " ");
        }
        return ztk_cluster_broadcast(command);
    }
    
    ztk_error("Usage: cluster [list|add <name> <address> <port>|connect <name>|broadcast <command>]");
    return -1;
}

int builtin_backup(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_list_backups();
        return 0;
    }
    
    if (strcmp(argv[1], "create") == 0 && argc > 4) {
        return ztk_backup_create(argv[2], argv[3], argv[4]);
    } else if (strcmp(argv[1], "restore") == 0 && argc > 3) {
        return ztk_backup_restore(argv[2], argv[3]);
    } else if (strcmp(argv[1], "encrypt") == 0 && argc > 3) {
        return ztk_backup_encrypt(argv[2], argv[3]);
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_list_backups();
        return 0;
    }
    
    ztk_error("Usage: backup [list|create <name> <source> <dest>|restore <name> <dest>|encrypt <name> <password>]");
    return -1;
}

int builtin_ai(int argc, char **argv, void *context) {
    if (argc < 2) {
        printf("AI: %s\n", ztk.ai_enabled ? "Enabled" : "Disabled");
        printf("Model: %s\n", ztk.ai_model);
        printf("Temperature: %.2f\n", ztk.ai_temperature);
        return 0;
    }
    
    if (strcmp(argv[1], "init") == 0 && argc > 2) {
        return ztk_ai_init(argv[2]);
    } else if (strcmp(argv[1], "chat") == 0) {
        ztk_ai_chat_loop();
        return 0;
    } else if (strcmp(argv[1], "ask") == 0 && argc > 2) {
        char question[2048] = "";
        for (int i = 2; i < argc; i++) {
            strcat(question, argv[i]);
            if (i < argc - 1) strcat(question, " ");
        }
        char response[MAX_LINE * 4];
        if (ztk_ai_chat(question, response, sizeof(response)) == 0) {
            printf("%s\n", response);
        }
        return 0;
    } else if (strcmp(argv[1], "clear") == 0) {
        ztk_ai_clear_conversation();
        return 0;
    } else if (strcmp(argv[1], "model") == 0 && argc > 2) {
        pthread_mutex_lock(&ztk.ai_lock);
        strncpy(ztk.ai_model, argv[2], sizeof(ztk.ai_model) - 1);
        pthread_mutex_unlock(&ztk.ai_lock);
        ztk_info("AI model set to: %s", ztk.ai_model);
        return 0;
    } else if (strcmp(argv[1], "temp") == 0 && argc > 2) {
        float temp = atof(argv[2]);
        pthread_mutex_lock(&ztk.ai_lock);
        ztk.ai_temperature = temp > 0 && temp <= 2 ? temp : 0.7;
        pthread_mutex_unlock(&ztk.ai_lock);
        ztk_info("AI temperature set to: %.2f", ztk.ai_temperature);
        return 0;
    }
    
    ztk_error("Usage: ai [init <token>|chat|ask <question>|clear|model <name>|temp <value>]");
    return -1;
}

int builtin_monitor(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_monitor_report();
        return 0;
    }
    
    if (strcmp(argv[1], "start") == 0) {
        ztk_monitor_start();
        return 0;
    } else if (strcmp(argv[1], "stop") == 0) {
        ztk_monitor_stop();
        return 0;
    } else if (strcmp(argv[1], "report") == 0) {
        ztk_monitor_report();
        return 0;
    }
    
    ztk_error("Usage: monitor [start|stop|report]");
    return -1;
}

int builtin_audit(int argc, char **argv, void *context) {
    ztk_print_header("Audit Log");
    printf("  %-20s %-15s %-30s %s\n", 
           "Timestamp", "User", "Command", "Result");
    printf("  %-20s %-15s %-30s %s\n", 
           "---------", "----", "-------", "------");
    
    pthread_mutex_lock(&ztk.audit_lock);
    
    for (int i = 0; i < ztk.audit_count; i++) {
        audit_entry_t *entry = &ztk.audit_log[i];
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
                 localtime(&entry->timestamp));
        
        printf("  %-20s %-15s %-30s %s\n",
               time_str, entry->user, entry->command,
               entry->result ? "Success" : "Failed");
    }
    
    pthread_mutex_unlock(&ztk.audit_lock);
    ztk_print_footer();
    return 0;
}

int builtin_http(int argc, char **argv, void *context) {
    if (argc < 2) {
        printf("HTTP Server: %s on port %d\n", 
               ztk.http_server_running ? "Running" : "Stopped",
               ztk.http_port);
        return 0;
    }
    
    if (strcmp(argv[1], "start") == 0) {
        int port = argc > 2 ? atoi(argv[2]) : 8080;
        return ztk_http_server_start(port);
    } else if (strcmp(argv[1], "stop") == 0) {
        ztk_http_server_stop();
        return 0;
    } else if (strcmp(argv[1], "status") == 0) {
        printf("HTTP Server: %s\n", ztk.http_server_running ? "Running" : "Stopped");
        printf("Port: %d\n", ztk.http_port);
        return 0;
    }
    
    ztk_error("Usage: http [start <port>|stop|status]");
    return -1;
}

int builtin_remote(int argc, char **argv, void *context) {
    if (argc < 2) {
        ztk_remote_loop();
        return 0;
    }
    
    if (strcmp(argv[1], "connect") == 0 && argc > 3) {
        return ztk_remote_connect(argv[2], atoi(argv[3]));
    } else if (strcmp(argv[1], "list") == 0) {
        ztk_remote_loop();
        return 0;
    }
    
    ztk_error("Usage: remote [connect <host> <port>|list]");
    return -1;
}

/* Utility Functions */
void ztk_log(const char *level, const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    time_t now = time(NULL);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    fprintf(stderr, "[%s] %s: ", time_str, level);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    va_end(args);
}

void ztk_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%sERROR:%s ", COLOR_RED, COLOR_DEFAULT);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void ztk_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%sWARNING:%s ", COLOR_YELLOW, COLOR_DEFAULT);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void ztk_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    printf("%sINFO:%s ", COLOR_GREEN, COLOR_DEFAULT);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void ztk_debug(const char *format, ...) {
    if (ztk.debug_mode) {
        va_list args;
        va_start(args, format);
        printf("%sDEBUG:%s ", COLOR_CYAN, COLOR_DEFAULT);
        vprintf(format, args);
        printf("\n");
        va_end(args);
    }
}

char *ztk_strdup_safe(const char *str) {
    if (!str) return NULL;
    size_t len = strlen(str) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

char *ztk_format_size(uint64_t size) {
    static char buffer[64];
    const char *units[] = {"B", "KB", "MB", "GB", "TB", "PB"};
    int unit_index = 0;
    double value = (double)size;
    
    while (value >= 1024 && unit_index < 5) {
        value /= 1024;
        unit_index++;
    }
    
    snprintf(buffer, sizeof(buffer), "%.2f %s", value, units[unit_index]);
    return buffer;
}

char *ztk_format_time(time_t t) {
    static char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&t));
    return buffer;
}

char *ztk_read_file(const char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) return NULL;
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char *content = malloc(size + 1);
    if (!content) {
        fclose(fp);
        return NULL;
    }
    
    fread(content, 1, size, fp);
    content[size] = '\0';
    fclose(fp);
    return content;
}

int ztk_write_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) return -1;
    fprintf(fp, "%s", content);
    fclose(fp);
    return 0;
}

int ztk_file_exists(const char *path) {
    return access(path, F_OK) == 0;
}

int ztk_is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) return 0;
    return S_ISDIR(st.st_mode);
}

char *ztk_get_absolute_path(const char *path) {
    static char buffer[MAX_PATH];
    if (realpath(path, buffer)) {
        return buffer;
    }
    return NULL;
}

char *ztk_expand_path(const char *path) {
    static char buffer[MAX_PATH];
    if (path[0] == '~') {
        snprintf(buffer, sizeof(buffer), "%s%s", getenv("HOME"), path + 1);
        return buffer;
    }
    return (char*)path;
}

void ztk_print_header(const char *title) {
    int width = 60;
    printf("\n%s╔", COLOR_CYAN);
    for (int i = 0; i < width - 2; i++) printf("═");
    printf("╗%s\n", COLOR_DEFAULT);
    printf("%s║", COLOR_CYAN);
    int padding = (width - 2 - strlen(title)) / 2;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s", title);
    for (int i = padding + strlen(title); i < width - 2; i++) printf(" ");
    printf("%s║%s\n", COLOR_CYAN, COLOR_DEFAULT);
    printf("%s╠", COLOR_CYAN);
    for (int i = 0; i < width - 2; i++) printf("═");
    printf("╣%s\n", COLOR_DEFAULT);
}

void ztk_print_footer(void) {
    int width = 60;
    printf("%s╚", COLOR_CYAN);
    for (int i = 0; i < width - 2; i++) printf("═");
    printf("╝%s\n", COLOR_DEFAULT);
}

/* Main Shell Loop */
void ztk_loop(void) {
    char input[MAX_LINE];
    char *args[MAX_ARGS];
    int argc;
    
    while (ztk.running) {
        /* Display prompt */
        char cwd[MAX_PATH];
        getcwd(cwd, sizeof(cwd));
        
        char *home = getenv("HOME");
        char *display_path = cwd;
        if (home && strncmp(cwd, home, strlen(home)) == 0) {
            display_path = cwd + strlen(home);
            printf("%s~%s", COLOR_BLUE, display_path);
        } else {
            printf("%s%s%s", COLOR_BLUE, cwd, COLOR_DEFAULT);
        }
        printf("%s$%s ", COLOR_GREEN, COLOR_DEFAULT);
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) continue;
        
        /* Add to history */
        if (ztk.history_count < ztk.history_max) {
            ztk.history[ztk.history_count++] = ztk_strdup_safe(input);
        }
        
        /* Parse command */
        argc = 0;
        char *token = strtok(input, " ");
        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;
        
        if (argc > 0) {
            /* Check for background execution */
            int background = 0;
            if (strcmp(args[argc - 1], "&") == 0) {
                background = 1;
                args[--argc] = NULL;
            }
            
            if (background) {
                pid_t pid = fork();
                if (pid == 0) {
                    ztk_execute_command(argc, args);
                    exit(0);
                } else if (pid > 0) {
                    ztk_add_job(pid, input);
                    printf("[%d] %d\n", ztk.jobs ? ztk.jobs->job_id : 1, pid);
                }
            } else {
                ztk_execute_command(argc, args);
            }
        }
    }
}

/* Main Program */
int main(int argc, char *argv[]) {
    /* Initialize curl */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Initialize ZTK */
    if (ztk_init() != 0) {
        fprintf(stderr, "Failed to initialize ZTK Shell\n");
        return 1;
    }
    
    /* Print banner */
    printf("%s\n", COLOR_CYAN);
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║           ZTK ENTERPRISE SHELL v%s                          ║\n", ZTK_VERSION);
    printf("║           %s                                         ║\n", ZTK_RELEASE);
    printf("║           Codename: %s                                      ║\n", ZTK_CODENAME);
    printf("║                                                              ║\n");
    printf("║           %s - %s                                    ║\n", ZTK_BUILD_DATE, ZTK_BUILD_TIME);
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_DEFAULT);
    
    /* Start monitor */
    ztk_monitor_start();
    
    /* Start AI thread if enabled */
    if (ztk.ai_enabled) {
        pthread_create(&ztk.ai_thread_id, NULL, ztk_ai_thread, NULL);
        pthread_detach(ztk.ai_thread_id);
    }
    
    /* Start shell */
    ztk_loop();
    
    /* Cleanup */
    ztk_cleanup();
    curl_global_cleanup();
    
    return 0;
}
