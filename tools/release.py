"""Validate reviewed CMake versions and publish their documented releases."""
import argparse
from dataclasses import dataclass
import json
import os
from pathlib import Path
import re
import subprocess
from urllib.parse import urljoin, urlsplit


ROOT = Path(__file__).resolve().parents[1]
VERSION = r"(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)\.(?:0|[1-9][0-9]*)"


def version_key(version):
    if not re.fullmatch(VERSION, version):
        raise ValueError(f"Expected MAJOR.MINOR.PATCH without leading zeros: {version}")
    return tuple(int(component) for component in version.split("."))


@dataclass(frozen=True)
class Release:
    version: str
    notes: str

    @property
    def tag(self):
        return "v" + self.version


def read_release(root=ROOT):
    root = Path(root)
    cmake = re.sub(r"#[^\n]*", "", (root / "CMakeLists.txt").read_text(encoding="utf-8"))
    projects = re.findall(r"\bproject\s*\(\s*AgentHarmony\b([^)]*)\)", cmake, re.I)
    if len(projects) != 1:
        raise ValueError("Expected one AgentHarmony project in CMakeLists.txt")
    versions = re.findall(r"\bVERSION\s+([^\s)]+)", projects[0], re.I)
    if len(versions) != 1:
        raise ValueError("Expected one AgentHarmony project VERSION")
    version = versions[0]
    version_key(version)
    document = (root / "docs/release-notes.md").read_text(encoding="utf-8")
    headings = list(re.finditer(r"^##[ \t]+(.+?)[ \t]*$", document, re.M))
    if not headings:
        raise ValueError("Release notes must contain a ## MAJOR.MINOR.PATCH heading")
    keys = [version_key(heading[1]) for heading in headings]
    if any(newer <= older for newer, older in zip(keys, keys[1:])):
        raise ValueError("Release-note versions must be unique and newest first")
    if headings[0][1] != version:
        raise ValueError(f"Newest release notes must match CMake version {version}")
    end = headings[1].start() if len(headings) > 1 else len(document)
    notes = document[headings[0].end():end].strip()
    content = re.sub(r"<!--.*?-->", "", notes, flags=re.S)
    content = re.sub(r"^\s*#+[^\n]*", "", content, flags=re.M)
    if not re.search(r"\w", content):
        raise ValueError(f"Release {version} needs substantive notes")
    return Release(version, notes + "\n")


def gh(*args, input=None):
    return subprocess.run(["gh", *args], input=input, text=True,
                          capture_output=True, check=True).stdout


def github_items(repository, resource):
    pages = json.loads(gh("api", "--paginate", "--slurp",
                          f"repos/{repository}/{resource}?per_page=100"))
    return [item for page in pages for item in page]


def github_notes(metadata, repository, sha):
    # The same Markdown is shown on Pages and GitHub Releases. Resolve its local
    # documentation links against the released source, not the release URL.
    base = f"https://github.com/{repository}/blob/{sha}/docs/"

    def link(match):
        target = match[2]
        if urlsplit(target).scheme or target.startswith(("/", "#")):
            return match[0]
        return match[1] + urljoin(base, target) + ")"

    return re.sub(r"(\[[^\]\n]+\]\()([^\s)]+)\)", link, metadata.notes)


def publish(metadata, sha, repository, dry_run=False, root=ROOT):
    if not re.fullmatch(r"[0-9a-f]{40}", sha):
        raise ValueError("Publication requires a full lowercase commit SHA")
    head = subprocess.check_output(["git", "-C", str(root), "rev-parse", "HEAD"], text=True).strip()
    if head != sha:
        raise ValueError("Checked-out HEAD must match the validated release SHA")
    if not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
        raise ValueError("Repository must be OWNER/REPO")
    tags = {item["name"]: item["commit"]["sha"] for item in github_items(repository, "tags")}
    releases = github_items(repository, "releases")
    versions = [version_key(tag[1:]) for tag in tags if re.fullmatch("v" + VERSION, tag)]
    versions.extend(version_key(item["tag_name"][1:]) for item in releases
                    if not item["draft"] and re.fullmatch("v" + VERSION, item["tag_name"]))
    if versions and version_key(metadata.version) < max(versions):
        raise ValueError(f"Version {metadata.version} is older than an existing release tag")
    existing = next((item for item in releases if item["tag_name"] == metadata.tag), None)
    if existing:
        if existing["draft"] or existing["prerelease"]:
            raise ValueError(f"Resolve the existing draft or prerelease for {metadata.tag} manually")
        if metadata.tag not in tags:
            raise ValueError(f"Published release {metadata.tag} has a missing tag")
        return "already released"
    if metadata.tag in tags and tags[metadata.tag] != sha:
        raise ValueError(f"Tag {metadata.tag} points to a different commit; refusing to move it")
    if dry_run:
        return "dry run: would release"
    if metadata.tag not in tags:
        # Create the ref first: the API rejects a concurrently created tag rather
        # than silently publishing from a different commit.
        gh("api", "--method", "POST", f"repos/{repository}/git/refs",
           "-f", "ref=refs/tags/" + metadata.tag, "-f", "sha=" + sha)
    gh("release", "create", metadata.tag, "--repo", repository,
       "--verify-tag", "--target", sha, "--title", metadata.tag, "--notes-file", "-",
       input=github_notes(metadata, repository, sha))
    return "released"


def summary(message):
    print(message)
    if os.environ.get("GITHUB_STEP_SUMMARY"):
        with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as output:
            output.write("## Release\n\n" + message + "\n")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("check", help="Check CMake and documented release metadata")
    release = commands.add_parser("publish", help="Publish reviewed metadata from a validated commit")
    release.add_argument("--sha", required=True)
    release.add_argument("--repo", default=os.environ.get("GITHUB_REPOSITORY"))
    release.add_argument("--dry-run", action="store_true", help="Read remote state without publishing")
    args = parser.parse_args()
    try:
        metadata = read_release()
        if args.command == "check":
            if os.environ.get("GITHUB_OUTPUT"):
                with open(os.environ["GITHUB_OUTPUT"], "a", encoding="utf-8") as output:
                    output.write(f"version={metadata.version}\n")
            print(f"CMake and release notes agree: {metadata.version} ({metadata.tag})")
        else:
            repository = args.repo or gh("repo", "view", "--json", "nameWithOwner", "--jq", ".nameWithOwner").strip()
            outcome = publish(metadata, args.sha, repository, args.dry_run)
            summary(f"{metadata.tag}: {outcome} (validated commit {args.sha}).")
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        detail = error.stderr if isinstance(error, subprocess.CalledProcessError) else str(error)
        summary(f"Release failed: {detail or error}")
        parser.exit(1)


if __name__ == "__main__":
    main()
