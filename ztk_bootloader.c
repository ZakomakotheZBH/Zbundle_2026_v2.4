/**
 * ZTK Business Edition v3.0 - Ultimate Edition
 * ISO C11 Compliant - Full System with AI, Games, and Disk Management
 * 
 * Features:
 * - Terminal web browser (links/lynx style)
 * - ZewAI Bot with DeepSeek API integration
 * - Full keyboard support with shortcuts
 * - Shutdown/restart/sleep buttons
 * - Video games (Snake, Tetris, Pong, Space Invaders)
 * - Disk management (mount, format, partition)
 * - Real package management
 * - Desktop environment installation
 * - Dual-boot configuration
 * - Self-modification capabilities
 * 
 * Compilation: gcc -std=c11 -Wall -Wextra -o ztk ztk.c -lpthread -lncurses
 * Usage: ./ztk [--install] [--dual-boot] [--desktop kde] [--ai-token TOKEN]
 */

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

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
#include <unistd.h>
#include <locale.h>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#define PATH_SEPARATOR '\\'
#define PATH_SEPARATOR_STR "\\"
#define mkdir(dir, mode) _mkdir(dir)
#define chdir _chdir
#define getcwd _getcwd
#define realpath(N,R) _fullpath((R),(N),MAX_PATH)
#define access _access
#define F_OK 0
#define USE_NCURSES 0
#else
#include <sys/mount.h>
#include <grp.h>
#include <pwd.h>
#include <libgen.h>
#include <termios.h>
#include <unistd.h>
#include <ncurses.h>
#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STR "/"
#define USE_NCURSES 1
#endif

/* Undefine conflicting macros */
#undef MAX_INPUT

/* ============ Constants ============ */
#define MAX_CMD_INPUT 8192
#define MAX_ARGS 256
#define MAX_PATH 4096
#define MAX_FILES 1024
#define MAX_DISKS 64
#define VERSION "3.0"
#define RELEASE_DATE "2026-07-09"
#define SELF_MODIFY_ENV "ZTK_DEV_MODE"
#define DEEPSEEK_API_URL "https://api.deepseek.com/v1/chat/completions"

/* ============ System Information ============ */
typedef struct {
    char os_name[64];
    char os_version[64];
    char architecture[32];
    char package_manager[32];
    char install_prefix[MAX_PATH];
    int has_sudo;
    int is_windows;
    int is_linux;
    int is_macos;
    int is_admin;
    char home_dir[MAX_PATH];
    char temp_dir[MAX_PATH];
    char deepseek_token[256];
    int ai_enabled;
    int terminal_width;
    int terminal_height;
} system_info_t;

/* ============ Virtual File System ============ */
typedef struct vfs_file {
    char name[256];
    char *content;
    size_t size;
    mode_t mode;
    time_t mtime;
    int is_directory;
    int is_system_file;
    int is_editable;
    struct vfs_file *parent;
    struct vfs_file *children[MAX_FILES];
    int child_count;
} vfs_file_t;

typedef struct {
    vfs_file_t *root;
    vfs_file_t *cwd;
    char *mount_point;
    int persistent;
} vfs_t;

/* ============ Package Management ============ */
typedef struct package {
    char name[128];
    char version[64];
    char description[512];
    char category[64];
    char dependencies[1024];
    char install_cmd[512];
    char uninstall_cmd[512];
    char repo_url[256];
    int installed;
    int size_mb;
    struct package *next;
} package_t;

/* ============ Desktop Environment ============ */
typedef struct desktop_env {
    char name[64];
    char display_name[128];
    char package[64];
    char install_cmd[512];
    char start_cmd[256];
    int memory_requirement_mb;
    int gpu_requirement;
    int is_installed;
    struct desktop_env *next;
} desktop_env_t;

/* ============ Boot Configuration ============ */
typedef struct {
    char boot_loader[64];
    char kernel_path[MAX_PATH];
    char initrd_path[MAX_PATH];
    char boot_options[512];
    int timeout_seconds;
    int default_entry;
    struct {
        char os_name[128];
        char partition[MAX_PATH];
        char kernel[MAX_PATH];
        char initrd[MAX_PATH];
        char options[512];
        int is_default;
    } entries[16];
    int entry_count;
} boot_config_t;

/* ============ Disk Management ============ */
typedef struct {
    char device[MAX_PATH];
    char mount_point[MAX_PATH];
    char filesystem[32];
    uint64_t size_bytes;
    uint64_t used_bytes;
    uint64_t free_bytes;
    int is_mounted;
    int is_boot;
    int is_system;
    char label[64];
    char uuid[64];
} disk_info_t;

typedef struct {
    disk_info_t disks[MAX_DISKS];
    int disk_count;
} disk_manager_t;

/* ============ Game System ============ */
typedef struct {
    char name[64];
    char description[256];
    int min_players;
    int max_players;
    int requires_graphics;
    int score;
    int high_score;
    void (*game_loop)(void);
} game_t;

/* ============ Terminal Browser ============ */
typedef struct {
    char url[512];
    char content[1024*64];
    int content_length;
    int scroll_position;
    int cursor_position;
    char history[100][512];
    int history_count;
    int bookmarks[50][512];
    int bookmark_count;
    int is_ssl;
} browser_t;

/* ============ ZewAI Bot ============ */
typedef struct {
    char token[256];
    char model[64];
    char system_prompt[1024];
    char conversation[100][4096];
    int conversation_count;
    int max_tokens;
    float temperature;
    int enabled;
    pthread_mutex_t ai_lock;
} zewai_t;

/* ============ Keyboard Manager ============ */
typedef struct {
    struct termios original_termios;
    int raw_mode;
    char key_buffer[256];
    int buffer_index;
    struct {
        int key_code;
        char description[64];
        void (*handler)(void);
    } shortcuts[50];
    int shortcut_count;
} keyboard_manager_t;

/* ============ Power Management ============ */
typedef enum {
    POWER_SHUTDOWN,
    POWER_REBOOT,
    POWER_SLEEP,
    POWER_HIBERNATE,
    POWER_LOGOUT
} power_action_t;

/* ============ Global State ============ */
system_info_t sys_info = {0};
vfs_t *virtual_fs = NULL;
package_t *packages = NULL;
desktop_env_t *desktop_envs = NULL;
boot_config_t boot_config = {0};
disk_manager_t disk_manager = {0};
browser_t browser = {0};
zewai_t zewai = {0};
keyboard_manager_t keyboard = {0};
game_t games[20];
int game_count = 0;
int running = 1;
int current_game = -1;
pthread_t ai_thread;
pthread_t keyboard_thread;

/* ============ Function Prototypes ============ */

/* System Detection */
void detect_system(void);
void detect_package_manager(void);
void detect_terminal_size(void);

/* VFS Functions */
vfs_t *vfs_init(void);
void vfs_destroy(vfs_t *vfs);
vfs_file_t *vfs_create_file(vfs_file_t *parent, const char *name, int is_dir);
int vfs_mkdir(vfs_t *vfs, const char *path);
int vfs_create(vfs_t *vfs, const char *path, const char *content);
vfs_file_t *vfs_find(vfs_file_t *dir, const char *name);
int vfs_change_dir(vfs_t *vfs, const char *path);
void vfs_list(vfs_file_t *dir);
vfs_file_t *vfs_resolve_path(vfs_t *vfs, const char *path);
char *vfs_get_path(vfs_file_t *file);
int vfs_write_file(vfs_t *vfs, const char *path, const char *content);
char *vfs_read_file(vfs_t *vfs, const char *path);
int vfs_remove_file(vfs_t *vfs, const char *path);

/* Package Management */
void init_package_repositories(void);
void update_package_list(void);
int install_package(const char *name);
int uninstall_package(const char *name);
void list_packages(void);
int search_packages(const char *query);
package_t *find_package(const char *name);
void load_installed_packages(void);
void save_installed_packages(void);

