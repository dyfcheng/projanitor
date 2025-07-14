/**
 * @file projanitor.c
 * @brief A C language version of the projanitor project integrity tool.
 *
 * This tool audits software development projects by identifying the project root,
 * cataloging relevant files, and generating a report on duplicate, orphaned,
 * and missing source files. It is designed to be a lightweight, single-file
 * utility with no external dependencies beyond the standard C library.
 *
 * @version 1.0.0 
 * @date 2025-07-14
 *
 * To Compile:
 * gcc -std=c99 -Wall -o projanitor projanitor3.c
 *
 * To Run (from your project directory):
 * ./projanitor [--verbose]
 */
#define _XOPEN_SOURCE 700 // Enable POSIX extensions for strdup, strtok_r, lstat
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <libgen.h>
#include <ctype.h>
#include <errno.h>

#define MAX_PATH_LEN 4096
#define MAX_LINE_LEN 2048
#define INITIAL_ARRAY_CAPACITY 64
#define HASH_MAP_SIZE 1024
#define MAX_SEARCH_DEPTH 3
#define MAX_SRC_BUFFER (MAX_LINE_LEN * 100) // Further increased buffer for safety

// --- Data Structures ---

typedef struct {
    char **items;
    int count;
    int capacity;
} StringArray;

typedef struct Node {
    char *key;
    StringArray *values;
    struct Node *next;
} Node;

typedef struct {
    Node **buckets;
    int size;
} HashMap;

// --- Forward Declarations ---
void init_string_array(StringArray *arr);
void add_to_string_array(StringArray *arr, const char *item);
bool string_array_contains(const StringArray *arr, const char *item);
void free_string_array(StringArray *arr);
int compare_paths(const void *a, const void *b);

HashMap* create_hash_map(int size);
void add_to_hash_map(HashMap *map, const char *key, const char *value);
StringArray* get_from_hash_map(const HashMap *map, const char *key);
void free_hash_map(HashMap *map);

void parse_arguments(int argc, char *argv[], StringArray *extensions, StringArray *exclude_dirs, StringArray *marker_files, bool *verbose);
bool find_project_root(char *root_path, const StringArray *marker_files, bool verbose);
char* get_project_name(const char *root_path);
void collect_build_files(const char *build_path, StringArray *build_files, bool verbose);
void analyze_project_files(const char *base_path, const StringArray *extensions, const StringArray *exclude_dirs, const StringArray *build_files, bool verbose, StringArray *all_files, HashMap *referenced_files, HashMap *found_files_map);
void parse_file_for_references(const char *file_path, HashMap *referenced_files, bool verbose);
void generate_report(const char *root_path, const char *project_name, const StringArray *all_files, const HashMap *referenced_files, const HashMap *found_files_map);

// --- Utility: Safe string 'ends_with' check ---
bool ends_with(const char *str, const char *suffix) {
    if (!str || !suffix) return false;
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) return false;
    return strcmp(str + len_str - len_suffix, suffix) == 0;
}

// --- Utility: Path comparison for sorting ---
int compare_paths(const void *a, const void *b) {
    const char *path1 = *(const char **)a;
    const char *path2 = *(const char **)b;
    if (!path1 || !path2) {
        fprintf(stderr, "Warning: Null path in compare_paths\n");
        return path1 ? 1 : (path2 ? -1 : 0);
    }
    char *path1_copy = strdup(path1);
    char *path2_copy = strdup(path2);
    if (!path1_copy || !path2_copy) {
        fprintf(stderr, "Warning: Memory allocation failed in compare_paths\n");
        free(path1_copy);
        free(path2_copy);
        return strcmp(path1, path2);
    }
    char *dir1 = strdup(path1);
    char *dir2 = strdup(path2);
    if (!dir1 || !dir2) {
        fprintf(stderr, "Warning: Memory allocation failed for dir in compare_paths\n");
        free(path1_copy);
        free(path2_copy);
        free(dir1);
        free(dir2);
        return strcmp(path1, path2);
    }
    char *dir1_copy = dirname(dir1);
    char *dir2_copy = dirname(dir2);
    char *base1 = basename(path1_copy);
    char *base2 = basename(path2_copy);
    if (!dir1_copy || !dir2_copy || !base1 || !base2) {
        fprintf(stderr, "Warning: basename/dirname failed in compare_paths\n");
        free(path1_copy);
        free(path2_copy);
        free(dir1);
        free(dir2);
        return strcmp(path1, path2);
    }
    int dir_cmp = strcmp(dir1_copy, dir2_copy);
    int result = dir_cmp ? dir_cmp : strcmp(base1, base2);
    free(path1_copy);
    free(path2_copy);
    free(dir1);
    free(dir2);
    return result;
}

// --- Main Execution ---

