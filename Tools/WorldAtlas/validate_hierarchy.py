"""Check editable atlas trees and national participant flags before starting Unreal."""
import json
from pathlib import Path
import sys

CONTENT = Path(__file__).resolve().parents[2] / 'Plugins/TMOPEngine/Content/WorldAtlas'


def validate_entry(entry, flag_ids):
    for participant in entry.get('participants', []):
        if 'flag' in participant and participant['flag'] not in flag_ids:
            raise ValueError(f"{entry['id']}: unknown flag {participant['flag']}")
    tree = entry.get('hierarchy')
    if tree is None:
        return
    if tree['as_of'] != '1986-02-28' or not 1 <= len(tree['nodes']) <= 512:
        raise ValueError(f"{entry['id']}: wrong date or node count")
    nodes = {node['id']: node for node in tree['nodes']}
    if len(nodes) != len(tree['nodes']) or '' in nodes:
        raise ValueError(f"{entry['id']}: duplicate or empty office ID")
    for node in nodes.values():
        if node['relation'] not in {'group', 'reports_to'}:
            raise ValueError(f"{entry['id']}: unknown relationship")
        if node['label'] not in entry['text'] or ('note' in node and node['note'] not in entry['text']):
            raise ValueError(f"{entry['id']}: missing localized office text")
        if any(not url.startswith('https://') for url in node.get('sources', [])):
            raise ValueError(f"{entry['id']}: invalid source URL")
        if node['relation'] == 'reports_to' and (not node['parent'] or not node.get('sources')):
            raise ValueError(f"{entry['id']}: reporting line needs a parent and source")
        seen = set()
        current = node['id']
        while current:
            if current not in nodes:
                raise ValueError(f"{entry['id']}: missing parent {current}")
            if current in seen or len(seen) >= 32:
                raise ValueError(f"{entry['id']}: cyclic or excessively deep tree")
            seen.add(current)
            current = nodes[current]['parent']


def main():
    world_path = Path(sys.argv[1]) if len(sys.argv) > 1 else CONTENT / 'world.json'
    flags = {row['id'] for row in json.loads((world_path.parent / 'flags_1986.json').read_text())['flags']}
    entries = json.loads(world_path.read_text())['entries']
    for entry in entries:
        validate_entry(entry, flags)
    print(f"Validated {sum('hierarchy' in e for e in entries)} trees and all participant flag references.")


if __name__ == '__main__':
    main()
