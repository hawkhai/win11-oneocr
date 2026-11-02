"""Exercise real downloads and failure isolation with a local HTTP fixture."""
import copy
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import importlib.util
import io
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import threading
import unittest
import zipfile

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / "scripts/ocr-classics.py"
spec = importlib.util.spec_from_file_location("ocr_classics", SCRIPT)
corpus = importlib.util.module_from_spec(spec)
spec.loader.exec_module(corpus)


class DownloadsTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        image = io.BytesIO()
        Image.new("RGB", (12, 9), "white").save(image, format="PNG")
        cls.png = image.getvalue()
        buffer = io.BytesIO()
        with zipfile.ZipFile(buffer, "w") as archive:
            archive.writestr("inputs/example.png", cls.png)
            archive.writestr("../../unselected.png", b"must not be extracted")
        cls.archive = buffer.getvalue()
        cls.requests = []

        class Handler(BaseHTTPRequestHandler):
            def do_GET(self):
                cls.requests.append(self.path)
                payload = {"/valid": cls.png, "/truncated": cls.png,
                           "/invalid": b"<html>not an image</html>",
                           "/archive": cls.archive}[self.path]
                self.send_response(200)
                self.send_header("Content-Length", str(len(payload)))
                self.end_headers()
                self.wfile.write(payload[:10] if self.path == "/truncated" else payload)
                self.close_connection = True

            def log_message(self, *args):
                pass

        cls.server = ThreadingHTTPServer(("127.0.0.1", 0), Handler)
        cls.thread = threading.Thread(target=cls.server.serve_forever, daemon=True)
        cls.thread.start()
        cls.base = f"http://127.0.0.1:{cls.server.server_port}"

    @classmethod
    def tearDownClass(cls):
        cls.server.shutdown()
        cls.server.server_close()
        cls.thread.join()

    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)

    def asset(self, endpoint="valid"):
        return {"name": "example", "file": "images/example.png",
                "image_url": self.base + "/" + endpoint, **corpus.image_metadata(self.png)}

    def test_download_and_idempotent_skip(self):
        fetcher = corpus.Fetcher({}, retries=0)
        self.assertEqual(fetcher.ensure(self.asset(), self.root), "downloaded")
        count = len(self.requests)
        self.assertEqual(fetcher.ensure(self.asset(), self.root), "skipped")
        self.assertEqual(len(self.requests), count)
        self.assertEqual((self.root / "images/example.png").read_bytes(), self.png)

    def test_interrupted_download_does_not_create_file(self):
        with self.assertRaises(Exception):
            corpus.Fetcher({}, retries=0).ensure(self.asset("truncated"), self.root)
        self.assertFalse((self.root / "images/example.png").exists())
        self.assertEqual(list(self.root.rglob("*.part")), [])

    def test_matching_hash_does_not_make_html_an_image(self):
        asset = self.asset("invalid")
        payload = b"<html>not an image</html>"
        asset.update(sha256=corpus.digest(payload), bytes=len(payload))
        with self.assertRaises(Exception):
            corpus.Fetcher({}, retries=0).ensure(asset, self.root)
        self.assertFalse((self.root / "images/example.png").exists())

    def test_failed_replacement_preserves_existing_bytes(self):
        path = self.root / "images/example.png"
        path.parent.mkdir()
        path.write_bytes(b"existing damaged file")
        with self.assertRaisesRegex(ValueError, "SHA-256"):
            corpus.Fetcher({}, retries=0).ensure(self.asset("invalid"), self.root)
        self.assertEqual(path.read_bytes(), b"existing damaged file")

    def test_archive_reads_only_selected_member_and_caches(self):
        downloads = {"demo": {"url": self.base + "/archive", "bytes": len(self.archive),
                              "sha256": corpus.digest(self.archive)}}
        asset = self.asset()
        asset.update(archive_id="demo", archive_member="inputs/example.png")
        fetcher = corpus.Fetcher(downloads, retries=0)
        before = self.requests.count("/archive")
        self.assertEqual(fetcher.ensure(asset, self.root), "downloaded")
        asset["file"] = "images/second.png"
        self.assertEqual(fetcher.ensure(asset, self.root), "downloaded")
        self.assertEqual(self.requests.count("/archive"), before + 1)
        self.assertEqual(sorted(p.name for p in self.root.rglob("*") if p.is_file()),
                         ["example.png", "second.png"])

    def test_paths_cannot_escape_output_directory(self):
        asset = self.asset()
        asset["file"] = "../escape.png"
        with self.assertRaises(ValueError):
            corpus.Fetcher({}, retries=0).ensure(asset, self.root)

    def test_cli_failure_is_nonzero_and_does_not_publish_index(self):
        original = json.loads(corpus.DEFAULT_MANIFEST.read_text(encoding="utf-8"))
        manifest = copy.deepcopy(original)
        template = manifest["cases"][0]
        manifest["cases"] = []
        for name, endpoint in (("valid", "valid"), ("invalid", "invalid")):
            case = dict(template)
            case.update(self.asset(endpoint), name=name, file=f"images/{name}.png", references=[])
            manifest["cases"].append(case)
        manifest["references"] = []
        manifest["downloads"] = {}
        manifest["acceptance"].update(input_count=2, min_chinese_inputs=0, min_input_sources=1)
        path = self.root / "manifest.json"
        path.write_text(json.dumps(manifest, ensure_ascii=False), encoding="utf-8")
        report = self.root / "report.json"
        result = subprocess.run([sys.executable, "-X", "utf8", str(SCRIPT), "fetch",
                                 "--manifest", str(path), "--report", str(report), "--retries", "0"],
                                capture_output=True, timeout=15)
        self.assertEqual(result.returncode, 1, result.stdout.decode("utf-8"))
        data = json.loads(report.read_text(encoding="utf-8"))
        self.assertFalse(data["passed"])
        self.assertEqual(data["counts"], {"downloaded": 1, "failed": 1})
        self.assertTrue((self.root / "images/valid.png").exists())
        self.assertFalse((self.root / "images/invalid.png").exists())
        self.assertFalse((self.root / "README.md").exists())


if __name__ == "__main__":
    unittest.main()
