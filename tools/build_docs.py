"""Build and validate executable C++ guides plus MrDocs reference into a static site."""
import argparse
import importlib.metadata
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

import yaml
from check_docs import check
from normalize_api import normalize

ROOT = Path(__file__).resolve().parents[1]
MRDOCS_RELEASE = "2026.9.29"
EXAMPLES = ("quickstart", "models", "schedule", "network", "spaces", "value_layer")


def run(*command):
    subprocess.run([str(part) for part in command], cwd=ROOT, check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mrdocs", default="mrdocs")
    parser.add_argument("--build-dir", type=Path, default=ROOT / "build" / "docs")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    if build == ROOT or build in ROOT.parents:
        parser.error("Use a dedicated build directory")
    version = subprocess.check_output([args.mrdocs, "--version"], text=True)
    if f"Release: {MRDOCS_RELEASE}" not in version:
        parser.error(f"MrDocs release {MRDOCS_RELEASE} is required")
    if importlib.metadata.version("mkdocs") != "1.6.1":
        parser.error("Install docs/requirements.txt (MkDocs 1.6.1)")
    build.mkdir(parents=True, exist_ok=True)
    core = build / "core"
    run("cmake", "-S", ROOT, "-B", core, "-DBUILD_TESTING=OFF",
        "-DAGENT_HARMONY_BUILD_DOCS=ON", "-DAGENT_HARMONY_BUILD_EXAMPLES=ON", "-DCMAKE_BUILD_TYPE=Release")
    run("cmake", "--build", core, "--config", "Release", "--parallel", "4")
    for example in EXAMPLES:
        run(core / "examples" / f"example_{example}")
    with tempfile.TemporaryDirectory(prefix="source-", dir=build) as temporary:
        source = Path(temporary)
        shutil.copytree(ROOT / "docs", source, dirs_exist_ok=True, ignore=shutil.ignore_patterns("api", "requirements.txt", "mkdocs.yml", "mrdocs.yml"))
        for page in source.rglob("*.md"):
            def example(match):
                code = (ROOT / "examples" / match[1]).read_text()
                return "```cpp\n" + code.rstrip() + "\n```"
            page.write_text(re.sub(r"<!-- example: ([\w]+\.cpp) -->", example, page.read_text()))
        run(args.mrdocs, "--config=" + str(ROOT / "docs" / "mrdocs.yml"),
            "--compilation-database=" + str(core / "compile_commands.json"), "--output=" + str(source / "api"))
        metadata = build / "api.xml"
        run(args.mrdocs, "--config=" + str(ROOT / "docs" / "mrdocs.yml"), "--generator=xml",
            "--compilation-database=" + str(core / "compile_commands.json"), "--output=" + str(metadata))
        normalized = normalize(source / "api", metadata)
        navigation = '<nav aria-label="Documentation"><a href="/Agent-Harmony/">Agent Harmony guides</a> &middot; <a href="/Agent-Harmony/api/">API index</a></nav>'
        for page in (source / "api").rglob("*.html"):
            page.write_text(page.read_text().replace("<body>", "<body>\n" + navigation, 1))
        for name in ("Model", "Schedule", "Field", "MultiagentField", "ContinuousSpace", "Edge"):
            pages = [page for page in (source / "api" / "agh").glob(name + "*.html")
                     if re.fullmatch(re.escape(name) + r"(?:-[0-9a-f]+)?\.html", page.name)]
            if len(pages) != 1 or "template" not in pages[0].read_text() or "Member Functions" not in pages[0].read_text():
                raise RuntimeError(f"Incomplete or duplicate template reference: {name}")
        print(f"Unified {normalized} constrained-template signatures with their member pages")
        config = yaml.safe_load((ROOT / "docs" / "mkdocs.yml").read_text())
        config.update(docs_dir=str(source), site_dir=str(build / "site"))
        configured = build / "mkdocs.yml"
        configured.write_text(yaml.safe_dump(config, sort_keys=False))
        run(sys.executable, "-m", "mkdocs", "build", "--strict", "--config-file", configured)
    errors = check(build / "site")
    if errors:
        raise SystemExit("\n".join(errors))
    (build / "site" / ".nojekyll").touch()
    print(f"Validated documentation: {build / 'site'}")


if __name__ == "__main__":
    main()
