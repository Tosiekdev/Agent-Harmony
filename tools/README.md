# Developer tools

These Python tools build and validate the C++ documentation and releases. They are optional
for library consumers and are not needed to build the core or its C++ tests.

## Tools

| File | Purpose |
| --- | --- |
| `build_docs.py` | Configure a dedicated C++ build, compile and run all six guide examples, generate MrDocs HTML/XML, assemble the MkDocs site, and validate its links and template references. |
| `check_docs.py` | Check generated HTML for missing local pages, anchors, and assets, plus links escaping the project path. External URLs are skipped; this check does not make network requests. |
| `normalize_api.py` | Internal helper used by `build_docs.py`. Merge MrDocs' split constrained-template signature/member pages using its XML metadata, remove duplicate index entries, and rewrite incoming links. |
| `release.py` | Validate matching CMake/release-note versions and publish their reviewed tags and GitHub Releases after CI and documentation validation. |
| `test_check_docs.py` | Regression scenarios for valid links, missing files/anchors, escaped paths, and empty output. |
| `test_normalize_api.py` | Regression scenarios for combining template signatures and members while preserving tables and inbound links, and leaving unaffected output intact. |
| `test_release.py` | Release metadata, numeric version ordering, publication, retries, conflicts, dry runs, and API failure regressions using mocked GitHub calls. |

## Dependencies and configuration

Use Python 3.9 or later, CMake 3.29 or later, a C++20 compiler, **MrDocs
2026.9.29**, and **MkDocs 1.6.1**. MrDocs is available from its
[official release](https://github.com/cppalliance/mrdocs/releases/tag/2026.9.29).
The build script checks the installed generator versions.

From the repository root, install the Python dependencies into an environment:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r docs/requirements.txt
```

The tracked generator settings live under `docs/`:

- [MkDocs configuration](../docs/mkdocs.yml): guide navigation, theme, search, and strict link checks.
- [MrDocs configuration](../docs/mrdocs.yml): public-header extraction, excluded implementation details, HTML generation, and documentation diagnostics treated as errors.

Their paths are relative to `docs/`. `build_docs.py` locates the repository from
its own location, so its operation does not depend on the current working directory.

## Build the complete documentation

```sh
.venv/bin/python tools/build_docs.py --mrdocs /path/to/mrdocs
```

If `mrdocs` is on `PATH`, omit `--mrdocs`. To use another output directory:

```sh
.venv/bin/python tools/build_docs.py --mrdocs /path/to/mrdocs --build-dir /tmp/agent-harmony-docs
```

The default output is `build/docs/site/index.html`. Supporting build artifacts
include `build/docs/core/`, the MrDocs `api.xml` corpus, and a generated
`build/docs/mkdocs.yml` pointing at temporary assembled sources. Generated files
are not committed. Config files and dependency manifests are excluded from the
published site.

The build embeds the actual `examples/*.cpp` sources into their guides, compiles
and runs those programs, checks complete template reference pages, and adds
navigation between the guides and API reference.

## Run checks independently

Run the tooling regressions:

```sh
.venv/bin/python -m unittest discover -s tools -p 'test_*.py'
```

Check an already generated site without rebuilding it:

```sh
.venv/bin/python tools/check_docs.py build/docs/site
```

The checker expects the `/Agent-Harmony/` hosting prefix. For another prefix,
pass `--base-path /other-project/`. `normalize_api.py` is an imported helper and
does not provide a standalone command.

See the [documentation development guide](../docs/documentation.md) for the
GitHub Actions and Pages publishing setup.

## Validate and preview releases

The release helper and its tests use Python's standard library. Metadata checks
do not need documentation generators or GitHub authentication:

```sh
python3 tools/release.py check
```

Publication reads the reviewed CMake version and the newest section in
`docs/release-notes.md`. It requires authenticated GitHub CLI access. Preview the
remote decision without creating tags or releases:

```sh
python3 tools/release.py publish --dry-run --sha "$(git rev-parse HEAD)"
```

The repository defaults to `GITHUB_REPOSITORY` in Actions or the GitHub CLI's
current repository locally; `--repo OWNER/REPO` overrides it. The SHA must be a
full commit hash matching the checkout. The workflow runs `publish --sha` with
the validated upstream CI SHA and its job-scoped `GITHUB_TOKEN`.

See the [release process](../docs/documentation.md#versions-and-releases) for
version selection, release-note format, publication gates, and retry instructions.
