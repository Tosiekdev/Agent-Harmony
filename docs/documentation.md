# Documentation development and publishing

The site combines authored Markdown guides with a generated MrDocs HTML reference.
The same command builds it locally and in GitHub Actions. Generated HTML is a build
artifact and is not committed. Source configuration lives in `docs/mkdocs.yml`
for the guide site and `docs/mrdocs.yml` for the API reference. Paths in these
files are relative to the `docs/` directory; the build script can be run from
any working directory.

## Local build

Install MrDocs release **2026.9.29** from its
[official releases](https://github.com/cppalliance/mrdocs/releases/tag/2026.9.29)
and place `mrdocs` on your path. The build script checks its release.
Install the pinned guide generator in a Python environment:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -r docs/requirements.txt
.venv/bin/python tools/build_docs.py --mrdocs /path/to/mrdocs
```

This configures a dedicated core extraction build, compiles and executes all six
C++ examples, extracts public core documentation with errors treated as failures,
builds guides in strict mode, and checks HTML links, anchors, and local assets.
The pinned MrDocs corpus splits constrained-template signatures from their member
records. A regression-tested pass unifies only adjacent matching declarations and
rewrites their links, preserving template constraints and member documentation.
The result is `build/docs/site/`. Documentation tools are optional for library
consumers. The experimental visualization guide is separate from the core reference.

Examples displayed in the guides come directly from the compiled `examples/*.cpp`
files. Edit those sources and their surrounding guide text together.

## GitHub Pages

CI and documentation checks run for every pull request, regardless of its target
branch. Push validation runs only on `master`.
Documentation PRs build and upload the complete HTML site for review without
publishing. A successful C++ CI run for a `master` push triggers documentation
publication through GitHub Pages. Failed CI does not publish. Deployments have
write permissions only in the deployment job and use the `github-pages` environment.

Before the first deployment, a repository administrator selects **Settings → Pages
→ Build and deployment → Source: GitHub Actions**. The expected project path is
`/Agent-Harmony/`; no custom domain or generated-content branch is required.
Only merged `master` content can deploy. Feature branches and pull requests cannot
publish the live site. See [GitHub's Pages setup](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site).

## Versions and releases

The root `CMakeLists.txt` declares the reviewed library version in
`project(AgentHarmony VERSION MAJOR.MINOR.PATCH ...)`. The newest heading in
[release notes](release-notes.md) must contain exactly that version, without a
`v` prefix. Automation uses `vMAJOR.MINOR.PATCH` for the Git tag and GitHub Release
title, and publishes that section's notes. Relative Markdown links in those notes
are resolved against the released documentation sources on GitHub.

Versions follow [Semantic Versioning](https://semver.org/). Use three numeric
components without leading zeros; prerelease suffixes, build metadata, and a
fourth CMake version component are not supported by this release process.
The public API is described by the [API contracts](contracts.md) and reference.
For `1.0.0` and later, increment the major version for incompatible API changes,
the minor version for compatible additions, and the patch version for compatible
bug fixes. During initial `0.x` development, use a minor increment for API additions
or breaking changes and a patch increment for compatible fixes. The first release
is `0.1.0`; the public API is not yet stable.

### Prepare a release PR

1. Choose a new version greater than the existing release tags. Update the root
   CMake project version and add a matching `## MAJOR.MINOR.PATCH` section at the
   top of the release notes in the same PR. Keep earlier sections in descending
   version order and preserve published notes. Describe additions, fixes, breaking
   changes, and migration requirements relevant to users.
2. Run `python3 tools/release.py check` and
   `python3 -m unittest discover -s tools -p 'test_*.py'`, then build the complete
   documentation using the local build command above. CI checks metadata for every
   PR, including PRs targeting a working branch.
3. Merge through the normal review process into `master`. The automation PR is
   stacked onto `fix/cpp-library-stabilization`: merge it there first, then merge
   the stabilization PR into `master`. A merge into the working branch does not
   publish a release.

Version changes are reviewed rather than inferred from commit messages. Merges
retaining an already released version skip publication. Wait for a release to
finish before merging the next release version.

### Automatic publication

A successful same-repository `master` push run of **C++ CI** triggers
**C++ documentation**. After its documentation build and validation succeed, a
separate release job checks out that CI run's exact commit SHA and publishes the
reviewed version. GitHub Pages deployment runs independently; a Pages deployment
failure does not block a release after the documentation build has passed.
PRs, failed/cancelled CI runs, and failed documentation builds cannot publish.

The release job alone receives `contents: write`, authenticated with
`GITHUB_TOKEN`; no personal access token is required. Repository and organization
rules must allow this job to create release tags and GitHub Releases. Existing
releases are not edited and tags are never moved. Source archives are supplied by
GitHub; there are no additional binary packages. Publication jobs for the same
version are serialized, and newer master documentation runs do not cancel an
active publication. The job summary reports released, already released, or failed.

### Inspect and retry

Preview publication from a checkout of the intended commit, with GitHub CLI
authenticated for the repository:

```sh
python3 tools/release.py publish --dry-run --sha "$(git rev-parse HEAD)"
```

This reads remote tags and releases without writing either. On a failure, inspect
the release job log and summary. Fix transient API or permission problems and
rerun the failed job in the original **C++ documentation** run. If the tag was
created but release creation failed, the retry uses that tag only when it points
to the same validated SHA. An already published version is skipped, including on
later merges using that version.

A conflicting unreleased tag, an existing draft/prerelease for the selected tag,
or a version older than an existing release tag fails visibly and requires manual
resolution. Retry the original run for incomplete publication rather than a later
commit sharing its version. Publish corrections to released content under a new
reviewed version; do not move a published tag or overwrite its release.
