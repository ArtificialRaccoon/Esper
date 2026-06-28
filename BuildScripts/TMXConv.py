#!/usr/bin/env python3
import argparse
import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

def encode_name(name: str) -> bytes:
    return name.encode("ascii", errors="replace")[:8].ljust(8, b" ")

def read_animations(tsx_path: Path, firstgid: int) -> list:
    anims = []
    for tile in ET.parse(tsx_path).findall(".//tile"):
        if (anim := tile.find("animation")) is not None:
            frames = [
                (int(f.get("tileid")) + firstgid, int(f.get("duration")))
                for f in anim.findall("frame")
            ]
            if frames:
                anims.append((int(tile.get("id")) + firstgid, frames))
    return anims

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

def convert(tmx_path: Path, out_path: Path) -> None:
    try:
        tmx_root = ET.parse(tmx_path).getroot()
    except Exception as e:
        sys.exit(f"Error: Failed to parse TMX file '{tmx_path}': {e}")

    tile_layers = tmx_root.findall(".//layer")
    if not tile_layers:
        sys.exit("Error: no tile layers found in map.")

    tsx_path = tmx_path.with_suffix(".tsx")
    if not tsx_path.is_file():
        sys.exit(f"Error: '{tsx_path}' not found.")

    firstgid = next(
        (int(ts.get("firstgid", 1)) for ts in tmx_root.findall(".//tileset")
         if Path(ts.get("source", "")).stem == tsx_path.stem),
        1
    )

    animations = read_animations(tsx_path, firstgid)
    tilewidth = int(tmx_root.get("tilewidth", 16))
    tileheight = int(tmx_root.get("tileheight", 16))

    TRIGGER_MAP = {
        "ACTION_BUTTON": 0,
        "PLAYER_TOUCH": 1,
        "AUTORUN": 2,
        "NONE": 3,
    }

    import json
    strings_json_path = Path(__file__).parent.parent / "assets" / "STRINGS.json"
    string_map = {}
    if strings_json_path.is_file():
        try:
            with open(strings_json_path, "r", encoding="utf-8") as f:
                string_map = json.load(f)
        except Exception as e:
            print(f"Warning: Failed to parse {strings_json_path}: {e}")
    
    events = []
    if (events_group := tmx_root.find(".//objectgroup[@name='EVENTS']")) is not None:
        for obj in events_group.findall("object"):
            x, y = int(float(obj.get("x", 0))), int(float(obj.get("y", 0)))
            w, h = int(float(obj.get("width", 0))), int(float(obj.get("height", 0)))
            
            event_id = int(obj.get("id", 0))
            tile_x = x // tilewidth
            tile_y = y // tileheight
            tile_w = max(1, w // tilewidth)
            tile_h = max(1, h // tileheight)
            end_tile_x = tile_x + tile_w - 1
            end_tile_y = tile_y + tile_h - 1
            
            props = parse_properties(obj)
            pages = props.get("Pages", [])
            
            events.append({
                "eventId": event_id,
                "tileX": tile_x,
                "tileY": tile_y,
                "endTileX": end_tile_x,
                "endTileY": end_tile_y,
                "pages": pages
            })
            

    with out_path.open("wb") as fout:
        fout.write(struct.pack("<3H", len(tile_layers), len(animations), len(events)))

        for tile_gid, frames in animations:
            flat = [val for f in frames for val in f]
            fout.write(struct.pack(f"<2H{len(flat)}H", tile_gid, len(frames), *flat))

        for layer in tile_layers:
            name = layer.get("name", "")
            width, height = int(layer.get("width")), int(layer.get("height"))

            if (data_el := layer.find("data")) is None:
                sys.exit(f"Error: layer '{name}' has no data element.")

            if (encoding := data_el.get("encoding", "")) != "csv":
                sys.exit(f"Error: layer '{name}' data is not CSV encoded (found '{encoding}'). Only CSV is supported.")

            data = [int(val) for val in (data_el.text or "").split(",") if val.strip()]

            if len(data) != width * height:
                sys.exit(f"Error: layer '{name}' data size mismatch. Expected {width * height}, got {len(data)}.")

            fout.write(encode_name(name) + struct.pack(f"<2H{len(data)}H", width, height, *data))

        for event in events:    
            fout.write(struct.pack("<HhhhhH", event["eventId"], event["tileX"], event["tileY"], event["endTileX"], event["endTileY"], len(event["pages"])))
            
            for page in event["pages"]:
                trigger_str = page.get("trigger", "NONE")
                trigger_val = TRIGGER_MAP.get(trigger_str, 3)
                graphic_frame = int(page.get("spriteFrame", 0))
                is_walkable = int(page.get("isWalkable", 1))
                var_threshold = int(page.get("variableThreshold", 0))
                
                def clean_cond(val):
                    if val is None or val == 0 or val == "0" or val == "":
                        return ""
                    return str(val)

                switch_cond_bytes = clean_cond(page.get("switchCondition", "")).encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                var_cond_bytes = clean_cond(page.get("variableCondition", "")).encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                self_switch_cond_bytes = clean_cond(page.get("selfSwitchCondition", "")).encode("ascii", errors="replace")[:7].ljust(8, b"\0")
                sprite_name_bytes = clean_cond(page.get("spriteName", "")).encode("ascii", errors="replace")[:7].ljust(8, b"\0")
                
                cmd_prop = page.get("command", "")
                if isinstance(cmd_prop, dict):
                    cmd_type = cmd_prop.get("type", "NONE")
                    if cmd_type == "TRANSFER":
                        map_name = cmd_prop.get("mapName", "")
                        tx = cmd_prop.get("tileX", 0)
                        ty = cmd_prop.get("tiley", cmd_prop.get("tileY", 0))
                        cmd_str = f"transfer {map_name} {tx} {ty}"
                    elif cmd_type == "PLAY_SFX":
                        sfx_name = cmd_prop.get("sfxName", "")
                        cmd_str = f"play_sfx {sfx_name}"
                    elif cmd_type == "SHOW_TEXT":
                        text = cmd_prop.get("text", "")
                        string_idx = string_map.get(text, -1)
                        cmd_str = f"show_text {string_idx}"
                    elif cmd_type == "OPEN_CHEST":
                        self_switch = cmd_prop.get("selfSwitch", "A")
                        text = cmd_prop.get("text", "")
                        string_idx = string_map.get(text, -1)
                        cmd_str = f"open_chest {self_switch} {string_idx}"
                    else:
                        cmd_str = "none"
                else:
                    cmd_str = str(cmd_prop) if cmd_prop is not None else ""
                
                cmd_bytes = cmd_str.encode("ascii", errors="replace")[:63].ljust(64, b"\0")                                
                fout.write(struct.pack("<BBBh24s24s8s8s64s", trigger_val, graphic_frame, is_walkable, var_threshold, switch_cond_bytes, var_cond_bytes, self_switch_cond_bytes, sprite_name_bytes, cmd_bytes))

    print(f"Layers: {len(tile_layers)}, Animations: {len(animations)}, Events: {len(events)} -> {out_path}")

def main() -> None:
    ap = argparse.ArgumentParser(description="Convert a Tiled .tmx+.tsx pair to a binary tilemap.")
    ap.add_argument("input", type=Path, help="Source .tmx file")
    ap.add_argument("output", type=Path, nargs="?", help="Output file (default: <input>.bin)")
    args = ap.parse_args()

    if not args.input.is_file():
        sys.exit(f"Error: '{args.input}' not found.")

    convert(args.input, args.output or args.input.with_suffix(".bin"))

if __name__ == "__main__":
    main()
