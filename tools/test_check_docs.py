"""Regression scenarios for checking a generated documentation tree."""
from pathlib import Path
import tempfile
import unittest

from check_docs import check


class DocumentationLinksTest(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)

    def write(self, name, text):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text)

    def test_valid_project_relative_files_anchors_and_assets(self):
        self.write("index.html", '<a href="/Agent-Harmony/guide/#step">Guide</a><img src="icon.svg"><a href="https://example.com/">External</a>')
        self.write("guide/index.html", '<h1 id="step">Step</h1><a href="../#start">Home</a>')
        self.write("icon.svg", '<svg></svg>')
        self.write("index.html", (self.root / "index.html").read_text() + '<div id="start"></div>')
        self.assertEqual(check(self.root), [])

    def test_missing_file_and_anchor_fail(self):
        self.write("index.html", '<a href="missing.html">Missing</a><a href="#absent">Missing anchor</a>')
        errors = check(self.root)
        self.assertEqual(len(errors), 2)
        self.assertTrue(any("missing file" in error for error in errors))
        self.assertTrue(any("missing anchor" in error for error in errors))

    def test_paths_escaping_project_or_directory_fail(self):
        self.write("index.html", '<a href="/guide/">Wrong base</a><a href="../outside.html">Outside</a>')
        self.assertEqual(len(check(self.root)), 2)

    def test_empty_output_fails(self):
        self.assertTrue(check(self.root))


if __name__ == "__main__":
    unittest.main()