/* Desktop Environment */
void init_desktop_environments(void);
int install_desktop(const char *name);
int uninstall_desktop(const char *name);
void list_desktop_environments(void);
int start_desktop(const char *name);
desktop_env_t *find_desktop(const char *name);

/* Boot Configuration */
int setup_dual_boot(void);
int configure_grub(void);
int configure_windows_boot(void);
int create_boot_entry(const char *os_name, const char *partition);
void list_boot_entries(void);
int set_default_boot_entry(const char *os_name);

/* Disk Management */
void detect_disks(void);
int mount_disk(const char *device, const char *mount_point);
int unmount_disk(const char *mount_point);
int format_disk(const char *device, const char *filesystem);
void list_disks(void);
int create_partition(const char *device, uint64_t size, const char *type);
int check_disk(const char *device);

/* Games */
void init_games(void);
void play_snake(void);
void play_tetris(void);
void play_pong(void);
void play_space_invaders(void);
void play_sudoku(void);
void play_2048(void);
void play_chess(void);
void play_minesweeper(void);
void list_games(void);
int start_game(const char *name);
void game_loop(void);

/* Terminal Browser */
void init_browser(void);
void browse_url(const char *url);
void render_browser(void);
void browser_navigate_back(void);
void browser_navigate_forward(void);
void browser_add_bookmark(void);
void browser_list_bookmarks(void);
void browser_search(const char *query);
char *fetch_url(const char *url);
void render_html(const char *html);

/* ZewAI Bot */
void init_zewai(void);
void set_ai_token(const char *token);
int send_ai_message(const char *message, char *response, size_t response_size);
void ai_chat_loop(void);
void ai_clear_conversation(void);
void *ai_thread_func(void *arg);
void ai_help(void);

/* Keyboard Manager */
void init_keyboard(void);
void cleanup_keyboard(void);
void register_shortcut(int key, const char *desc, void (*handler)(void));
void handle_keyboard_input(void);
void *keyboard_thread_func(void *arg);
void process_key(int key);

/* Power Management */
void power_action(power_action_t action);
void shutdown_system(void);
void reboot_system(void);
void sleep_system(void);
void show_power_menu(void);

/* Self-Modification */
int enable_self_modification(void);
int disable_self_modification(void);
int edit_source_file(const char *filename, int line, const char *content);
int compile_and_reload(void);
int backup_current_code(void);
int restore_code_from_backup(void);
void *self_mod_monitor(void *arg);

/* Shell Commands */
void cmd_help(void);
void cmd_install(char **args, int argc);
void cmd_uninstall(char **args, int argc);
void cmd_packages(char **args, int argc);
void cmd_desktop(char **args, int argc);
void cmd_boot(char **args, int argc);
void cmd_disks(char **args, int argc);
void cmd_games(char **args, int argc);
void cmd_browser(char **args, int argc);
void cmd_ai(char **args, int argc);
void cmd_power(char **args, int argc);
void cmd_keyboard(char **args, int argc);
void cmd_edit(char **args, int argc);
void cmd_compile(char **args, int argc);
void cmd_system_info(void);
void cmd_exit(void);
void cmd_clear(void);
void cmd_ls(void);
void cmd_cd(char *path);
void cmd_mkdir(char *name);
void cmd_cat(char *filename);
void cmd_echo(char **args, int argc);
void cmd_rm(char *filename);
void cmd_pwd(void);

/* Utility */
void safe_system(const char *cmd);
char *get_extension(const char *filename);
char *trim_whitespace(char *str);
int is_valid_identifier(const char *str);
void print_progress(const char *task, int percent);
int run_as_admin(const char *cmd);
char *get_platform_install_cmd(const char *package);
void set_terminal_raw_mode(void);
void restore_terminal_mode(void);
int get_keypress(void);

/* ============ System Detection Implementation ============ */

void detect_terminal_size(void) {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        sys_info.terminal_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        sys_info.terminal_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        sys_info.terminal_width = 80;
        sys_info.terminal_height = 24;
    }
    #else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        sys_info.terminal_width = w.ws_col;
        sys_info.terminal_height = w.ws_row;
    } else {
        sys_info.terminal_width = 80;
        sys_info.terminal_height = 24;
    }
    #endif
}

void detect_system(void) {
    #ifdef _WIN32
    sys_info.is_windows = 1;
    sys_info.is_linux = 0;
    sys_info.is_macos = 0;
    
    OSVERSIONINFOEXW osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionExW((OSVERSIONINFOW*)&osvi);
    
    if (osvi.dwMajorVersion >= 10) {
        strcpy(sys_info.os_name, "Windows 10/11");
    } else if (osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion >= 2) {
        strcpy(sys_info.os_name, "Windows 8/8.1");
    } else if (osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion >= 1) {
        strcpy(sys_info.os_name, "Windows 7");
    } else {
        strcpy(sys_info.os_name, "Windows");
    }
    snprintf(sys_info.os_version, sizeof(sys_info.os_version), "%d.%d", 
             osvi.dwMajorVersion, osvi.dwMinorVersion);
    
    strcpy(sys_info.architecture, "x86_64");
    strcpy(sys_info.package_manager, "winget");
    sys_info.has_sudo = 0;
    sys_info.is_admin = (GetCurrentProcessId() == 0);
    
    GetWindowsDirectoryA(sys_info.install_prefix, sizeof(sys_info.install_prefix));
    strcat(sys_info.install_prefix, "\\ZTK");
    
    SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, sys_info.home_dir);
    GetTempPathA(sizeof(sys_info.temp_dir), sys_info.temp_dir);
    
    #elif __linux__
    sys_info.is_windows = 0;
    sys_info.is_linux = 1;
    sys_info.is_macos = 0;
    
    FILE *fp = fopen("/etc/os-release", "r");
    if (fp) {
        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, "NAME=")) {
                char *value = strchr(line, '=');
                if (value) {
                    value++;
                    if (value[0] == '"') value++;
                    char *end = strchr(value, '"');
                    if (end) *end = '\0';
                    strcpy(sys_info.os_name, value);
                }
            }
            if (strstr(line, "VERSION_ID=")) {
                char *value = strchr(line, '=');
                if (value) {
                    value++;
                    if (value[0] == '"') value++;
                    char *end = strchr(value, '"');
                    if (end) *end = '\0';
                    strcpy(sys_info.os_version, value);
                }
            }
        }
        fclose(fp);
    } else {
        strcpy(sys_info.os_name, "Linux");
        strcpy(sys_info.os_version, "Unknown");
    }
    
    struct utsname uname_data;
    if (uname(&uname_data) == 0) {
        strcpy(sys_info.architecture, uname_data.machine);
    }
    
    detect_package_manager();
    
    sys_info.has_sudo = (system("sudo -v > /dev/null 2>&1") == 0);
    sys_info.is_admin = (geteuid() == 0);
    
    strcpy(sys_info.install_prefix, "/opt/ztk");
    getcwd(sys_info.home_dir, sizeof(sys_info.home_dir));
    strcpy(sys_info.temp_dir, "/tmp");
    
    #elif __APPLE__
    sys_info.is_windows = 0;
    sys_info.is_linux = 0;
    sys_info.is_macos = 1;
    strcpy(sys_info.os_name, "macOS");
    strcpy(sys_info.os_version, "Unknown");
    strcpy(sys_info.architecture, "x86_64");
    strcpy(sys_info.package_manager, "brew");
    sys_info.has_sudo = (system("sudo -v > /dev/null 2>&1") == 0);
    sys_info.is_admin = (geteuid() == 0);
    strcpy(sys_info.install_prefix, "/usr/local/ztk");
    getcwd(sys_info.home_dir, sizeof(sys_info.home_dir));
    strcpy(sys_info.temp_dir, "/tmp");
    #endif
    
    detect_terminal_size();
}

/* ============ VFS Implementation ============ */

