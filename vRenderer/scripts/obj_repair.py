import os
import sys
import re
from difflib import SequenceMatcher

# Search for .obj file (3d mesh) in input folder.
# Check if this file has references to .mtl file (materials used by this mesh).
# If it does have references - locate this .mtl file in the same folder.
# If it has references to texture files, check if these texture files exist in any subfolder of input folder (referenced files might be broken and absolute not being relevant for running system)
# If there are such files, repair reference paths to them in .mtl file making them local and relevant If any of the step fails script should abort and log the reason to console

def similarity_ratio(a, b):
    # Remove common extensions before comparing
    a = os.path.splitext(a.lower())[0]
    b = os.path.splitext(b.lower())[0]
    return SequenceMatcher(None, a, b).ratio()

def find_similar_textures(target_texture, available_textures, threshold=0.6):
    """Find similar texture names above threshold similarity."""
    target_base = os.path.basename(target_texture.lower())
    similarities = []
    
    for tex_path in available_textures.values():
        tex_base = os.path.basename(tex_path.lower())
        ratio = similarity_ratio(target_base, tex_base)
        if ratio >= threshold:
            similarities.append((tex_path, ratio))
    
    # Sort by similarity ratio, highest first
    return sorted(similarities, key=lambda x: x[1], reverse=True)

def prompt_texture_replacement(original_tex, similar_textures):
    """Ask user whether to use a similar texture as replacement."""
    print(f"\nLooking for texture: {original_tex}")
    
    if not similar_textures:
        print("No similar textures found.")
        return None
        
    print("Similar textures found:")
    for idx, (tex_path, ratio) in enumerate(similar_textures, 1):
        print(f"{idx}. {tex_path} (similarity: {ratio:.2%})")
    
    while True:
        choice = input("\nSelect texture number to use as replacement (or 'n' to skip): ").strip().lower()
        if choice == 'n':
            return None
        try:
            choice_idx = int(choice) - 1
            if 0 <= choice_idx < len(similar_textures):
                return similar_textures[choice_idx][0]
        except ValueError:
            pass
        print("Invalid choice. Please try again.")
        
def log_and_abort(message):
    print(f"[ABORT] {message}")
    sys.exit(1)

def get_obj_file(folder):
    obj_files = [f for f in os.listdir(folder) if f.lower().endswith('.obj')]
    if not obj_files:
        log_and_abort("No .obj file found in the given folder.")
    if len(obj_files) > 1:
        log_and_abort("Multiple .obj files found. Please ensure only one .obj file is present.")
    return os.path.join(folder, obj_files[0])

def find_mtl_reference(obj_file_path):
    with open(obj_file_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            if line.strip().lower().startswith('mtllib'):
                # Format: mtllib file.mtl
                parts = line.strip().split()
                if len(parts) > 1:
                    return parts[1]
    return None

def get_mtl_file(folder, mtl_filename):
    mtl_path = os.path.join(folder, mtl_filename)
    if not os.path.isfile(mtl_path):
        log_and_abort(f"Referenced .mtl file '{mtl_filename}' not found in the folder.")
    return mtl_path

def collect_texture_files(folder):
    texture_files = dict()
    for root, _, files in os.walk(folder):
        for fname in files:
            # Any common image format (extend if needed)
            if fname.lower().endswith(('.jpg', '.jpeg', '.png', '.bmp', '.tga', '.dds', '.tiff', '.tif')):
                texture_files[fname.lower()] = os.path.relpath(os.path.join(root, fname), folder)
    return texture_files

def parse_and_fix_mtl(mtl_path, folder, texture_files):
    map_line_re = re.compile(r"^(map_\w+)\s+(.+)$", re.IGNORECASE)
    updated_lines = []
    replacements_made = False
    missing_textures = []
    
    # First pass - collect all missing textures
    with open(mtl_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = map_line_re.match(line.strip())
            if match:
                _, tex_path = match.groups()
                tex_filename = os.path.basename(tex_path).lower()
                if tex_filename not in texture_files:
                    missing_textures.append(tex_path)

    # Create replacement dictionary through user interaction
    texture_replacements = {}
    if missing_textures:
        print("\n[WARNING] Some textures are missing. Starting interactive replacement process...")
        for missing_tex in missing_textures:
            similar = find_similar_textures(missing_tex, texture_files)
            replacement = prompt_texture_replacement(missing_tex, similar)
            if replacement:
                texture_replacements[missing_tex.lower()] = replacement
                replacements_made = True
            else:
                print(f"No replacement selected for {missing_tex}")

    # Second pass - apply replacements and write file
    with open(mtl_path, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = map_line_re.match(line.strip())
            if match:
                keyword, tex_path = match.groups()
                tex_filename = os.path.basename(tex_path).lower()
                
                if tex_filename in texture_files:
                    # Direct match found
                    new_rel_path = texture_files[tex_filename].replace("\\", "/")
                    updated_line = f"{keyword} {new_rel_path}\n"
                    print(f"[INFO] Direct match - Rewriting {keyword}: {tex_path} -> {new_rel_path}")
                elif tex_path.lower() in texture_replacements:
                    # Using replacement texture
                    new_rel_path = texture_replacements[tex_path.lower()].replace("\\", "/")
                    updated_line = f"{keyword} {new_rel_path}\n"
                    print(f"[INFO] Using replacement - Rewriting {keyword}: {tex_path} -> {new_rel_path}")
                else:
                    # No match or replacement found
                    print(f"[WARNING] No replacement found for texture: {tex_path}")
                    updated_line = line
                updated_lines.append(updated_line)
            else:
                updated_lines.append(line)

    if not replacements_made and missing_textures:
        log_and_abort("No valid texture replacements were selected for missing textures.")

    # Write back the updated .mtl file
    with open(mtl_path, 'w', encoding='utf-8') as fout:
        fout.writelines(updated_lines)
    print(f"\n[SUCCESS] .mtl file '{os.path.basename(mtl_path)}' has been updated with correct texture paths.")

def main():
    if len(sys.argv) != 2:
        print("Usage: python script.py <folder_path>")
        sys.exit(1)
    folder = sys.argv[1]
    if not os.path.isdir(folder):
        log_and_abort("Provided path is not a valid folder.")
    
    print(f"[INFO] Working in folder: {folder}")
    
    obj_file = get_obj_file(folder)
    print(f"[INFO] Found OBJ file: {os.path.basename(obj_file)}")
    
    mtl_filename = find_mtl_reference(obj_file)
    if not mtl_filename:
        log_and_abort("The .obj file does not reference any .mtl file.")
    print(f"[INFO] .obj references .mtl file: {mtl_filename}")
    
    mtl_file = get_mtl_file(folder, mtl_filename)
    
    texture_files = collect_texture_files(folder)
    if not texture_files:
        print("[WARNING] No texture files found in the folder or its subfolders.")
    else:
        print(f"[INFO] Found {len(texture_files)} texture file(s) in the folder and subfolders.")
    
    parse_and_fix_mtl(mtl_file, folder, texture_files)
    print("[DONE] Processing complete.")

if __name__ == "__main__":
    main()