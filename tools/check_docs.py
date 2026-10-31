"""Check generated HTML links, anchors, and project-relative assets without network access."""
from html.parser import HTMLParser
from pathlib import Path
from urllib.parse import unquote, urlsplit
import argparse


class Page(HTMLParser):
    def __init__(self, path):
        super().__init__(convert_charrefs=True)
        self.ids = set()
        self.links = []
        self.feed(path.read_text(encoding="utf-8"))

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        for name in ("id", "name" if tag == "a" else "id"):
            if attrs.get(name):
                self.ids.add(attrs[name])
        for name in ("href", "src"):
            if attrs.get(name):
                self.links.append(attrs[name])


def check(site, base_path="/Agent-Harmony/"):
    site = Path(site).resolve()
    pages = {path: Page(path) for path in site.rglob("*.html")}
    errors = []
    for path, page in pages.items():
        for link in page.links:
            url = urlsplit(link)
            if url.scheme or url.netloc:
                continue
            destination = unquote(url.path)
            if destination.startswith("/"):
                if not destination.startswith(base_path):
                    errors.append(f"{path.relative_to(site)}: escapes project path: {link}")
                    continue
                target = site / destination[len(base_path):]
            else:
                target = path.parent / destination if destination else path
            target = target.resolve()
            if not target.is_relative_to(site):
                errors.append(f"{path.relative_to(site)}: escapes site directory: {link}")
                continue
            if target.is_dir():
                target /= "index.html"
            if not target.is_file():
                errors.append(f"{path.relative_to(site)}: missing file: {link}")
            elif url.fragment and target.suffix == ".html":
                target_page = pages.get(target)
                if target_page is None or unquote(url.fragment) not in target_page.ids:
                    errors.append(f"{path.relative_to(site)}: missing anchor: {link}")
    if not pages:
        errors.append("No HTML documentation was generated")
    return errors


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("site", type=Path)
    parser.add_argument("--base-path", default="/Agent-Harmony/")
    args = parser.parse_args()
    issues = check(args.site, args.base_path)
    if issues:
        raise SystemExit("\n".join(issues))
    print("Documentation links, anchors, and assets verified")