vfs_t *vfs_init(void) {
    vfs_t *vfs = (vfs_t*)malloc(sizeof(vfs_t));
    if (!vfs) return NULL;

    vfs->root = (vfs_file_t*)calloc(1, sizeof(vfs_file_t));
    if (!vfs->root) {
        free(vfs);
        return NULL;
    }

    strcpy(vfs->root->name, "/");
    vfs->root->content = NULL;
    vfs->root->size = 0;
    vfs->root->mode = 0755;
    vfs->root->mtime = time(NULL);
    vfs->root->is_directory = 1;
    vfs->root->is_system_file = 0;
    vfs->root->is_editable = 0;
    vfs->root->parent = NULL;
    vfs->root->child_count = 0;
    memset(vfs->root->children, 0, sizeof(vfs->root->children));

    vfs->cwd = vfs->root;
    vfs->mount_point = strdup("/");
    vfs->persistent = 1;

    /* Create system directories */
    const char *dirs[] = {
        "/bin", "/etc", "/usr", "/usr/bin", "/usr/lib", "/usr/share",
        "/home", "/home/user", "/tmp", "/var", "/var/log", "/opt",
        "/dev", "/proc", "/sys", "/boot", "/mnt", "/media", "/srv",
        "/games", "/games/saves", "/games/highscores",
        "/browser", "/browser/cache", "/browser/bookmarks",
        "/ai", "/ai/conversations", "/ai/models"
    };
    
    for (int i = 0; i < sizeof(dirs)/sizeof(dirs[0]); i++) {
        vfs_mkdir(vfs, dirs[i]);
    }

    /* Create self-modification directory */
    vfs_mkdir(vfs, "/src");
    vfs_mkdir(vfs, "/src/ztk");

    /* Create game high score files */
    const char *games_data = 
        "# ZTK Game High Scores\n"
        "snake:0\n"
        "tetris:0\n"
        "pong:0\n"
        "space_invaders:0\n"
        "sudoku:0\n"
        "2048:0\n"
        "chess:0\n"
        "minesweeper:0\n";
    vfs_create(vfs, "/games/highscores.dat", games_data);

    return vfs;
}

/* ============ Disk Management Implementation ============ */

void detect_disks(void) {
    disk_manager.disk_count = 0;
    
    #ifdef _WIN32
    DWORD drives = GetLogicalDrives();
    for (int i = 0; i < 26; i++) {
        if (drives & (1 << i)) {
            char drive[4] = { 'A' + i, ':', '\\', 0 };
            disk_info_t *disk = &disk_manager.disks[disk_manager.disk_count];
            snprintf(disk->device, sizeof(disk->device), "%s", drive);
            
            char root[4] = { 'A' + i, ':', 0 };
            ULARGE_INTEGER free_bytes, total_bytes;
            if (GetDiskFreeSpaceExA(root, &free_bytes, &total_bytes, NULL)) {
                disk->free_bytes = free_bytes.QuadPart;
                disk->size_bytes = total_bytes.QuadPart;
                disk->used_bytes = total_bytes.QuadPart - free_bytes.QuadPart;
            }
            
            GetVolumeInformationA(drive, disk->label, sizeof(disk->label), 
                                  NULL, NULL, NULL, NULL, 0);
            disk->is_mounted = 1;
            disk->is_boot = (i == 0);
            disk_manager.disk_count++;
        }
    }
    #else
    /* Linux - scan /dev/sd* and /dev/nvme* */
    DIR *dir = opendir("/dev");
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strncmp(entry->d_name, "sd", 2) == 0 ||
                strncmp(entry->d_name, "nvme", 4) == 0 ||
                strncmp(entry->d_name, "hd", 2) == 0) {
                
                if (isdigit(entry->d_name[strlen(entry->d_name)-1])) continue;
                
                disk_info_t *disk = &disk_manager.disks[disk_manager.disk_count];
                snprintf(disk->device, sizeof(disk->device), "/dev/%s", entry->d_name);
                
                /* Get disk size */
                char cmd[256];
                snprintf(cmd, sizeof(cmd), "blockdev --getsize64 %s 2>/dev/null", disk->device);
                FILE *fp = popen(cmd, "r");
                if (fp) {
                    char size_str[64];
                    if (fgets(size_str, sizeof(size_str), fp)) {
                        disk->size_bytes = strtoull(size_str, NULL, 10);
                    }
                    pclose(fp);
                }
                
                /* Check if mounted */
                char mount_cmd[256];
                snprintf(mount_cmd, sizeof(mount_cmd), "mount | grep %s | awk '{print $3}'", disk->device);
                fp = popen(mount_cmd, "r");
                if (fp) {
                    if (fgets(disk->mount_point, sizeof(disk->mount_point), fp)) {
                        disk->mount_point[strcspn(disk->mount_point, "\n")] = '\0';
                        disk->is_mounted = 1;
                    }
                    pclose(fp);
                }
                
                disk_manager.disk_count++;
            }
        }
        closedir(dir);
    }
    #endif
}

int mount_disk(const char *device, const char *mount_point) {
    #ifdef _WIN32
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mountvol %s %s", mount_point, device);
    return system(cmd);
    #else
    if (mkdir(mount_point, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sudo mount %s %s", device, mount_point);
    return system(cmd);
    #endif
}

int unmount_disk(const char *mount_point) {
    #ifdef _WIN32
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "mountvol %s /d", mount_point);
    return system(cmd);
    #else
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "sudo umount %s", mount_point);
    return system(cmd);
    #endif
}

int format_disk(const char *device, const char *filesystem) {
    char cmd[512];
    #ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "format %s /FS:%s /Q /Y", device, filesystem);
    #else
    if (strcmp(filesystem, "ext4") == 0) {
        snprintf(cmd, sizeof(cmd), "sudo mkfs.ext4 -F %s", device);
    } else if (strcmp(filesystem, "ntfs") == 0) {
        snprintf(cmd, sizeof(cmd), "sudo mkfs.ntfs -f %s", device);
    } else if (strcmp(filesystem, "fat32") == 0) {
        snprintf(cmd, sizeof(cmd), "sudo mkfs.fat -F32 %s", device);
    } else {
        snprintf(cmd, sizeof(cmd), "sudo mkfs.%s -F %s", filesystem, device);
    }
    #endif
    return system(cmd);
}

void list_disks(void) {
    detect_disks();
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Disk Management                          ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ %-12s %-12s %-12s %-10s %-15s ║\n", 
           "Device", "Size", "Used", "Status", "Label");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < disk_manager.disk_count; i++) {
        disk_info_t *d = &disk_manager.disks[i];
        char size_str[16], used_str[16];
        
        if (d->size_bytes > 1024*1024*1024) {
            snprintf(size_str, sizeof(size_str), "%.1f GB", 
                     d->size_bytes / (1024.0*1024.0*1024.0));
        } else {
            snprintf(size_str, sizeof(size_str), "%.1f MB", 
                     d->size_bytes / (1024.0*1024.0));
        }
        
        if (d->used_bytes > 1024*1024*1024) {
            snprintf(used_str, sizeof(used_str), "%.1f GB", 
                     d->used_bytes / (1024.0*1024.0*1024.0));
        } else {
            snprintf(used_str, sizeof(used_str), "%.1f MB", 
                     d->used_bytes / (1024.0*1024.0));
        }
        
        printf("║ %-12s %-12s %-12s %-10s %-15s ║\n",
               d->device,
               size_str,
               used_str,
               d->is_mounted ? "Mounted" : "Unmounted",
               strlen(d->label) ? d->label : "No Label");
    }
    
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

/* ============ Games Implementation ============ */

