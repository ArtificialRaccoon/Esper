#!/usr/bin/env python3
import json
import os
import struct
import sys

def main():
    try:
        with open("db.json", "r") as f:
            data = json.load(f)
    except FileNotFoundError:
        sys.exit("Error: db.json not found")
        
    version = data.get("version", 1)
    start_map = data.get("start_map", "")
    start_x = data.get("start_x", 0)
    start_y = data.get("start_y", 0)
    maps = data.get("maps", [])
    
    # Resolve starting map name/index
    if isinstance(start_map, str):
        if start_map not in maps:
            sys.exit(f"Error: start_map '{start_map}' not found in maps list")
        start_map_index = maps.index(start_map)
    else:
        start_map_index = start_map if isinstance(start_map, int) else 0
        
    out_path = "assets/GAMEDB.BIN"
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    
    with open(out_path, "wb") as fout:
        # Header layout (little endian):
        # char magic[4] ("BEAR"), uint16_t version, mapCount, startMapIndex, startTileX, startTileY, reserved (0)
        fout.write(struct.pack("<4sHHHHHH", b"BEAR", version, len(maps), start_map_index, start_x, start_y, 0))
        # Write map filename records (each 32 bytes, null-padded/terminated)
        for map_name in maps:
            fout.write(map_name.encode("ascii", errors="replace")[:31].ljust(32, b"\0"))
            
    print(f"Database compiled to {out_path}. Maps count: {len(maps)}")

if __name__ == "__main__":
    main()
