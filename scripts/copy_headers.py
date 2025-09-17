#!/usr/bin/env python3
"""
Copy header-only libraries to external directory after bootstrapping
"""

import os
import shutil
from pathlib import Path

def main():
    external_dir = Path('external')
    src_dir = external_dir / 'src'
    
    # Copy tiny_obj_loader.h
    tiny_obj_src = src_dir / 'tiny_obj_loader' / 'tiny_obj_loader.h'
    if tiny_obj_src.exists():
        shutil.copy2(tiny_obj_src, external_dir / 'tiny_obj_loader.h')
        print("Copied tiny_obj_loader.h")
    
    # Copy stb_image.h
    stb_src = src_dir / 'stb' / 'stb_image.h'
    if stb_src.exists():
        shutil.copy2(stb_src, external_dir / 'stb_image.h')
        print("Copied stb_image.h")
    
    # Copy stb_image_write.h
    stb_write_src = src_dir / 'stb' / 'stb_image_write.h'
    if stb_write_src.exists():
        shutil.copy2(stb_write_src, external_dir / 'stb_image_write.h')
        print("Copied stb_image_write.h")

if __name__ == '__main__':
    main()
