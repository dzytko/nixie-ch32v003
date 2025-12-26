import argparse
import subprocess
import re
from datetime import datetime, timezone
from pathlib import Path
import tempfile
import os

TAG_REGEX = re.compile(r"^v(\d+)\.(\d+)\.(\d+)$")

def run_git(cmd):
    try:
        out = subprocess.check_output(cmd, stderr=subprocess.DEVNULL, text=True)
        return out.strip()
    except Exception:
        return None

def get_git_tag():
    return run_git([GIT_EXEC, "describe", "--tags", "--abbrev=0"])

def get_git_commit():
    return run_git([GIT_EXEC, "rev-parse", "--short", "HEAD"])

def parse_version_from_tag(tag):
    if not tag:
        return 99, 99, 99

    match = TAG_REGEX.match(tag)
    if not match:
        return 99, 99, 99

    return int(match.group(1)), int(match.group(2)), int(match.group(3))


def generate_header_content(major, minor, patch, tag, commit, build_date):
    tag_str = tag or ""
    commit_str = commit or ""
    return f""" // Auto-generated file. Do not edit.

#ifndef VERSION_H
#define VERSION_H

#define GIT_VERSION_MAJOR {major}
#define GIT_VERSION_MINOR {minor}
#define GIT_VERSION_PATCH {patch}

#define GIT_TAG "{tag_str}"
#define GIT_COMMIT "{commit_str}"
#define BUILD_DATE "{build_date}"

#endif /* VERSION_H */
"""

def main():
    global GIT_EXEC
    parser = argparse.ArgumentParser(description="Generate C header with git version info.")
    parser.add_argument("--output", "-o", required=True, default="include/version.h", help="Output header path (default: include/version.h)")
    parser.add_argument("--git-exec", required=True, help="Path to git executable to use (default: git)")
    args = parser.parse_args()

    GIT_EXEC = args.git_exec

    tag = get_git_tag()
    commit = get_git_commit()
    major, minor, patch = parse_version_from_tag(tag)

    build_date = datetime.now(timezone.utc).replace(microsecond=0).isoformat().replace('+00:00', 'Z')

    content = generate_header_content(major, minor, patch, tag, commit, build_date)
    out_path = Path(args.output)
    with open(out_path, "w") as f:
        f.write(content)

if __name__ == "__main__":
    main()