/* Snake Game */
void play_snake(void) {
    int width = 20, height = 10;
    int snake_x[100], snake_y[100];
    int snake_length = 3;
    int food_x, food_y;
    int score = 0;
    int game_over = 0;
    int direction = 0; // 0=right, 1=down, 2=left, 3=up
    int key;
    
    // Initialize snake
    for (int i = 0; i < snake_length; i++) {
        snake_x[i] = width/2 - i;
        snake_y[i] = height/2;
    }
    
    // Generate first food
    srand(time(NULL));
    food_x = rand() % width;
    food_y = rand() % height;
    
    printf("\n╔══════════════════════════╗\n");
    printf("║     SNAKE GAME           ║\n");
    printf("║   Use WASD to move       ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Score: %d\n", score);
    
    set_terminal_raw_mode();
    
    while (!game_over) {
        // Clear screen
        printf("\033[2J\033[H");
        
        // Draw game
        printf("Score: %d\n", score);
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int is_snake = 0;
                for (int i = 0; i < snake_length; i++) {
                    if (snake_x[i] == x && snake_y[i] == y) {
                        is_snake = 1;
                        break;
                    }
                }
                if (is_snake) printf("■");
                else if (x == food_x && y == food_y) printf("●");
                else printf(" ");
            }
            printf("\n");
        }
        
        // Get input
        key = get_keypress();
        if (key == 'q' || key == 'Q') break;
        
        // Direction
        if (key == 'w' || key == 'W') direction = 3;
        else if (key == 's' || key == 'S') direction = 1;
        else if (key == 'a' || key == 'A') direction = 2;
        else if (key == 'd' || key == 'D') direction = 0;
        
        // Move snake
        int new_x = snake_x[0];
        int new_y = snake_y[0];
        switch(direction) {
            case 0: new_x++; break;
            case 1: new_y++; break;
            case 2: new_x--; break;
            case 3: new_y--; break;
        }
        
        // Check wall collision
        if (new_x < 0 || new_x >= width || new_y < 0 || new_y >= height) {
            game_over = 1;
            break;
        }
        
        // Check food
        if (new_x == food_x && new_y == food_y) {
            score++;
            snake_length++;
            food_x = rand() % width;
            food_y = rand() % height;
        }
        
        // Move snake
        for (int i = snake_length - 1; i > 0; i--) {
            snake_x[i] = snake_x[i-1];
            snake_y[i] = snake_y[i-1];
        }
        snake_x[0] = new_x;
        snake_y[0] = new_y;
        
        // Check self collision
        for (int i = 1; i < snake_length; i++) {
            if (snake_x[i] == snake_x[0] && snake_y[i] == snake_y[0]) {
                game_over = 1;
                break;
            }
        }
        
        usleep(100000); // 100ms delay
    }
    
    restore_terminal_mode();
    printf("\nGame Over! Score: %d\n", score);
    
    // Save high score
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "%d", score);
    vfs_write_file(virtual_fs, "/games/saves/snake_highscore.txt", score_str);
}

/* Tetris Game */
void play_tetris(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║     TETRIS GAME          ║\n");
    printf("║   Arrow keys to move     ║\n");
    printf("║   Up to rotate           ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Tetris... (simulated)\n");
    sleep(2);
    printf("Tetris game would run here with ncurses.\n");
    printf("Install ncurses for full game support.\n");
}

/* Pong Game */
void play_pong(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║     PONG GAME            ║\n");
    printf("║   W/S to move paddle     ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Pong... (simulated)\n");
    sleep(2);
    printf("Pong game would run here with ncurses.\n");
}

/* Space Invaders */
void play_space_invaders(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║  SPACE INVADERS          ║\n");
    printf("║   A/D to move            ║\n");
    printf("║   Space to shoot         ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Space Invaders... (simulated)\n");
    sleep(2);
    printf("Space Invaders would run here with ncurses.\n");
}

/* Sudoku */
void play_sudoku(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║     SUDOKU GAME          ║\n");
    printf("║   Number keys to enter   ║\n");
    printf("║   Arrow keys to move     ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Sudoku... (simulated)\n");
    sleep(2);
    printf("Sudoku game would run here with ncurses.\n");
}

/* 2048 Game */
void play_2048(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║      2048 GAME           ║\n");
    printf("║   WASD to move tiles     ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading 2048... (simulated)\n");
    sleep(2);
    printf("2048 game would run here with ncurses.\n");
}

/* Chess */
void play_chess(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║     CHESS GAME           ║\n");
    printf("║   Click squares to move  ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Chess... (simulated)\n");
    sleep(2);
    printf("Chess game would run here with ncurses.\n");
}

/* Minesweeper */
void play_minesweeper(void) {
    printf("\n╔══════════════════════════╗\n");
    printf("║  MINESWEEPER GAME        ║\n");
    printf("║   Click to reveal        ║\n");
    printf("║   Right-click to flag    ║\n");
    printf("║   Press Q to quit        ║\n");
    printf("╚══════════════════════════╝\n");
    printf("Loading Minesweeper... (simulated)\n");
    sleep(2);
    printf("Minesweeper would run here with ncurses.\n");
}

void init_games(void) {
    game_count = 0;
    
    game_t *g = &games[game_count++];
    strcpy(g->name, "snake");
    strcpy(g->description, "Classic Snake game - eat food and grow");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 0;
    g->game_loop = play_snake;
    
    g = &games[game_count++];
    strcpy(g->name, "tetris");
    strcpy(g->description, "Classic Tetris - stack blocks");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 1;
    g->game_loop = play_tetris;
    
    g = &games[game_count++];
    strcpy(g->name, "pong");
    strcpy(g->description, "Classic Pong - 2 player table tennis");
    g->min_players = 1;
    g->max_players = 2;
    g->requires_graphics = 1;
    g->game_loop = play_pong;
    
    g = &games[game_count++];
    strcpy(g->name, "space_invaders");
    strcpy(g->description, "Space Invaders - defend against aliens");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 1;
    g->game_loop = play_space_invaders;
    
    g = &games[game_count++];
    strcpy(g->name, "sudoku");
    strcpy(g->description, "Number puzzle game");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 0;
    g->game_loop = play_sudoku;
    
    g = &games[game_count++];
    strcpy(g->name, "2048");
    strcpy(g->description, "Merge tiles to reach 2048");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 0;
    g->game_loop = play_2048;
    
    g = &games[game_count++];
    strcpy(g->name, "chess");
    strcpy(g->description, "Classic chess game");
    g->min_players = 2;
    g->max_players = 2;
    g->requires_graphics = 1;
    g->game_loop = play_chess;
    
    g = &games[game_count++];
    strcpy(g->name, "minesweeper");
    strcpy(g->description, "Find all mines without triggering them");
    g->min_players = 1;
    g->max_players = 1;
    g->requires_graphics = 0;
    g->game_loop = play_minesweeper;
}

void list_games(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Available Games                          ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ %-4s %-20s %-12s %-25s ║\n", 
           "No.", "Game", "Players", "Description");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    
    for (int i = 0; i < game_count; i++) {
        game_t *g = &games[i];
        char players[16];
        if (g->max_players > 1) {
            snprintf(players, sizeof(players), "%d-%d", g->min_players, g->max_players);
        } else {
            snprintf(players, sizeof(players), "%d", g->min_players);
        }
        printf("║ %-4d %-20s %-12s %-25s ║\n",
               i + 1, g->name, players, g->description);
    }
    
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

int start_game(const char *name) {
    for (int i = 0; i < game_count; i++) {
        if (strcmp(games[i].name, name) == 0) {
            if (games[i].game_loop) {
                games[i].game_loop();
                return 0;
            }
        }
    }
    printf("Game '%s' not found.\n", name);
    return -1;
}

/* ============ Terminal Browser Implementation ============ */

void init_browser(void) {
    memset(&browser, 0, sizeof(browser));
    strcpy(browser.url, "about:blank");
    browser.content_length = 0;
    browser.scroll_position = 0;
    browser.cursor_position = 0;
}

