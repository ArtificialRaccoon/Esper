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

    exits = []
    if (exits_group := tmx_root.find(".//objectgroup[@name='EXITS']")) is not None:
        for obj in exits_group.findall("object"):
            x, y = int(float(obj.get("x", 0))), int(float(obj.get("y", 0)))
            w, h = int(float(obj.get("width", 0))), int(float(obj.get("height", 0)))
            props = {p.get("name", "").lower(): p.get("value", "") for p in obj.findall("properties/property")}
            exits.append({
                "target_map_id": props.get("targetmapid", ""),
                "startTx": x // tilewidth,
                "startTy": y // tileheight,
                "endTx": (x + w - 1) // tilewidth,
                "endTy": (y + h - 1) // tileheight,
                "targetX": int(props.get("targetx", 0)),
                "targetY": int(props.get("targety", 0)),
            })

    with out_path.open("wb") as fout:
        fout.write(struct.pack("<3H", len(tile_layers), len(animations), len(exits)))

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

        for ext in exits:
            fout.write(encode_name(ext["target_map_id"]))
            fout.write(struct.pack("<6H", ext["startTx"], ext["startTy"], ext["endTx"], ext["endTy"], ext["targetX"], ext["targetY"]))

    print(f"Layers: {len(tile_layers)}, Animations: {len(animations)}, Exits: {len(exits)} -> {out_path}")

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
