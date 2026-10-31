"""Validate reviewed CMake versions and publish their documented releases."""
import argparse
from dataclasses import dataclass
from pathlib import Path
import re


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


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("check",))
    args = parser.parse_args()
    try:
        metadata = read_release()
    except (OSError, ValueError) as error:
        parser.exit(1, f"Release validation failed: {error}\n")
    print(f"CMake and release notes agree: {metadata.version} ({metadata.tag})")


if __name__ == "__main__":
    main()