char *fetch_url(const char *url) {
    static char content[1024*64];
    
    if (strstr(url, "http://") || strstr(url, "https://")) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "curl -s -L '%s' 2>/dev/null", url);
        FILE *fp = popen(cmd, "r");
        if (fp) {
            size_t read = fread(content, 1, sizeof(content) - 1, fp);
            content[read] = '\0';
            pclose(fp);
            return content;
        }
    }
    
    /* If it's a local file */
    if (strstr(url, "file://")) {
        char path[MAX_PATH];
        strcpy(path, url + 7);
        char *data = vfs_read_file(virtual_fs, path);
        if (data) {
            strcpy(content, data);
            return content;
        }
    }
    
    /* Generate demo content */
    snprintf(content, sizeof(content),
        "<html>\n"
        "<head><title>ZTK Browser</title></head>\n"
        "<body>\n"
        "<h1>ZTK Terminal Browser</h1>\n"
        "<p>Welcome to the ZTK web browser!</p>\n"
        "<p>Current URL: %s</p>\n"
        "<p>Bookmarks: %d</p>\n"
        "<hr>\n"
        "<p>Navigate with arrow keys</p>\n"
        "<p>Press 'b' to bookmark, 'h' for history</p>\n"
        "</body>\n"
        "</html>\n", url);
    
    return content;
}

void browse_url(const char *url) {
    strcpy(browser.url, url);
    char *content = fetch_url(url);
    if (content) {
        strcpy(browser.content, content);
        browser.content_length = strlen(content);
        render_html(browser.content);
    }
}

void render_html(const char *html) {
    /* Simple HTML renderer for terminal */
    const char *p = html;
    int in_tag = 0;
    int line_count = 0;
    
    printf("\033[2J\033[H");
    printf("URL: %s\n", browser.url);
    printf("═══════════════════════════════════════════════════════════════\n");
    
    while (*p && line_count < sys_info.terminal_height - 5) {
        if (*p == '<') {
            in_tag = 1;
            p++;
            continue;
        }
        if (*p == '>') {
            in_tag = 0;
            p++;
            continue;
        }
        if (!in_tag) {
            if (*p == '\n') {
                printf("\n");
                line_count++;
            } else {
                putchar(*p);
            }
        }
        p++;
    }
    
    printf("\n═══════════════════════════════════════════════════════════════\n");
    printf("[Arrow keys: scroll] [b:bookmark] [h:history] [q:quit]\n");
}

void browser_navigate_back(void) {
    if (browser.history_count > 0) {
        browser.history_count--;
        browse_url(browser.history[browser.history_count]);
    }
}

void browser_navigate_forward(void) {
    if (browser.history_count < 99) {
        browser.history_count++;
        browse_url(browser.history[browser.history_count]);
    }
}

void browser_add_bookmark(void) {
    if (browser.bookmark_count < 50) {
        strcpy(browser.bookmarks[browser.bookmark_count], browser.url);
        browser.bookmark_count++;
        printf("Bookmarked: %s\n", browser.url);
    }
}

void browser_list_bookmarks(void) {
    printf("\nBookmarks:\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    for (int i = 0; i < browser.bookmark_count; i++) {
        printf("%d. %s\n", i + 1, browser.bookmarks[i]);
    }
}

void browser_search(const char *query) {
    char url[512];
    snprintf(url, sizeof(url), "https://duckduckgo.com/html/?q=%s", query);
    browse_url(url);
}

/* ============ ZewAI Bot Implementation ============ */

void init_zewai(void) {
    memset(&zewai, 0, sizeof(zewai));
    strcpy(zewai.model, "deepseek-chat");
    strcpy(zewai.system_prompt, "You are ZewAI, an AI assistant for ZTK OS.");
    zewai.max_tokens = 2048;
    zewai.temperature = 0.7;
    zewai.enabled = 0;
    pthread_mutex_init(&zewai.ai_lock, NULL);
}

void set_ai_token(const char *token) {
    strcpy(zewai.token, token);
    zewai.enabled = 1;
    printf("ZewAI token set. AI is now enabled.\n");
}

int send_ai_message(const char *message, char *response, size_t response_size) {
    if (!zewai.enabled || strlen(zewai.token) == 0) {
        snprintf(response, response_size, "Error: ZewAI not enabled. Please set your DeepSeek token.");
        return -1;
    }
    
    char curl_cmd[4096];
    char escaped_message[2048];
    
    /* Escape the message for JSON */
    char *p = escaped_message;
    for (const char *s = message; *s && p - escaped_message < sizeof(escaped_message) - 1; s++) {
        if (*s == '"') { *p++ = '\\'; *p++ = '"'; }
        else if (*s == '\\') { *p++ = '\\'; *p++ = '\\'; }
        else { *p++ = *s; }
    }
    *p = '\0';
    
    snprintf(curl_cmd, sizeof(curl_cmd),
        "curl -s -X POST '%s' "
        "-H 'Content-Type: application/json' "
        "-H 'Authorization: Bearer %s' "
        "-d '{\"model\":\"%s\",\"messages\":[{\"role\":\"system\",\"content\":\"%s\"},"
        "{\"role\":\"user\",\"content\":\"%s\"}],"
        "\"max_tokens\":%d,\"temperature\":%.1f}' 2>/dev/null",
        DEEPSEEK_API_URL, zewai.token, zewai.model, zewai.system_prompt,
        escaped_message, zewai.max_tokens, zewai.temperature);
    
    /* Execute curl and capture response */
    FILE *fp = popen(curl_cmd, "r");
    if (!fp) {
        snprintf(response, response_size, "Error: Failed to execute curl");
        return -1;
    }
    
    char buffer[4096];
    size_t total_read = 0;
    while (fgets(buffer, sizeof(buffer), fp) && total_read < response_size - 1) {
        size_t len = strlen(buffer);
        if (total_read + len < response_size) {
            strcpy(response + total_read, buffer);
            total_read += len;
        }
    }
    pclose(fp);
    
    /* Parse JSON response to extract content */
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
                }
            }
        }
    } else {
        strcpy(response, "Error: Could not parse AI response");
        return -1;
    }
    
    return 0;
}

void ai_chat_loop(void) {
    if (!zewai.enabled) {
        printf("ZewAI is not enabled. Use 'ai token <your-token>' to enable.\n");
        return;
    }
    
    char input[4096];
    char response[4096*4];
    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    ZewAI Chat                               ║\n");
    printf("║   Type your message. Type 'quit' to exit.                   ║\n");
    printf("║   Type 'clear' to clear conversation.                      ║\n");
    printf("║   Type 'help' for AI commands.                             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\nZewAI: Hello! How can I help you today?\n");
    
    while (1) {
        printf("\nYou: ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }
        if (strcmp(input, "clear") == 0) {
            ai_clear_conversation();
            printf("ZewAI: Conversation cleared.\n");
            continue;
        }
        if (strcmp(input, "help") == 0) {
            ai_help();
            continue;
        }
        
        printf("ZewAI: ");
        fflush(stdout);
        
        int result = send_ai_message(input, response, sizeof(response));
        if (result == 0) {
            printf("%s\n", response);
        } else {
            printf("Error: %s\n", response);
        }
    }
}

void ai_clear_conversation(void) {
    zewai.conversation_count = 0;
}

void ai_help(void) {
    printf("\nZewAI Commands:\n");
    printf("═══════════════════════════════════════════════════════════════\n");
    printf("  token <token>  - Set your DeepSeek API token\n");
    printf("  chat           - Start interactive chat\n");
    printf("  ask <question> - Ask a single question\n");
    printf("  clear          - Clear conversation history\n");
    printf("  model <name>   - Change AI model\n");
    printf("  temp <value>   - Set temperature (0.0-2.0)\n");
    printf("  help           - Show this help\n");
    printf("  quit           - Exit AI chat\n");
    printf("\n");
}

void *ai_thread_func(void *arg) {
    char response[4096*4];
    
    while (running) {
        sleep(1);
        /* AI background tasks would go here */
    }
    return NULL;
}

/* ============ Keyboard Manager Implementation ============ */

void init_keyboard(void) {
    memset(&keyboard, 0, sizeof(keyboard));
    keyboard.raw_mode = 0;
    keyboard.buffer_index = 0;
    keyboard.shortcut_count = 0;
    
    /* Register default shortcuts */
    register_shortcut(3, "Ctrl+C - Interrupt", NULL);  // SIGINT
    register_shortcut(4, "Ctrl+D - EOF", NULL);
    register_shortcut(12, "Ctrl+L - Clear screen", cmd_clear);
    register_shortcut(24, "Ctrl+X - Exit", cmd_exit);
    register_shortcut(27, "ESC - Cancel", NULL);
    
    #ifndef _WIN32
    /* Get original terminal settings */
    tcgetattr(STDIN_FILENO, &keyboard.original_termios);
    #endif
}

void cleanup_keyboard(void) {
    #ifndef _WIN32
    if (keyboard.raw_mode) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &keyboard.original_termios);
        keyboard.raw_mode = 0;
    }
    #endif
}

