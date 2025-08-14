let numFiles = 0;

function loadFiles() {
	const filesList = document.getElementById("files_list");
	filesList.innerHTML = '';
	fetch('/Uploads/')
		.then(response => {
			if (!response.ok)
				throw new Error(`HTTP error! status: ${response.status}`);
			return response.json();
		})
		.then(files => {
			files.forEach(filename => {
				createFileElement(filename);
			});
			numFiles = files.length;
		})
		.catch(error => {
			console.error('Error loading files:', error);
		});
}

function createFileElement(filename) {
	const filesList = document.getElementById("files_list");
	const fileContainer = document.createElement("div");
	fileContainer.className = "file-item";
	const downloadLink = document.createElement("a");
	downloadLink.href = `/Uploads/${filename}`;
	downloadLink.download = filename;
	downloadLink.textContent = filename;
	downloadLink.className = "file-download-link";

	const deleteButton = document.createElement("button");
	deleteButton.textContent = "Delete";
	deleteButton.className = "file-delete-button";
	deleteButton.onclick = () => deleteFile(filename);

	fileContainer.appendChild(downloadLink);
	fileContainer.appendChild(deleteButton);
	filesList.appendChild(fileContainer);
}


function deleteFile(filename) {
	if (!confirm(`are you suure you want to delete ${filename}?`))
		return;
	fetch(`/Uploads/${filename}`, {
		method: 'DELETE'
	})
		.then(response => {
			if (response.ok) {
				alert(`${filename} is gone...`);
				loadFiles();
			} else
				alert(`failed to delete ${filename}. status: ${response.status}`);
		})
		.catch(error => {
			console.error('error:', error);
			alert('ERROR!');
		});
}

document.addEventListener('DOMContentLoaded', loadFiles);