int main(int argc, char *argv[]) {
    // Default configurations
    StringArray extensions, exclude_dirs, marker_files;
    init_string_array(&extensions);
    init_string_array(&exclude_dirs);
    init_string_array(&marker_files);

    // Updated extensions per spec
    add_to_string_array(&extensions, ".c");
    add_to_string_array(&extensions, ".h");
    add_to_string_array(&extensions, ".json");
    add_to_string_array(&extensions, ".py");
    add_to_string_array(&extensions, ".cmake");
    add_to_string_array(&extensions, ".md");
    add_to_string_array(&extensions, ".sh");
    add_to_string_array(&extensions, "CMakeLists.txt");

    // Updated exclude directories per spec
    add_to_string_array(&exclude_dirs, ".git");
    add_to_string_array(&exclude_dirs, "build");
    add_to_string_array(&exclude_dirs, "build_logs");
    add_to_string_array(&exclude_dirs, "doc");

    // Marker files per spec
    add_to_string_array(&marker_files, "LICENSE");
    add_to_string_array(&marker_files, "sdkconfig");
    add_to_string_array(&marker_files, "dependencies.lock");
    add_to_string_array(&marker_files, "CMakeLists.txt");

    bool verbose = false;
    parse_arguments(argc, argv, &extensions, &exclude_dirs, &marker_files, &verbose);

    char root_path[MAX_PATH_LEN];
    if (!find_project_root(root_path, &marker_files, verbose)) {
        fprintf(stderr, "⚠️ Warning: Project root could not be found. Using current directory as fallback.\n");
        if (getcwd(root_path, sizeof(root_path)) == NULL) {
            fprintf(stderr, "❌ Error: Cannot get current directory: %s\n", strerror(errno));
            free_string_array(&extensions);
            free_string_array(&exclude_dirs);
            free_string_array(&marker_files);
            return 1;
        }
    }
    printf("✅ Project root set to: %s\n", root_path);

    // Change to root directory
    if (chdir(root_path) != 0) {
        fprintf(stderr, "❌ Error: Cannot change to project root %s: %s\n", root_path, strerror(errno));
        free_string_array(&extensions);
        free_string_array(&exclude_dirs);
        free_string_array(&marker_files);
        return 1;
    }

    // Get project name
    char *project_name = get_project_name(root_path);
    if (!project_name) {
        project_name = strdup("Unknown");
        if (!project_name) {
            fprintf(stderr, "❌ Error: Memory allocation failed for project name\n");
            free_string_array(&extensions);
            free_string_array(&exclude_dirs);
            free_string_array(&marker_files);
            return 1;
        }
    }

    // Collect system-generated files from build directory
    StringArray build_files;
    init_string_array(&build_files);
    char build_path[MAX_PATH_LEN];
    if (snprintf(build_path, sizeof(build_path), "%s/build", root_path) >= sizeof(build_path)) {
        fprintf(stderr, "Warning: Build path too long: %s/build\n", root_path);
    } else {
        if (verbose) fprintf(stderr, "Info: Collecting build files from %s\n", build_path);
        collect_build_files(build_path, &build_files, verbose);
    }

    StringArray all_files;
    init_string_array(&all_files);
    HashMap *referenced_files = create_hash_map(HASH_MAP_SIZE);
    HashMap *found_files_map = create_hash_map(HASH_MAP_SIZE);

    printf("🔍 Analyzing project files...\n");
    analyze_project_files(root_path, &extensions, &exclude_dirs, &build_files, verbose, &all_files, referenced_files, found_files_map);
    generate_report(root_path, project_name, &all_files, referenced_files, found_files_map);

    // Cleanup
    free(project_name);
    free_string_array(&extensions);
    free_string_array(&exclude_dirs);
    free_string_array(&marker_files);
    free_string_array(&all_files);
    free_string_array(&build_files);
    free_hash_map(referenced_files);
    free_hash_map(found_files_map);

    return 0;
}