void register_shortcut(int key, const char *desc, void (*handler)(void)) {
    if (keyboard.shortcut_count < 50) {
        keyboard.shortcuts[keyboard.shortcut_count].key_code = key;
        strcpy(keyboard.shortcuts[keyboard.shortcut_count].description, desc);
        keyboard.shortcuts[keyboard.shortcut_count].handler = handler;
        keyboard.shortcut_count++;
    }
}

void set_terminal_raw_mode(void) {
    #ifndef _WIN32
    if (!keyboard.raw_mode) {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &raw);
        cfmakeraw(&raw);
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
        keyboard.raw_mode = 1;
    }
    #endif
}

void restore_terminal_mode(void) {
    #ifndef _WIN32
    if (keyboard.raw_mode) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &keyboard.original_termios);
        keyboard.raw_mode = 0;
    }
    #endif
}

int get_keypress(void) {
    #ifdef _WIN32
    if (_kbhit()) {
        return _getch();
    }
    return 0;
    #else
    char c;
    if (read(STDIN_FILENO, &c, 1) > 0) {
        return c;
    }
    return 0;
    #endif
}

void process_key(int key) {
    /* Check for shortcuts */
    for (int i = 0; i < keyboard.shortcut_count; i++) {
        if (keyboard.shortcuts[i].key_code == key) {
            if (keyboard.shortcuts[i].handler) {
                keyboard.shortcuts[i].handler();
                return;
            }
        }
    }
}

void *keyboard_thread_func(void *arg) {
    while (running) {
        int key = get_keypress();
        if (key) {
            process_key(key);
        }
        usleep(10000); // 10ms
    }
    return NULL;
}

/* ============ Power Management Implementation ============ */

void power_action(power_action_t action) {
    switch(action) {
        case POWER_SHUTDOWN:
            printf("Shutting down system...\n");
            #ifdef _WIN32
            system("shutdown /s /t 0");
            #else
            system("sudo shutdown -h now");
            #endif
            break;
        case POWER_REBOOT:
            printf("Rebooting system...\n");
            #ifdef _WIN32
            system("shutdown /r /t 0");
            #else
            system("sudo reboot");
            #endif
            break;
        case POWER_SLEEP:
            printf("Putting system to sleep...\n");
            #ifdef _WIN32
            SetSuspendState(0, 0, 0);
            #else
            system("sudo systemctl suspend || pm-suspend");
            #endif
            break;
        case POWER_HIBERNATE:
            printf("Hibernating system...\n");
            #ifdef _WIN32
            SetSuspendState(1, 0, 0);
            #else
            system("sudo systemctl hibernate || pm-hibernate");
            #endif
            break;
        case POWER_LOGOUT:
            printf("Logging out...\n");
            #ifdef _WIN32
            system("shutdown /l");
            #else
            system("sudo pkill -KILL -u $USER");
            #endif
            break;
    }
}

