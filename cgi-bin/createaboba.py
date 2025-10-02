#!/usr/bin/env python3
import os
from datetime import datetime

log_file_path = "./Pages/Uploads/aboba_log.txt"

try:
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_entry = f"createaboba.py executed at {timestamp}\n"
    
    with open(log_file_path, "a") as f:
        f.write(log_entry)
    
    marker_file = "./Pages/Uploads/aboba_was_here.txt"
    with open(marker_file, "w") as f:
        f.write(f"aboba was created at {timestamp}\n")
        f.write("this file was created by createaboba.py CGI script\n")
        f.write("this script produces no output to the browser!\n")

except Exception as e:
    error_log = "./Pages/Uploads/aboba_error.txt"
    with open(error_log, "w") as f:
        f.write(f"error in createaboba.py: {str(e)}\n")