// --- Argument Parsing ---
void parse_arguments(int argc, char *argv[], StringArray *extensions, StringArray *exclude_dirs, StringArray *marker_files, bool *verbose) {
    int opt;
    struct option long_options[] = {
        {"extensions", required_argument, 0, 'e'},
        {"exclude-dirs", required_argument, 0, 'd'},
        {"marker-files", required_argument, 0, 'm'},
        {"verbose", no_argument, 0, 'v'},
        {0, 0, 0, 0}
    };

    bool extensions_set = false, exclude_set = false, markers_set = false;
    while ((opt = getopt_long(argc, argv, "e:d:m:v", long_options, NULL)) != -1) {
        char *token;
        char *saveptr;
        char *input_str_cpy = NULL;

        switch (opt) {
            case 'e':
                if (!extensions_set) {
                    free_string_array(extensions);
                    init_string_array(extensions);
                    extensions_set = true;
                }
                input_str_cpy = strdup(optarg);
                if (!input_str_cpy) { perror("strdup"); exit(EXIT_FAILURE); }
                for (token = strtok_r(input_str_cpy, ",", &saveptr); token; token = strtok_r(NULL, ",", &saveptr)) {
                    add_to_string_array(extensions, token);
                }
                free(input_str_cpy);
                break;
            case 'd':
                if (!exclude_set) {
                    free_string_array(exclude_dirs);
                    init_string_array(exclude_dirs);
                    exclude_set = true;
                }
                input_str_cpy = strdup(optarg);
                if (!input_str_cpy) { perror("strdup"); exit(EXIT_FAILURE); }
                for (token = strtok_r(input_str_cpy, ",", &saveptr); token; token = strtok_r(NULL, ",", &saveptr)) {
                    add_to_string_array(exclude_dirs, token);
                }
                free(input_str_cpy);
                break;
            case 'm':
                if (!markers_set) {
                    free_string_array(marker_files);
                    init_string_array(marker_files);
                    markers_set = true;
                }
                input_str_cpy = strdup(optarg);
                if (!input_str_cpy) { perror("strdup"); exit(EXIT_FAILURE); }
                for (token = strtok_r(input_str_cpy, ",", &saveptr); token; token = strtok_r(NULL, ",", &saveptr)) {
                    add_to_string_array(marker_files, token);
                }
                free(input_str_cpy);
                break;
            case 'v':
                *verbose = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [--extensions=c,h,json,py,cmake,md,sh,CMakeLists.txt] [--exclude-dirs=.git,build,build_logs,doc] [--marker-files=LICENSE,sdkconfig,dependencies.lock,CMakeLists.txt] [--verbose]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

// --- Core Logic ---

bool has_valid_extension(const char *filename, const StringArray *extensions) {
    if (!filename || !extensions) {
        fprintf(stderr, "Warning: Null filename or extensions in has_valid_extension\n");
        return false;
    }
    for (int i = 0; i < extensions->count; i++) {
        const char *ext = extensions->items[i];
        if (!ext) continue;
        if (strcmp(ext, filename) == 0) return true;
        if (ext[0] == '.' && ends_with(filename, ext)) return true;
    }
    return false;
}

bool is_system_file(const char *filename, const StringArray *build_files) {
    if (!filename || !build_files) {
        fprintf(stderr, "Warning: Null filename or build_files in is_system_file\n");
        return false;
    }
    char *filename_copy = strdup(filename);
    if (!filename_copy) {
        fprintf(stderr, "Warning: Memory allocation failed in is_system_file\n");
        return false;
    }
    char *base = basename(filename_copy);
    bool is_system = string_array_contains(build_files, base);
    free(filename_copy);
    return is_system;
}

bool check_marker_files(const char *path, const StringArray *marker_files, bool verbose) {
    if (!path || !marker_files) {
        if (verbose) fprintf(stderr, "Warning: Null path or marker_files in check_marker_files\n");
        return false;
    }
    struct stat st;
    int found_count = 0;
    StringArray missing_files;
    init_string_array(&missing_files);

    for (int i = 0; i < marker_files->count; i++) {
        if (!marker_files->items[i]) continue;
        char marker_path[MAX_PATH_LEN];
        if (strlen(path) + strlen(marker_files->items[i]) + 1 >= sizeof(marker_path)) {
            if (verbose) fprintf(stderr, "Warning: Path too long for marker: %s/%s\n", path, marker_files->items[i]);
            continue;
        }
        snprintf(marker_path, sizeof(marker_path), "%s/%s", path, marker_files->items[i]);
        if (stat(marker_path, &st) == 0 && S_ISREG(st.st_mode)) {
            found_count++;
            if (verbose) fprintf(stderr, "Info: Found marker %s in %s\n", marker_files->items[i], path);
        } else {
            add_to_string_array(&missing_files, marker_files->items[i]);
        }
    }

    if (verbose && found_count < marker_files->count) {
        fprintf(stderr, "Info: Directory %s missing markers: ", path);
        for (int i = 0; i < missing_files.count; i++) {
            fprintf(stderr, "%s%s", missing_files.items[i], i < missing_files.count - 1 ? ", " : "\n");
        }
    }

    free_string_array(&missing_files);
    return found_count == marker_files->count;
}

bool find_project_root(char *root_path, const StringArray *marker_files, bool verbose) {
    if (!root_path || !marker_files) {
        if (verbose) fprintf(stderr, "Warning: Null root_path or marker_files in find_project_root\n");
        return false;
    }
    char current_path[MAX_PATH_LEN];
    if (getcwd(current_path, sizeof(current_path)) == NULL) {
        if (verbose) fprintf(stderr, "Error: getcwd() failed: %s\n", strerror(errno));
        return false;
    }

    // Search order: current, up 1-3, down 1-3
    StringArray search_paths;
    init_string_array(&search_paths);
    add_to_string_array(&search_paths, current_path);
    if (verbose) fprintf(stderr, "Info: Checking directory %s\n", current_path);

    // Up 1-3 levels
    char temp_path[MAX_PATH_LEN];
    strcpy(temp_path, current_path);
    for (int i = 1; i <= MAX_SEARCH_DEPTH; i++) {
        char *last_slash = strrchr(temp_path, '/');
        if (last_slash == temp_path) {
            temp_path[1] = '\0';
        } else if (last_slash != NULL) {
            *last_slash = '\0';
            add_to_string_array(&search_paths, temp_path);
            if (verbose) fprintf(stderr, "Info: Checking directory %s\n", temp_path);
        } else {
            break;
        }
    }

    // Down 1-3 levels
    StringArray down_paths;
    init_string_array(&down_paths);
    add_to_string_array(&down_paths, current_path);
    for (int depth = 1; depth <= MAX_SEARCH_DEPTH; depth++) {
        for (int i = 0; i < down_paths.count; i++) {
            if (!down_paths.items[i]) continue;
            DIR *dir = opendir(down_paths.items[i]);
            if (!dir) {
                if (verbose) fprintf(stderr, "Warning: Cannot open directory %s: %s\n", down_paths.items[i], strerror(errno));
                continue;
            }
            struct dirent *entry;
            while ((entry = readdir(dir)) != NULL) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                char full_path[MAX_PATH_LEN];
                if (strlen(down_paths.items[i]) + strlen(entry->d_name) + 1 >= sizeof(full_path)) {
                    if (verbose) fprintf(stderr, "Warning: Path too long: %s/%s\n", down_paths.items[i], entry->d_name);
                    continue;
                }
                snprintf(full_path, sizeof(full_path), "%s/%s", down_paths.items[i], entry->d_name);
                struct stat st;
                if (stat(full_path, &st) != 0) {
                    if (verbose) fprintf(stderr, "Warning: Cannot stat %s: %s\n", full_path, strerror(errno));
                    continue;
                }
                if (S_ISDIR(st.st_mode)) {
                    add_to_string_array(&search_paths, full_path);
                    if (verbose) fprintf(stderr, "Info: Checking directory %s\n", full_path);
                    if (depth < MAX_SEARCH_DEPTH) {
                        add_to_string_array(&down_paths, full_path);
                    }
                }
            }
            closedir(dir);
        }
    }
    free_string_array(&down_paths);

    // Check each path for all marker files
    for (int i = 0; i < search_paths.count; i++) {
        if (!search_paths.items[i]) continue;
        if (check_marker_files(search_paths.items[i], marker_files, verbose)) {
            strcpy(root_path, search_paths.items[i]);
            free_string_array(&search_paths);
            return true;
        }
    }

    free_string_array(&search_paths);
    return false;
}

char* get_project_name(const char *root_path) {
    if (!root_path) {
        fprintf(stderr, "Warning: Null root_path in get_project_name\n");
        return NULL;
    }
    char cmake_path[MAX_PATH_LEN];
    if (snprintf(cmake_path, sizeof(cmake_path), "%s/CMakeLists.txt", root_path) >= sizeof(cmake_path)) {
        fprintf(stderr, "Warning: Path too long for CMakeLists.txt: %s\n", root_path);
        return NULL;
    }
    FILE *file = fopen(cmake_path, "r");
    if (!file) {
        fprintf(stderr, "Warning: Could not open %s: %s\n", cmake_path, strerror(errno));
        return NULL;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), file)) {
        char *project_pos = strstr(line, "project(");
        if (project_pos) {
            char *start = project_pos + strlen("project(");
            char *end = strchr(start, ')');
            if (end) {
                size_t len = end - start;
                char *name = (char *)malloc(len + 1);
                if (!name) {
                    fclose(file);
                    fprintf(stderr, "Warning: Memory allocation failed in get_project_name\n");
                    return NULL;
                }
                strncpy(name, start, len);
                name[len] = '\0';
                fclose(file);
                return name;
            }
        }
    }
    fclose(file);
    return NULL;
}

