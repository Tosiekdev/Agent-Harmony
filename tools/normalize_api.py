"""Unify MrDocs' split constrained-template record/signature pages using its XML corpus.

MrDocs 2026.9.29 can emit adjacent declarations of one constrained class as two
records: one with the template head and one with its member interface. Match only
that exact metadata shape. Preserve the member-bearing page and its member URLs.
"""
from collections import defaultdict
from html import unescape
import os
from pathlib import Path
import re
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET


HREF = re.compile(r'(href=["\'])([^"\']+)(["\'])')
PRE = re.compile(r"<pre\b[^>]*>.*?</pre>", re.S)
ROW = re.compile(r"<tr\b[^>]*>.*?</tr>", re.S)


def normalize(api, metadata):
    api = Path(api).resolve()
    groups = defaultdict(list)
    corpus = ET.parse(metadata).getroot()
    namespace_pages = {api / "index.html"}
    for namespace in corpus.findall("namespace"):
        anchor = namespace.findtext("anchor")
        if anchor:
            namespace_pages.update(api.rglob(anchor + ".html"))
    for record in corpus.findall("record"):
        location = record.find("loc/defLoc")
        if location is not None:
            groups[(record.findtext("parent"), record.findtext("name"), location.findtext("sourcePath"))].append(record)
    replacements = {}
    for records in groups.values():
        if len(records) != 2:
            continue
        templates = [r for r in records if r.find("template/requires") is not None and r.find("interface") is None]
        members = [r for r in records if r.find("template") is None and r.find("interface") is not None]
        if len(templates) != 1 or len(members) != 1:
            continue
        template, member = templates[0], members[0]
        lines = [int(r.findtext("loc/defLoc/lineNumber")) for r in records]
        if abs(lines[0] - lines[1]) != 1:
            continue
        def page(record):
            paths = list(api.rglob(record.findtext("anchor") + ".html"))
            if len(paths) != 1:
                raise RuntimeError("Cannot uniquely resolve a MrDocs class page")
            return paths[0]
        source, destination = page(template), page(member)
        if source.parent != destination.parent:
            raise RuntimeError("Split class pages unexpectedly have different parents")
        signature = PRE.search(source.read_text())
        if signature is None or "template" not in signature[0]:
            raise RuntimeError("Constrained-template signature was not rendered")
        rendered, count = PRE.subn(lambda _: signature[0], destination.read_text(), count=1)
        if count != 1:
            raise RuntimeError("Member page has no synopsis to normalize")
        destination.write_text(rendered)
        replacements[source] = destination

    def target(page, href):
        url = urlsplit(unescape(href))
        if url.scheme or url.netloc or not url.path:
            return None
        return (page.parent / unquote(url.path)).resolve()

    for page in api.rglob("*.html"):
        if page in replacements:
            continue
        text = page.read_text()
        # Namespace/index type tables contain one row for each split record.
        if page in namespace_pages:
            text = ROW.sub(lambda row: "" if any(target(page, link[2]) in replacements for link in HREF.finditer(row[0])) else row[0], text)
        def rewrite(link):
            original = target(page, link[2])
            if original not in replacements:
                return link[0]
            relative = Path(os.path.relpath(replacements[original], page.parent)).as_posix()
            parsed = urlsplit(unescape(link[2]))
            if parsed.fragment:
                relative += "#" + parsed.fragment
            return link[1] + relative + link[3]
        page.write_text(HREF.sub(rewrite, text))
    tagfile = api / "reference.tag.xml"
    if tagfile.exists():
        text = tagfile.read_text()
        for source, destination in replacements.items():
            text = text.replace(source.relative_to(api).as_posix(), destination.relative_to(api).as_posix())
        tagfile.write_text(text)
    for source in replacements:
        source.unlink()
    return len(replacements)
