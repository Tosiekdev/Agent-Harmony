# Documentation development and publishing

The site combines authored Markdown guides with a generated MrDocs HTML reference.
The same command builds it locally and in GitHub Actions. Generated HTML is a build
artifact and is not committed.

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

CI runs only for pull requests targeting `master` and pushes to `master`.
Documentation PRs build and upload the complete HTML site for review without
publishing. A successful C++ CI run for a `master` push triggers documentation
publication through GitHub Pages. Failed CI does not publish. Deployments have
write permissions only in the deployment job and use the `github-pages` environment.

Before the first deployment, a repository administrator selects **Settings → Pages
→ Build and deployment → Source: GitHub Actions**. The expected project path is
`/Agent-Harmony/`; no custom domain or generated-content branch is required.
Only merged `master` content can deploy. Feature branches and pull requests cannot
publish the live site. See [GitHub's Pages setup](https://docs.github.com/en/pages/getting-started-with-github-pages/configuring-a-publishing-source-for-your-github-pages-site).
