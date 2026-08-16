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
        "EVENT_TOUCH": 2,
        "AUTORUN": 3,
        "PARALLEL": 4,
        "NONE": 5,
    }

    MOVE_TYPE_MAP = {
        "FIXED": 0,
        "RANDOM_WANDER": 1,
        "PATH_FOLLOW": 2,
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
    
    paths_json_path = Path(__file__).parent.parent / "assets" / "PATHS.json"
    path_map = {}
    if paths_json_path.is_file():
        try:
            with open(paths_json_path, "r", encoding="utf-8") as f:
                path_map = json.load(f)
        except Exception as e:
            print(f"Warning: Failed to parse {paths_json_path}: {e}")
    
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
            
            obj_type = (obj.get("type") or obj.get("class") or "").strip().upper()
            is_actor = False
            if obj_type in ("NPC", "ACTOR"):
                is_actor = True
            elif props.get("isActor") is True or str(props.get("eventType", "")).strip().upper() == "ACTOR":
                is_actor = True
            else:
                for page in pages:
                    sname = str(page.get("spriteName", "")).strip()
                    sframe = int(page.get("spriteFrame", 0))
                    mtype = str(page.get("moveType", "FIXED")).strip().upper()
                    if (sname and sframe == 0) or mtype != "FIXED":
                        is_actor = True
                        break
            
            event_type = 1 if is_actor else 0

            events.append({
                "eventId": event_id,
                "eventType": event_type,
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
            fout.write(struct.pack("<HBhhhhH", event["eventId"], event["eventType"], event["tileX"], event["tileY"], event["endTileX"], event["endTileY"], len(event["pages"])))
            
            for page in event["pages"]:
                trigger_str = page.get("trigger", "NONE")
                trigger_val = TRIGGER_MAP.get(trigger_str, 5)
                graphic_frame = int(page.get("spriteFrame", 0))
                is_walkable = int(page.get("isWalkable", 1))
                var_threshold = int(page.get("variableThreshold", 0))
                
                move_type_str = page.get("moveType", "FIXED")
                move_type_val = MOVE_TYPE_MAP.get(move_type_str, 0)
                move_speed = int(page.get("moveSpeed", 1))
                move_frequency = int(page.get("moveFrequency", 30))
                
                def clean_cond(val):
                    if val is None:
                        return ""
                    s = str(val).strip()
                    if s == "0" or s == "":
                        return ""
                    return s

                move_path_bytes = clean_cond(page.get("movePath", "")).encode("ascii", errors="replace")[:15].ljust(16, b"\0")
                switch_cond_bytes = clean_cond(page.get("switchCondition", "")).encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                var_cond_bytes = clean_cond(page.get("variableCondition", "")).encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                self_var_cond_bytes = clean_cond(page.get("selfVariableCondition", "")).encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                self_switch_cond_bytes = clean_cond(page.get("selfSwitchCondition", "")).encode("ascii", errors="replace")[:7].ljust(8, b"\0")
                sprite_name_bytes = clean_cond(page.get("spriteName", "")).encode("ascii", errors="replace")[:7].ljust(8, b"\0")
                
                def expand_command_list(c_list):
                    expanded = []
                    for c_prop in c_list:
                        if isinstance(c_prop, dict):
                            c_type = c_prop.get("type", "NONE")
                            if c_type == "SHOW_TEXT":
                                if (inc_self := c_prop.get("incSelfVar", "")):
                                    expanded.append({"type": "CONTROL_SELF_VAR", "varName": inc_self, "op": "ADD", "value": 1})
                                elif (self_sw := c_prop.get("selfSwitch", "")):
                                    expanded.append({"type": "CONTROL_SELF_SWITCH", "switch": self_sw, "value": True})
                                elif (inc_v := c_prop.get("incVar", "")):
                                    expanded.append({"type": "CONTROL_VAR", "varName": inc_v, "op": "ADD", "value": 1})
                                expanded.append(c_prop)
                            else:
                                expanded.append(c_prop)
                    return expanded

                def write_raw_cmd(c_type, c_op, c_val, c_extra, c_str):
                    str_bytes = c_str.encode("ascii", errors="replace")[:23].ljust(24, b"\0")
                    fout.write(struct.pack("<BBhh24s2x", c_type, c_op, c_val, c_extra, str_bytes))

                def serialize_command(c_prop):
                    c_type = c_prop.get("type", "NONE")
                    if c_type == "SHOW_TEXT":
                        text = c_prop.get("text", "")
                        string_idx = string_map.get(text, -1)
                        write_raw_cmd(1, 0, string_idx, 0, "")
                    elif c_type == "CONTROL_SELF_SWITCH":
                        sw = c_prop.get("switch", "A")
                        val = 1 if c_prop.get("value", True) else 0
                        write_raw_cmd(2, val, 0, 0, sw)
                    elif c_type == "CONTROL_SELF_VAR":
                        var_name = c_prop.get("varName", "")
                        op_val = 1 if c_prop.get("op", "ADD") == "ADD" else 0
                        val = c_prop.get("value", 1)
                        write_raw_cmd(3, op_val, val, 0, var_name)
                    elif c_type == "CONTROL_VAR":
                        var_name = c_prop.get("varName", "")
                        op_val = 1 if c_prop.get("op", "ADD") == "ADD" else 0
                        val = c_prop.get("value", 1)
                        write_raw_cmd(4, op_val, val, 0, var_name)
                    elif c_type == "PLAY_SFX":
                        sfx = c_prop.get("sfxName", "")
                        write_raw_cmd(5, 0, 0, 0, sfx)
                    elif c_type == "TRANSFER" or c_type == "TRANSFER_PLAYER":
                        m_name = c_prop.get("mapName", "")
                        tx = c_prop.get("tileX", 0)
                        ty = c_prop.get("tiley", c_prop.get("tileY", 0))
                        write_raw_cmd(6, 0, tx, ty, m_name)
                    elif c_type == "SET_MOVE_ROUTE":
                        target_id = int(c_prop.get("targetId", 0))
                        bypass_col = 1 if c_prop.get("bypassCollision", False) else 0
                        path_str = c_prop.get("movePath", "")
                        path_id = path_map.get(path_str, -1)
                        if path_id == -1 and path_str:
                            print(f"Warning: Move path '{path_str}' not found in PATHS.json. Make sure to compile paths.")
                        write_raw_cmd(7, bypass_col, target_id, path_id, "")
                    elif c_type == "WAIT":
                        frames = int(c_prop.get("frames", 60))
                        write_raw_cmd(8, 0, frames, 0, "")
                    elif c_type == "WAIT_FOR_MOVEMENT":
                        target_id = int(c_prop.get("targetId", 0))
                        write_raw_cmd(9, 0, target_id, 0, "")
                    elif c_type == "FADE_OUT":
                        speed = int(c_prop.get("speed", 2))
                        write_raw_cmd(10, 0, speed, 0, "")
                    elif c_type == "FADE_IN":
                        speed = int(c_prop.get("speed", 2))
                        write_raw_cmd(11, 0, speed, 0, "")
                    elif c_type == "PLAY_BGM":
                        bgm = c_prop.get("bgmName", "")
                        write_raw_cmd(12, 0, 0, 0, bgm)
                    elif c_type == "SET_FACING":
                        target_id = int(c_prop.get("targetId", 0))
                        dir_str = str(c_prop.get("direction", "DOWN")).strip().upper()
                        DIR_MAP = { "DOWN": 0, "RIGHT": 1, "UP": 2, "LEFT": 3 }
                        dir_val = DIR_MAP.get(dir_str, 0)
                        write_raw_cmd(13, dir_val, target_id, 0, "")
                    elif c_type == "SET_SPEED":
                        target_id = int(c_prop.get("targetId", 0))
                        speed = int(c_prop.get("speed", 2))
                        write_raw_cmd(14, speed, target_id, 0, "")
                    elif c_type == "SHOW_CHOICES":
                        raw_choices = c_prop.get("choices", [])
                        if not isinstance(raw_choices, list):
                            raw_choices = []
                        write_raw_cmd(15, len(raw_choices), 0, 0, "")
                        for opt in raw_choices:
                            if isinstance(opt, dict):
                                opt_text = opt.get("text", "")
                                str_idx = string_map.get(opt_text, -1)
                                sub_cmds = opt.get("commands", [])
                                if not isinstance(sub_cmds, list):
                                    sub_cmds = []
                                expanded_subs = expand_command_list(sub_cmds)
                                fout.write(struct.pack("<HH", str_idx, len(expanded_subs)))
                                for sc in expanded_subs:
                                    serialize_command(sc)
                            elif isinstance(opt, str):
                                str_idx = string_map.get(opt, -1)
                                fout.write(struct.pack("<HH", str_idx, 0))

                cmd_list = page.get("commands", [])
                if not isinstance(cmd_list, list):
                    cmd_list = []
                single_cmd = page.get("command", None)
                if isinstance(single_cmd, dict):
                    cmd_list.append(single_cmd)

                expanded_page_cmds = expand_command_list(cmd_list)
                command_count = len(expanded_page_cmds)
                fout.write(struct.pack("<BBBhBBB16s24s24s24s8s8sH", trigger_val, graphic_frame, is_walkable, var_threshold, move_type_val, move_speed, move_frequency, move_path_bytes, switch_cond_bytes, var_cond_bytes, self_var_cond_bytes, self_switch_cond_bytes, sprite_name_bytes, command_count))

                for cmd in expanded_page_cmds:
                    serialize_command(cmd)

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
