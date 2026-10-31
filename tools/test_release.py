"""Regression coverage for release metadata and publication."""
from pathlib import Path
from contextlib import redirect_stdout
import io
import json
import os
import subprocess
import tempfile
import unittest
from unittest.mock import patch

from release import Release, main, publish, read_release, version_key


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


class PublicationTest(unittest.TestCase):
    sha = "a" * 40
    repository = "Tosiekdev/Agent-Harmony"

    def setUp(self):
        self.metadata = Release("0.1.0", "- Initial release; see [migration](migration.md).\n")
        self.tags = []
        self.releases = []
        self.calls = []
        self.failure = None
        self.fail_read = False
        mocked = patch("release.gh", side_effect=self.github)
        mocked.start()
        self.addCleanup(mocked.stop)
        head = patch("release.subprocess.check_output", return_value=self.sha + "\n")
        head.start()
        self.addCleanup(head.stop)

    def github(self, *args, **kwargs):
        self.calls.append((args, kwargs))
        if self.failure and self.failure in args:
            raise subprocess.CalledProcessError(1, ["gh", *args], stderr="API unavailable")
        if "--slurp" in args:
            if self.fail_read:
                raise subprocess.CalledProcessError(1, ["gh", *args], stderr="Forbidden")
            items = self.tags if "/tags?" in args[-1] else self.releases
            return json.dumps([items[:1], items[1:]])
        return "{}"

    def run_publish(self, **kwargs):
        return publish(self.metadata, self.sha, self.repository, **kwargs)

    def writes(self):
        return [call for call in self.calls if "POST" in call[0] or call[0][:2] == ("release", "create")]

    def test_initial_release_creates_exact_tag_and_publishes_documented_notes(self):
        self.assertEqual(self.run_publish(), "released")
        tag, release = self.writes()
        self.assertIn("ref=refs/tags/v0.1.0", tag[0])
        self.assertIn("sha=" + self.sha, tag[0])
        self.assertEqual(release[0][:3], ("release", "create", "v0.1.0"))
        self.assertIn("--verify-tag", release[0])
        self.assertIn(self.sha, release[0])
        self.assertIn("--notes-file", release[0])
        self.assertIn(f"https://github.com/{self.repository}/blob/{self.sha}/docs/migration.md",
                      release[1]["input"])

    def test_already_released_version_skips_even_for_later_commits(self):
        self.tags = [{"name": "v0.1.0", "commit": {"sha": "b" * 40}}]
        self.releases = [{"tag_name": "v0.1.0", "draft": False, "prerelease": False}]
        self.assertEqual(self.run_publish(), "already released")
        self.assertEqual(self.writes(), [])

    def test_subsequent_release_uses_numeric_versions_and_all_pages(self):
        self.metadata = Release("0.10.0", "- New milestone.\n")
        self.tags = [{"name": "unrelated", "commit": {"sha": "b" * 40}},
                     {"name": "v0.9.9", "commit": {"sha": "c" * 40}}]
        self.assertEqual(self.run_publish(), "released")
        self.assertIn("ref=refs/tags/v0.10.0", self.writes()[0][0])

    def test_version_regression_fails_without_writes(self):
        self.tags = [{"name": "v0.2.0", "commit": {"sha": "b" * 40}}]
        with self.assertRaisesRegex(ValueError, "older"):
            self.run_publish()
        self.assertEqual(self.writes(), [])

    def test_matching_tag_recovers_incomplete_publication_without_retagging(self):
        self.tags = [{"name": "v0.1.0", "commit": {"sha": self.sha}}]
        self.assertEqual(self.run_publish(), "released")
        self.assertEqual(len(self.writes()), 1)
        self.assertEqual(self.writes()[0][0][:2], ("release", "create"))

    def test_conflicting_unreleased_tag_fails_without_writes(self):
        self.tags = [{"name": "v0.1.0", "commit": {"sha": "b" * 40}}]
        with self.assertRaisesRegex(ValueError, "different commit"):
            self.run_publish()
        self.assertEqual(self.writes(), [])

    def test_draft_or_prerelease_is_not_overwritten(self):
        for draft, prerelease in ((True, False), (False, True)):
            with self.subTest(draft=draft, prerelease=prerelease):
                self.tags = [{"name": "v0.1.0", "commit": {"sha": self.sha}}]
                self.releases = [{"tag_name": "v0.1.0", "draft": draft, "prerelease": prerelease}]
                with self.assertRaisesRegex(ValueError, "existing draft or prerelease"):
                    self.run_publish()
                self.assertEqual(self.writes(), [])

    def test_release_with_missing_tag_fails_without_writes(self):
        self.releases = [{"tag_name": "v0.1.0", "draft": False, "prerelease": False}]
        with self.assertRaisesRegex(ValueError, "missing tag"):
            self.run_publish()
        self.assertEqual(self.writes(), [])

    def test_failed_api_reads_do_not_mean_no_releases_exist(self):
        self.fail_read = True
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_publish()
        self.assertEqual(self.writes(), [])

    def test_failed_tag_creation_never_attempts_release(self):
        self.failure = "POST"
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_publish()
        self.assertEqual(len(self.writes()), 1)

    def test_failed_release_creation_is_reported(self):
        self.failure = "create"
        with self.assertRaises(subprocess.CalledProcessError):
            self.run_publish()
        self.assertEqual(len(self.writes()), 2)

    def test_dry_run_reads_remote_state_without_writes(self):
        self.assertEqual(self.run_publish(dry_run=True), "dry run: would release")
        self.assertEqual(self.writes(), [])

    def test_invalid_or_unchecked_sha_fails_before_github_access(self):
        for sha in ("master", "a" * 39, "b" * 40):
            with self.subTest(sha=sha):
                with self.assertRaises(ValueError):
                    publish(self.metadata, sha, self.repository)
        self.assertEqual(self.calls, [])


class ReleaseCommandTest(unittest.TestCase):
    def test_check_exposes_the_version_as_a_workflow_output(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "output"
            with patch.dict(os.environ, {"GITHUB_OUTPUT": str(output)}), \
                    patch("sys.argv", ["release.py", "check"]), \
                    patch("release.read_release", return_value=Release("0.1.0", "- Notes.\n")), \
                    redirect_stdout(io.StringIO()):
                main()
            self.assertEqual(output.read_text(), "version=0.1.0\n")

    def test_publication_outcome_and_api_failure_reach_the_job_summary(self):
        for outcome in ("released", "already released", "failure"):
            with self.subTest(outcome=outcome), tempfile.TemporaryDirectory() as directory:
                summary = Path(directory) / "summary"
                error = subprocess.CalledProcessError(1, ["gh", "api"], stderr="Forbidden")
                with patch.dict(os.environ, {"GITHUB_STEP_SUMMARY": str(summary)}), \
                        patch("sys.argv", ["release.py", "publish", "--sha", "a" * 40,
                                           "--repo", "Tosiekdev/Agent-Harmony"]), \
                        patch("release.read_release", return_value=Release("0.1.0", "- Notes.\n")), \
                        patch("release.publish", return_value=outcome,
                              side_effect=error if outcome == "failure" else None), \
                        redirect_stdout(io.StringIO()):
                    if outcome == "failure":
                        with self.assertRaises(SystemExit) as failure:
                            main()
                        self.assertEqual(failure.exception.code, 1)
                    else:
                        main()
                expected = "Release failed: Forbidden" if outcome == "failure" else "v0.1.0: " + outcome
                self.assertIn(expected, summary.read_text())


if __name__ == "__main__":
    unittest.main()