void collect_build_files(const char *build_path, StringArray *build_files, bool verbose) {
    if (!build_path || !build_files) {
        if (verbose) fprintf(stderr, "Warning: Null build_path or build_files in collect_build_files\n");
        return;
    }
    DIR *dir = opendir(build_path);
    if (!dir) {
        if (verbose) fprintf(stderr, "Warning: Cannot open build directory %s: %s\n", build_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        char full_path[MAX_PATH_LEN];
        if (strlen(build_path) + strlen(entry->d_name) + 1 >= sizeof(full_path)) {
            if (verbose) fprintf(stderr, "Warning: Path too long: %s/%s\n", build_path, entry->d_name);
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", build_path, entry->d_name);
        struct stat st;
        if (stat(full_path, &st) != 0) {
            if (verbose) fprintf(stderr, "Warning: Cannot stat %s: %s\n", full_path, strerror(errno));
            continue;
        }
        if (S_ISREG(st.st_mode)) {
            add_to_string_array(build_files, entry->d_name);
            if (verbose) fprintf(stderr, "Info: Added build file %s\n", entry->d_name);
        } else if (S_ISDIR(st.st_mode)) {
            collect_build_files(full_path, build_files, verbose);
        }
    }
    closedir(dir);
}

void parse_file_for_references(const char *file_path, HashMap *referenced_files, bool verbose) {
    if (!file_path || !referenced_files) {
        if (verbose) fprintf(stderr, "Warning: Null file_path or referenced_files in parse_file_for_references\n");
        return;
    }
    if (verbose) fprintf(stderr, "Info: Parsing file %s\n", file_path);
    FILE *file = fopen(file_path, "r");
    if (!file) {
        if (verbose) fprintf(stderr, "Warning: Could not open file %s: %s\n", file_path, strerror(errno));
        return;
    }

    char line[MAX_LINE_LEN];
    bool in_src_block = false;
    char src_buffer[MAX_SRC_BUFFER] = {0};
    size_t src_len = 0;

    while (fgets(line, sizeof(line), file)) {
        // Strip leading/trailing whitespace
        char *trimmed = line;
        while (isspace(*trimmed)) trimmed++;
        char *end = trimmed + strlen(trimmed) - 1;
        while (end > trimmed && isspace(*end)) *end-- = '\0';

        // Handle C/C++ includes
        if (!in_src_block && strstr(trimmed, "#include \"")) {
            char *start = strstr(trimmed, "#include \"") + strlen("#include \"");
            char *end_quote = strchr(start, '"');
            if (end_quote) {
                size_t len = end_quote - start;
                if (len > 0 && len < MAX_PATH_LEN && !strchr(start, '<')) { // Exclude <*.h>
                    char *ref_name = (char *)malloc(len + 1);
                    if (!ref_name) {
                        if (verbose) fprintf(stderr, "Error: Memory allocation failed for ref_name in %s\n", file_path);
                        fclose(file);
                        return;
                    }
                    strncpy(ref_name, start, len);
                    ref_name[len] = '\0';
                    add_to_hash_map(referenced_files, ref_name, file_path);
                    if (verbose) fprintf(stderr, "Info: Found include reference %s in %s\n", ref_name, file_path);
                    free(ref_name);
                }
            }
        }
        // Handle CMake SRC lists
        else if (ends_with(file_path, "CMakeLists.txt") || ends_with(file_path, ".cmake")) {
            if (strstr(trimmed, "set(SRC") || strstr(trimmed, "target_sources(")) {
                in_src_block = true;
                src_len = 0;
                if (strlen(trimmed) < sizeof(src_buffer)) {
                    strncpy(src_buffer, trimmed, sizeof(src_buffer) - 1);
                    src_len = strlen(src_buffer);
                    if (verbose) fprintf(stderr, "Info: Started SRC block in %s\n", file_path);
                } else {
                    if (verbose) fprintf(stderr, "Warning: Line too long in %s\n", file_path);
                }
            } else if (in_src_block) {
                if (src_len + strlen(trimmed) + 1 < sizeof(src_buffer)) {
                    strcat(src_buffer, " ");
                    strcat(src_buffer, trimmed);
                    src_len += strlen(trimmed) + 1;
                } else {
                    if (verbose) fprintf(stderr, "Warning: SRC buffer overflow in %s\n", file_path);
                    in_src_block = false;
                }
                if (strchr(trimmed, ')')) {
                    in_src_block = false;
                    // Parse SRC buffer
                    char *buffer_copy = strdup(src_buffer);
                    if (!buffer_copy) {
                        if (verbose) fprintf(stderr, "Error: Memory allocation failed for buffer_copy in %s\n", file_path);
                        fclose(file);
                        return;
                    }
                    char *token = strtok(buffer_copy, " \t\n");
                    while (token) {
                        if (token[0] != '(' && token[0] != ')' && strcmp(token, "set") != 0 && strcmp(token, "SRC") != 0 && strcmp(token, "target_sources") != 0) {
                            char *ref_name = strdup(token);
                            if (!ref_name) {
                                if (verbose) fprintf(stderr, "Error: Memory allocation failed for ref_name in %s\n", file_path);
                                free(buffer_copy);
                                fclose(file);
                                return;
                            }
                            add_to_hash_map(referenced_files, ref_name, file_path);
                            if (verbose) fprintf(stderr, "Info: Found SRC reference %s in %s\n", ref_name, file_path);
                            free(ref_name);
                        }
                        token = strtok(NULL, " \t\n");
                    }
                    free(buffer_copy);
                    if (verbose) fprintf(stderr, "Info: Ended SRC block in %s\n", file_path);
                }
            }
        }
    }
    fclose(file);
}

void analyze_project_files(const char *base_path, const StringArray *extensions, const StringArray *exclude_dirs, const StringArray *build_files, bool verbose, StringArray *all_files, HashMap *referenced_files, HashMap *found_files_map) {
    if (!base_path || !extensions || !exclude_dirs || !build_files || !all_files || !referenced_files || !found_files_map) {
        if (verbose) fprintf(stderr, "Warning: Invalid arguments to analyze_project_files\n");
        return;
    }
    if (verbose) fprintf(stderr, "Info: Analyzing directory %s\n", base_path);
    DIR *dir = opendir(base_path);
    if (!dir) {
        if (verbose) fprintf(stderr, "Warning: Cannot open directory %s: %s\n", base_path, strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        if (!entry->d_name) {
            if (verbose) fprintf(stderr, "Warning: Null directory entry in %s\n", base_path);
            continue;
        }
        char full_path[MAX_PATH_LEN];
        if (strlen(base_path) + strlen(entry->d_name) + 1 >= sizeof(full_path)) {
            if (verbose) fprintf(stderr, "Warning: Path too long: %s/%s\n", base_path, entry->d_name);
            continue;
        }
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        if (lstat(full_path, &st) != 0) {
            if (verbose) fprintf(stderr, "Warning: Cannot stat %s: %s\n", full_path, strerror(errno));
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (string_array_contains(exclude_dirs, entry->d_name) && strcmp(entry->d_name, "build") != 0) {
                if (verbose) fprintf(stderr, "Info: Skipping excluded directory: %s\n", full_path);
                continue;
            }
            analyze_project_files(full_path, extensions, exclude_dirs, build_files, verbose, all_files, referenced_files, found_files_map);
        } else if (S_ISREG(st.st_mode)) {
            if (has_valid_extension(entry->d_name, extensions) && !is_system_file(entry->d_name, build_files)) {
                add_to_string_array(all_files, full_path);
                add_to_hash_map(found_files_map, entry->d_name, full_path);
                if (verbose) fprintf(stderr, "Info: Processing file %s\n", full_path);
                parse_file_for_references(full_path, referenced_files, verbose);
            }
        } else if (S_ISLNK(st.st_mode) && verbose) {
            fprintf(stderr, "Warning: Skipping symlink: %s\n", full_path);
        }
    }
    closedir(dir);
}

void generate_report(const char *root_path, const char *project_name, const StringArray *all_files, const HashMap *referenced_files, const HashMap *found_files_map) {
    if (!root_path || !project_name || !all_files || !referenced_files || !found_files_map) {
        fprintf(stderr, "Error: Invalid arguments to generate_report\n");
        return;
    }

    // Sort all_files for list_exist
    qsort(all_files->items, all_files->count, sizeof(char *), compare_paths);

    // Collect subfolders (excluding no-go areas)
    StringArray subfolders;
    init_string_array(&subfolders);
    StringArray exclude_dirs;
    init_string_array(&exclude_dirs);
    add_to_string_array(&exclude_dirs, ".git");
    add_to_string_array(&exclude_dirs, "build");
    add_to_string_array(&exclude_dirs, "build_logs");
    add_to_string_array(&exclude_dirs, "doc");

    DIR *dir = opendir(root_path);
    if (dir) {
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            char full_path[MAX_PATH_LEN];
            if (strlen(root_path) + strlen(entry->d_name) + 1 >= sizeof(full_path)) continue;
            snprintf(full_path, sizeof(full_path), "%s/%s", root_path, entry->d_name);
            struct stat st;
            if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode) && !string_array_contains(&exclude_dirs, entry->d_name)) {
                add_to_string_array(&subfolders, entry->d_name);
            }
        }
        closedir(dir);
    }
    qsort(subfolders.items, subfolders.count, sizeof(char *), compare_paths);

    // Statistics
    int counts[8] = {0}; // .c, .h, CMakeLists.txt, .cmake, .sh, .json, .py, .md
    for (int i = 0; i < all_files->count; i++) {
        if (!all_files->items[i]) continue;
        const char *file = all_files->items[i];
        char *file_copy = strdup(file);
        if (!file_copy) continue;
        char *basename_result = basename(file_copy);
        if (ends_with(file, ".c")) counts[0]++;
        else if (ends_with(file, ".h")) counts[1]++;
        else if (strcmp(basename_result, "CMakeLists.txt") == 0) counts[2]++;
        else if (ends_with(file, ".cmake")) counts[3]++;
        else if (ends_with(file, ".sh")) counts[4]++;
        else if (ends_with(file, ".json")) counts[5]++;
        else if (ends_with(file, ".py")) counts[6]++;
        else if (ends_with(file, ".md")) counts[7]++;
        free(file_copy);
    }

    // --- Summary ---
    printf("\n=== Summary ===\n");
    printf("Project name: %s\n", project_name);
    printf("Project root folder: %s\n", root_path);
    printf("Key subfolders:\n");
    if (subfolders.count == 0) {
        printf("  (None)\n");
    } else {
        for (int i = 0; i < subfolders.count; i++) {
            printf("  - %s\n", subfolders.items[i]);
        }
    }
    printf("File structure:\n");
    if (all_files->count == 0) {
        printf("  (None)\n");
    } else {
        for (int i = 0; i < all_files->count; i++) {
            printf("  - %s\n", all_files->items[i]);
        }
    }

    // --- Statistics ---
    printf("\n=== Statistics ===\n");
    printf("Total # of files of interest: %d\n", all_files->count);
    printf("# of .c: %d\n", counts[0]);
    printf("# of .h: %d\n", counts[1]);
    printf("# of CMakeLists.txt: %d\n", counts[2]);
    printf("# of .cmake files: %d\n", counts[3]);
    printf("# of .sh: %d\n", counts[4]);
    printf("# of .json: %d\n", counts[5]);
    printf("# of .py: %d\n", counts[6]);
    printf("# of .md: %d\n", counts[7]);

    // --- Warnings: Duplicates ---
    printf("\n=== Warnings ===\n");
    const char *exts[] = {".c", ".h", ".py", ".sh"};
    for (int e = 0; e < 4; e++) {
        printf("%s files with identical names:\n", exts[e]);
        bool found = false;
        for (int i = 0; i < found_files_map->size; i++) {
            for (Node *node = found_files_map->buckets[i]; node; node = node->next) {
                if (!node->key || !node->values) continue;
                if (node->values->count > 1 && ends_with(node->key, exts[e])) {
                    found = true;
                    printf("  %s:\n", node->key);
                    StringArray sorted_paths;
                    init_string_array(&sorted_paths);
                    for (int j = 0; j < node->values->count; j++) {
                        if (node->values->items[j]) {
                            add_to_string_array(&sorted_paths, node->values->items[j]);
                        }
                    }
                    qsort(sorted_paths.items, sorted_paths.count, sizeof(char *), compare_paths);
                    for (int j = 0; j < sorted_paths.count; j++) {
                        printf("    %s\n", sorted_paths.items[j]);
                    }
                    free_string_array(&sorted_paths);
                }
            }
        }
        if (!found) printf("  (None)\n");
    }

    // --- Errors: Orphan and Missing Files ---
    StringArray orphan_files;
    init_string_array(&orphan_files);
    for (int i = 0; i < all_files->count; i++) {
        if (!all_files->items[i]) continue;
        char *dup_path = strdup(all_files->items[i]);
        if (!dup_path) continue;
        char *file_basename = basename(dup_path);
        if (!file_basename) {
            free(dup_path);
            continue;
        }
        if (!get_from_hash_map(referenced_files, file_basename)) {
            add_to_string_array(&orphan_files, all_files->items[i]);
        }
        free(dup_path);
    }
    qsort(orphan_files.items, orphan_files.count, sizeof(char *), compare_paths);

    StringArray missing_files;
    init_string_array(&missing_files);
    HashMap *missing_refs = create_hash_map(HASH_MAP_SIZE);
    for (int i = 0; i < referenced_files->size; i++) {
        for (Node *node = referenced_files->buckets[i]; node; node = node->next) {
            if (!node->key || !node->values) continue;
            if (!get_from_hash_map(found_files_map, node->key)) {
                add_to_string_array(&missing_files, node->key);
                for (int j = 0; j < node->values->count; j++) {
                    if (node->values->items[j]) {
                        add_to_hash_map(missing_refs, node->key, node->values->items[j]);
                    }
                }
            }
        }
    }
    qsort(missing_files.items, missing_files.count, sizeof(char *), compare_paths);

    printf("\n=== Errors ===\n");
    printf("Orphan files: %d\n", orphan_files.count);
    printf("Missing files: %d\n", missing_files.count);

    printf("\n=== Details of Orphan Files ===\n");
    if (orphan_files.count > 0) {
        for (int i = 0; i < orphan_files.count; i++) {
            if (orphan_files.items[i]) printf("- %s\n", orphan_files.items[i]);
        }
    } else {
        printf("(None)\n");
    }

    printf("\n=== Details of Missing Files ===\n");
    if (missing_files.count > 0) {
        for (int i = 0; i < missing_files.count; i++) {
            if (!missing_files.items[i]) continue;
            printf("- %s\n", missing_files.items[i]);
            printf("    referenced by:\n");
            StringArray *refs = get_from_hash_map(missing_refs, missing_files.items[i]);
            if (refs) {
                StringArray sorted_refs;
                init_string_array(&sorted_refs);
                for (int j = 0; j < refs->count; j++) {
                    if (refs->items[j]) {
                        add_to_string_array(&sorted_refs, refs->items[j]);
                    }
                }
                qsort(sorted_refs.items, sorted_refs.count, sizeof(char *), compare_paths);
                for (int j = 0; j < sorted_refs.count; j++) {
                    printf("      %s\n", sorted_refs.items[j]);
                }
                free_string_array(&sorted_refs);
            }
        }
    } else {
        printf("(None)\n");
    }

    free_string_array(&orphan_files);
    free_string_array(&missing_files);
    free_hash_map(missing_refs);
    free_string_array(&subfolders);
    free_string_array(&exclude_dirs);
}

