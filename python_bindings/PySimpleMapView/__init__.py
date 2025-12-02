import os
import platform
import subprocess

try:
    import PySide6.QtCore
    import PySide6.QtWidgets
    import PySide6.QtPositioning
    import PySide6.QtNetwork
except ImportError:
    print("CRITICAL ERROR: PySimpleMapView requires 'PySide6'. Please run: pip install PySide6")
    raise

if platform.system() == "Windows":
    qt_bin_path = None

    try:
        result = subprocess.run(["qmake", "-query", "QT_INSTALL_BINS"], 
                                capture_output=True, text=True, shell=True)
        if result.returncode == 0:
            path = result.stdout.strip()
            if os.path.exists(path):
                qt_bin_path = path
    except Exception:
        pass

    if not qt_bin_path:
        for env_var in ['QT_DIR', 'QT6_DIR', 'Qt6_DIR']:
            root_path = os.environ.get(env_var)
            if root_path:
                candidate = os.path.join(root_path, "bin")
                if os.path.exists(candidate):
                    qt_bin_path = candidate
                    break

    if qt_bin_path and hasattr(os, 'add_dll_directory'):
        try:
            os.add_dll_directory(qt_bin_path)
        except Exception as e:
            print(f"Warning: Failed to add Qt DLL directory: {e}")

from .PySimpleMapView import *