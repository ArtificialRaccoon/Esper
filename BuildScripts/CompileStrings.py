#!/usr/bin/env python3
import json
import os
import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

def parse_value(el, val_type):
    if val_type == "class":
        return parse_properties(el)
    elif val_type == "list":
        return [parse_value(item, item.get("type", "string")) for item in el.findall("item")]
    
    val = el.get("value")
    if val is None:
        val = el.text or ""
    if val_type == "int":
        try:
            return int(val)
        except ValueError:
            return 0
    elif val_type == "bool":
        return val == "true"
    return val

def parse_properties(el):
    props = {}
    properties_el = el.find("properties")
    if properties_el is not None:
        for prop in properties_el.findall("property"):
            name = prop.get("name")
            prop_type = prop.get("type", "string")
            props[name] = parse_value(prop, prop_type)
    return props

def extract_strings_from_commands(cmd_list, strings_set):
    if not isinstance(cmd_list, list):
        return
    for cmd_prop in cmd_list:
        if isinstance(cmd_prop, dict):
            cmd_type = cmd_prop.get("type", "NONE")
            if cmd_type == "SHOW_TEXT":
                text = cmd_prop.get("text", "")
                if text:
                    strings_set.add(text)
            elif cmd_type == "SHOW_CHOICES":
                choices = cmd_prop.get("choices", [])
                if isinstance(choices, list):
                    for opt in choices:
                        if isinstance(opt, dict):
                            opt_text = opt.get("text", "")
                            if opt_text:
                                strings_set.add(opt_text)
                            opt_cmds = opt.get("commands", [])
                            extract_strings_from_commands(opt_cmds, strings_set)
                        elif isinstance(opt, str):
                            if opt:
                                strings_set.add(opt)

def main():
    raw_asset_dir = Path("RawAssetData")
    if not raw_asset_dir.is_dir():
        sys.exit("Error: RawAssetData directory not found")

    tmx_files = list(raw_asset_dir.glob("*.tmx"))
    
    strings = set()

    for tmx_path in tmx_files:
        try:
            tmx_root = ET.parse(tmx_path).getroot()
        except Exception as e:
            print(f"Warning: Failed to parse TMX file '{tmx_path}': {e}")
            continue

        if (events_group := tmx_root.find(".//objectgroup[@name='EVENTS']")) is not None:
            for obj in events_group.findall("object"):
                props = parse_properties(obj)
                pages = props.get("Pages", [])
                for page in pages:
                    cmd_list = page.get("commands", [])
                    if not isinstance(cmd_list, list):
                        cmd_list = []
                    single_cmd = page.get("command", None)
                    if isinstance(single_cmd, dict):
                        cmd_list.append(single_cmd)
                    extract_strings_from_commands(cmd_list, strings)

    sorted_strings = sorted(list(strings))
    string_count = len(sorted_strings)

    assets_dir = Path("assets")
    os.makedirs(assets_dir, exist_ok=True)

    json_path = assets_dir / "STRINGS.json"
    bin_path = assets_dir / "STRINGS.BIN"

    string_map = {text: idx for idx, text in enumerate(sorted_strings)}
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(string_map, f, indent=2)

    lookup_table = []
    string_data_bytes = bytearray()
    
    current_offset = 2 + string_count * 6

    for idx, text in enumerate(sorted_strings):
        encoded = text.encode("ascii", errors="replace")
        length = len(encoded)
        
        lookup_table.append((idx, current_offset))

        string_data_bytes.extend(struct.pack("<H", length))
        string_data_bytes.extend(encoded)

        current_offset += 2 + length

    with open(bin_path, "wb") as fout:
        fout.write(struct.pack("<H", string_count))
        for idx, offset in lookup_table:
            fout.write(struct.pack("<HI", idx, offset))
        fout.write(string_data_bytes)

    print(f"Strings compiled successfully. Count: {string_count}")
    print(f"  -> JSON: {json_path}")
    print(f"  -> BIN:  {bin_path}")

if __name__ == "__main__":
    main()
