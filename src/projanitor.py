# === File: projanitor.py ===
#  20250712 (dc)  -- commenced
#
#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
projanitor: A house-cleaning program that audits the integrity of a software development
project. It identifies the project root, catalogs all relevant files, and creates a report
detailing duplicate, orphaned, and missing source files.

Usage:
    python projanitor.py [options]
Options:
    --extensions STR    Comma-separated file extensions/names (default: c,h,json,py,cmake,md,sh,CMakeLists.txt)
    --exclude-dirs STR  Comma-separated directories to exclude (default: .git,build,build_logs,doc)
    --marker-files STR  Comma-separated files to identify project root (default: LICENSE,sdkconfig,dependencies.lock,CMakeLists.txt)
    --verbose           Enable detailed warning messages
Requires Python 3.6+.
"""
import argparse
import os
import re
import sys
from pathlib import Path
from collections import defaultdict, Counter
from typing import List, Set, Dict, Optional, Tuple


# --- Configurable Constants ---
DEFAULT_FILES_OF_INTEREST = [
    "*.c",
    "*.h",
    "*.json",
    "*.py",
    "*.cmake",
    "*.md",
    "*.sh",
    "CMakeLists.txt",
]
DEFAULT_FOLDERS_TO_EXCLUDE = {".git", "build", "build_logs", "doc"}
DEFAULT_ROOT_MARKER_FILES = [
    "LICENSE",
    "sdkconfig",
    "dependencies.lock",
    "CMakeLists.txt",
]
MAX_SEARCH_DEPTH = 3

# Derived sets for efficient matching
VALID_EXTENSIONS = {ext for pattern in DEFAULT_FILES_OF_INTEREST if pattern.startswith("*.") for ext in [pattern[1:]]}
VALID_FILENAMES = {pattern for pattern in DEFAULT_FILES_OF_INTEREST if not pattern.startswith("*")}

def parse_args() -> argparse.Namespace:
    """Parse command-line arguments for configuration."""
    parser = argparse.ArgumentParser(
        description="Project file audit tool",
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--extensions",
        default=",".join(
            [ext[2:] if ext.startswith("*.") else ext for ext in DEFAULT_FILES_OF_INTEREST]
        ),
        help="Comma-separated file extensions or names to include",
    )
    parser.add_argument(
        "--exclude-dirs",
        default=",".join(DEFAULT_FOLDERS_TO_EXCLUDE),
        help="Comma-separated directories to exclude",
    )
    parser.add_argument(
        "--marker-files",
        default=",".join(DEFAULT_ROOT_MARKER_FILES),
        help="Comma-separated files to identify project root",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Enable detailed warning messages",
    )
    return parser.parse_args()

def find_project_root(start_path: Path, marker_files: List[str], verbose: bool) -> Optional[Path]:
    """
    Find the project root by searching up to MAX_SEARCH_DEPTH levels up and down.
    Returns the first directory containing at least one marker file.
    """
    start_path = start_path.resolve()
    checked = {start_path}
    up_paths = [start_path]

    # Collect up to MAX_SEARCH_DEPTH levels up
    curr = start_path
    for _ in range(MAX_SEARCH_DEPTH):
        curr = curr.parent
        if curr not in checked:
            up_paths.append(curr)
            checked.add(curr)

    # Check upwards first
    for path in up_paths:
        if any((path / marker).is_file() for marker in marker_files):
            print(f"Project root found at: {path}")
            return path

    # Downward BFS up to MAX_SEARCH_DEPTH levels
    for up in up_paths:
        queue = [up]
        depth = 0
        while queue and depth < MAX_SEARCH_DEPTH:
            next_level = []
            for folder in queue:
                try:
                    for sub in folder.iterdir():
                        if sub.is_dir() and not sub.is_symlink() and sub not in checked:
                            checked.add(sub)
                            if any((sub / marker).is_file() for marker in marker_files):
                                print(f"Project root found at: {sub}")
                                return sub
                            next_level.append(sub)
                except PermissionError as e:
                    if verbose:
                        print(f"Warning: Permission denied accessing {folder}: {e}", file=sys.stderr)
                except Exception as e:
                    if verbose:
                        print(f"Warning: Error accessing {folder}: {e}", file=sys.stderr)
            queue = next_level
            depth += 1

    print("❌ Project root could not be found.", file=sys.stderr)
    return None

def get_project_name(root_path: Path, verbose: bool) -> str:
    """Parse the root CMakeLists.txt to extract the project name."""
    cmake_path = root_path / "CMakeLists.txt"
    try:
        with cmake_path.open("r", encoding="utf-8") as f:
            for line in f:
                match = re.search(r"project\s*\(\s*(\w+)\s*\)", line, re.IGNORECASE)
                if match:
                    return match.group(1)
        if verbose:
            print(f"Warning: Could not parse project name from {cmake_path}", file=sys.stderr)
    except FileNotFoundError:
        if verbose:
            print(f"Warning: CMakeLists.txt not found at {cmake_path}", file=sys.stderr)
    except UnicodeDecodeError:
        if verbose:
            print(f"Warning: Could not decode {cmake_path} as UTF-8", file=sys.stderr)
    except PermissionError as e:
        if verbose:
            print(f"Warning: Permission denied reading {cmake_path}: {e}", file=sys.stderr)
    return "Unknown"

def matches_interest_patterns(file_path: Path) -> bool:
    """Check if a file matches the patterns of interest."""
    return file_path.name in VALID_FILENAMES or file_path.suffix in VALID_EXTENSIONS

def analyze_project_files(
    root_path: Path, exclude_dirs: Set[str], verbose: bool
) -> Tuple[List[Path], Dict[str, Set[Path]], Dict[str, List[Path]], Dict[str, Path], List[str]]:
    """
    Walk the project to compile file lists and statistics in a single pass.
    Excludes specified directories and parses file references.
    """
    list_exist: List[Path] = []
    list_referenced: Dict[str, Set[Path]] = defaultdict(set)
    found_files_map: Dict[str, List[Path]] = defaultdict(list)
    key_subfolders: Dict[str, Path] = {}
    no_go_areas: List[str] = []

    print("\nAnalyzing project files...")
    for dirpath, dirnames, filenames in os.walk(root_path, topdown=True):
        current_path = Path(dirpath)
        # Prune excluded directories
        original_dirs = list(dirnames)
        dirnames[:] = [d for d in dirnames if d not in exclude_dirs]
        for pruned_dir in set(original_dirs) - set(dirnames):
            no_go_areas.append(str(current_path / pruned_dir))

        # Skip processing in excluded directories
        if any(x in current_path.parts for x in exclude_dirs):
            continue

        # Collect subfolders
        if current_path != root_path:
            rel = str(current_path.relative_to(root_path))
            key_subfolders[rel] = current_path

        # Process files
        for filename in filenames:
            file_path = current_path / filename
            if file_path.is_symlink():
                if verbose:
                    print(f"Warning: Skipping symlink {file_path}", file=sys.stderr)
                continue

            if matches_interest_patterns(file_path):
                list_exist.append(file_path)
                found_files_map[file_path.name].append(file_path)

            # Parse file content for references
            try:
                with file_path.open("r", encoding="utf-8") as f:
                    for line in f:
                        # C/C++ includes
                        if file_path.suffix in [".c", ".h"]:
                            refs = re.findall(r'#\s*include\s*"([^"]+)"', line)
                            for ref in refs:
                                try:
                                    ref_path = (file_path.parent / ref).resolve().name
                                    list_referenced[ref_path].add(file_path)
                                except FileNotFoundError:
                                    if verbose:
                                        print(f"Warning: Reference {ref} in {file_path} not found", file=sys.stderr)
                        # CMake references
                        elif file_path.name.lower().endswith((".cmake", "cmakelists.txt")):
                            refs = re.findall(
                                r'\b([a-zA-Z0-9][a-zA-Z0-9_.-]*?\.(?:c|h|sh|py|json|md|cmake))\b',
                                line,
                            )
                            for ref in refs:
                                try:
                                    ref_path = (file_path.parent / ref).resolve().name
                                    list_referenced[ref_path].add(file_path)
                                except FileNotFoundError:
                                    if verbose:
                                        print(f"Warning: Reference {ref} in {file_path} not found", file=sys.stderr)
                        # Python imports
                        elif file_path.suffix == ".py":
                            refs = re.findall(r'from\s+([a-zA-Z0-9_.-]+)\s+import', line)
                            for ref in refs:
                                list_referenced[f"{ref}.py"].add(file_path)
            except UnicodeDecodeError:
                if verbose:
                    print(f"Warning: Could not decode {file_path} as UTF-8, skipping", file=sys.stderr)
            except PermissionError as e:
                if verbose:
                    print(f"Warning: Permission denied reading {file_path}: {e}", file=sys.stderr)
            except FileNotFoundError:
                if verbose:
                    print(f"Warning: File {file_path} not found, skipping", file=sys.stderr)

    return list_exist, list_referenced, found_files_map, key_subfolders, no_go_areas

def generate_report(
    project_name: str,
    root_path: Path,
    list_exist: List[Path],
    list_referenced: Dict[str, Set[Path]],
    found_files_map: Dict[str, List[Path]],
    key_subfolders: Dict[str, Path],
    no_go_areas: List[str],
) -> None:
    """Generate a formatted audit report for the project."""
    if not list_exist:
        print("Warning: No files of interest found in project.", file=sys.stderr)

    exist_set = set(p.name for p in list_exist)
    ref_set = set(list_referenced.keys())
    orphan_basenames = sorted(list(exist_set - ref_set))
    missing_basenames = sorted(list(ref_set - exist_set))

    # --- Summary ---
    print("\n=== Summary ===")
    print(f"Project name: {project_name}")
    print(f"Project root folder: {root_path}")
    print("Key subfolders:")
    if key_subfolders:
        for rel, path in sorted(key_subfolders.items()):
            print(f"  - {rel}: {path}")
    else:
        print("  (None)")
    print("Excluded directories:")
    if no_go_areas:
        for area in sorted(no_go_areas):
            print(f"  - {area}")
    else:
        print("  (None)")

    # --- Statistics ---
    print("\n=== Statistics ===")
    counts = Counter(p.suffix.lower() for p in list_exist)
    cmake_count = len([p for p in list_exist if p.name.lower() == "cmakelists.txt"])

    print(f"Total # of files of interest: {len(list_exist)}")
    for ext in (".c", ".h", ".cmake", ".sh", ".json", ".py", ".md"):
        print(f"# of {ext}: {counts.get(ext.lower(), 0)}")
    print(f"# of CMakeLists.txt: {cmake_count}")

    # --- Warnings (Duplicates) ---
    print("\n=== Warnings ===")
    for ext in (".c", ".h", ".py", ".sh"):
        print(f"{ext} files with identical names:")
        dups = 0
        for name, paths in sorted(found_files_map.items()):
            if name.lower().endswith(ext.lower()) and len(paths) > 1:
                print(f"  {name}: {paths[0]}")
                for path in sorted(paths)[1:]:
                    print(f"         {path}")
                dups += 1
        if not dups:
            print("  (None)")

    # --- Errors ---
    print("\n=== Errors ===")
    print(f"# of orphan files: {len(orphan_basenames)}")
    if orphan_basenames:
        print("\nDetails of Orphan Files:")
        for name in orphan_basenames:
            for path in found_files_map[name]:
                print(f"- {path}")

    print(f"\n# of missing files: {len(missing_basenames)}")
    if missing_basenames:
        print("\nDetails of Missing Files:")
        for name in missing_basenames:
            print(f"\n- {name}")
            print("  referenced by:")
            for ref_path in sorted(list_referenced[name]):
                print(f"    - {ref_path}")

def main() -> None:
    """Main execution function."""
    args = parse_args()
    files_of_interest = [
        f"*.{ext}" if not ext.startswith("CMakeLists") else ext
        for ext in args.extensions.split(",")
    ]
    global VALID_EXTENSIONS, VALID_FILENAMES
    VALID_EXTENSIONS = {
        ext for pattern in files_of_interest if pattern.startswith("*.") for ext in [pattern[1:]]
    }
    VALID_FILENAMES = {pattern for pattern in files_of_interest if not pattern.startswith("*")}
    exclude_dirs = set(args.exclude_dirs.split(","))
    marker_files = args.marker_files.split(",")

    start_path = Path(".").resolve()
    root_path = find_project_root(start_path, marker_files, args.verbose)
    if not root_path:
        sys.exit(1)

    project_name = get_project_name(root_path, args.verbose)
    list_exist, list_referenced, found_files_map, key_subfolders, no_go_areas = analyze_project_files(
        root_path, exclude_dirs, args.verbose
    )

    generate_report(
        project_name,
        root_path,
        list_exist,
        list_referenced,
        found_files_map,
        key_subfolders,
        no_go_areas,
    )

if __name__ == "__main__":
    main()