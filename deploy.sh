# Path Configuration
root_folder="/home/yoyofun/PacMan-CRC48"
dest_folder="$root_folder/PacMan"
exe_name="CRC48-PacMan"
release_build="$root_folder/build"

echo "[1/4] Creating destination folder..."
mkdir -p "$dest_folder"

echo "[2/4] Copying resources and executable..."
cp -f "$root_folder/Map.txt" "$dest_folder/Map.txt"
cp -f "$root_folder/yo_stylesheet.qss" "$dest_folder/yo_stylesheet.qss"
cp -f "$release_build/$exe_name" "$dest_folder/$exe_name"

echo "[3/4] Running linuxdeployqt..."

echo ""
echo "=========================================="
echo "Deployment Complete!"
echo "Output Path: $dest_folder"
echo "=========================================="
read -p "Press enter to continue"
