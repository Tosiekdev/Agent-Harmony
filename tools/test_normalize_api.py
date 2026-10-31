"""Check the constrained-template metadata workaround against small generated sites."""
from pathlib import Path
import tempfile
import unittest

from normalize_api import normalize
from check_docs import check


class TemplateReferenceTest(unittest.TestCase):
    def test_signature_members_and_inbound_links_share_one_page(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            api = root / "api"
            (api / "agh").mkdir(parents=True)
            (api / "agh" / "record.html").write_text('<h1>Field</h1><pre>class Field;</pre><h2>Members</h2><p id="add">addAgent</p><table><tr><td><a href="template.html">Self type</a></td></tr></table>')
            (api / "agh" / "template.html").write_text('<h1>Field</h1><pre>template&lt;typename... A&gt; class Field;</pre>')
            (api / "index.html").write_text('<table><tr><td><a href="agh/record.html">Field</a></td></tr><tr><td><a href="agh/template.html">Field</a></td></tr></table>')
            (api / "use.html").write_text('<a href="agh/template.html">Field</a>')
            metadata = root / "api.xml"
            metadata.write_text('''<mrdocs>
              <record><name>Field</name><parent>agh</parent><anchor>record</anchor>
                <loc><defLoc><sourcePath>include/Field.hpp</sourcePath><lineNumber>11</lineNumber></defLoc></loc><interface><public/></interface></record>
              <record><name>Field</name><parent>agh</parent><anchor>template</anchor>
                <loc><defLoc><sourcePath>include/Field.hpp</sourcePath><lineNumber>10</lineNumber></defLoc></loc><template><params/><requires>nonempty</requires></template></record>
            </mrdocs>''')
            self.assertEqual(normalize(api, metadata), 1)
            rendered = (api / "agh" / "record.html").read_text()
            self.assertIn("template&lt;", rendered)
            self.assertIn("addAgent", rendered)
            self.assertIn("Self type", rendered)
            self.assertEqual((api / "index.html").read_text().count(">Field</a>"), 1)
            self.assertIn('agh/record.html', (api / "use.html").read_text())
            self.assertFalse((api / "agh" / "template.html").exists())
            self.assertEqual(check(root), [])

    def test_unaffected_metadata_is_left_intact(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "index.html").write_text("<h1>Already correct</h1>")
            metadata = root / "metadata.xml"
            metadata.write_text("<mrdocs/>")
            self.assertEqual(normalize(root, metadata), 0)
            self.assertEqual((root / "index.html").read_text(), "<h1>Already correct</h1>")
