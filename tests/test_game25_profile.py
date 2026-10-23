"""Regression checks for the isolated 2.5 address/layout profile."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts"))
import gen_macos_layouts as layouts
import plugin_requirements


class ProfileTests(unittest.TestCase):
    def test_unported_hashes_remain_visible_to_preflight(self):
        baseline = json.loads((ROOT / "cyberpunk2077_addresses.json").read_text())
        profile = json.loads((ROOT / "data/addresses/2.5.json").read_text())
        entries = {int(e["hash"]): e for e in profile["Addresses"]}
        self.assertEqual(len(entries), len(profile["Addresses"]))
        self.assertTrue({int(e["hash"]) for e in baseline["Addresses"]} <= entries.keys())
        unresolved = {h for h, e in entries.items() if not e.get("verified")}
        self.assertTrue(unresolved)
        for h in unresolved:
            self.assertEqual(int(entries[h]["offset"].split(":")[1], 16), 0)
        # Simulate compiled constants: an unported requirement must be reported.
        with patch.object(plugin_requirements, "constants", return_value=unresolved), \
             patch.object(Path, "read_bytes", return_value=b""):
            required = plugin_requirements.requirements(Path("plugin.dylib"), profile)
        self.assertEqual({h for h, verified in required if not verified}, unresolved)

    def test_layout_verification_fails_for_non_assert_compile_errors(self):
        with tempfile.TemporaryDirectory() as tmp, \
             patch.object(layouts, "GENERATED", Path(tmp)), \
             patch.object(layouts.subprocess, "run", return_value=subprocess.CompletedProcess(
                 [], 1, "", "profile.cpp:1: error: unknown type name 'BadType'\n")):
            self.assertNotEqual(layouts.verify(), 0)

    def test_changed_scriptable_parent_is_generated_and_repeatable(self):
        source = ROOT / "include/RED4ext/Scripting/Natives/Generated/ink/IEffect.hpp"
        dump = json.loads((ROOT / "data/rtti_layout_macos_25.json").read_text())
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp, "IEffect.hpp")
            target.write_text(source.read_text())
            def generate():
                header = layouts.parse(target)
                planner = layouts.Planner(dump, {header.rtti: header}, {"IScriptable": (0x40, 8, 0x40)})
                planner.clang(header.rtti)
                return layouts.render(header, planner.plans[header.rtti])
            first = generate()
            self.assertIn("struct IEffect : IScriptable", first)
            self.assertIn("#else\nstruct IEffect : ISerializable", first)
            target.write_text(first)
            self.assertEqual(generate(), first)


if __name__ == "__main__":
    unittest.main()