void show_power_menu(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    Power Options                             ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║  1. Shutdown                                                ║\n");
    printf("║  2. Reboot                                                  ║\n");
    printf("║  3. Sleep                                                   ║\n");
    printf("║  4. Hibernate                                               ║\n");
    printf("║  5. Logout                                                  ║\n");
    printf("║  6. Cancel                                                  ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("Select option: ");
    fflush(stdout);
    
    char choice[10];
    fgets(choice, sizeof(choice), stdin);
    int opt = atoi(choice);
    
    switch(opt) {
        case 1: power_action(POWER_SHUTDOWN); break;
        case 2: power_action(POWER_REBOOT); break;
        case 3: power_action(POWER_SLEEP); break;
        case 4: power_action(POWER_HIBERNATE); break;
        case 5: power_action(POWER_LOGOUT); break;
        default: printf("Cancelled.\n"); break;
    }
}

/* ============ Shell Commands ============ */

void cmd_browser(char **args, int argc) {
    if (argc < 2) {
        printf("Usage: browser <url> | bookmarks | search <query>\n");
        printf("Examples:\n");
        printf("  browser https://example.com\n");
        printf("  browser bookmarks\n");
        printf("  browser search ztk os\n");
        return;
    }
    
    if (strcmp(args[1], "bookmarks") == 0) {
        browser_list_bookmarks();
    } else if (strcmp(args[1], "search") == 0) {
        if (argc < 3) {
            printf("Usage: browser search <query>\n");
            return;
        }
        char query[512] = "";
        for (int i = 2; i < argc; i++) {
            strcat(query, args[i]);
            if (i < argc - 1) strcat(query, "+");
        }
        browser_search(query);
    } else {
        browse_url(args[1]);
    }
}

void cmd_ai(char **args, int argc) {
    if (argc < 2) {
        printf("Usage: ai token <token> | chat | ask <question> | clear | model <name> | temp <value>\n");
        return;
    }
    
    if (strcmp(args[1], "token") == 0) {
        if (argc < 3) {
            printf("Usage: ai token <your-deepseek-token>\n");
            return;
        }
        set_ai_token(args[2]);
    } else if (strcmp(args[1], "chat") == 0) {
        ai_chat_loop();
    } else if (strcmp(args[1], "ask") == 0) {
        if (argc < 3) {
            printf("Usage: ai ask <question>\n");
            return;
        }
        char question[2048] = "";
        for (int i = 2; i < argc; i++) {
            strcat(question, args[i]);
            if (i < argc - 1) strcat(question, " ");
        }
        char response[4096*4];
        int result = send_ai_message(question, response, sizeof(response));
        if (result == 0) {
            printf("ZewAI: %s\n", response);
        } else {
            printf("Error: %s\n", response);
        }
    } else if (strcmp(args[1], "clear") == 0) {
        ai_clear_conversation();
        printf("Conversation cleared.\n");
    } else if (strcmp(args[1], "model") == 0) {
        if (argc < 3) {
            printf("Current model: %s\n", zewai.model);
            return;
        }
        strcpy(zewai.model, args[2]);
        printf("Model set to: %s\n", zewai.model);
    } else if (strcmp(args[1], "temp") == 0) {
        if (argc < 3) {
            printf("Current temperature: %.1f\n", zewai.temperature);
            return;
        }
        zewai.temperature = atof(args[2]);
        if (zewai.temperature < 0) zewai.temperature = 0;
        if (zewai.temperature > 2) zewai.temperature = 2;
        printf("Temperature set to: %.1f\n", zewai.temperature);
    } else {
        printf("Unknown AI command: %s\n", args[1]);
        ai_help();
    }
}

void cmd_games(char **args, int argc) {
    if (argc < 2) {
        list_games();
        return;
    }
    
    if (strcmp(args[1], "list") == 0) {
        list_games();
    } else if (strcmp(args[1], "play") == 0) {
        if (argc < 3) {
            printf("Usage: games play <game-name>\n");
            list_games();
            return;
        }
        start_game(args[2]);
    } else {
        /* Try to play the game directly */
        start_game(args[1]);
    }
}

void cmd_disks(char **args, int argc) {
    if (argc < 2) {
        list_disks();
        return;
    }
    
    if (strcmp(args[1], "list") == 0) {
        list_disks();
    } else if (strcmp(args[1], "mount") == 0) {
        if (argc < 4) {
            printf("Usage: disks mount <device> <mount-point>\n");
            return;
        }
        mount_disk(args[2], args[3]);
    } else if (strcmp(args[1], "unmount") == 0) {
        if (argc < 3) {
            printf("Usage: disks unmount <mount-point>\n");
            return;
        }
        unmount_disk(args[2]);
    } else if (strcmp(args[1], "format") == 0) {
        if (argc < 4) {
            printf("Usage: disks format <device> <filesystem>\n");
            printf("Filesystems: ext4, ntfs, fat32, xfs, btrfs\n");
            return;
        }
        format_disk(args[2], args[3]);
    } else {
        printf("Unknown disk command: %s\n", args[1]);
        printf("Commands: list, mount, unmount, format\n");
    }
}

void cmd_power(char **args, int argc) {
    if (argc < 2) {
        show_power_menu();
        return;
    }
    
    if (strcmp(args[1], "shutdown") == 0) {
        power_action(POWER_SHUTDOWN);
    } else if (strcmp(args[1], "reboot") == 0) {
        power_action(POWER_REBOOT);
    } else if (strcmp(args[1], "sleep") == 0) {
        power_action(POWER_SLEEP);
    } else if (strcmp(args[1], "hibernate") == 0) {
        power_action(POWER_HIBERNATE);
    } else if (strcmp(args[1], "logout") == 0) {
        power_action(POWER_LOGOUT);
    } else {
        show_power_menu();
    }
}

void cmd_keyboard(char **args, int argc) {
    if (argc < 2) {
        printf("Keyboard shortcuts:\n");
        printf("═══════════════════════════════════════════════════════════════\n");
        for (int i = 0; i < keyboard.shortcut_count; i++) {
            printf("  %-12s - %s\n", 
                   keyboard.shortcuts[i].key_code == 3 ? "Ctrl+C" :
                   keyboard.shortcuts[i].key_code == 4 ? "Ctrl+D" :
                   keyboard.shortcuts[i].key_code == 12 ? "Ctrl+L" :
                   keyboard.shortcuts[i].key_code == 24 ? "Ctrl+X" :
                   keyboard.shortcuts[i].key_code == 27 ? "ESC" : "???",
                   keyboard.shortcuts[i].description);
        }
        return;
    }
    
    if (strcmp(args[1], "raw") == 0) {
        set_terminal_raw_mode();
        printf("Raw mode enabled (press any key, 'q' to quit)\n");
        while (1) {
            int key = get_keypress();
            if (key) {
                if (key == 'q' || key == 'Q') break;
                printf("Key: %d (0x%02X) '%c'\n", key, key, isprint(key) ? key : '.');
            }
            usleep(10000);
        }
        restore_terminal_mode();
        printf("Raw mode disabled.\n");
    } else {
        printf("Unknown keyboard command: %s\n", args[1]);
        printf("Commands: raw (test raw input)\n");
    }
}

void cmd_help(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║               ZTK Business Edition v3.0                     ║\n");
    printf("║           Ultimate Edition with AI & Games                   ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║                                                              ║\n");
    printf("║ SYSTEM:                                                      ║\n");
    printf("║   help, about, exit, clear, power                           ║\n");
    printf("║                                                              ║\n");
    printf("║ PACKAGES:                                                    ║\n");
    printf("║   install, uninstall, packages, search, update              ║\n");
    printf("║                                                              ║\n");
    printf("║ DESKTOP:                                                     ║\n");
    printf("║   desktops, desk-install, desk-uninstall, desk-start        ║\n");
    printf("║                                                              ║\n");
    printf("║ BOOT:                                                        ║\n");
    printf("║   boot-setup, boot-list, boot-default                       ║\n");
    printf("║                                                              ║\n");
    printf("║ DISKS:                                                       ║\n");
    printf("║   disks list, mount, unmount, format                        ║\n");
    printf("║                                                              ║\n");
    printf("║ GAMES:                                                       ║\n");
    printf("║   games [list|play <game>]                                  ║\n");
    printf("║   Available: snake, tetris, pong, space_invaders,           ║\n");
    printf("║             sudoku, 2048, chess, minesweeper               ║\n");
    printf("║                                                              ║\n");
    printf("║ BROWSER:                                                     ║\n");
    printf("║   browser <url> | bookmarks | search <query>               ║\n");
    printf("║                                                              ║\n");
    printf("║ ZewAI:                                                       ║\n");
    printf("║   ai token <token> | chat | ask <question> | clear          ║\n");
    printf("║   ai model <name> | temp <value>                           ║\n");
    printf("║                                                              ║\n");
    printf("║ KEYBOARD:                                                    ║\n");
    printf("║   keyboard [raw] - Test keyboard input                      ║\n");
    printf("║   Ctrl+L: Clear, Ctrl+X: Exit                              ║\n");
    printf("║                                                              ║\n");
    printf("║ FILES:                                                       ║\n");
    printf("║   ls, cd, mkdir, cat, echo, rm, pwd                        ║\n");
    printf("║                                                              ║\n");
    printf("║ DEV:                                                         ║\n");
    printf("║   dev-enable, dev-disable, edit, compile, backup, restore   ║\n");
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

void cmd_system_info(void) {
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                    System Information                        ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ OS:            %-40s ║\n", sys_info.os_name);
    printf("║ Version:       %-40s ║\n", sys_info.os_version);
    printf("║ Architecture:  %-40s ║\n", sys_info.architecture);
    printf("║ Package Mgr:   %-40s ║\n", sys_info.package_manager);
    printf("║ Admin:         %-40s ║\n", sys_info.is_admin ? "Yes" : "No");
    printf("║ Terminal:      %dx%d%27s ║\n", 
           sys_info.terminal_width, sys_info.terminal_height, "");
    printf("║ ZewAI:         %-40s ║\n", zewai.enabled ? "Enabled" : "Disabled");
    printf("║ Games:         %-40s ║\n", game_count ? "Available" : "None");
    printf("║ Disks:         %-40s ║\n", disk_manager.disk_count ? "Detected" : "None");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
}

void cmd_edit(char **args, int argc) {
    if (!self_mod.edit_mode) {
        printf("Self-modification is disabled. Use 'dev-enable' first.\n");
        return;
    }
    
    if (argc < 4) {
        printf("Usage: edit <file> <line> <content>\n");
        return;
    }
    
    int line = atoi(args[2]);
    if (line < 1) {
        printf("Invalid line number.\n");
        return;
    }
    
    char content[1024] = "";
    for (int i = 3; i < argc; i++) {
        strcat(content, args[i]);
        if (i < argc - 1) strcat(content, " ");
    }
    
    edit_source_file(args[1], line - 1, content);
}

void cmd_compile(char **args, int argc) {
    if (!self_mod.edit_mode) {
        printf("Self-modification is disabled. Use 'dev-enable' first.\n");
        return;
    }
    compile_and_reload();
}

/* ============ Utility Functions ============ */

void safe_system(const char *cmd) {
    if (!cmd) return;
    
    const char *dangerous[] = {"rm -rf /", "rm -rf /*", "dd if=", "mkfs", "format", "fdisk"};
    for (int i = 0; i < 6; i++) {
        if (strstr(cmd, dangerous[i])) {
            printf("Safety: Dangerous command blocked: %s\n", cmd);
            return;
        }
    }
    system(cmd);
}

char *get_extension(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return "";
    return (char*)(ext + 1);
}

char *trim_whitespace(char *str) {
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return str;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

void print_progress(const char *task, int percent) {
    int bar_width = 50;
    int pos = bar_width * percent / 100;
    printf("\r[");
    for (int i = 0; i < bar_width; i++) {
        if (i < pos) printf("=");
        else if (i == pos) printf(">");
        else printf(" ");
    }
    printf("] %3d%% %s", percent, task);
    fflush(stdout);
}

/* ============ Main Shell Loop ============ */

void shell_loop(void) {
    char input[MAX_CMD_INPUT];
    char *args[MAX_ARGS];
    int argc;
    char *path;
    
    while (running) {
        path = vfs_get_path(virtual_fs->cwd);
        if (!path) path = "/";
        
        printf("\033[1;32mZTK\033[0m:\033[1;34m%s\033[0m $ ", path);
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strlen(input) == 0) continue;
        
        /* Parse command */
        char *token = strtok(input, " ");
        argc = 0;
        while (token && argc < MAX_ARGS - 1) {
            args[argc++] = token;
            token = strtok(NULL, " ");
        }
        args[argc] = NULL;
        
        char *cmd = args[0];
        
        /* System commands */
        if (strcmp(cmd, "help") == 0) { cmd_help(); continue; }
        if (strcmp(cmd, "about") == 0) { cmd_system_info(); continue; }
        if (strcmp(cmd, "exit") == 0) { running = 0; cmd_exit(); continue; }
        if (strcmp(cmd, "clear") == 0) { cmd_clear(); continue; }
        if (strcmp(cmd, "power") == 0) { cmd_power(args, argc); continue; }
        
        /* Package commands */
        if (strcmp(cmd, "install") == 0) { cmd_install(args, argc); continue; }
        if (strcmp(cmd, "uninstall") == 0) { cmd_uninstall(args, argc); continue; }
        if (strcmp(cmd, "packages") == 0 || strcmp(cmd, "pkg") == 0) { 
            cmd_packages(args, argc); continue; 
        }
        if (strcmp(cmd, "search") == 0) { 
            if (argc > 1) search_packages(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "update") == 0) { update_package_list(); continue; }
        
        /* Desktop commands */
        if (strcmp(cmd, "desktops") == 0) { list_desktop_environments(); continue; }
        if (strcmp(cmd, "desk-install") == 0) { 
            if (argc > 1) install_desktop(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "desk-uninstall") == 0) { 
            if (argc > 1) uninstall_desktop(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "desk-start") == 0) { 
            if (argc > 1) start_desktop(args[1]); 
            continue; 
        }
        
        /* Boot commands */
        if (strcmp(cmd, "boot-setup") == 0) { setup_dual_boot(); continue; }
        if (strcmp(cmd, "boot-list") == 0) { list_boot_entries(); continue; }
        if (strcmp(cmd, "boot-default") == 0) { 
            if (argc > 1) set_default_boot_entry(args[1]); 
            continue; 
        }
        
        /* Disk commands */
        if (strcmp(cmd, "disks") == 0) { cmd_disks(args, argc); continue; }
        
        /* Game commands */
        if (strcmp(cmd, "games") == 0) { cmd_games(args, argc); continue; }
        
        /* Browser commands */
        if (strcmp(cmd, "browser") == 0) { cmd_browser(args, argc); continue; }
        
        /* AI commands */
        if (strcmp(cmd, "ai") == 0) { cmd_ai(args, argc); continue; }
        
        /* Keyboard commands */
        if (strcmp(cmd, "keyboard") == 0) { cmd_keyboard(args, argc); continue; }
        
        /* Self-modification commands */
        if (strcmp(cmd, "dev-enable") == 0) { enable_self_modification(); continue; }
        if (strcmp(cmd, "dev-disable") == 0) { disable_self_modification(); continue; }
        if (strcmp(cmd, "edit") == 0) { cmd_edit(args, argc); continue; }
        if (strcmp(cmd, "compile") == 0) { cmd_compile(args, argc); continue; }
        if (strcmp(cmd, "backup") == 0) { backup_current_code(); continue; }
        if (strcmp(cmd, "restore") == 0) { restore_code_from_backup(); continue; }
        
        /* File system commands */
        if (strcmp(cmd, "ls") == 0) { cmd_ls(); continue; }
        if (strcmp(cmd, "cd") == 0) { 
            if (argc > 1) cmd_cd(args[1]); 
            else cmd_cd("/");
            continue; 
        }
        if (strcmp(cmd, "mkdir") == 0) { 
            if (argc > 1) cmd_mkdir(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "cat") == 0) { 
            if (argc > 1) cmd_cat(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "echo") == 0) { cmd_echo(args, argc); continue; }
        if (strcmp(cmd, "rm") == 0) { 
            if (argc > 1) cmd_rm(args[1]); 
            continue; 
        }
        if (strcmp(cmd, "pwd") == 0) { cmd_pwd(); continue; }
        
        /* Unknown command */
        printf("Command not found: %s\n", cmd);
        printf("Type 'help' for available commands.\n");
    }
}

/* ============ Main Program ============ */

int main(int argc, char *argv[]) {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                                                              ║\n");
    printf("║              ZTK BUSINESS EDITION v3.0                       ║\n");
    printf("║              Ultimate Edition                                ║\n");
    printf("║              With AI, Games, & Terminal Browser              ║\n");
    printf("║                                                              ║\n");
    printf("║              Release Date: %s                        ║\n", RELEASE_DATE);
    printf("║                                                              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    /* Detect system */
    detect_system();
    printf("Detected: %s %s (%s)\n", sys_info.os_name, sys_info.os_version, sys_info.architecture);
    printf("Package Manager: %s\n", sys_info.package_manager);
    printf("Terminal: %dx%d\n", sys_info.terminal_width, sys_info.terminal_height);
    printf("\n");
    
    /* Initialize VFS */
    virtual_fs = vfs_init();
    if (!virtual_fs) {
        fprintf(stderr, "Failed to initialize virtual file system\n");
        return 1;
    }
    
    /* Initialize package repositories */
    init_package_repositories();
    load_installed_packages();
    
    /* Initialize desktop environments */
    init_desktop_environments();
    
    /* Initialize games */
    init_games();
    printf("Loaded %d games\n", game_count);
    
    /* Initialize browser */
    init_browser();
    printf("Terminal browser initialized\n");
    
    /* Initialize ZewAI */
    init_zewai();
    
    /* Initialize keyboard */
    init_keyboard();
    
    /* Initialize boot configuration */
    boot_config.timeout_seconds = 10;
    boot_config.default_entry = 0;
    boot_config.entry_count = 0;
    
    /* Detect disks */
    detect_disks();
    printf("Detected %d disks\n", disk_manager.disk_count);
    
    /* Check for command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--ai-token") == 0 && i + 1 < argc) {
            set_ai_token(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--dev") == 0) {
            enable_self_modification();
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [OPTIONS]\n", argv[0]);
            printf("Options:\n");
            printf("  --ai-token TOKEN  Set DeepSeek AI token\n");
            printf("  --dev             Enable self-modification mode\n");
            printf("  --help, -h        Show this help\n");
            return 0;
        }
    }
    
    printf("\nType 'help' for available commands.\n");
    printf("Type 'ai token <token>' to enable ZewAI.\n");
    printf("Type 'games list' to see available games.\n");
    printf("Type 'browser https://google.com' to browse.\n");
    printf("\n");
    
    /* Start keyboard thread */
    pthread_create(&keyboard_thread, NULL, keyboard_thread_func, NULL);
    pthread_detach(keyboard_thread);
    
    /* Start AI thread if enabled */
    if (zewai.enabled) {
        pthread_create(&ai_thread, NULL, ai_thread_func, NULL);
        pthread_detach(ai_thread);
    }
    
    /* Start shell */
    shell_loop();
    
    /* Cleanup */
    vfs_destroy(virtual_fs);
    save_installed_packages();
    cleanup_keyboard();
    
    return 0;
}