// --- Data Structure Implementations ---

void init_string_array(StringArray *arr) {
    if (!arr) return;
    arr->items = (char **)malloc(INITIAL_ARRAY_CAPACITY * sizeof(char *));
    if (!arr->items) {
        perror("Failed to allocate memory for string array");
        exit(EXIT_FAILURE);
    }
    arr->count = 0;
    arr->capacity = INITIAL_ARRAY_CAPACITY;
}

void add_to_string_array(StringArray *arr, const char *item) {
    if (!arr || !item) {
        fprintf(stderr, "Warning: Null arr or item in add_to_string_array\n");
        return;
    }
    if (arr->count >= arr->capacity) {
        arr->capacity *= 2;
        char **new_items = (char **)realloc(arr->items, arr->capacity * sizeof(char *));
        if (!new_items) {
            perror("Failed to reallocate memory for string array");
            free(arr->items);
            exit(EXIT_FAILURE);
        }
        arr->items = new_items;
    }
    arr->items[arr->count] = strdup(item);
    if (!arr->items[arr->count]) {
        perror("Failed to duplicate string for array");
        exit(EXIT_FAILURE);
    }
    arr->count++;
}

bool string_array_contains(const StringArray *arr, const char *item) {
    if (!arr || !item) return false;
    for (int i = 0; i < arr->count; i++) {
        if (arr->items[i] && strcmp(arr->items[i], item) == 0) return true;
    }
    return false;
}

