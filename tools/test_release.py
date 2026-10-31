"""Regression coverage for release metadata and publication."""
from pathlib import Path
import tempfile
import unittest

from release import read_release, version_key


class ReleaseMetadataTest(unittest.TestCase):
    def setUp(self):
        temporary = tempfile.TemporaryDirectory()
        self.addCleanup(temporary.cleanup)
        self.root = Path(temporary.name)
        (self.root / "docs").mkdir()
        self.write()

    def write(self, version="0.1.0", notes="## 0.1.0\n\n### Added\n\n- Initial release.\n"):
        (self.root / "CMakeLists.txt").write_text(
            f"cmake_minimum_required(VERSION 3.29)\n"
            f"project(AgentHarmony VERSION {version} LANGUAGES CXX)\n",
            encoding="utf-8")
        (self.root / "docs/release-notes.md").write_text(
            "# Release notes\n\n" + notes, encoding="utf-8")

    def test_version_matches_and_only_current_notes_are_selected(self):
        self.write("0.2.0", "## 0.2.0\n\n- New API.\n\n## 0.1.0\n\n- Old API.\n")
        metadata = read_release(self.root)
        self.assertEqual(metadata.version, "0.2.0")
        self.assertEqual(metadata.tag, "v0.2.0")
        self.assertEqual(metadata.notes, "- New API.\n")

    def test_missing_or_mismatched_release_fails(self):
        for notes in ("", "## 0.2.0\n\n- Wrong version.\n"):
            with self.subTest(notes=notes):
                self.write(notes=notes)
                with self.assertRaises(ValueError):
                    read_release(self.root)
        (self.root / "docs/release-notes.md").unlink()
        with self.assertRaises(FileNotFoundError):
            read_release(self.root)

    def test_empty_and_heading_only_notes_fail(self):
        for body in ("", "### Fixed\n", "<!-- TODO: describe this release -->\n", "- \n"):
            with self.subTest(body=body):
                self.write(notes="## 0.1.0\n\n" + body)
                with self.assertRaises(ValueError):
                    read_release(self.root)

    def test_duplicate_or_out_of_order_versions_fail(self):
        for notes in (
            "## 0.1.0\n- New.\n## 0.1.0\n- Duplicate.\n",
            "## 0.1.0\n- New.\n## 0.2.0\n- Newer.\n",
        ):
            with self.subTest(notes=notes):
                self.write(notes=notes)
                with self.assertRaises(ValueError):
                    read_release(self.root)

    def test_only_normal_semver_versions_are_accepted(self):
        for version in ("01.2.3", "1.02.3", "1.2.03", "1.2", "1.2.3.4",
                        "1.2.3-rc.1", "1.2.3+build", "v1.2.3", "-1.2.3"):
            with self.subTest(version=version):
                with self.assertRaises(ValueError):
                    version_key(version)
                self.write(version, f"## {version}\n\n- Invalid.\n")
                with self.assertRaises(ValueError):
                    read_release(self.root)

    def test_versions_are_compared_numerically(self):
        self.assertGreater(version_key("0.10.0"), version_key("0.9.9"))
        self.assertGreater(version_key("1.0.0"), version_key("0.99.99"))
        self.assertEqual(version_key("0.0.0"), (0, 0, 0))

    def test_multiline_project_ignores_comments_and_other_projects(self):
        (self.root / "CMakeLists.txt").write_text(
            "# project(AgentHarmony VERSION 9.9.9)\n"
            "project(Other VERSION 3.0.0)\n"
            "project(AgentHarmony\n VERSION 0.1.0\n LANGUAGES CXX)\n")
        self.assertEqual(read_release(self.root).version, "0.1.0")


if __name__ == "__main__":
    unittest.main()
