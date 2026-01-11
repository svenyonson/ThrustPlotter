// upload_page.h
#ifndef UPLOAD_PAGE_H
#define UPLOAD_PAGE_H

#include <pgmspace.h>

const char UPLOAD_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>File Upload - Thrust Meter</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; max-width: 600px; margin: 50px auto; padding: 20px; }
        h1 { color: #333; }
        .upload-form { background: #f5f5f5; padding: 20px; border-radius: 8px; }
        input[type="file"] { margin: 10px 0; }
        button { background: #007bff; color: white; padding: 10px 20px; border: none; border-radius: 4px; cursor: pointer; }
        button:hover { background: #0056b3; }
        .message { margin-top: 15px; padding: 10px; border-radius: 4px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
        .file-list { margin-top: 20px; }
        .file-item { padding: 8px; background: white; margin: 5px 0; border-radius: 4px; }
    </style>
</head>
<body>
    <h1> 📁 Web File Upload</h1>
    <p>Upload your web application files (HTML, CSS, JS) to /web directory.</p>
    
    <div class="upload-form">
        <h3>Upload File</h3>
        <input type="file" id="fileInput">
        <button onclick="uploadFile()">Upload</button>
        <div id="message"></div>
    </div>
    
    <div class="file-list">
        <h3>Current Files</h3>
        <div id="fileList">Loading...</div>
    </div>
    
    <script>
        function uploadFile() {
            const fileInput = document.getElementById('fileInput');
            const file = fileInput.files[0];
            const msg = document.getElementById('message');
            
            if (!file) {
                msg.className = 'message error';
                msg.textContent = 'Please select a file';
                return;
            }
            
            const formData = new FormData();
            formData.append('file', file);
            
            msg.className = 'message';
            msg.textContent = 'Uploading...';
            
            fetch('/upload', {
                method: 'POST',
                body: formData
            })
            .then(response => response.text())
            .then(data => {
                msg.className = "message success";
                msg.textContent = "File uploaded: " + file.name;
                fileInput.value = null;
                loadFileList();
            })
            .catch(error => {
                msg.className = "message error";
                msg.textContent = "Upload failed: " + error;
            });
        }
        
        function loadFileList() {
            fetch('/api/files')
                .then(response => response.json())
                .then(files => {
                    const list = document.getElementById('fileList');
                    if (files.length === 0) {
                        list.innerHTML = "<p>No files uploaded yet</p>";
                    } else {
                        list.innerHTML = files.map(f => 
                            "<div class=\"file-item\">" + f + "</div>"
                        ).join("");
                    }
                });
        }
        
        loadFileList();
    </script>
</body>
</html>
)=====";

#endif