void free_string_array(StringArray *arr) {
    if (!arr) return;
    if (arr->items) {
        for (int i = 0; i < arr->count; i++) {
            free(arr->items[i]);
            arr->items[i] = NULL; // Prevent double-free
        }
        free(arr->items);
        arr->items = NULL;
    }
    arr->count = 0;
    arr->capacity = 0;
}

unsigned int hash(const char *key, int size) {
    if (!key) return 0;
    unsigned int hash_val = 5381;
    int c;
    while ((c = *key++)) {
        hash_val = ((hash_val << 5) + hash_val) + c;
    }
    return hash_val % size;
}

HashMap* create_hash_map(int size) {
    HashMap *map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) {
        perror("Failed to allocate memory for hash map");
        exit(EXIT_FAILURE);
    }
    map->size = size;
    map->buckets = (Node**)calloc(size, sizeof(Node*));
    if (!map->buckets) {
        perror("Failed to allocate memory for hash map buckets");
        free(map);
        exit(EXIT_FAILURE);
    }
    return map;
}

void add_to_hash_map(HashMap *map, const char *key, const char *value) {
    if (!map || !key || !value) {
        fprintf(stderr, "Warning: Null map, key, or value in add_to_hash_map\n");
        return;
    }
    unsigned int index = hash(key, map->size);
    Node *head = map->buckets[index];
    for (Node *curr = head; curr; curr = curr->next) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            add_to_string_array(curr->values, value);
            return;
        }
    }

    Node *newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        perror("Failed to allocate memory for hash map node");
        exit(EXIT_FAILURE);
    }
    newNode->key = strdup(key);
    if (!newNode->key) {
        perror("Failed to allocate memory for hash map key");
        free(newNode);
        exit(EXIT_FAILURE);
    }
    newNode->values = (StringArray*)malloc(sizeof(StringArray));
    if (!newNode->values) {
        perror("Failed to allocate memory for hash map values");
        free(newNode->key);
        free(newNode);
        exit(EXIT_FAILURE);
    }

    init_string_array(newNode->values);
    add_to_string_array(newNode->values, value);
    newNode->next = head;
    map->buckets[index] = newNode;
}

StringArray* get_from_hash_map(const HashMap *map, const char *key) {
    if (!map || !key) return NULL;
    unsigned int index = hash(key, map->size);
    for (Node *curr = map->buckets[index]; curr; curr = curr->next) {
        if (curr->key && strcmp(curr->key, key) == 0) {
            return curr->values;
        }
    }
    return NULL;
}

void free_hash_map(HashMap *map) {
    if (!map) return;
    for (int i = 0; i < map->size; i++) {
        Node *head = map->buckets[i];
        while (head) {
            Node *temp = head;
            head = head->next;
            free(temp->key);
            free_string_array(temp->values);
            free(temp->values);
            free(temp);
        }
    }
    free(map->buckets);
    free(map);
}
