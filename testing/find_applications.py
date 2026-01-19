#!/usr/bin/env python3
"""
Script to find all applications on a Linux system, their descriptions, and icons.
Uses GTK's GIO library to query the desktop environment directly.
"""

import gi
gi.require_version('Gio', '2.0')
gi.require_version('Gtk', '3.0')

from gi.repository import Gio, Gtk
from pathlib import Path


def get_icon_file_path(icon):
    """
    Get the actual file path for an icon.
    Handles both FileIcon (direct file path) and ThemedIcon (theme-based icons).
    """
    if icon is None:
        return None
    
    # If it's a FileIcon, get the file path directly
    if isinstance(icon, Gio.FileIcon):
        file_obj = icon.get_file()
        if file_obj:
            path = file_obj.get_path()
            if path and Path(path).exists():
                return path
    
    # For ThemedIcon or other icon types, resolve via IconTheme
    icon_string = icon.to_string()
    if not icon_string:
        return None
    
    # Remove prefixes like "themed-icon" or "file://" if present
    if icon_string.startswith('themed-icon:'):
        icon_name = icon_string.replace('themed-icon:', '').split(',')[0]
    elif icon_string.startswith('file://'):
        # Handle file:// URIs
        from urllib.parse import unquote
        path = unquote(icon_string.replace('file://', ''))
        if Path(path).exists():
            return path
        return None
    else:
        icon_name = icon_string
    
    # Use IconTheme to resolve themed icons to actual file paths
    theme = Gtk.IconTheme.get_default()
    
    # Try different sizes (larger icons are usually better quality)
    for size in [512, 256, 128, 96, 64, 48, 32]:
        icon_info = theme.lookup_icon(icon_name, size, 0)
        if icon_info:
            filename = icon_info.get_filename()
            if filename and Path(filename).exists():
                return filename
    
    # If no file found, return the icon name as fallback
    return icon_name


def find_all_applications():
    """
    Find all applications using GTK's GIO library.
    Returns a list of dicts with 'name', 'description', and 'icon' keys.
    """
    applications = []
    
    # Get all applications registered in the system
    all_apps = Gio.AppInfo.get_all()
    
    for app in all_apps:
        # Skip if should not be shown
        if app.should_show() is False:
            continue
        
        # Get application name
        name = app.get_display_name() or app.get_name()
        if not name:
            continue
        
        # Get description
        description = app.get_description() or ''
        
        # Get icon and resolve to file path
        icon = app.get_icon()
        icon_path = get_icon_file_path(icon) if icon else None
        
        applications.append({
            'name': name,
            'description': description,
            'icon': icon_path or ''
        })
    
    # Remove duplicates (same name) - keep first occurrence
    seen = set()
    unique_apps = []
    for app in applications:
        if app['name'] not in seen:
            seen.add(app['name'])
            unique_apps.append(app)
    
    # Sort by name
    unique_apps.sort(key=lambda x: x['name'].lower())
    
    return unique_apps


def main():
    """Main function to find and display all applications."""
    print("Scanning for applications...")
    applications = find_all_applications()
    
    print(f"\nFound {len(applications)} applications:\n")
    print("-" * 80)
    
    for app in applications:
        print(f"Name: {app['name']}")
        if app['description']:
            print(f"Description: {app['description']}")
        else:
            print("Description: (No description available)")
        if app['icon']:
            print(f"Icon: {app['icon']}")
        else:
            print("Icon: (No icon found)")
        print("-" * 80)
    
    # Also save to a file
    output_file = Path('applications_list.txt')
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write(f"Total applications found: {len(applications)}\n\n")
        for app in applications:
            f.write(f"Name: {app['name']}\n")
            if app['description']:
                f.write(f"Description: {app['description']}\n")
            else:
                f.write("Description: (No description available)\n")
            if app['icon']:
                f.write(f"Icon: {app['icon']}\n")
            else:
                f.write("Icon: (No icon found)\n")
            f.write("\n")
    
    print(f"\nResults also saved to: {output_file}")


if __name__ == '__main__':
    main()
